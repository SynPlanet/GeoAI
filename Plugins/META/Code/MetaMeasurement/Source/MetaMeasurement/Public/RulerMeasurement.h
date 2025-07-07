// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "RulerController.h"
#include "RulerLines.h"
#include "RulerMeasurement.generated.h"

class UPromptRulerPopupWidget;
class URulerPopupWidget;
struct FGeographicalEllipsoid;
class URulerSplineWidget;
class URulerPoint;
class URulerPointButton;
class UGeographicalContext;
class UPlayerScreenProjectionLibrary;

UENUM(BlueprintType)
enum class ERulerMode : uint8
{
        None,
        Ruler,
        RulerEdit
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRulerModeChanged, ERulerMode, NewMode);
DECLARE_DELEGATE_OneParam(FRulerLengthUpdated, const FRulerInfo&);
DECLARE_DELEGATE_OneParam(FRulerSaved, const FRulerInfo&);
DECLARE_DELEGATE_OneParam(FCartographicDragEnabled, bool);

UCLASS()
class METAMEASUREMENT_API ARulerMeasurement : public AActor
{
        GENERATED_BODY()

public:
        ARulerMeasurement();

        virtual void Tick(float DeltaSeconds) override;

        virtual void BeginDestroy() override;

        virtual void OnConstruction(const FTransform& Transform) override;

        UPROPERTY(BlueprintAssignable, Category = "RulerMeasurement")
        FOnRulerModeChanged OnRulerModeChanged;

        FRulerLengthUpdated RulerLengthUpdated;

        FRulerSaved RulerSaved;

        FCartographicDragEnabled CartographicDragEnabled;

        /**
        * @brief Creates a polygon from a set of points.
        * Used to adjust the precision of the points if the first and last points in the input data do not match.
        * @param Input A set of points to create the polygon.
        * @return A set of points that make up the polygon.
        */
        UFUNCTION(BlueprintPure)
        static TArray<FVector> CreateRulerPolygon(const TArray<FVector>& Input);

        /**
        * @brief Rounds a floating-point number to the first decimal place.
        * @param Value The input value.
        * @return The rounded value.
        */
        UFUNCTION(BlueprintPure)
        static float RoundFloat(float Value);

        /**
        * @brief Clears the time ruler.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void ClearTempRuler();

        /**
        * @brief Sets the ruler information.
        *
        * This method updates the current ruler information based on the provided FRulerInfo object.
        *
        * @param NewRulerInfo An object containing the new ruler data.
        */
        UFUNCTION(BlueprintCallable)
        void SetRulerInfo(FRulerInfo NewRulerInfo);
        /**
        * @brief Returns the current ruler mode.
        *
        * This method provides information about the current ruler mode.
        *
        * @return The current ruler mode as an ERulerMode enumeration value.
        */
        UFUNCTION(BlueprintPure, Category = "RulerMeasurement")
        ERulerMode GetRulerMode() const;

        /**
        * @brief Sets the measurement mode for the ruler.
        *
        * This method changes the current ruler measurement mode to a new one and notifies about it via the appropriate event.
        *
        * @param NewMode The new measurement mode for the ruler.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void SetRulerMode(ERulerMode NewMode);

        /**
        * @brief Gets the marker for placing the ruler.
        *
        * This method returns a marker object that specifies the location to place the ruler.
        *
        * @return An AActor object representing the location marker for the ruler.
        */
        UFUNCTION(BlueprintPure, Category = "RulerMeasurementPlacement|Marker")
        AActor* GetRulerPlacementLocationMarker() const noexcept;

        /**
        * @brief Returns the player controller associated with the current ruler measurement.
        *
        * This method gets the APlayerController object associated with the world in which the measurement is being performed.
        *
        * @return A pointer to an APlayerController object representing the player controller. If the controller is not found, returns nullptr.
        */
        UFUNCTION(BlueprintCallable)
        APlayerController* GetPlayerController() const;

