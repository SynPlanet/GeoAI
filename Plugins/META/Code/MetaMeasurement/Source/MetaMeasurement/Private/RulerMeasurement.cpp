// META all rights received (c)

#include "RulerMeasurement.h"

#include "GeographicalContext.h"
#include "GeographicalTransforms.h"
#include "RulerPopupWidget.h"
#include "RulerPoint.h"
#include "RulerPointButton.h"
#include "RulerSplineWidget.h"
#include "Algo/ForEach.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

namespace
{
        template <typename UWidget>
                requires(std::is_base_of_v<UUserWidget, UWidget>)
        void RemoveWidget(TObjectPtr<UWidget>& Widget)
        {
                if (!IsValid(Widget))
                {
                        return;
                }
                Widget->RemoveFromParent();
                Widget = nullptr;
        }
} // namespace

bool ARulerMeasurement::IsControllerAvailable() const
{
        return IsValid(GetRulerControllerByGuid(RulerInfo.Guid));
}

void ARulerMeasurement::CreateRulerController(const FRulerInfo& Info, bool ShowRulerArea)
{
        if (Info.Positions.IsEmpty() || IsControllerAvailable() || !IsValid(RulerControllerClass))
        {
                return;
        }
        ARulerController* const Controller = GetWorld()->SpawnActor<ARulerController>(RulerControllerClass);
        if (!IsValid(Controller))
        {
                return;
        }

        Controller->SetShowRulerArea(ShowRulerArea);
        Controller->Tags.Add(*Info.Guid);
        Controller->SetRulerInfo(Info);
        Controller->Initialization();
        RulerControllers.Add(Controller);
}

TArray<FVector> ARulerMeasurement::CreateRulerPolygon(const TArray<FVector>& Input)
{
        if (Input.Num() < 4)
        {
                return Input;
        }

        TArray<FVector> Result = Input;
        if (const FVector& First = Result[0]; !(Result.Last() - First).IsNearlyZero())
        {
                Result.Pop();
                Result.Emplace(First);
        }
        return Result;
}

