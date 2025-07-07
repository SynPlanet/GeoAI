#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "earcut.hpp"
#include <vector>
#include "EarcutUnreal.h"
#include "Blueprint/SlateBlueprintLibrary.h"

/**
* SPolygonWidget - Slate Widget for drawing a custom filled polygon
*/
class SPolygonWidget : public SLeafWidget
{
public:
        SLATE_BEGIN_ARGS(SPolygonWidget) : _PolygonPoints(), _FillColor(FLinearColor::White) {}

        SLATE_ARGUMENT(TArray<FVector2D>, PolygonPoints)
        SLATE_ARGUMENT(FLinearColor, FillColor)

        SLATE_END_ARGS()

        /** Slate constructor, initializes polygon points and fill color */
        void Construct(const FArguments& InArgs);

        /**
        * Sets the fill color for the polygon widget and invalidates the widget to force a redraw.
        *
        * @param InColor The color that will be set as the fill color for the polygon.
        */
        void SetFillColor(const FLinearColor& InColor);

        /**
        * Sets the 3D points for the polygon widget and forces its redrawing.
        *
        * @param InPoints An array of 3D points (FVector) defining the shape of the polygon in 3D space.
        */
        void SetPolygonPoints(const TArray<FVector>& InPoints);

        /**
        * @brief Sets the visibility of the polygon.
        * @param bVisible The visibility flag of the polygon.
        */
        void SetPolygonVisibility(bool bVisible);

protected:
        /** The main method for drawing a polygon on the screen */
        virtual int32 OnPaint(const FPaintArgs& Args,
                              const FGeometry& AllottedGeometry,
                              const FSlateRect& MyCullingRect,
                              FSlateWindowElementList& OutDrawElements,
                              int32 LayerId,
                              const FWidgetStyle& InWidgetStyle,
                              bool bParentEnabled) const override;

private:
        TArray<FVector2D> PolygonPoints;
        FLinearColor FillColor = FLinearColor::Gray;
        APlayerController* PlayerController = nullptr;
        TArray<FVector> PolygonPoints3D;
        bool bIsVisible = true;

        /**
        * Triangulates a polygon based on the given 2D points.
        *
        * @param Points A set of point coordinates that define the shape of the polygon.
        * @param OutIndices An array of indices of the vertices that form the triangles produced by the triangulation.
        */
        static void PerformTriangulation(const TArray<FVector2D>& Points, TArray<SlateIndex>& OutIndices);

        /**
        * Transforms a set of 3D world positions into 2D viewport coordinates, applying an optional offset.
        *
        * @param WorldPositions An array of 3D world positions to transform into viewport coordinates.
        * @param bPlayerViewportRelative Determines whether the transformation is relative to the player's viewport.
        * @param Offset An optional offset applied to positions that are behind the camera.
        * @return An array of 2D viewport coordinates corresponding to the input world positions, taking into account the optional offset adjustments.
        */
        UFUNCTION(BlueprintPure)
        TArray<FVector2D> ConvertWorldToViewportCoordinatesBidirectionalAndOffset(const TArray<FVector>& WorldPositions,
                                                                                  bool bPlayerViewportRelative,
                                                                                  float Offset) const;

        /**
        * Computes the desired size of the polygon widget given the given polygon points.
        * If no points are given, the default size is returned.
        *
        * @param LayoutScaleMultiplier The scale multiplier for the layout, used for adaptive resizing.
        * @return The desired size of the widget as a 2D vector. If no polygon points are given, the default size (100, 100) is returned.
        */
        virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

        /**
        * Returns the player controller associated with the widget's world context.
        *
        * @return A pointer to the APlayerController if available, or nullptr if not found.
        */
        static APlayerController* GetPlayerController();

        /**
        * ClipPolygonAgainstEdge - Clips the given polygon against the boundary defined by the inside/outside condition.
        *
        * This method takes a polygon represented by an array of 2D points and clips it against the boundary defined by the `IsInside` condition. Points that are inside the boundary are
        * kept, while points that are outside the boundary are discarded. Intersections between the polygon's edges
        * and the boundary are calculated using the `ComputeIntersection` function.
        *
        * @param Polygon An array of 2D points representing the polygon to clip.
        * @param IsInside A reference to a function that determines whether a point is inside the clipper.
        * @param ComputeIntersection A reference to a function that computes the intersection of an edge with the boundary.
        * @return An array of 2D points representing the clipped polygon.
        */
        static TArray<FVector2D> ClipPolygonAgainstEdge(const TArray<FVector2D>& Polygon,
                                                        TFunctionRef<bool(const FVector2D&)> IsInside,
                                                        TFunctionRef<FVector2D(const FVector2D&, const FVector2D&)> ComputeIntersection);

        /**
        * Clips the given polygon to fit within the given rectangular bounds.
        *
        * @param InPolygon An array of points representing the vertices of the polygon.
        * @param ClipRect The rectangular bounds to which the polygon is clipped.
        * @return A new array of points representing the clipped polygon.
        */
        TArray<FVector2D> ClipPolygonToRect(const TArray<FVector2D>& InPolygon, const FSlateRect& ClipRect) const;

        /**
        * Tests whether the specified polygon self-intersects.
        *
        * @param Points An array of the polygon's vertices as 2D vectors.
        * The order of the vertices must be sequential, closing into a polygon.
        * @return true if the polygon has intersecting edges; false if the polygon does not intersect with itself.
        */
        bool IsSelfIntersecting(const TArray<FVector2D>& Points) const;

        /**
        * Determines whether two line segments intersect.
        *
        * @param p The starting point of the first line segment.
        * @param q The ending point of the first line segment.
        * @param r The starting point of the second line segment.
        * @param s The end point of the second segment.
        * @return True if the segments intersect; otherwise, false.
        */
        bool DoSegmentsIntersect(const FVector2D& p, const FVector2D& q, const FVector2D& r, const FVector2D& s) const;
};