        /**
        * @brief Creates a ruler with the given parameters.
        *
        * @param Guid A unique identifier for the ruler.
        * @param bCreateRandomGuid Specifies whether a random unique identifier should be generated.
        * @param bShowRulerArea Specifies whether the ruler area should be shown. Defaults to false.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void CreateRuler(const FString& Guid, bool bCreateRandomGuid, bool bShowRulerArea = false);

        /**
        * @brief Creates a point for the ruler.
        *
        * This method is used to add a new point on the ruler. The implementation is done in Blueprint.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void CreateRulerPoint();

        /**
        * @brief Creates a new ruler point at the specified location.
        * @param Location The location at which to create the new ruler point.
        * @param Tolerance The tolerance for determining the proximity of points to close the polygon.
        */
        void CreateRulerPointAtLocation(const FVector& Location, const TOptional<float>& Tolerance = NullOpt);

        /**
        * @brief Ends the ruler process.
        *
        * This method is called to indicate the end of ruler measurements.
        * Can be implemented in Blueprint to perform additional actions.
        */
        void RulerEnd();

        /**
        * @brief Saves the state of the current ruler.
        *
        * This method is intended to save the current ruler information.
        * The implementation must be provided on the Blueprints side.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void SaveCurrentRuler();

        /**
        * @brief Updates the ruler placement based on the spline.
        *
        * This method is called to update the position of the ruler associated with the spline,
        * providing the ability to be implemented in Blueprints.
        */
        UFUNCTION(Category = "RulerMeasurement")
        void UpdateRulerPlacementLocationSpline();

        /**
        * @brief Updates the visibility and position of the ruler placement marker.
        *
        * Adjusts the marker's visibility and updates its position and orientation based on the projection from the screen to the world, taking into account the given tolerances.
        *
        * @param bVisible Determines whether the marker should be visible (true) or hidden (false).
        * @param DistanceTolerance Maximum distance tolerance allowed for updating marker location.
        */
        UFUNCTION(BlueprintCallable, Category = "ActorPlacement|Marker")
        void UpdateRulerPlacementLocationMarker(bool bVisible, double DistanceTolerance = 10) noexcept;

        /**
        * @brief Generates points between the given vectors in 2D space.
        *
        * This method creates an array of points between successive vectors from the input array based on the specified step.
        * Additional points are generated along the line connecting each vector, taking into account the step length.
        *
        * @param Points An array of vectors (FVector2D) for which to generate intermediate points.
        * @param Step The distance between the generated points.
        * @return An array (TArray<FVector2D>) of the generated points, including the endpoints of the original array.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        TArray<FVector2D> GeneratePointsBetweenVectors2D(const TArray<FVector2D>& Points, float Step);

        /**
        * @brief Returns information about the ruler being created.
        * @return Information about the ruler being created.
        */
        const FRulerInfo& GetCurrentInfo() const UE_LIFETIMEBOUND;

        /**
        * @brief Highlights the ruler in a different color.
        * @param RulerController The controller that owns the highlighted ruler.
        */
        void HighlightRuler(ARulerController* RulerController) const;

        /**
        * @brief Generates additional points between vectors in 3D space.
        *
        * The method creates evenly distributed points between adjacent vectors,
        * based on the specified step. If the input array contains less than two elements
        * or the step is less than or equal to zero, the method returns the input array unchanged.
        *
        * @param InputPoints An array of source vectors between which to pad points.
        * @param Step The step for generating points. Must be positive.
        * @return A new array of vectors containing the original points and the generated points between them.
        */
        UFUNCTION(BlueprintPure)
        static TArray<FVector> GeneratePointsBetweenVectors3D(const TArray<FVector>& InputPoints, float Step);

