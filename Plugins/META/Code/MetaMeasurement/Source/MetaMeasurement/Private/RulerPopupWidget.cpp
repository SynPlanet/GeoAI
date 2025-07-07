// META all rights received (c)

#include "RulerPopupWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

const ARulerMeasurement* URulerPopupWidget::GetRulerMeasurement()
{
        if (!IsValid(RulerMeasurement))
        {
                RulerMeasurement = Cast<ARulerMeasurement>(UGameplayStatics::GetActorOfClass(GetWorld(), ARulerMeasurement::StaticClass()));
        }

        return RulerMeasurement;
}

void URulerPopupWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
        Super::NativeTick(MyGeometry, InDeltaTime);
        AdjustScreenPosition();
}

const ARulerController* URulerPopupWidget::GetRulerController()
{
        return RulerController;
}

void URulerPopupWidget::SetTextPopup(float Length, float Area)
{
        if (!IsValid(LengthPromptRulerPopupWidget))
        {
                return;
        }

        FString TotalLength = ConversionLengthToActualUnitAndRounding(Length);
        FString DisplayTextLength = (bShowRulerArea && Area > 0) ? TEXT("A: ") + TotalLength : TotalLength;
        LengthPromptRulerPopupWidget->SetTextPopup(FText::FromString(DisplayTextLength));

        if (!bShowRulerArea || Area <= 0)
        {
                AreaPromptRulerPopupWidget->SetVisibility(ESlateVisibility::Collapsed);
                return;
        }

        AreaPromptRulerPopupWidget->SetVisibility(ESlateVisibility::Visible);
        AreaPromptRulerPopupWidget->SetTextPopup(FText::FromString(TEXT("Area: ") + ConversionAreaToActualUnitAndRounding(Area)));
}

FString URulerPopupWidget::ConversionLengthToActualUnitAndRounding(float Length)
{
        const ARulerMeasurement* Measurement = GetRulerMeasurement();
        if (!IsValid(Measurement) || Length <= 0)
        {
                return FString::SanitizeFloat(Length) + TEXT(" m");
        }

        const ARulerController* Controller = GetRulerController();
        if (!IsValid(Controller))
        {
                return Measurement->FormatUnitsWithSpaces(Length) + TEXT(" m");
        }

        switch (Controller->GetRulerLengtUnitType())
        {
                case ERulerUnitType::Meter:
                        return Measurement->FormatUnitsWithSpaces(Length) + TEXT(" m");

                case ERulerUnitType::Kilometer:
                        return Measurement->FormatUnitsWithSpaces(Length * UE_M_TO_KM) + TEXT(" кm");

                default:
                        return FString::SanitizeFloat(Length) + TEXT(" m");
        }
}

FString URulerPopupWidget::ConversionAreaToActualUnitAndRounding(float Area)
{
        const ARulerMeasurement* Measurement = GetRulerMeasurement();
        const ARulerController* Controller = GetRulerController();

        if (!IsValid(Controller) || !IsValid(Measurement))
        {
                return FString::SanitizeFloat(Area) + TEXT(" m2");
        }

        switch (Controller->GetRulerAreaUnitType())
        {
                case ERulerUnitType::Meter:
                        return Measurement->FormatUnitsWithSpaces(Area) + TEXT(" m2");

                case ERulerUnitType::SquareMeter:
                        return Measurement->FormatUnitsWithSpaces(Area) + TEXT(" m2");

                case ERulerUnitType::Kilometer:
                        return Measurement->FormatUnitsWithSpaces(Area * M2ToKm2) + TEXT(" кm2");

                case ERulerUnitType::SquareKilometer:
                        return Measurement->FormatUnitsWithSpaces(Area * M2ToKm2) + TEXT(" кm2");

                case ERulerUnitType::Hectare:
                        return Measurement->FormatUnitsWithSpaces(Area * M2ToHectare) + TEXT(" hectare");

                case ERulerUnitType::HundredPart:
                        return Measurement->FormatUnitsWithSpaces(Area * M2ToAre) + TEXT(" square meters");

                default:
                        return FString::SanitizeFloat(Area) + TEXT(" m2");
        }
}

void URulerPopupWidget::AddPromptRulerPopupToWidget(UPromptRulerPopupWidget* PromptPopupWidgetToAdd)
{
        if (!IsValid(PromptPopupWidgetToAdd) || !IsValid(VerticalBoxPopup))
        {
                return;
        }

        VerticalBoxPopup->AddChildToVerticalBox(PromptPopupWidgetToAdd);
}

void URulerPopupWidget::SetDefaultText() const
{
        LengthPromptRulerPopupWidget->SetTextPopup(DefaultText);
}

void URulerPopupWidget::SetParentLocation(FVector NewParenLocation)
{
        ParentLocation = NewParenLocation;
}

void URulerPopupWidget::SetRulerController(ARulerController* NewRulerController)
{
        RulerController = NewRulerController;
}

void URulerPopupWidget::SetShowRulerArea(bool NewShowRulerArea)
{
        bShowRulerArea = NewShowRulerArea;
}

void URulerPopupWidget::AdjustScreenPosition()
{
        APlayerController* const Owner = GetOwningPlayer();
        FVector2D ScreenPosition;
        if (ParentLocation.IsNearlyZero() || !Owner->ProjectWorldLocationToScreen(ParentLocation, ScreenPosition))
        {
                return;
        }

        FVector2D ViewportPosition;
        USlateBlueprintLibrary::ScreenToViewport(Owner, ScreenPosition, ViewportPosition);
        if (ViewportPosition.IsNearlyZero())
        {
                return;
        }

        SetPositionInViewport(ViewportPosition + ViewportOffset, false);
}