void ARulerMeasurement::UpdateRuler()
{
        const TStrongObjectPtr<APlayerController> Controller = CurrentController.Pin();
        if (!IsValid(PopupWidget) || !Controller.IsValid())
        {
                return;
        }
        float MouseX;
        float MouseY;
        UWidgetLayoutLibrary::GetMousePositionScaledByDPI(Controller.Get(), MouseX, MouseY);
        PopupWidget->SetPositionInViewport({ MouseX, MouseY }, false);
        PopupWidget->SetVisibility(RulerMode == ERulerMode::RulerEdit ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        if (FMath::IsNearlyZero(SplineDistance))
        {
                PopupWidget->SetDefaultText();
                return;
        }
        RulerInfo.Length = RoundFloat(SplineDistance * UE_CM_TO_M);
        std::ignore = RulerLengthUpdated.ExecuteIfBound(RulerInfo);
        PopupWidget->SetTextPopup(RulerInfo.Length, 0.f);
}

void ARulerMeasurement::RemoveInvalidControllers()
{
        for (auto It = RulerControllers.CreateIterator(); It; ++It)
        {
                if (IsValid(*It))
                {
                        continue;
                }
                It.RemoveCurrent();
        }
}

double ARulerMeasurement::GetDistanceToleranceForAreaPolygon() const
{
        return DistanceToleranceForAreaPolygon;
}

void ARulerMeasurement::BeginDestroy()
{
        RemoveWidget(PopupWidget);
        RemoveWidget(SplineWidget);
        RemoveWidget(RulerLinesWidget);
        Algo::ForEach(RulerPointWidgets, &UWidget::RemoveFromParent);
        Super::BeginDestroy();
}

ARulerMeasurement::ARulerMeasurement()
{
        PrimaryActorTick.bCanEverTick = true;
        PrimaryActorTick.bStartWithTickEnabled = true;
}

void ARulerMeasurement::Tick(float DeltaSeconds)
{
        Super::Tick(DeltaSeconds);

        RemoveInvalidControllers();
        UpdateRulerPlacementLocationMarker(true);
        UpdateRulerPlacementLocationSpline();
        UpdateRuler();
        if (FVector CameraLocation; GetCameraLocation(CameraLocation))
        {
                DistanceToleranceForAreaPolygon = (GetRulerPlacementLocationMarker()->GetActorLocation() - CameraLocation).Length() * UE_CM_TO_M;
        }
}

void ARulerMeasurement::OnConstruction(const FTransform& Transform)
{
        Super::OnConstruction(Transform);

        if (IsValid(RulerPlacementLocationMarker))
        {
                return;
        }

        if (AActor* const FoundActor = UGameplayStatics::GetActorOfClass(this, RulerPlacementLocationMarkerClass); IsValid(FoundActor))
        {
                RulerPlacementLocationMarker = FoundActor;
        }
        else
        {
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = this;
                SpawnParams.Name = TEXT("RulerLocationMarker");
                RulerPlacementLocationMarker = GetWorld()->SpawnActor<AActor>(RulerPlacementLocationMarkerClass.Get(), SpawnParams);
        }
}

float ARulerMeasurement::RoundFloat(float Value)
{
        static constexpr float Modifier = 10.f;
        return static_cast<float>(UKismetMathLibrary::Round(Value * Modifier)) / Modifier;
}

void ARulerMeasurement::ClearTempRuler()
{
        PreviousPoint = FVector::ZeroVector;
        TotalDistance = 0.;
        SplineDistance = 0.;
        RulerPoints.Empty();
        RulerInfo.Reset();
        RemoveWidget(RulerLinesWidget);
        RemoveWidget(SplineWidget);
        RemoveWidget(PopupWidget);
        for (URulerPoint* const RulerPoint : RulerPointWidgets)
        {
                RulerPoint->RemoveFromParent();
        }
        RulerPointWidgets.Empty();
}

void ARulerMeasurement::SetRulerInfo(FRulerInfo NewRulerInfo)
{
        RulerInfo = NewRulerInfo;
}

ERulerMode ARulerMeasurement::GetRulerMode() const
{
        return RulerMode;
}

void ARulerMeasurement::SetRulerMode(ERulerMode NewMode)
{
        if (RulerMode != NewMode)
        {
                RulerMode = NewMode;
                OnRulerModeChanged.Broadcast(NewMode);
        }
}

AActor* ARulerMeasurement::GetRulerPlacementLocationMarker() const noexcept
{
        return RulerPlacementLocationMarker;
}

APlayerController* ARulerMeasurement::GetPlayerController() const
{
        return Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}

void ARulerMeasurement::CreateRuler(const FString& Guid, bool bCreateRandomGuid, bool bShowRulerArea)
{
        bShowArea = bShowRulerArea;
        RulerInfo.Guid = Guid.IsEmpty() ? FGuid::NewGuid().ToString() : Guid;
        SetRulerMode(ERulerMode::RulerEdit);

        if (!IsValid(PopupWidget))
        {
                PopupWidget = CreateWidget<URulerPopupWidget>(GetWorld(), PopupWidgetClass);
                PopupWidget->AddToViewport();
        }
}

void ARulerMeasurement::CreateRulerPointAtLocation(const FVector& Location, const TOptional<float>& Tolerance)
{
        if (!IsValid(RulerLinesWidget))
        {
                RulerLinesWidget = CreateWidget<URulerLines>(GetWorld(), RulerLinesWidgetClass);
                RulerLinesWidget->AddToViewport(-100);
        }

        if (!IsValid(SplineWidget))
        {
                SplineWidget = CreateWidget<URulerSplineWidget>(GetWorld(), SplineWidgetClass);
                SplineWidget->AddToViewport();
        }

        if (PreviousPoint.IsNearlyZero())
        {
                PreviousPoint = Location;
                RulerPoints.AddUnique(PreviousPoint);
                RulerLinesWidget->SetPoints(RulerPoints);
                RulerPointWidgets.Add(CreateRulerPointWidget(Location));
                return;
        }

        if ((PreviousPoint - Location).IsNearlyZero() || AreVectorsCloseOnViewport(Location, PreviousPoint, 10))
        {
                return;
        }

        RulerPointWidgets.Add(CreateRulerPointWidget(Location));
        RulerPoints.Add(Location);
        const TArray<FVector> EllipsoidPoints = GeneratePointsAndRelativeToEllipsoid(RulerPoints, GetStepForSegments(), false);
        RulerLinesWidget->SetPoints(EllipsoidPoints);

        if (Location.IsNearlyZero())
        {
                return;
        }

        PreviousPoint = Location;
        TotalDistance = CalculateTotalDistance(EllipsoidPoints);
        RulerInfo.Length = RoundFloat(TotalDistance * UE_CM_TO_M);

        if (!CanCreatePolygon(RulerPoints, Tolerance.Get(DistanceToleranceForAreaPolygon)))
        {
                std::ignore = RulerLengthUpdated.ExecuteIfBound(RulerInfo);
                return;
        }

        RulerPoints = CreateRulerPolygon(RulerPoints);
        RulerInfo.Positions = RulerPoints;
        SaveCurrentRuler();
}

void ARulerMeasurement::CreateRulerPoint()
{
        CreateRulerPointAtLocation(GetRulerPlacementLocationMarker()->GetActorLocation());
}

void ARulerMeasurement::RulerEnd()
{
        PreviousPoint = FVector::ZeroVector;
        TotalDistance = 0.f;
}

void ARulerMeasurement::SaveCurrentRuler()
{
        SetRulerMode(ERulerMode::Ruler);
        CreateRulerController({ RulerInfo.Guid, RulerInfo.Length, RulerInfo.Name, RulerPoints }, bShowArea);
        ClearTempRuler();
        std::ignore = RulerSaved.ExecuteIfBound(RulerInfo);
}

void ARulerMeasurement::UpdateRulerPlacementLocationSpline()
{
        const FVector LocationMarker = GetRulerPlacementLocationMarker()->GetActorLocation();
        FVector2D ScreenPosition;
        bool bTargetBehindCamera;
        if (PreviousPoint.IsNearlyZero() || !CurrentController.IsValid()
            || !ProjectWorldToScreenBidirectional(CurrentController.Get(), LocationMarker, ScreenPosition, bTargetBehindCamera, false) || bTargetBehindCamera)
        {
                return;
        }
        const TArray<FVector> PointsAtEllipsoid = GeneratePointsAndRelativeToEllipsoid({ PreviousPoint, LocationMarker }, StepForSegments, false);
        SplineDistance = TotalDistance + CalculateTotalDistance(PointsAtEllipsoid);
        TArray<FVector2D> SplinePoints;
        for (const FVector& Point : PointsAtEllipsoid)
        {
                const bool bSuccess = ProjectWorldToScreenBidirectional(CurrentController.Get(), Point, ScreenPosition, bTargetBehindCamera, false);
                FVector2D ViewportPosition;
                USlateBlueprintLibrary::ScreenToViewport(this, ScreenPosition, ViewportPosition);
                if (bSuccess && !bTargetBehindCamera && !ViewportPosition.IsNearlyZero())
                {
                        SplinePoints.Emplace(ViewportPosition);
                }
        }
        SplineWidget->SetSplinePoints(GeneratePointsBetweenVectors2D(SplinePoints, StepForSpline));
}

void ARulerMeasurement::UpdateRulerPlacementLocationMarker(bool bVisible, double DistanceTolerance) noexcept
{
        AActor* const Marker = RulerPlacementLocationMarker;

        if (!IsValid(Marker))
        {
                return;
        }

        if (bVisible == Marker->IsHidden())
        {
                Marker->SetHidden(!bVisible);
        }

        if (!bVisible)
        {
                return;
        }

        FHitResult Hit;
        FVector2D ScreenLocation;
        const APlayerController* const PlayerController = GetPlayerController();
        PlayerController->GetMousePosition(ScreenLocation.X, ScreenLocation.Y);
        LineTraceFromPlayerScreen(PlayerController, Hit, ScreenLocation, ECC_Visibility, FLT_MAX);

        if (!Hit.bBlockingHit || Hit.ImpactPoint.Equals(Marker->GetActorLocation(), DistanceTolerance))
        {
                return;
        }

        Marker->SetActorLocation(Hit.ImpactPoint);
        if (const APawn* const ControlledPawn = PlayerController->GetPawn(); IsValid(ControlledPawn))
        {
                Marker->SetActorRotation(FRotationMatrix::MakeFromX(ControlledPawn->GetActorLocation() - Hit.ImpactPoint).Rotator());
        }
}

template <typename TPoint>
void AddPointsBetweenVectors(TArray<TPoint>& OutPoints, const TPoint& Start, const TPoint& End, float Step)
{
        const TPoint Segment = End - Start;
        const float Length = Segment.Size();

        if (Length <= 0.f)
        {
                return;
        }

        const TPoint Direction = Segment / Length;

        for (float Distance = Step; Distance < Length; Distance += Step)
        {
                OutPoints.Add(Start + Direction * Distance);
        }
}

TArray<FVector2D> ARulerMeasurement::GeneratePointsBetweenVectors2D(const TArray<FVector2D>& InputPoints, float Step)
{
        TArray<FVector2D> GeneratedPoints;

        if (InputPoints.Num() < 2 || Step <= 0.0f)
        {
                return InputPoints;
        }

        for (int32 i = 0; i < InputPoints.Num() - 1; ++i)
        {
                const FVector2D& Start = InputPoints[i];
                const FVector2D& End = InputPoints[i + 1];
                AddPointsBetweenVectors(GeneratedPoints, Start, End, Step);
                GeneratedPoints.Add(End);
        }
        return GeneratedPoints;
}

const FRulerInfo& ARulerMeasurement::GetCurrentInfo() const
{
        return RulerInfo;
}

void ARulerMeasurement::HighlightRuler(ARulerController* RulerController) const
{
        RulerController->ChangeRulerColor(RulerHighlightColor);
        for (ARulerController* const Controller : GetAllRulerControllers())
        {
                if (Controller == RulerController)
                {
                        continue;
                }
                Controller->ChangeRulerColor(RulerStandardColor);
        }
}

TArray<FVector> ARulerMeasurement::GeneratePointsBetweenVectors3D(const TArray<FVector>& InputPoints, float Step)
{
        TArray<FVector> GeneratedPoints;

        if (InputPoints.Num() < 2 || Step <= 0.0f)
        {
                return InputPoints;
        }

        for (int32 i = 0; i < InputPoints.Num() - 1; ++i)
        {
                const FVector& Start = InputPoints[i];
                const FVector& End = InputPoints[i + 1];
                GeneratedPoints.Add(Start);
                AddPointsBetweenVectors(GeneratedPoints, Start, End, Step);
        }

        GeneratedPoints.Add(InputPoints.Last());
        return GeneratedPoints;
}

bool ARulerMeasurement::CanCreatePolygon(const TArray<FVector>& Vertices, float DistanceTolerance)
{
        return Vertices.Num() >= 4 && FVector::DistSquared(Vertices[0], Vertices.Last()) <= DistanceTolerance * DistanceTolerance;
}

float ARulerMeasurement::CalculatePolygonArea(const TArray<FVector>& Vertices, float DistanceTolerance)
{
        if (!CanCreatePolygon(Vertices, DistanceTolerance))
        {
                return 0.0f;
        }

        FVector Normal = FVector::ZeroVector;
        const int32 NumVertices = Vertices.Num();
        for (int32 i = 0; i < NumVertices; ++i)
        {
                const FVector& Current = Vertices[i];
                const FVector& Next = Vertices[(i + 1) % NumVertices];
                Normal.X += (Current.Y - Next.Y) * (Current.Z + Next.Z);
                Normal.Y += (Current.Z - Next.Z) * (Current.X + Next.X);
                Normal.Z += (Current.X - Next.X) * (Current.Y + Next.Y);
        }
        Normal.Normalize();

        const FQuat AlignQuat = FQuat::FindBetweenNormals(Normal, FVector::UpVector);

        TArray<FVector> TransformedVertices;
        TransformedVertices.SetNum(NumVertices);
        for (int32 i = 0; i < NumVertices; ++i)
        {
                TransformedVertices[i] = AlignQuat.RotateVector(Vertices[i]);
        }

        float Area = 0.0f;
        for (int32 i = 0; i < NumVertices; ++i)
        {
                const FVector& Current = TransformedVertices[i];
                const FVector& Next = TransformedVertices[(i + 1) % NumVertices];
                Area += (Current.X * Next.Y - Next.X * Current.Y);
        }

        return FMath::Abs(Area) * 0.5f * UE_CM2_TO_M2;
}

TArray<FRulerInfo> ARulerMeasurement::GetRulersInfo() const
{
        TArray<FRulerInfo> RulerInfoArray;
        for (const ARulerController* const RulerController : GetAllRulerControllers())
        {
                if (!IsValid(RulerController))
                {
                        continue;
                }
                RulerInfoArray.Add(RulerController->GetRulerInfo());
        }

        return RulerInfoArray;
}

const TArray<ARulerController*>& ARulerMeasurement::GetAllRulerControllers() const
{
        return RulerControllers;
}

ARulerController* ARulerMeasurement::GetRulerControllerByGuid(const FString& Guid) const
{
        for (ARulerController* const RulerController : GetAllRulerControllers())
        {
                if (IsValid(RulerController) && Guid == RulerController->GetRulerInfo().Guid)
                {
                        return RulerController;
                }
        }
        return nullptr;
}

void ARulerMeasurement::DestroyRuler(const FString& Guid)
{
        if (ARulerController* const Ruler = GetRulerControllerByGuid(Guid); IsValid(Ruler))
        {
                Ruler->Destroy();
                RulerControllers.Remove(Ruler);
        }
}

void ARulerMeasurement::DestroyAllRulers()
{
        for (ARulerController* const RulerController : GetAllRulerControllers())
        {
                if (IsValid(RulerController))
                {
                        RulerController->Destroy();
                }
        }

        RulerControllers.Empty();
}

FVector ARulerMeasurement::CalculateCenterPoint(const TArray<FVector>& PolygonVertices)
{
        if (PolygonVertices.IsEmpty())
        {
                return FVector::ZeroVector;
        }

        FVector Center = FVector::ZeroVector;

        for (const int32 NumVertices = PolygonVertices.Num(); const FVector& Vertex : PolygonVertices)
        {
                Center += Vertex / NumVertices;
        }

        return Center;
}

void ARulerMeasurement::VisualizeRulerByInfo(const FRulerInfo& InRulerInfo)
{
        if (InRulerInfo.Positions.IsEmpty() || InRulerInfo.Length <= 0)
        {
                return;
        }
        TArray<FVector> UnrealPositions;
        Algo::Transform(InRulerInfo.Positions, UnrealPositions, [this](const FVector& Pos) { return UGeographicalTransforms::EcefToUnreal(Pos, this); });
        CreateRulerController({ InRulerInfo.Guid, InRulerInfo.Length, InRulerInfo.Name, UnrealPositions }, bShowArea);
}

void ARulerMeasurement::ExitRuler()
{
        SaveCurrentRuler();

        SetRulerMode(ERulerMode::None);
}

void ARulerMeasurement::SetRulerLengthUnitType(const FString& RulerGuid, ERulerUnitType LengthUnitType)
{
        if (ARulerController* const RulerController = GetRulerControllerByGuid(RulerGuid); IsValid(RulerController))
        {
                RulerController->SetLengthUnitType(LengthUnitType);
        }
}

void ARulerMeasurement::SetRulerAreaUnitType(const FString& RulerGuid, ERulerUnitType AreaUnitType) const
{
        if (ARulerController* const RulerController = GetRulerControllerByGuid(RulerGuid); IsValid(RulerController))
        {
                RulerController->SetAreaUnitType(AreaUnitType);
        }
}

ERulerUnitType ARulerMeasurement::FindRulerLengthUnitType(const FString& LengthUnitType)
{
        static const TMap<FString, ERulerUnitType> RulerUnitTypeMap = { { "meter", ERulerUnitType::Meter }, { "kilometer", ERulerUnitType::Kilometer } };

        if (const ERulerUnitType* const EnumValue = RulerUnitTypeMap.Find(LengthUnitType); EnumValue)
        {
                return *EnumValue;
        }
        return ERulerUnitType::Meter;
}

ERulerUnitType ARulerMeasurement::FindRulerAreaUnitType(const FString& AreaUnitType)
{
        static const TMap<FString, ERulerUnitType> RulerUnitTypeMap = { { "squareKilometer", ERulerUnitType::SquareKilometer },
                                                                        { "squareMeter", ERulerUnitType::SquareMeter },
                                                                        { "hectare", ERulerUnitType::Hectare },
                                                                        { "hundredPart", ERulerUnitType::HundredPart } };

        if (const ERulerUnitType* const EnumValue = RulerUnitTypeMap.Find(AreaUnitType); EnumValue)
        {
                return *EnumValue;
        }
        return ERulerUnitType::SquareMeter;
}

bool ARulerMeasurement::GetCameraLocation(FVector& OutCameraLocation) const
{
        const APlayerController* const PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (!IsValid(PlayerController) || !IsValid(PlayerController->PlayerCameraManager))
        {
                return false;
        }
        OutCameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
        return true;
}

FString ARulerMeasurement::FormatUnitsWithSpaces(float Number)
{
        FNumberFormattingOptions Options;
        Options.UseGrouping = true;
        Options.MinimumFractionalDigits = 0;
        Options.MaximumFractionalDigits = 1;

        FString FormattedString = FText::AsNumber(Number, &Options).ToString();
        FormattedString.ReplaceInline(TEXT(","), TEXT(" "));
        FormattedString.ReplaceInline(TEXT("."), TEXT(","));

        return FormattedString;
}

bool ARulerMeasurement::ProjectWorldToScreenBidirectional(const APlayerController* PlayerController,
                                                          const FVector& WorldPosition,
                                                          FVector2D& ScreenPosition,
                                                          bool& bTargetBehindCamera,
                                                          bool bPlayerViewportRelative)
{

        ULocalPlayer* const LocalPlayer = IsValid(PlayerController) ? PlayerController->GetLocalPlayer() : nullptr;
        if (!IsValid(LocalPlayer) || !IsValid(LocalPlayer->ViewportClient))
        {
                return false;
        }

        FSceneViewProjectionData ProjectionData;
        if (!LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, /*out*/ ProjectionData))
        {
                ScreenPosition = FVector2D::ZeroVector;
        }

        const FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
        const FIntRect ViewRectangle = ProjectionData.GetConstrainedViewRect();

        FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
        if (Result.W < 0.f)
        {
                bTargetBehindCamera = true;
        }
        else
        {
                bTargetBehindCamera = false;
        }

        if (Result.W == 0.f)
        {
                Result.W = 1.f;
        }

        const float Rhw = 1.f / FMath::Abs(Result.W);
        FVector Projected = FVector(Result.X, Result.Y, Result.Z) * Rhw;

        // Normalize to 0..1 UI Space
        const float NormX = (Projected.X / 2.f) + 0.5f;
        const float NormY = 1.f - (Projected.Y / 2.f) - 0.5f;

        Projected.X = static_cast<float>(ViewRectangle.Min.X) + (NormX * static_cast<float>(ViewRectangle.Width()));
        Projected.Y = static_cast<float>(ViewRectangle.Min.Y) + (NormY * static_cast<float>(ViewRectangle.Height()));

        ScreenPosition = FVector2D(Projected.X, Projected.Y);

        if (bPlayerViewportRelative)
        {
                ScreenPosition -= FVector2D(ProjectionData.GetConstrainedViewRect().Min);
        }

        return true;
}

