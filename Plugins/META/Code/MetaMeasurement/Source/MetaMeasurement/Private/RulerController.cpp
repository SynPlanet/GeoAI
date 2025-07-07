// META all rights received (c)

#include "RulerController.h"

#include "GeographicalTransforms.h"
#include "RulerLines.h"
#include "RulerPopupWidget.h"
#include "RulerSelection.h"
#include "RulerPointButton.h"
#include "Algo/ForEach.h"
#include "Kismet/GameplayStatics.h"

namespace
{
        template <typename UWidget>
                requires(std::is_base_of_v<UUserWidget, UWidget>)
        void RemoveWidgetSafely(TObjectPtr<UWidget>& Widget)
        {
                if (!IsValid(Widget))
                {
                        return;
                }
                Widget->RemoveFromParent();
                Widget = nullptr;
        }
} // namespace

void FRulerInfo::Reset()
{
        Length = 0.;
        Guid.Empty();
        Name.Empty();
        Positions.Empty();
}

void ARulerController::Initialization()
{
        const FRulerInfo Info = GetRulerInfo();
        if (Info.Positions.Num() < 2)
        {
                return;
        }

        ARulerMeasurement* const Measurement = GetRulerMeasurement();

        if (!IsValid(Measurement))
        {
                return;
        }

        PointsAtEllipsoid = Measurement->GeneratePointsAndRelativeToEllipsoid(Info.Positions, Measurement->GetStepForSegments(), false);

        RulerLinesWidget = CreateWidget<URulerLines>(GetWorld(), RulerLinesWidgetClass);
        RulerLinesWidget->SetPoints(PointsAtEllipsoid);
        RulerLinesWidget->AddToViewport(RulerLinesWidgetZOrder);

        RulerPopupWidget = CreateWidget<URulerPopupWidget>(GetWorld(), RulerPopupWidgetClass);
        RulerPopupWidget->SetRulerController(this);
        RulerPopupWidget->SetShowRulerArea(bShowArea);
        RulerPopupWidget->SetParentLocation(Info.Positions.Last());
        RulerPopupWidget->AddToViewport(RulerPopupWidgetZOrder);
        RulerPopupWidget->AdjustScreenPosition();

        bRulerIsPolygon = (Info.Positions.Last() - Info.Positions[0]).IsNearlyZero();
        const int32 Count = bRulerIsPolygon ? Info.Positions.Num() - 1 : Info.Positions.Num();
        for (int32 i = 0; i < Count; ++i)
        {
                URulerPoint* PointWidget = CreateWidget<URulerPoint>(GetWorld(), RulerPointWidgetClass);
                PointWidget->SetRulerController(this);
                PointWidget->SetPointLocation(Info.Positions[i]);
                PointWidget->AddToViewport();
                RulerPointWidgets.Add(PointWidget);
                RulerPointButtonPressed.AddUObject(PointWidget, &URulerPoint::Click);
                RulerPointButtonUnpressed.AddUObject(PointWidget, &URulerPoint::Release);
        }
        if (bRulerIsPolygon)
        {
                PolygonArea = Measurement->CalculatePolygonArea(PointsAtEllipsoid, Measurement->GetDistanceToleranceForAreaPolygon());
                CreatePolygonSelection();
        }

        UpdateRuler(true);
        std::ignore = RulerSavedDelegate.ExecuteIfBound(GetRulerInfoInEcef(GetRulerInfo()));
}

void ARulerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
        Super::EndPlay(EndPlayReason);

        RemoveWidgetSafely(RulerLinesWidget);
        RemoveWidgetSafely(RulerPopupWidget);
        RemoveWidgetSafely(RulerSelection);
        Algo::ForEach(RulerPointWidgets, &UWidget::RemoveFromParent);
}

FRulerInfo ARulerController::GetRulerInfo() const
{
        return RulerInfo;
}

void ARulerController::SetRulerVisibilty(bool bIsVisible)
{
        const ESlateVisibility WidgetsVisibility = bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
        for (URulerPoint* const Point : RulerPointWidgets)
        {
                Point->SetVisibility(WidgetsVisibility);
        }
        RulerLinesWidget->SetVisibility(WidgetsVisibility);
        RulerPopupWidget->SetVisibility(WidgetsVisibility);
        RulerSelection->SetPolygonVisibility(bIsVisible);
}

