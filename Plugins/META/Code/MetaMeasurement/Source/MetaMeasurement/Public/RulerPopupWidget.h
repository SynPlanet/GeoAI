// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "PromptRulerPopupWidget.h"
#include "RulerMeasurement.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "RulerPopupWidget.generated.h"

class UVerticalBox;
/**
 *
 */
UCLASS()
class METAMEASUREMENT_API URulerPopupWidget : public UUserWidget
{
        GENERATED_BODY()

public:
        /**
        * Gets the ruler measurement associated with this widget.
        *
        * If the ruler measurement is not yet set, this method attempts to find
        * and assign it by casting an actor of class ARulerMeasurement in the current world.
        *
        * @return A pointer to an ARulerMeasurement object if found, or nullptr if no valid object was found.
        */
        UFUNCTION(BlueprintCallable)
        const ARulerMeasurement* GetRulerMeasurement();

        virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

        /**
        * Gets the ruler controller associated with this widget.
        *
        * If the ruler controller is not yet valid, this method attempts to find
        * and assign it by casting an actor of class ARulerController in the current world.
        *
        * @return A pointer to an ARulerController object if found, or nullptr if no valid object was found.
        */
        UFUNCTION(BlueprintCallable)
        const ARulerController* GetRulerController();

        /**
        * Updates the display text in the popup widget for distance and area measurements.
        *
        * This method updates the distance text using the provided `Distance` value. If area measurements are enabled
        * and a valid area value is provided, the area text in the popup widget is also updated.
        * If the required widgets are invalid, they are created and dynamically added to the display.
        *
        * @param Length The numeric value of the distance to display in the popup after converting to the appropriate unit of
        * measurement.
        * @param Area The numeric value of the area to display in the popup after converting to the appropriate unit of measurement.
        * If area display is disabled or the value is less than or equal to zero, area updating is skipped.
        */
        UFUNCTION(BlueprintCallable)
        void SetTextPopup(float Length, float Area);

        /**
        * Converts the given length to the appropriate unit (e.g. meters, kilometers)
        * depending on the ruler controller's unit type, and rounds the value for display.
        *
        * If the ruler measurement or controller is invalid, by default the method returns
        * the length as a string in meters.
        *
        * @param Length The numeric length value to convert and format for display.
        * @return A formatted string representing the length in the appropriate unit, or meters by default.
        */
        FString ConversionLengthToActualUnitAndRounding(float Length);

        /**
        * Converts the given area to the appropriate unit and rounds the value.
        *
        * The conversion and formatting are based on the current ruler unit type set by the ruler controller.
        * If the ruler controller or ruler dimension is invalid, the area is returned in square meters (m2).
        * Supported units include: meter, square meter, kilometer, square kilometer, hectare, and hundredths.
        *
        * @param Area The original area value to be converted and formatted.
        * @return A formatted string representing the area in the appropriate unit, with spaces and a suffix.
        */
        FString ConversionAreaToActualUnitAndRounding(float Area);

        /**
          * Adds the provided ruler popup widget to the vertical container.
          *
          * This method checks that the input widget and container are valid before attempting to
          * add the child widget. If the input widget or container are invalid,
          * the operation is aborted.
          *
          * @param PromptPopupWidgetToAdd The ruler popup widget to add to the vertical container.
          */
        UFUNCTION(BlueprintCallable)
        void AddPromptRulerPopupToWidget(UPromptRulerPopupWidget* PromptPopupWidgetToAdd);

        /**
        * @brief Sets the default text for the widget.
        */
        void SetDefaultText() const;

        /**
        * Sets a new parent location for this widget.
        *
        * This method updates the coordinates used to set the parent object's location.
        *
        * @param NewParenLocation The new location in space that will be assigned to the parent object.
        */
        void SetParentLocation(FVector NewParenLocation);

        /**
        * Sets the ruler controller for this widget.
        *
        * This method allows you to assign the specified ruler controller to this widget.
        *
        * @param NewRulerController A pointer to a new ARulerController object that will be associated with the widget.
        */
        void SetRulerController(ARulerController* NewRulerController);

        /**
        * Sets the ruler area to be shown.
        *
        * This method controls the visibility state of the ruler area in the widget.
        *
        * @param NewShowRulerArea A new boolean status that determines whether the ruler area should be shown.
        */
        void SetShowRulerArea(bool NewShowRulerArea);

        /**
        * @brief Adjusts the widget's position on the screen to match the ruler's position in the world.
        */
        UFUNCTION(BlueprintCallable)
        void AdjustScreenPosition();

protected:
        UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UPromptRulerPopupWidget> LengthPromptRulerPopupWidget;

        UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UPromptRulerPopupWidget> AreaPromptRulerPopupWidget;

        UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UVerticalBox> VerticalBoxPopup;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<ARulerMeasurement> RulerMeasurement;

        UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
        TObjectPtr<ARulerController> RulerController;

        UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
        bool bShowRulerArea;

        UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
        FVector ParentLocation;

        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
        FText DefaultText = FText::FromString(TEXT("Начальная точка измерения"));

        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RulerPopupWidget")
        FVector2D ViewportOffset = FVector2D::ZeroVector;

        static constexpr float M2ToKm2 = 0.000001;
        static constexpr float M2ToHectare = 0.0001;
        static constexpr float M2ToAre = 0.01;
};