TArray<FVector> ARulerMeasurement::AdjustPointsRelativeToEllipsoid(const TArray<FVector>& PointsBetweenVectors3D,
                                                                   const TArray<FVector>& OriginalPoints,
                                                                   bool bGaps) const
{
        if (OriginalPoints.Num() < 2 || PointsBetweenVectors3D.Num() < 2)
        {
                return OriginalPoints;
        }

        TArray<FVector> Result;
        Result.Reserve(PointsBetweenVectors3D.Num());
        const FGeographicalEllipsoid& Ellipsoid = UGeographicalContext::GetEllipsoid();

        const auto EllipsoidOffset = [&Ellipsoid, this](const FVector& Point)
        {
                const FVector SurfacePoint = ProjectPointOntoEllipsoid(Point, Ellipsoid);
                return FVector::DotProduct(Point - SurfacePoint, UGeographicalMathLibrary::CalculateSurfaceNormal(SurfacePoint, Ellipsoid));
        };

        for (int32 k = 0; k < OriginalPoints.Num() - 1; ++k)
        {
                const FVector& FirstPoint = OriginalPoints[k];
                const FVector& LastPoint = OriginalPoints[k + 1];
                const double MaxOffset = (EllipsoidOffset(FirstPoint) + EllipsoidOffset(LastPoint)) * 0.5;

                Result.Add(FirstPoint);

                int32 Begin = INDEX_NONE;
                int32 End = INDEX_NONE;

                // Find the range of points in PointsBetweenVectors3D corresponding to the segment from FirstPoint to LastPoint.
                // // To correctly handle a closed polygon, select the first occurrence of FirstPoint and the last occurrence of LastPoint after it.
                for (int32 j = 0; j < PointsBetweenVectors3D.Num(); ++j)
                {
                        if (PointsBetweenVectors3D[j] == FirstPoint && Begin == INDEX_NONE)
                        {
                                Begin = j;
                        }
                        if (Begin != INDEX_NONE && PointsBetweenVectors3D[j] == LastPoint && j > Begin)
                        {
                                End = j; // update to the last occurrence after Begin
                        }
                }

                if (Begin != INDEX_NONE && End != INDEX_NONE && End > Begin + 1)
                {
                        TArray<FVector> Segment;
                        for (int32 j = Begin; j <= End; ++j)
                        {
                                Segment.Add(PointsBetweenVectors3D[j]);
                        }
                        const int32 LastIdx = Segment.Num() - 1;
                        const int32 HalfSize = LastIdx / 2;
                        for (int32 i = 1; i < LastIdx; ++i)
                        {
                                const FVector& Point = Segment[i];
                                FVector SurfacePoint = ProjectPointOntoEllipsoid(Point, Ellipsoid);
                                FVector SurfaceNormal = UGeographicalMathLibrary::CalculateSurfaceNormal(SurfacePoint, Ellipsoid);
                                const double Alpha = i <= HalfSize ? static_cast<double>(i) / HalfSize : static_cast<double>(LastIdx - i) / HalfSize;
                                const double HeightOffset = MaxOffset * Alpha;
                                FVector FinalPoint = SurfacePoint + SurfaceNormal * HeightOffset;
                                Result.Add(FinalPoint);
                        }
                }
                Result.Add(LastPoint);

                if (bGaps && k < OriginalPoints.Num() - 2)
                {
                        FVector CurrentEnd = LastPoint;
                        FVector NextStart = OriginalPoints[k + 1];
                        FVector Direction = (NextStart - CurrentEnd).GetSafeNormal();
                        FVector ShiftedStart = CurrentEnd + Direction;
                        Result.Add(ShiftedStart);
                }
        }
        return Result;
}