void ARulerController::UpdateRuler(bool bSendRpcNewRulerInfo)
{
        UpdateRulerPosition();
        UpdateRulerDistanceAndArea();
        UpdateRulerUI();
        UpdateRulerSelection();

        if (bSendRpcNewRulerInfo)
        {
                std::ignore = SendRulerInfoDelegate.ExecuteIfBound(GetRulerInfoInEcef(GetRulerInfo()));
        }
}

void ARulerController::ChangeRulerColor(FLinearColor NewColor)
{
        if (!IsValid(RulerLinesWidget))
        {
                return;
        }
        RulerLinesWidget->ChangeLinesColor(NewColor);

        if (RulerPointWidgets.IsEmpty())
        {
                return;
        }

        for (URulerPoint* const Point : RulerPointWidgets)
        {
                Point->ChangePointColor(NewColor);
        }
}

ERulerUnitType ARulerController::GetRulerLengtUnitType() const
{
        return RulerLengthUnitType;
}

ERulerUnitType ARulerController::GetRulerAreaUnitType() const
{
        return RulerAreaUnitType;
}

void ARulerController::SetRulerInfo(FRulerInfo NewRulerInfo)
{
        RulerInfo = NewRulerInfo;
}

void ARulerController::SetLengthUnitType(ERulerUnitType LengthUnitType)
{
        if (RulerLengthUnitType != LengthUnitType)
        {
                RulerLengthUnitType = LengthUnitType;
                UpdateRuler();
        }
}

void ARulerController::SetAreaUnitType(ERulerUnitType AreaUnitType)
{
        if (RulerAreaUnitType != AreaUnitType)
        {
                RulerAreaUnitType = AreaUnitType;
                UpdateRuler();
        }
}

const TArray<URulerPoint*>& ARulerController::GetRulerPoints() const
{
        return RulerPointWidgets;
}

void ARulerController::UpdateRulerPosition()
{
        if (!IsValid(RulerLinesWidget))
        {
                return;
        }

        const TArray<FVector> CachedPointArray = GetRulerInfo().Positions;
        RulerInfo.Positions.Empty();

        RulerPointWidgets.RemoveAll(std::not_fn(IsValid));

        for (URulerPoint* PointWidget : RulerPointWidgets)
        {
                const FVector PointLocation = PointWidget->GetPointLocation();
                RulerInfo.Positions.Add(PointLocation);
                PointWidget->GetRulerPointButton()->SetParentLocation(PointLocation);
        }

        if (CachedPointArray.Last() == CachedPointArray[0])
        {
                FVector LastPoint = RulerInfo.Positions[0];
                RulerInfo.Positions.Add(LastPoint);
        }

        ARulerMeasurement* const Measurement = GetRulerMeasurement();
        PointsAtEllipsoid = Measurement->GeneratePointsAndRelativeToEllipsoid(RulerInfo.Positions, Measurement->GetStepForSegments(), false);
        RulerLinesWidget->SetPoints(PointsAtEllipsoid);
}

void ARulerController::UpdateRulerDistanceAndArea()
{
        ARulerMeasurement* const Measurement = GetRulerMeasurement();

        if (!IsValid(Measurement) || PointsAtEllipsoid.IsEmpty())
        {
                return;
        }

        FRulerInfo NewRulerInfo = GetRulerInfo();
        NewRulerInfo.Length = FMath::RoundToFloat(ARulerMeasurement::CalculateTotalDistance(PointsAtEllipsoid) * UE_CM_TO_M);
        SetRulerInfo(NewRulerInfo);

        if (!bShowArea && !bRulerIsPolygon)
        {
                return;
        }

        PolygonArea = Measurement->CalculatePolygonArea(PointsAtEllipsoid, Measurement->GetDistanceToleranceForAreaPolygon());
}

void ARulerController::UpdateRulerUI()
{
        if (!IsValid(RulerPopupWidget))
        {
                return;
        }

        const FRulerInfo Info = GetRulerInfo();
        RulerPopupWidget->SetTextPopup(Info.Length, PolygonArea);
        RulerPopupWidget->SetParentLocation(Info.Positions.Last());
}