        /**
        * @brief Checks if a polygon can be created.
        *
        * This method determines if a polygon can be created based on the given vertices and distance tolerance limit.
        *
        * @param Vertices An array of vertices that define the intended polygon.
        * @param DistanceTolerance The distance tolerance between the first and last vertices to close the polygon.
        * @return true if the polygon can be created; false otherwise.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        bool CanCreatePolygon(const TArray<FVector>& Vertices, float DistanceTolerance);

        /**
        * @brief Calculates the area of ​​a polygon defined by a set of vertices.
        *
        * This method determines the area of ​​a polygon with the given vertices, ensuring that a polygon can be created based on the specified distance tolerance. The result is calculated in square meters.
        *
        * @param Vertices An array of FVector structures representing the vertices of the polygon.
        * @param DistanceTolerance The distance tolerance used to check whether the polygon can be created.
        * @return The calculated area of ​​the polygon in square meters. Returns 0.0f if the polygon cannot be created.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        float CalculatePolygonArea(const TArray<FVector>& Vertices, float DistanceTolerance);

        /**
        * @brief Gets information about all rulers.
        *
        * This method returns an array of FRulerInfo objects containing information about
        * each ruler associated with the current ruler controllers.
        *
        * @return An array of FRulerInfo objects containing data about each ruler.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        TArray<FRulerInfo> GetRulersInfo() const;

        /**
        * @brief Destroys the ruler actor with the specified GUID.
        *
        * This method destroys the ruler actor with the provided GUID. The ruler actor is identified by its unique GUID.
        * If the ruler actor with the specified GUID does not exist, no action is performed.
        *
        * @param Guid The unique identifier (GUID) of the ruler actor to destroy.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void DestroyRuler(const FString& Guid);

        /**
        * @brief Destroys all existing rulers.
        *
        * This method removes all ruler objects managed by the controllers by calling their destroy method.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void DestroyAllRulers();

        /**
        * @brief Returns the ruler controller associated with the specified GUID.
        *
        * This method searches through all available ruler controllers and returns the one
        * that matches the provided GUID. If no matching controller is found, returns nullptr.
        *
        * @param Guid The unique identifier of the ruler controller to get.
        * @return A pointer to the ARulerController that matches the provided GUID, or nullptr if no match is found.
        */
        UFUNCTION(BlueprintPure, Category = "RulerMeasurement")
        ARulerController* GetRulerControllerByGuid(const FString& Guid) const;

        /**
        * @brief Returns all ARulerController instances present in the current world.
        *
        * This method loops through the current game world and collects references to all active ARulerController objects.
        * Useful for accessing or manipulating multiple ruler controllers at once.
        *
        * @return An array of ARulerController pointers representing all the ruler controllers found in the world.
        */
        UFUNCTION(BlueprintPure, Category = "RulerMeasurement")
        const TArray<ARulerController*>& GetAllRulerControllers() const;

        /**
        * @brief Calculates the center point of a polygon based on its vertices.
        *
        * This method takes an array of vertices representing a polygon and calculates the center point of the polygon.
        * If the array is empty, the method returns FVector::ZeroVector.
        *
        * @return The calculated center point of the polygon.
        */
        UFUNCTION(BlueprintPure, Category = "RulerMeasurement")
        static FVector CalculateCenterPoint(const TArray<FVector>& Points);

        /**
        * @brief Renders the rulers based on the information provided.
        *
        * This method allows you to create a visual representation of the ruler using data from the FRulerInfo object.
        *
        * @param InRulerInfo An object containing the information for rendering the ruler.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void VisualizeRulerByInfo(const FRulerInfo& InRulerInfo);

        /**
        * @brief Exits the ruler.
        *
        * This method exits the current use of the ruler, saves the data,
        * deletes the existing rulers, and switches to the non-ruler state.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void ExitRuler();

        /**
        * @brief Sets the length unit type for the ruler.
        *
        * This method changes the length unit type for a ruler identified by a unique identifier.
        *
        * @param RulerGuid The unique identifier for the ruler.
        * @param LengthUnitType The new length unit type.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void SetRulerLengthUnitType(const FString& RulerGuid, ERulerUnitType LengthUnitType);

        /**
        * @brief Sets the area unit type for the ruler.
        *
        * This method updates the area unit type used by the ruler identified by the specified GUID.
        *
        * @param RulerGuid The unique identifier for the ruler.
        * @param AreaUnitType The area unit.
        */
        UFUNCTION(BlueprintCallable, Category = "RulerMeasurement")
        void SetRulerAreaUnitType(const FString& RulerGuid, ERulerUnitType AreaUnitType) const;

        /**
        * @brief Determines the length unit type of the ruler.
        *
        * This method returns the appropriate length unit type based on the input string.
        * If the specified type is not found, the default value of Meter is used.
        *
        * @param LengthUnitType The string representation of the length unit type (e.g. "meter", "kilometer").
        * @return The length unit type corresponding to the input string.
        * If the input string does not match any type, Meter is returned.
        */
        static ERulerUnitType FindRulerLengthUnitType(const FString& LengthUnitType);

