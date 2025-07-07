#include "PolygonWidget.h"

#include "RulerMeasurement.h"

void SPolygonWidget::Construct(const FArguments& InArgs)
{
        PolygonPoints = InArgs._PolygonPoints;
        FillColor = InArgs._FillColor;
        PlayerController = GetPlayerController();
}

void SPolygonWidget::SetFillColor(const FLinearColor& InColor)
{
        FillColor = InColor;
        Invalidate(EInvalidateWidget::Paint);
}

FVector2D SPolygonWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
        if (PolygonPoints.IsEmpty())
        {
                return FVector2D(100.f, 100.f);
        }

        FVector2D Min(FLT_MAX, FLT_MAX), Max(-FLT_MAX, -FLT_MAX);

        for (const FVector2D& Point : PolygonPoints)
        {
                Min.X = FMath::Min(Min.X, Point.X);
                Min.Y = FMath::Min(Min.Y, Point.Y);
                Max.X = FMath::Max(Max.X, Point.X);
                Max.Y = FMath::Max(Max.Y, Point.Y);
        }

        return Max - Min;
}

TArray<FVector2D> SPolygonWidget::ConvertWorldToViewportCoordinatesBidirectionalAndOffset(const TArray<FVector>& WorldPositions,
                                                                                          bool bPlayerViewportRelative,
                                                                                          float Offset) const
{
        TArray<FVector2D> ScreenPositions;
        bool bTargetBehindCamera;

        if (!PlayerController)
        {
                return ScreenPositions;
        }

        ScreenPositions.Reserve(WorldPositions.Num());

        for (const FVector& WorldPos : WorldPositions)
        {
                FVector2D ScreenPos;
                FVector2D ViewportPos;
                ARulerMeasurement::ProjectWorldToScreenBidirectional(PlayerController, WorldPos, ScreenPos, bTargetBehindCamera, bPlayerViewportRelative);
                {
                        USlateBlueprintLibrary::ScreenToViewport(GEngine->GameViewport->GetWorld(), ScreenPos, ViewportPos);

                        if (bTargetBehindCamera)
                        {
                                ScreenPositions.Emplace(ARulerMeasurement::OffsetVectorByStep(ViewportPos, Offset));
                        }
                        else
                        {
                                ScreenPositions.Emplace(ViewportPos);
                        }
                }
        }

        return ScreenPositions;
}

APlayerController* SPolygonWidget::GetPlayerController()
{
        if (GEngine && GEngine->GameViewport)
        {
                if (const UWorld* World = GEngine->GameViewport->GetWorld(); IsValid(World))
                {
                        return World->GetFirstPlayerController();
                }
        }
        return nullptr;
}

void SPolygonWidget::SetPolygonPoints(const TArray<FVector>& InPoints)
{
        PolygonPoints3D = InPoints;
}

void SPolygonWidget::SetPolygonVisibility(bool bVisible)
{
        bIsVisible = bVisible;
}

int32 SPolygonWidget::OnPaint(const FPaintArgs& Args,
                              const FGeometry& AllottedGeometry,
                              const FSlateRect& MyCullingRect,
                              FSlateWindowElementList& OutDrawElements,
                              int32 LayerId,
                              const FWidgetStyle& InWidgetStyle,
                              bool bParentEnabled) const
{
        // Get coordinates of polygon points (in viewport coordinates)
        TArray<FVector2D> Points = ConvertWorldToViewportCoordinatesBidirectionalAndOffset(PolygonPoints3D, false, 1000);
        if (Points.Num() < 3 || !bIsVisible)
        {
                return LayerId;
        }

        // Calculate screen boundaries via GameViewport
        FSlateRect ExtendedRect = MyCullingRect;
        if (GEngine && GEngine->GameViewport)
        {
                FVector2D ViewportSize;
                GEngine->GameViewport->GetViewportSize(ViewportSize);
                ExtendedRect = FSlateRect(0, 0, ViewportSize.X, ViewportSize.Y);
                // Extend the borders by 100 pixels on all sides
                ExtendedRect = FSlateRect(ExtendedRect.Left - 100.f, ExtendedRect.Top - 100.f, ExtendedRect.Right + 400.f, ExtendedRect.Bottom + 300.f);
        }

        // Clip the polygon to the extended rectangle
        Points = ClipPolygonToRect(Points, ExtendedRect);
        if (Points.Num() < 3)
        {
                return LayerId;
        }

        // If self-intersecting edges are detected, do not draw the polygon
        if (IsSelfIntersecting(Points))
        {
                return LayerId;
        }

        TArray<SlateIndex> TriIndices;
        PerformTriangulation(Points, TriIndices);
        if (TriIndices.Num() < 3)
        {
                return LayerId;
        }

        const FPaintGeometry& PaintGeom = AllottedGeometry.ToPaintGeometry();
        const FSlateRenderTransform& RenderTransform = PaintGeom.GetAccumulatedRenderTransform();
        const FLinearColor VertexColor = InWidgetStyle.GetColorAndOpacityTint() * FillColor;

        TArray<FSlateVertex> Vertices;
        Vertices.Reserve(Points.Num());

        for (const FVector2D& Pt : Points)
        {
                FVector2D TransformedPt = RenderTransform.TransformPoint(Pt);
                FSlateVertex Vertex;
                Vertex.Position = FVector2f(TransformedPt);
                Vertex.Color = VertexColor.ToFColor(true);
                Vertex.TexCoords[0] = Vertex.TexCoords[1] = 0.f;
                Vertex.TexCoords[2] = Vertex.TexCoords[3] = 0.f;
                Vertices.Add(Vertex);
        }

        FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, FSlateResourceHandle(), Vertices, TriIndices, nullptr, 0, 0);
        return LayerId;
}

