// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RulerPoint.generated.h"

class URulerPointButton;
class ARulerController;

UCLASS()
class METAMEASUREMENT_API URulerPoint : public UUserWidget
{
        GENERATED_BODY()

public:
        virtual void NativeConstruct() override;

        virtual void NativeDestruct() override;

        /**
        * Performs a widget state update on every tick.
        *
        * This method is called on every tick of time to update the widget logic and state.
        * Actions related to changing the button position,
        * updating dependencies between the parent element and the button, and other operations that
        * occur over time can be performed within this method.
        *
        * @param MyGeometry The widget geometry, containing information about its size and position.
        * @param InDeltaTime The time elapsed since the previous tick, expressed in seconds.
        */
        virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

        /**
        * Generates a set of 2D points representing a circle that can be used for drawing.
        * The points are calculated based on the specified center, radius, and number of segments to approximate the circle.
        *
        * @param Center The center of the circle, represented as a 2D vector.
        * @param Radius The radius of the circle.
        * @param NumSegments The number of segments used to represent the circle. A higher value creates a smoother circle.
        * @return An array of 2D points representing the circle.
        */
        UFUNCTION(BlueprintCallable)
        TArray<FVector2D> MakeCircleForDraw(const FVector2D& Center, float Radius, int NumSegments) const;

        /**
        * @brief Sets new ruler points.
        * @param NewPoints The new ruler points.
        */
        void SetPoints(const TArray<FVector>& NewPoints);

        /**
        * Gets the ruler dimension associated with this widget.
        *
        * If the ruler measurement has not yet been set, this method attempts to find
        * and assign it by casting an actor of class ARulerMeasurement in the current world.
        *
        * @return A pointer to an ARulerMeasurement object if found, or nullptr if no valid object was found.
        */
        UFUNCTION(BlueprintPure)
        ARulerMeasurement* GetRulerMeasurement();

        /**
        * Returns a pointer to the RulerController object associated with this URulerPoint instance.
        *
        * @return A pointer to the ARulerController object that controls this ruler point.
        */
        UFUNCTION(BlueprintPure)
        ARulerController* GetRulerController();

        /**
        * Returns the current location of the ruler point's parent.
        *
        * This location is used to position objects associated with this ruler point.
        *
        * @return A vector representing the 3D location of the ruler's parent point.
        */
        UFUNCTION(BlueprintPure)
        FVector GetPointLocation() const;

        /**
        * Sets a new location for the ruler's parent point.
        *
        * If the passed location is different from the current one, it is updated.
        *
        * @param NewParentLocation The new 3D location of the parent point.
        */
        void SetPointLocation(const FVector NewPointLocation);

        /**
        * Handles a click event for a ruler point.
        *
        * When the method is called, selects the ruler controller associated with the point,
        * checks it for validity, and initiates the propagation of updated ruler positions
        * via the corresponding event. If the controller is invalid, nothing is done.
        */
        UFUNCTION(BlueprintCallable)
        void Click();

        /**
        * Releases resources associated with the ruler point and initiates the creation of a polygon if necessary.
        *
        * The method gets the ruler controller associated with the URulerPoint and checks it for validity.
        * If the controller is valid, the method for checking and creating the polygon is called,
        * and the state of the ruler dropdown points is updated.
        * If the controller is invalid, no action is performed.
        */
        UFUNCTION(BlueprintCallable)
        void Release();

        /**
        * Updates the ruler point position based on the new mouse position and optionally sends data to update the ruler.
        * This method changes the ruler point position, updates the ruler point button widget, and initiates a ruler change via the ruler controller.
        *
        * @param MousePosition The new mouse position as a 2D vector.
        * @param bSendRpcNewRulerInfo If true, initiates sending a remote call to update the ruler information.
        */
        UFUNCTION(BlueprintCallable)
        void UpdateRulerPoint(const FVector2D& MousePosition, bool bSendRpcNewRulerInfo);

        /**
        * Sets the parent point's location in world coordinates based on the specified screen position.
        * Performs a ray trace from the screen into game space to determine the intersection point.
        *
        * @param ScreenPLocation Screen coordinates used to determine the new location of the parent point.
        */
        void SetPointLocationFromScreenLocation(const FVector2D& ScreenPLocation);