        /**
        * @brief Determines the ruler area unit type.
        *
        * This method returns the appropriate ruler area unit type based on the provided string.
        * If the string does not match any known types, the default value (SquareMeter) is returned.
        *
        * @param AreaUnitType The area unit type name as a string.
        * @return ERulerUnitType corresponding to the specified string.
        */
        static ERulerUnitType FindRulerAreaUnitType(const FString& AreaUnitType);

        /**
        * @brief Gets the current camera position.
        *
        * This method retrieves the camera location and writes it to the FVector object passed in as an argument.
        *
        * @param OutCameraLocation A reference to the FVector object that will receive the current camera location.
        * @return A Boolean indicating whether the operation was successful. Returns true if the camera position was retrieved successfully; otherwise, false.
        */
        UFUNCTION(BlueprintPure)
        bool GetCameraLocation(FVector& OutCameraLocation) const;

        /**
        * @brief Formats a number with spaces using the given parameters.
        *
        * This method converts a number to a string, applying formatting that includes using spaces as thousand separators and replacing the decimal separator.
        *
        * @param Number The number to format.
        * @return The formatted string representation of the number.
        */
        UFUNCTION(BlueprintPure)
        static FString FormatUnitsWithSpaces(float Number);

        /**
        * @brief Projects the world position into screen space, taking into account visibility behind the camera.
        *
        * This method converts the world position to screen coordinates and determines whether the object is visible from behind the camera.
        *
        * @param PlayerController Pointer to the player controller used to get the view information.
        * @param WorldPosition The 3D world position to transform.
        * @param ScreenPosition A 2D vector to write the resulting screen coordinates to.
        * @param bTargetBehindCamera A Boolean value indicating whether the target is behind the camera.
        * @param bPlayerViewportRelative Determines whether the coordinates should be relative to the player viewport.
        * @return Returns true if the transformation succeeded, false on error.
        */
        UFUNCTION(BlueprintPure)
        static bool ProjectWorldToScreenBidirectional(const APlayerController* PlayerController,
                                                      const FVector& WorldPosition,
                                                      FVector2D& ScreenPosition,
                                                      bool& bTargetBehindCamera,
                                                      bool bPlayerViewportRelative);

        /**
        * @brief Adjusts the points to align with the specified ellipsoid.
        *
        * This method changes the positions of the specified 3D points to align with the specified ellipsoid,
        * subject to specified gaps and offsets.
        *
        * @param PointsBetweenVectors3D An array of 3D points representing the intermediate points between vector segments.
        * @param OriginalPoints An array of original 3D points defining the segments to adjust.
        * @param bGaps Specifies whether to take into account gaps between adjusted segments.
        * @return A new array of 3D points adjusted with respect to the ellipsoid.
        */
        UFUNCTION(BlueprintPure)
        TArray<FVector> AdjustPointsRelativeToEllipsoid(const TArray<FVector>& PointsBetweenVectors3D, const TArray<FVector>& OriginalPoints, bool bGaps) const;

        /**
        * @brief Projects a point onto the surface of the ellipsoid.
        *
        * This method calculates the closest point on the surface of the geographic ellipsoid to the specified point.
        *
        * @param Point The point in space to project onto the ellipsoid.
        * @param Ellipsoid The geographic ellipsoid to project onto.
        * @return The point on the surface of the ellipsoid that is closest to the specified point.
        */
        UFUNCTION(BlueprintPure)
        static FVector ProjectPointOntoEllipsoid(const FVector& Point, const FGeographicalEllipsoid& Ellipsoid);

        /**
        * @brief Generates points and corrects them relative to the ellipsoid.
        *
        * This method creates intermediate points between the given vectors, and then
        * corrects their locations relative to the ellipsoid using the given parameters.
        *
        * @param Points An array of vectors defining the original points.
        * @param Step A step that specifies the interval between generated points.
        * @param Gaps A flag indicating whether to consider gaps in point processing.
        *
        * @return Returns an array of ellipsoid-corrected points.
        */
        UFUNCTION(BlueprintPure)
        TArray<FVector> GeneratePointsAndRelativeToEllipsoid(const TArray<FVector>& Points, float Step, bool Gaps);