void SPolygonWidget::PerformTriangulation(const TArray<FVector2D>& Points, TArray<SlateIndex>& OutIndices)
{
        OutIndices.Reset();
        if (Points.Num() < 3)
        {
                return;
        }

        std::vector<std::vector<FVector2f>> Polygon(1);
        Polygon[0].reserve(Points.Num());

        for (const FVector2D& Pt : Points)
        {
                Polygon[0].emplace_back(FVector2f(Pt));
        }

        const std::vector<uint32> EarCutIndices = mapbox::earcut<uint32>(Polygon);

        OutIndices.Append(EarCutIndices.data(), EarCutIndices.size());
}

TArray<FVector2D> SPolygonWidget::ClipPolygonAgainstEdge(const TArray<FVector2D>& Polygon,
                                                         TFunctionRef<bool(const FVector2D&)> IsInside,
                                                         TFunctionRef<FVector2D(const FVector2D&, const FVector2D&)> ComputeIntersection)
{
        TArray<FVector2D> ClippedPolygon;
        if (Polygon.Num() == 0)
        {
                return ClippedPolygon;
        }

        FVector2D PrevPoint = Polygon.Last();
        bool bPrevInside = IsInside(PrevPoint);

        for (const FVector2D& CurrPoint : Polygon)
        {
                bool bCurrInside = IsInside(CurrPoint);

                if (bCurrInside)
                {
                        if (!bPrevInside)
                        {
                                ClippedPolygon.Add(ComputeIntersection(PrevPoint, CurrPoint));
                        }
                        ClippedPolygon.Add(CurrPoint);
                }
                else if (bPrevInside)
                {
                        ClippedPolygon.Add(ComputeIntersection(PrevPoint, CurrPoint));
                }

                PrevPoint = CurrPoint;
                bPrevInside = bCurrInside;
        }
        return ClippedPolygon;
}

TArray<FVector2D> SPolygonWidget::ClipPolygonToRect(const TArray<FVector2D>& InPolygon, const FSlateRect& ClipRect) const
{
        TArray<FVector2D> Clipped = InPolygon;

        // Clipping by the left border: keep points where X >= ClipRect.Left
        Clipped = ClipPolygonAgainstEdge(
                Clipped,
                [ClipRect](const FVector2D& Pt) { return Pt.X >= ClipRect.Left; },
                [ClipRect](const FVector2D& A, const FVector2D& B)
                {
                        float t = (ClipRect.Left - A.X) / (B.X - A.X);
                        return FVector2D(ClipRect.Left, A.Y + t * (B.Y - A.Y));
                });

        // Clipping by the right edge: keep points where X <= ClipRect.Right
        Clipped = ClipPolygonAgainstEdge(
                Clipped,
                [ClipRect](const FVector2D& Pt) { return Pt.X <= ClipRect.Right; },
                [ClipRect](const FVector2D& A, const FVector2D& B)
                {
                        float t = (ClipRect.Right - A.X) / (B.X - A.X);
                        return FVector2D(ClipRect.Right, A.Y + t * (B.Y - A.Y));
                });

        // Clipping by the top edge: keep points where Y >= ClipRect.Top
        Clipped = ClipPolygonAgainstEdge(
                Clipped,
                [ClipRect](const FVector2D& Pt) { return Pt.Y >= ClipRect.Top; },
                [ClipRect](const FVector2D& A, const FVector2D& B)
                {
                        float t = (ClipRect.Top - A.Y) / (B.Y - A.Y);
                        return FVector2D(A.X + t * (B.X - A.X), ClipRect.Top);
                });

        // Clipping by the bottom edge: keep points where Y <= ClipRect.Bottom
        Clipped = ClipPolygonAgainstEdge(
                Clipped,
                [ClipRect](const FVector2D& Pt) { return Pt.Y <= ClipRect.Bottom; },
                [ClipRect](const FVector2D& A, const FVector2D& B)
                {
                        float t = (ClipRect.Bottom - A.Y) / (B.Y - A.Y);
                        return FVector2D(A.X + t * (B.X - A.X), ClipRect.Bottom);
                });

        return Clipped;
}

// Check if the polygon has self-intersection (ignore adjacent edges)
bool SPolygonWidget::IsSelfIntersecting(const TArray<FVector2D>& Points) const
{
        const int32 Num = Points.Num();
        if (Num < 4)
        {
                return false;
        }

        for (int32 i = 0; i < Num; ++i)
        {
                const FVector2D A = Points[i];
                const FVector2D B = Points[(i + 1) % Num];

                // Start at i+2 to skip adjacent edge (and special case between first and last segment)
                for (int32 j = i + 2; j < Num; ++j)
                {
                        // Skip first and last edge intersection
                        if (i == 0 && j == Num - 1)
                        {
                                continue;
                        }
                        const FVector2D C = Points[j];
                        const FVector2D D = Points[(j + 1) % Num];

                        if (DoSegmentsIntersect(A, B, C, D))
                        {
                                return true;
                        }
                }
        }
        return false;
}

bool SPolygonWidget::DoSegmentsIntersect(const FVector2D& p, const FVector2D& q, const FVector2D& r, const FVector2D& s) const
{
        auto Orientation = [](const FVector2D& a, const FVector2D& b, const FVector2D& c) -> float
        { return (b.Y - a.Y) * (c.X - b.X) - (b.X - a.X) * (c.Y - b.Y); };

        float o1 = Orientation(p, q, r);
        float o2 = Orientation(p, q, s);
        float o3 = Orientation(r, s, p);
        float o4 = Orientation(r, s, q);

        if ((o1 * o2 < 0) && (o3 * o4 < 0))
        {
                return true;
        }
        return false;
}