FVector ARulerMeasurement::ProjectPointOntoEllipsoid(const FVector& Point, const FGeographicalEllipsoid& Ellipsoid)
{
        const FVector Direction = (Point - Ellipsoid.Center).GetSafeNormal();
        const FRay Ray{ Ellipsoid.Center, Direction };
        const FRayEllipsoidIntersections Intersections = UGeographicalMathLibrary::CalculateRayEllipsoidIntersections(Ray, Ellipsoid, false);

        return Intersections.Near.IsSet() ? Intersections.Near.GetValue() : Point;
}

TArray<FVector> ARulerMeasurement::GeneratePointsAndRelativeToEllipsoid(const TArray<FVector>& Points, float Step, bool Gaps)
{
        return AdjustPointsRelativeToEllipsoid(GeneratePointsBetweenVectors3D(Points, Step), Points, Gaps);
}

TArray<FVector2D> ARulerMeasurement::GenerateFillLines(const TArray<FVector2D>& PolygonPoints, float Step)
{
        TArray<FVector2D> FillPoints;
        const int32 NumPoints = PolygonPoints.Num();
        if (NumPoints < 3)
        {
                return FillPoints;
        }

        // 1. Polygon Bounding Box
        float MinX = PolygonPoints[0].X, MaxX = PolygonPoints[0].X;
        float MinY = PolygonPoints[0].Y, MaxY = PolygonPoints[0].Y;

        for (const FVector2D& P : PolygonPoints)
        {
                MinX = FMath::Min(MinX, P.X);
                MaxX = FMath::Max(MaxX, P.X);
                MinY = FMath::Min(MinY, P.Y);
                MaxY = FMath::Max(MaxY, P.Y);
        }

        // 2. Generating lines along Y with Step (horizontal lines)
        for (float Y = MinY; Y <= MaxY; Y += Step)
        {
                TArray<float> Intersections;

                // 3. Find intersections with a polygon
                for (int32 i = 0; i < NumPoints; ++i)
                {
                        const FVector2D P1 = PolygonPoints[i];
                        const FVector2D P2 = PolygonPoints[(i + 1) % NumPoints]; // Close the polygon

                        // Check for intersection of a horizontal line Y = const
                        if ((P1.Y <= Y && P2.Y >= Y) || (P2.Y <= Y && P1.Y >= Y)) // Segment intersects a line
                        {
                                const float DeltaY = P2.Y - P1.Y;
                                const float DeltaX = P2.X - P1.X;
                                const float Ratio = FMath::IsNearlyZero(DeltaY) ? 0 : (Y - P1.Y) / DeltaY;
                                Intersections.Add(P1.X + Ratio * DeltaX);
                        }
                }

                // 4. Sorting intersections and creating lines
                if (Intersections.Num() >= 2)
                {
                        Intersections.Sort(); // Sort by X

                        // Go by pairs and create lines
                        for (int32 k = 0; k + 1 < Intersections.Num(); k += 2)
                        {
                                FillPoints.Emplace(Intersections[k], Y);
                                FillPoints.Emplace(Intersections[k + 1], Y);
                        }
                }
        }
        return FillPoints;
}