        /**
        * @brief Generates fill lines within the specified polygon.
        *
        * This method creates an array of points representing the horizontal fill lines within the bounds of the
        * passed polygon. The lines are spaced at a certain step along the Y axis.
        *
        * @param PolygonPoints An array of points representing the polygon's vertices.
        * The polygon must be closed.
        * @param Step The step between horizontal fill lines.
        *
        * @return Returns an array of points representing the fill lines within the polygon.
        * If the polygon contains fewer than three points, an empty array is returned.
        */
        UFUNCTION(BlueprintPure)
        static TArray<FVector2D> GenerateFillLines(const TArray<FVector2D>& PolygonPoints, float Step);

        /**
        * @brief Calculates the total distance between points.
        *
        * This method sums the distances between successive points in the given array.
        *
        * @param Points An array of points (FVector) used to calculate the distance.
        * @return The total distance between points, represented as a floating point number.
        */
        UFUNCTION(BlueprintPure)
        static float CalculateTotalDistance(const TArray<FVector>& Points);

        /**
        * @brief Returns the step for segments.
        *
        * This method provides the current step value used for segmentation in the ARulerMeasurement object.
        *
        * @return The step value for segments.
        */
        UFUNCTION(BlueprintPure)
        float GetStepForSegments() const;

        /**
        * @brief Shifts the vector by the given step in the direction of its normalized magnitude.
        *
        * This method calculates the direction of the input vector, normalizes it, and shifts it by the given step.
        * If the input vector is close to zero, it is returned unchanged.
        *
        * @param InputVector The input vector to be shifted.
        * @param Step The amount by which to shift the vector.
        * @return The new vector shifted by the given step.
        */
        UFUNCTION(BlueprintPure)
        static FVector2D OffsetVectorByStep(const FVector2D& InputVector, float Step);

        /**
        * @brief Returns the class of the ruler popup widget.
        * @return The popup widget class of type TSubclassOf<URulerPopupWidget>.
        */
        UFUNCTION(BlueprintPure)
        TSubclassOf<URulerPopupWidget> GetPopupWidgetClass();

        /**
        * @brief Returns the popup class for the time ruler.
        * @return The class of the time ruler popup.
        */
        UFUNCTION(BlueprintPure)
        TSubclassOf<UPromptRulerPopupWidget> GetPromptRulerPopupClass() const;

        /**
        * @brief Returns the class for the push button widget on a ruler dot.
        * @return The class for the push button widget on a ruler dot.
        */
        UFUNCTION(BlueprintPure)
        TSubclassOf<URulerPointButton> GetRulerPointButtonWidgetClass() const;

        /**
        * @brief Checks if two vectors are close to each other on screen, given a distance tolerance.
        *
        * This function calculates the screen positions of two 3D vectors, projects them onto the 2D viewport
        * and determines if the distance between them is within the given tolerance.
        *
        * @param FirstVector The first 3D vector to compare.
        * @param SecondVector The second 3D vector to compare.
        * @param DistanceTolerance The maximum distance allowed between vectors in screen space.
        * @return true if the distance between the vectors in screen space is less than the tolerance, false otherwise.
        */
        bool AreVectorsCloseOnViewport(const FVector& FirstVector, const FVector& SecondVector, float DistanceTolerance) const;

        /**
        * @brief Creates and initializes a ruler point widget at the specified location.
        *
        * This method creates a new ruler point widget, adds it to the viewport,
        * and sets its parent position to the specified coordinates.
        * If the widget cannot be created, nullptr is returned.
        *
        * @param Location The position in the world where the ruler point widget will be created.
        * @return A pointer to the created URulerPoint widget, or nullptr if widget creation fails.
        */
        URulerPoint* CreateRulerPointWidget(const FVector& Location);

