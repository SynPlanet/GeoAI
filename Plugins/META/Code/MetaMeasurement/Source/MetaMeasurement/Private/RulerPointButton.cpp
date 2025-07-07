// META all rights received (c)

#include "RulerPointButton.h"
#include "RulerMeasurement.h"
#include "RulerPoint.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void URulerPointButton::NativeConstruct()
{
        Super::NativeConstruct();
        Button->OnHovered.AddDynamic(this, &URulerPointButton::OnHoveredButton);
        Button->OnUnhovered.AddDynamic(this, &URulerPointButton::OnUnHoveredButton);
        Button->OnPressed.AddDynamic(this, &URulerPointButton::OnPressedButton);
        Button->OnReleased.AddDynamic(this, &URulerPointButton::OnReleasedButton);
}

void URulerPointButton::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
        Super::NativeTick(MyGeometry, InDeltaTime);

        if (DeltaTime > 0.5)
        {
                DeltaTime = 0.0;
                UpdateParent(true);
        }
        else
        {
                DeltaTime += InDeltaTime;
                UpdateParent(false);
        }

        SetWidgetPositionByControllerPosition();
}

const FVector URulerPointButton::GetParentLocation()
{
        return ParentLocation;
}

void URulerPointButton::SetParentLocation(const FVector NewParentLocation)
{
        if (NewParentLocation != ParentLocation)
        {
                ParentLocation = NewParentLocation;
        }
}

void URulerPointButton::SetParent(URulerPoint* NewParent)
{
        ParentReference = NewParent;
}

ARulerMeasurement* URulerPointButton::GetRulerMeasurement()
{
        if (!IsValid(RulerMeasurement))
        {
                RulerMeasurement = Cast<ARulerMeasurement>(UGameplayStatics::GetActorOfClass(GetWorld(), ARulerMeasurement::StaticClass()));
        }

        return RulerMeasurement;
}

void URulerPointButton::RulerModeChanged(ERulerMode NewMode)
{
        if (ERulerMode::Ruler == NewMode)
        {
                Button->SetVisibility(ESlateVisibility::Visible);
        }
}

void URulerPointButton::OnHoveredButton()
{
        APlayerController* const PlayerController = GetOwningPlayer();
        ARulerMeasurement* const Measurement = GetRulerMeasurement();

        if (!IsValid(Measurement) || !IsValid(PlayerController))
        {
                return;
        }

        if (Measurement->GetRulerMode() == ERulerMode::RulerEdit)
        {
                Button->SetVisibility(ESlateVisibility::Hidden);
                return;
        }

        Button->SetVisibility(ESlateVisibility::Visible);
        std::ignore = RulerMeasurement->CartographicDragEnabled.ExecuteIfBound(true);
        PlayerController->CurrentMouseCursor = EMouseCursor::GrabHand;
}

void URulerPointButton::OnUnHoveredButton()
{
        APlayerController* const PlayerController = GetOwningPlayer();
        ARulerMeasurement* const Measurement = GetRulerMeasurement();

        if (!IsValid(Measurement) || !IsValid(PlayerController))
        {
                return;
        }

        if (Measurement->GetRulerMode() == ERulerMode::RulerEdit)
        {
                return;
        }

        Button->SetVisibility(ESlateVisibility::Visible);

        if (bButtonPressed)
        {
                return;
        }

        std::ignore = Measurement->CartographicDragEnabled.ExecuteIfBound(false);
        PlayerController->CurrentMouseCursor = EMouseCursor::Default;

        UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, nullptr, EMouseLockMode::DoNotLock, false, true);
}

void URulerPointButton::OnPressedButton()
{
        if (bIsLocked == true)
        {
                return;
        }

        ARulerMeasurement* Measurement = GetRulerMeasurement();
        ARulerController* RController = GetRulerController();

        if (!IsValid(Measurement) || !IsValid(RController))
        {
                return;
        }

        if (Measurement->GetRulerMode() == ERulerMode::RulerEdit)
        {
                return;
        }

        bButtonPressed = true;
        RulerController->RulerPointButtonPressed.Broadcast();
}

void URulerPointButton::OnReleasedButton()
{
        APlayerController* const PlayerController = GetOwningPlayer();
        ARulerMeasurement* const Measurement = GetRulerMeasurement();
        ARulerController* const RController = GetRulerController();

        if (!IsValid(Measurement) || !IsValid(PlayerController) || !IsValid(RController))
        {
                return;
        }

        if (Measurement->GetRulerMode() == ERulerMode::RulerEdit)
        {
                return;
        }

        if (bButtonPressed)
        {
                UpdateParent(true);
        }

        bButtonPressed = false;

        RulerController->RulerPointButtonUnpressed.Broadcast();

        UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, nullptr, EMouseLockMode::DoNotLock, false, true);
}

void URulerPointButton::UpdateParent(bool bSendRpcNewRulerInfo)
{
        if (!bButtonPressed)
        {
                return;
        }

        APlayerController* const PlayerController = GetOwningPlayer();
        const ARulerMeasurement* const Measurement = GetRulerMeasurement();

        if (!IsValid(Measurement) || !IsValid(PlayerController))
        {
                return;
        }

        if (Measurement->GetRulerMode() == ERulerMode::RulerEdit)
        {
                return;
        }

        ParentReference->UpdateRulerPoint(
                USlateBlueprintLibrary::TransformVectorAbsoluteToLocal(GetCachedGeometry(), UWidgetLayoutLibrary::GetMousePositionOnViewport(PlayerController)),
                bSendRpcNewRulerInfo);
}

void URulerPointButton::SetWidgetPositionByControllerPosition()
{
        const APlayerController* const PlayerController = GetOwningPlayer();

        if (!IsValid(PlayerController))
        {
                return;
        }

        FVector2D ScreenPosition;
        FVector2D ViewportPosition;

        bool bTargetBehindCamera;
        ARulerMeasurement::ProjectWorldToScreenBidirectional(PlayerController, GetParentLocation(), ScreenPosition, bTargetBehindCamera, false);
        USlateBlueprintLibrary::ScreenToViewport(GetWorld(), ScreenPosition, ViewportPosition);

        if (ViewportPosition.IsNearlyZero())
        {
                return;
        }

        SetPositionInViewport(ViewportPosition -= ButtonPositionOffset, false);
}

ARulerController* URulerPointButton::GetRulerController()
{
        return RulerController;
}

void URulerPointButton::SetRulerController(ARulerController* NewRulerController)
{
        RulerController = NewRulerController;
}