float ARulerMeasurement::CalculateTotalDistance(const TArray<FVector>& Points)
{
        float TotalDistance = 0.0f;
        bool bIsFirst = true;
        FVector PreviousPoint;

        for (const FVector& Point : Points)
        {
                if (bIsFirst)
                {
                        PreviousPoint = Point;
                        bIsFirst = false;
                        continue;
                }

                TotalDistance += FVector::Dist(PreviousPoint, Point);
                PreviousPoint = Point;
        }

        return TotalDistance;
}

float ARulerMeasurement::GetStepForSegments() const
{
        return StepForSegments;
}

FVector2D ARulerMeasurement::OffsetVectorByStep(const FVector2D& InputVector, float Step)
{
        if (InputVector.IsNearlyZero())
        {
                return InputVector;
        }

        const FVector2D Direction = InputVector.GetSafeNormal();
        return InputVector + (Direction * Step);
}

TSubclassOf<URulerPopupWidget> ARulerMeasurement::GetPopupWidgetClass()
{
        return PopupWidgetClass;
}

TSubclassOf<UPromptRulerPopupWidget> ARulerMeasurement::GetPromptRulerPopupClass() const
{
        return PromptRulerPopupClass;
}

TSubclassOf<URulerPointButton> ARulerMeasurement::GetRulerPointButtonWidgetClass() const
{
        return RulerPointButtonWidgetClass;
}

