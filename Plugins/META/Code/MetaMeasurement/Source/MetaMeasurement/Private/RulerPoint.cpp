// META all rights received (c)

#include "RulerPoint.h"

#include "RulerPopupWidget.h"
#include "RulerPointButton.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "RulerController.h"

void URulerPoint::NativeConstruct()
{
        Super::NativeConstruct();

        RulerPointButtonWidget = CreateWidget<URulerPointButton>(GetWorld(), GetRulerMeasurement()->GetRulerPointButtonWidgetClass());

        if (IsValid(RulerPointButtonWidget))
        {
                RulerPointButtonWidget->SetParentLocation(GetPointLocation());
                RulerPointButtonWidget->SetParent(this);
                RulerPointButtonWidget->AddToViewport(ZOrderRulerPoint);
                RulerPointButtonWidget->SetRulerController(GetRulerController());
        }
}

void URulerPoint::NativeDestruct()
{
        Super::NativeDestruct();

        if (IsValid(RulerPointButtonWidget))
        {
                RulerPointButtonWidget->RemoveFromParent();
        }
}

void URulerPoint::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
        Super::NativeTick(MyGeometry, InDeltaTime);

        APlayerController* const PlayerController = GetOwningPlayer();

        if (!IsValid(PlayerController))
        {
                return;
        }

        FVector2D ViewportPosition;
        FVector2D MousePosition;
        bool bTargetBehindCamera;

        ProjectWorldToScreenBidirectionalAndConvertToViewport(bTargetBehindCamera, ViewportPosition);
        UWidgetLayoutLibrary::GetMousePositionScaledByDPI(PlayerController, MousePosition.X, MousePosition.Y);

        if (FVector2D::Distance(ViewportPosition, MousePosition) < DistanceOffcetForScalePoint)
        {
                ScaleForPoint = 1.5;
        }
        else
        {
                ScaleForPoint = 1;
        }
}

TArray<FVector2D> URulerPoint::MakeCircleForDraw(const FVector2D& Center, float Radius, int NumSegments) const
{
        TArray<FVector2D> ResultPoints;
        for (int i = 0; i <= NumSegments; i++)
        {
                const float Angle = (2.0f * PI / NumSegments) * i;
                ResultPoints.Add(Center + FVector2D(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius));
        }
        return ResultPoints;
}

void URulerPoint::SetPoints(const TArray<FVector>& NewPoints)
{
        Points = NewPoints;
}

ARulerMeasurement* URulerPoint::GetRulerMeasurement()
{
        if (!IsValid(RulerMeasurement))
        {
                RulerMeasurement = Cast<ARulerMeasurement>(UGameplayStatics::GetActorOfClass(GetWorld(), ARulerMeasurement::StaticClass()));
        }

        return RulerMeasurement;
}

ARulerController* URulerPoint::GetRulerController()
{
        return RulerController;
}

FVector URulerPoint::GetPointLocation() const
{
        return PointLocation;
}

void URulerPoint::SetPointLocation(const FVector NewPointLocation)
{
        if (NewPointLocation != PointLocation)
        {
                PointLocation = NewPointLocation;
        }
}

void URulerPoint::Click()
{
        ARulerController* Controller = GetRulerController();
        if (!IsValid(Controller))
        {
                return;
        }

        Controller->RulerPointsDragged.Broadcast(Controller->GetRulerInfo().Positions);
}

void URulerPoint::Release()
{
        ARulerController* Controller = GetRulerController();
        if (!IsValid(Controller))
        {
                return;
        }

        Controller->CheckAndCreatePolygon();
        Controller->RulerPointsDropped.Broadcast();
}

void URulerPoint::UpdateRulerPoint(const FVector2D& MousePosition, bool bSendRpcNewRulerInfo)
{
        ARulerController* Controller = GetRulerController();
        if (!IsValid(RulerPointButtonWidget) || !IsValid(Controller))
        {
                return;
        }

        SetPointLocationFromScreenLocation(MousePosition);
        RulerPointButtonWidget->SetParentLocation(GetPointLocation());

        Controller->UpdateRuler(bSendRpcNewRulerInfo);
        Controller->RulerPointsDragged.Broadcast(Controller->GetRulerInfo().Positions);
}

void URulerPoint::SetPointLocationFromScreenLocation(const FVector2D& ScreenPLocation)
{
        ARulerMeasurement* Measurement = GetRulerMeasurement();

        if (!IsValid(Measurement))
        {
                return;
        }

        FHitResult OutHitResult;
        if (!Measurement->LineTraceFromPlayerScreen(Measurement->GetPlayerController(), OutHitResult, ScreenPLocation, ECC_GameTraceChannel2, FLT_MAX)
            || !OutHitResult.bBlockingHit)
        {
                return;
        }
        SetPointLocation(OutHitResult.Location);
}

void URulerPoint::ProjectWorldToScreenBidirectionalAndConvertToViewport(bool& bTargetBehindCamera, FVector2D& ViewportPosition) const
{
        const APlayerController* const PlayerController = GetOwningPlayer();

        if (!IsValid(PlayerController))
        {
                return;
        }

        FVector2D ScreenPosition;
        ARulerMeasurement::ProjectWorldToScreenBidirectional(PlayerController, GetPointLocation(), ScreenPosition, bTargetBehindCamera, false);
        USlateBlueprintLibrary::ScreenToViewport(GEngine->GameViewport->GetWorld(), ScreenPosition, ViewportPosition);
}

int32 URulerPoint::NativePaint(const FPaintArgs& Args,
                               const FGeometry& AllottedGeometry,
                               const FSlateRect& MyCullingRect,
                               FSlateWindowElementList& OutDrawElements,
                               int32 LayerId,
                               const FWidgetStyle& InWidgetStyle,
                               bool bParentEnabled) const
{
        const APlayerController* const PlayerController = GetOwningPlayer();

        if (!IsValid(PlayerController))
        {
                return LayerId;
        }

        int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

        FVector2D ViewportPosition;
        bool bTargetBehindCamera;

        ProjectWorldToScreenBidirectionalAndConvertToViewport(bTargetBehindCamera, ViewportPosition);

        if (bTargetBehindCamera && ViewportPosition.IsNearlyZero(1))
        {
                return LayerId;
        }

        FPaintContext Context(AllottedGeometry, MyCullingRect, OutDrawElements, MaxLayer, InWidgetStyle, bParentEnabled);

        UWidgetBlueprintLibrary::DrawLines(Context,
                                           MakeCircleForDraw(ViewportPosition, InnerCircleRadius * ScaleForPoint, CircleNumSegments),
                                           RulerPointInnertColor,
                                           true,
                                           (InnerCircleThickness * ScaleForPoint) * ThicknessScale);

        UWidgetBlueprintLibrary::DrawLines(
                Context,
                MakeCircleForDraw(ViewportPosition, (OuterCircleRadius * ScaleForPoint) + OuterCircleRadiusOffset, CircleNumSegments),
                RulerPointOutertColor,
                true,
                OuterCircleThickness * ScaleForPoint);

        return MaxLayer;
}

void URulerPoint::ChangePointColor(FLinearColor NewColor)
{
        if (RulerPointOutertColor != NewColor)
        {
                RulerPointOutertColor = NewColor;
        }
}

URulerPointButton* URulerPoint::GetRulerPointButton()
{
        return RulerPointButtonWidget;
}

void URulerPoint::SetRulerController(ARulerController* NewRulerController)
{
        RulerController = NewRulerController;
}