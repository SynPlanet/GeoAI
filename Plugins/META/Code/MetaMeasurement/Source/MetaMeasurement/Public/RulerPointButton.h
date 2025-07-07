// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "RulerMeasurement.h"
#include "Components/Button.h"
#include "RulerPointButton.generated.h"

class URulerPoint;
class ARulerMeasurement;
class UUserWidget;

/**
 *
 */
UCLASS()
class METAMEASUREMENT_API URulerPointButton : public UUserWidget
{
        GENERATED_BODY()
public:
        /**
        * Performs actions necessary for initializing the widget after its creation.
        *
        * This method is called after the widget has been created and added to the UI hierarchy.
        * Intended for configuring the widget, binding events, and performing other operations
        * during its constructive initialization.
        */
        virtual void NativeConstruct() override;

        /**
        * Performs a widget state update on every tick.
        *
        * This method is called on every tick of time to update the widget logic and state.
        * Within this method, actions related to changing the position of the button,
        * updating dependencies between the parent element and the button, and other operations that occur over time can be performed.
        *
        * @param MyGeometry The widget geometry, containing information about its size and position.
        * @param InDeltaTime The time elapsed since the previous tick, expressed in seconds.
        */
        virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

        /**
        * Returns the current location of the ruler's parent point.
        *
        * This location is used to position objects associated with this ruler point.
        *
        * @return A vector representing the 3D location of the ruler's parent point.
        */
        UFUNCTION(BlueprintCallable)
        const FVector GetParentLocation();

        /**
        * Sets the new location of the ruler's parent point.
        *
        * This location is used to update the position of objects associated with this ruler point.
        * If the new location is different from the current location, it will be updated.
        *
        * @param NewParentLocation A new vector representing the 3D location of the ruler's parent point.
        */
        UFUNCTION(BlueprintCallable)
        void SetParentLocation(FVector NewParentLocation);

        /**
        * Sets the parent object of the ruler point.
        *
        * This is used to associate the button with the ruler point it belongs to.
        *
        * @param NewParent A new URulerPoint object to set as the parent.
        */
        void SetParent(URulerPoint* NewParent);

        /**
        * Gets the ruler measurement associated with this button.
        *
        * This method ensures that the associated ruler measurement object is correct and initializes it if it has not been set yet. It looks for an actor of the specified class
        * in the current game world and returns the corresponding instance.
        *
        * @return A pointer to the ruler measurement actor associated with this button.
        */
        ARulerMeasurement* GetRulerMeasurement();

        /**
        * Handles the ruler mode change event.
        *
        * This method updates the visibility of the button based on the new ruler mode.
        *
        * @param NewMode The new ruler mode that determines the visibility of the button.
        */
        UFUNCTION()
        void RulerModeChanged(ERulerMode NewMode);

        /**
        * Handles the hover behavior of the button.
        *
        * This method adjusts the visibility of the button and changes the player's mouse cursor
        * based on the current ruler mode of the associated dimension object.
        * If the ruler is in edit mode, the button will be hidden; otherwise,
        * it will be made visible and cartographic drag functionality will be enabled.
        */
        UFUNCTION()
        void OnHoveredButton();

        /**
        * Handles the behavior when the cursor leaves the button.
        *
        * This method is called when the button loses its hover state, updating its visibility and interaction properties,
        * while resetting the cursor and input modes. It ensures the correct behavior
        * depending on the current ruler mode and the internal state of the button.
        */
        UFUNCTION()
        void OnUnHoveredButton();

        /**
        * Handles the press of the ruler point button.
        *
        * The method checks the state of the button, the ruler mode, and the availability of the required objects,
        * and then performs the appropriate actions if the press is valid.
        * If the button is disabled or the ruler mode is set to RulerEdit, execution stops.
        */
        UFUNCTION()
        void OnPressedButton();

        /**
        * Performs the actions associated with releasing the ruler point button.
        *
        * This method is called when the button is released. It checks the state of the ruler's measurement system
        * and performs appropriate updates, such as notifying parent components and
        * setting input modes. No action is taken if the measurement system or player's
        * controller is invalid, or if the ruler is in edit mode.
        */
        UFUNCTION()
        void OnReleasedButton();

        /**
        * Updates the parent object reference with the new ruler point data.
        *
        * This method updates the parent with the new ruler point position and, if necessary,
        * sends the updated ruler information via a remote procedure call (RPC).
        *
        * @param bSendRpcNewRulerInfo Specifies whether to send the new ruler information via RPC.
        */
        void UpdateParent(bool bSendRpcNewRulerInfo);

        /**
        * Updates the widget's position based on the controller's position in the game world.
        *
        * This function calculates the widget's position in the viewport by projecting the parent's world
        * position onto the screen position and then converting it to viewport coordinates.
        * If certain conditions are met, such as the ruler being in
        * edit mode or the input being invalid, the function returns without making any changes.
        *
        * The final widget position is shifted slightly to align.
        */
        void SetWidgetPositionByControllerPosition();

        ARulerController* GetRulerController();

        void SetRulerController(ARulerController* NewRulerController);

        UPROPERTY(BlueprintReadWrite)
        bool bIsLocked = false;

protected:
        FVector ParentLocation;

        TObjectPtr<URulerPoint> ParentReference;

        TObjectPtr<ARulerMeasurement> RulerMeasurement;

        UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
        TObjectPtr<UButton> Button;

        bool bButtonPressed = false;

        float DeltaTime;

        FVector2D ButtonPositionOffset = FVector2D(12.0f, 12.0f); // Offset for button position
        UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
        TObjectPtr<ARulerController> RulerController;
};