bool ARulerMeasurement::AreVectorsCloseOnViewport(const FVector& FirstVector, const FVector& SecondVector, float DistanceTolerance) const
{
        const APlayerController* const PlayerController = CurrentController.Get();
        FVector2d FirstVector2D;
        FVector2d SecondVector2D;
        PlayerController->ProjectWorldLocationToScreen(FirstVector, FirstVector2D);
        PlayerController->ProjectWorldLocationToScreen(SecondVector, SecondVector2D);

        FVector2D FirstViewportPos;
        FVector2d SecondViewportPos;
        USlateBlueprintLibrary::ScreenToViewport(GetWorld(), FirstVector2D, FirstViewportPos);
        USlateBlueprintLibrary::ScreenToViewport(GetWorld(), SecondVector2D, SecondViewportPos);

        return DistanceTolerance > FVector2D::Distance(FirstViewportPos, SecondViewportPos);
}

URulerPoint* ARulerMeasurement::CreateRulerPointWidget(const FVector& Location)
{
        TObjectPtr<URulerPoint> RulerPointWidget = CreateWidget<URulerPoint>(GetWorld(), RulerPointWidgetClass);

        if (!IsValid(RulerPointWidget))
        {
                return nullptr;
        }

        RulerPointWidget->AddToViewport(ZOrderRulerPoint);
        RulerPointWidget->SetPointLocation(Location);

        URulerPointButton* const Button = RulerPointWidget->GetRulerPointButton();
        if (IsValid(Button))
        {
                OnRulerModeChanged.AddDynamic(Button, &URulerPointButton::RulerModeChanged);
        }

        return RulerPointWidget;
}

bool ARulerMeasurement::LineTraceFromPlayerScreen(const APlayerController* PlayerController,
                                                  FHitResult& OutHitResult,
                                                  const FVector2D& ScreenLocation,
                                                  ECollisionChannel CollisionChannel,
                                                  double TraceDistance)
{
        FVector WorldLocation;
        FVector WorldDirection;
        PlayerController->DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, WorldLocation, WorldDirection);
        FCollisionQueryParams NewParams;
        NewParams.AddIgnoredActor(PlayerController->GetPawn());
        NewParams.bTraceComplex = false;

        return GetWorld()->LineTraceSingleByChannel(OutHitResult, WorldLocation, WorldLocation + WorldDirection * TraceDistance, CollisionChannel, NewParams);
}

void ARulerMeasurement::SetController(APlayerController* Controller)
{
        CurrentController = Controller;
}