        /**
        * @brief Creates and initializes a ruler point widget at the specified location.
        *
        * This method creates a new line point widget, adds it to the viewport
        * and sets its parent position to the given coordinates.
        * If the widget cannot be created, nullptr is returned.
        *
        * @return A pointer to the created URulerPoint widget, or nullptr if widget creation failed.
        */
        UFUNCTION(BlueprintCallable)
        bool LineTraceFromPlayerScreen(const APlayerController* PC,
                                       FHitResult& OutHitResult,
                                       const FVector2D& ScreenLocation,
                                       ECollisionChannel CollisionChannel,
                                       double TraceDistance);

        /**
        * @brief Sets the player controller for
        * @param Controller The current player controller.
        */
        void SetController(APlayerController* Controller);

        /**
        * @brief Returns the distance tolerance for the region polygon.
        * @return The distance tolerance for the region polygon.
        */
        double GetDistanceToleranceForAreaPolygon() const;

private:
        /**
        * @brief Checks if the controller for the current identifier is available.
        * @return The availability of the controller.
        */
        bool IsControllerAvailable() const;

        /**
        * @brief Creates a ruler controller with the given information.
        * @param Info The information about the ruler to create.
        */
        void CreateRulerController(const FRulerInfo& Info, bool ShowRulerArea);

        /**
        * @brief Updates the visualization with the available data.
        */
        UFUNCTION(BlueprintCallable)
        void UpdateRuler();

        /**
        * @brief Removes all invalid controllers.
        */
        void RemoveInvalidControllers();

protected:
        UPROPERTY(VisibleAnywhere, Category = "RulerMeasurementPlacement|Marker")
        TObjectPtr<AActor> RulerPlacementLocationMarker;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurementPlacement|Marker")
        TSubclassOf<AActor> RulerPlacementLocationMarkerClass;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RulerMeasurement|ActorPlacement")
        double DistanceToleranceForAreaPolygon = 1.f;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|ActorPlacement")
        double MinActorPlacementAngleDeviation = PI / 8;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|ActorPlacement")
        TSubclassOf<AActor> RulerClass;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|ActorPlacement")
        TSubclassOf<ARulerController> RulerControllerClass;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruler")
        TArray<URulerPoint*> RulerPointWidgets;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ruler")
        TSubclassOf<URulerLines> RulerLinesWidgetClass;

        UPROPERTY(VisibleAnywhere, Category = "RulerMeasurement|ActorPlacement")
        TArray<ARulerController*> RulerControllers;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|Widgets")
        TSubclassOf<URulerSplineWidget> SplineWidgetClass;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|Widgets")
        TSubclassOf<URulerPopupWidget> PopupWidgetClass;

        UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ruler")
        TObjectPtr<URulerLines> RulerLinesWidget;

        UPROPERTY(VisibleAnywhere, Category = "RulerMeasurement|Widgets")
        TObjectPtr<URulerSplineWidget> SplineWidget;

        UPROPERTY(VisibleAnywhere, Category = "RulerMeasurement|Widgets")
        TObjectPtr<URulerPopupWidget> PopupWidget;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|Highlight")
        FLinearColor RulerStandardColor = FColor::FromHex("FFC149FF");

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement|Highlight")
        FLinearColor RulerHighlightColor = FColor::FromHex("FF2600FF");

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RulerMeasurement")
        ERulerMode RulerMode = ERulerMode::None;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RulerMeasurement")
        FVector PreviousPoint = FVector::ZeroVector;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement")
        TSubclassOf<URulerPoint> RulerPointWidgetClass;

        UPROPERTY(EditDefaultsOnly, Category = "RulerMeasurement")
        TSubclassOf<URulerPointButton> RulerPointButtonWidgetClass;

private:
        UPROPERTY(EditDefaultsOnly)
        TSubclassOf<UPromptRulerPopupWidget> PromptRulerPopupClass;

        UPROPERTY()
        TWeakObjectPtr<APlayerController> CurrentController;

        double TotalDistance = 0.;
        double SplineDistance = 0.;
        float StepForSegments = 10000000.0;
        float StepForSpline = 15.f;
        bool bShowArea = false;
        int ZOrderRulerPoint = 400; // required value for correct priority of ruler points display

        FRulerInfo RulerInfo;
        TArray<FVector> RulerPoints;
};