ARulerMeasurement* ARulerController::GetRulerMeasurement()
{
        if (!IsValid(RulerMeasurement))
        {
                RulerMeasurement = Cast<ARulerMeasurement>(UGameplayStatics::GetActorOfClass(GetWorld(), ARulerMeasurement::StaticClass()));
        }

        return RulerMeasurement;
}

void ARulerController::CreatePolygonSelection()
{
        if (PointsAtEllipsoid.Num() < 4)
        {
                return;
        }

        if (!RulerSelectionWidgetClass)
        {
                UE_LOG(LogTemp, Warning, TEXT("Poligon widget class not selected"));

                return;
        }

        RulerSelection = CreateWidget<URulerSelection>(GetWorld(), RulerSelectionWidgetClass);
        RulerSelection->SetPolygonPoints(PointsAtEllipsoid);
        RulerSelection->AddToViewport(RulerSelectionWidgetZOrder);
}

void ARulerController::UpdateRulerSelection()
{
        if (!bRulerIsPolygon || PointsAtEllipsoid.IsEmpty())
        {
                return;
        }

        if (!IsValid(RulerSelection))
        {
                CreatePolygonSelection();
                return;
        }

        if (RulerSelection->GetPolygonPoints().Num() < 4)
        {
                RulerSelection->RemoveFromParent();
                RulerSelection = nullptr;
                return;
        }

        RulerSelection->SetPolygonPoints(PointsAtEllipsoid);
        RulerSelection->PolygonSelectionUpdatePoints(PointsAtEllipsoid);
}

void ARulerController::SetLock(bool bIsLocked) const
{
        if (RulerPointWidgets.IsEmpty())
        {
                return;
        }

        for (URulerPoint* const PointWidget : RulerPointWidgets)
        {
                PointWidget->GetRulerPointButton()->bIsLocked = bIsLocked;
        }

        if (!IsValid(RulerSelection))
        {
                return;
        }

        RulerSelection->SetPolygonVisibility(!bIsLocked);
}

void ARulerController::CheckAndCreatePolygon()
{
        if (bRulerIsPolygon || PointsAtEllipsoid.IsEmpty())
        {
                return;
        }

        ARulerMeasurement* const Measurement = GetRulerMeasurement();

        if (!IsValid(Measurement))
        {
                return;
        }

        PolygonArea = Measurement->CalculatePolygonArea(PointsAtEllipsoid, Measurement->GetDistanceToleranceForAreaPolygon());

        if (PolygonArea == 0)
        {
                return;
        }

        FRulerInfo Info = GetRulerInfo();

        TArray<FVector> CachedRulerInfoPositions = Measurement->CreateRulerPolygon(Info.Positions);
        Info.Positions = CachedRulerInfoPositions;
        SetRulerInfo(Info);

        if (RulerPointWidgets.IsEmpty())
        {
                return;
        }

        RulerPointWidgets.Last()->RemoveFromParent();
        RulerPointWidgets.Pop();

        bRulerIsPolygon = true;
        PointsAtEllipsoid.Empty();

        PointsAtEllipsoid = Measurement->GeneratePointsAndRelativeToEllipsoid(Info.Positions, Measurement->GetStepForSegments(), false);

        if (!IsValid(RulerLinesWidget))
        {
                return;
        }

        RulerLinesWidget->SetPoints(PointsAtEllipsoid);

        UpdateRulerUI();
        std::ignore = SendRulerInfoDelegate.ExecuteIfBound(GetRulerInfoInEcef(Info));
        CreatePolygonSelection();
}

void ARulerController::SetShowRulerArea(bool NewShowRulerArea)
{
        bShowArea = NewShowRulerArea;
}

FRulerInfo ARulerController::GetRulerInfoInEcef(FRulerInfo Info)
{
        TArray<FVector> ECEFPosition;
        for (FVector Point : Info.Positions)
        {
                ECEFPosition.Add(UGeographicalTransforms::UnrealToEcef(Point, this));
        }

        FRulerInfo EcefInfo = Info;
        EcefInfo.Positions = ECEFPosition;
        return EcefInfo;
}