        /**
        * Projects the world coordinates of the point onto the screen in both directions and converts them to viewport coordinates.
        *
        * This method calculates the screened coordinates of the point in world space and translates them into viewport
        * coordinates for further work inside the application. It also determines whether the point is behind the view camera.
        *
        * @param bTargetBehindCamera Returns a value indicating whether the target is behind the camera.
        * @param ViewportPosition Filled with the coordinates of the point in viewport units after the calculations.
        */
        void ProjectWorldToScreenBidirectionalAndConvertToViewport(bool& bTargetBehindCamera, FVector2D& ViewportPosition) const;

        /**
        * Changes the color of the point to the specified new color.
        *
        * This method allows you to set a new color for the outer segment of the point.
        * If the current color is different from the new color, it will be updated.
        *
        * @param NewColor The new color for the outer segment of the point.
        */
        UFUNCTION(BlueprintCallable)
        void ChangePointColor(FLinearColor NewColor);

        /**
        * Returns a reference to the ruler control point button.
        *
        * This method provides access to the button widget associated with the ruler control point.
        * It can be used to perform further operations on this widget, such as
        * changing its properties or adding event handlers.
        *
        * @return A pointer to the ruler control point button widget.
        */
        URulerPointButton* GetRulerPointButton();

        /**
        * Sets a new controller for the current ruler.
        *
        * This method allows you to assign a new ruler controller object that will
        * define the interaction and behavior of this ruler point.
        *
        * @param NewRulerController The new ruler controller to install.
        */
        void SetRulerController(ARulerController* NewRulerController);

protected:
        TArray<FVector> Points;

        UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
        TObjectPtr<URulerPointButton> RulerPointButtonWidget;

        UPROPERTY(VisibleAnywhere)
        TObjectPtr<ARulerMeasurement> RulerMeasurement;

        UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
        TObjectPtr<ARulerController> RulerController;

        UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
        FVector PointLocation;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|Highlight")
        FLinearColor RulerPointInnertColor = FColor::FromHex("FFFFFFFF");

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|Highlight")
        FLinearColor RulerPointOutertColor = FColor::FromHex("FFC149FF");

        int ZOrderRulerPoint = 400; // required value for correct priority of ruler points display

        float DistanceOffcetForScalePoint = 25; // optimal value of minimum distance, exceeding which scale point increases

        float ScaleForPoint = 1;

        float InnerCircleRadius = 4;

        float OuterCircleRadius = 5;

        int CircleNumSegments = 72; // Number of circle segments, i.e. how "round" the circle will be. Optimal value

        float InnerCircleThickness = 6;

        float OuterCircleThickness = 4;

        float ThicknessScale = 1.5;

        float OuterCircleRadiusOffset = 3;

        /**
        * Handles custom rendering of the widget.
        *
        * This method is used to perform custom rendering on a widget, drawing items on the specified
        * geometry and layer. It handles visual attributes such as shapes, colors, and line thicknesses, and calls
        * helper functions to perform the correct rendering logic based on the widget's state.
        *
        * @param Args Contains information needed to render widgets, such as the execution context.
        * @param AllottedGeometry A geometry that defines the widget's size and position for layout.
        * @param MyCullingRect A rectangle used to cull off-screen elements to optimize rendering.
        * @param OutDrawElements A list of draw elements where the widget can add its rendering commands.
        * @param LayerId The starting layer ID for the layer to draw on; custom rendering can add elements to higher layers.
        * @param InWidgetStyle Slate widget style used to draw the widget, providing colors and effects.
        * @param bParentEnabled Specifies whether the parent widget is enabled; affects appearance and behavior.
        * @return Maximum layer ID used for rendering, including custom drawing layers.
        */
        virtual int32 NativePaint(const FPaintArgs& Args,
                                  const FGeometry& AllottedGeometry,
                                  const FSlateRect& MyCullingRect,
                                  FSlateWindowElementList& OutDrawElements,
                                  int32 LayerId,
                                  const FWidgetStyle& InWidgetStyle,
                                  bool bParentEnabled) const override;
};