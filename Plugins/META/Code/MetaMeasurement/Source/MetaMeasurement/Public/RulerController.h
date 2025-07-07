// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "RulerPoint.h"
#include "GameFramework/Actor.h"
#include "RulerController.generated.h"

class URulerLines;
class URulerPopupWidget;
class URulerSelection;
class ARulerMeasurement;

USTRUCT(BlueprintType)
struct FRulerInfo
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Guid{};

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Length = 0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Name{};

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FVector> Positions;

        /**
          * @brief Clears the data in the structure.
          */
        void Reset();
};

UENUM(BlueprintType)
enum class ERulerUnitType : uint8
{
        Meter,
        Kilometer,
        SquareKilometer,
        SquareMeter,
        Hectare,
        HundredPart
};

DECLARE_MULTICAST_DELEGATE_OneParam(FRulerPointsDragged, TArray<FVector>);
DECLARE_MULTICAST_DELEGATE(FRulerPointsDropped);
DECLARE_MULTICAST_DELEGATE(FRulerPointButtonPressed);
DECLARE_MULTICAST_DELEGATE(FRulerPointButtonUnpressed);

DECLARE_DELEGATE_OneParam(FSendRulerInfo, const FRulerInfo&);
DECLARE_DELEGATE_OneParam(FRulerSaved, const FRulerInfo&);

UCLASS()
class METAMEASUREMENT_API ARulerController : public AActor
{
        GENERATED_BODY()

public:
        FRulerPointButtonPressed RulerPointButtonPressed;

        FRulerPointButtonUnpressed RulerPointButtonUnpressed;

        FRulerPointsDragged RulerPointsDragged;

        FRulerPointsDropped RulerPointsDropped;

        FSendRulerInfo SendRulerInfoDelegate;

        FRulerSaved RulerSavedDelegate;

        /**
        * @brief Initializes the ruler controller.
        */
        void Initialization();

        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

        /**
        * Returns the ruler information associated with this RulerController instance.
        *
        * @return A FRulerInfo structure containing information about the ruler,
        * including its GUID, length, name, and positions.
        */
        FRulerInfo GetRulerInfo() const;

        /**
        * @brief Sets the visibility of the controlled ruler.
        * @param bIsVisible The visibility of the ruler.
        */
        void SetRulerVisibilty(bool bIsVisible);

        /**
        * @brief Toggles the locking of ruler dots.
        * @param bIsLocked The dots lock flag.
        */
        void SetLock(bool bIsLocked) const;

        /**
        * Updates the state or properties of the ruler. Intended to be overridden by a Blueprint implementation.
        * This method is called to reflect or propagate changes to the ruler instance.
        */
        void UpdateRuler(bool bSendRpcNewRulerInfo = true);

        /**
        * Changes the color of the ruler. Intended to be implemented in Blueprints.
        *
        * @param NewColor The new color to apply to the ruler, represented as an FLinearColor.
        */
        void ChangeRulerColor(FLinearColor NewColor);

        /**
        * Returns the unit type used to measure the length of the ruler.
        *
        * @return An ERulerUnitType enumeration value representing the current
        * unit type for measuring the length of the ruler.
         */
        UFUNCTION(BlueprintPure)
        ERulerUnitType GetRulerLengtUnitType() const;

        /**
        * Returns the unit type used to measure the area of ​​the ruler.
        *
        * @return An ERulerUnitType enumeration value representing the current unit type for measuring the area.
        */
        UFUNCTION(BlueprintPure)
        ERulerUnitType GetRulerAreaUnitType() const;

        /**
        * Updates the ruler information for this RulerController instance.
        *
        * @param NewRulerInfo The new ruler information to be set, containing details such as GUID,
        * length, name, and positions.
        */
        void SetRulerInfo(FRulerInfo NewRulerInfo);

        /**
        * Sets the unit type used to measure the length of the ruler. This will update the internal state of the
        * ruler to use the specified length unit type and update its properties accordingly.
        *
        * @param LengthUnitType The new length unit type to set for the ruler. Expected values ​​are taken from the
        * ERulerUnitType enumeration.
        */
        void SetLengthUnitType(ERulerUnitType LengthUnitType);

        /**
        * Sets the unit type used to measure area in this RulerController instance.
        * If the specified unit type is different from the current one, the ruler's area unit type will be updated,
        * and the ruler's properties will be updated accordingly.
        *
        * @param AreaUnitType The new unit type to use to measure area.
        * Expected values ​​are taken from the ERulerUnitType enumeration.
        */
        void SetAreaUnitType(ERulerUnitType AreaUnitType);

        /**
        * @brief Returns an array of ruler point actors.
        * @return An array of ruler point actors.
        */
        const TArray<URulerPoint*>& GetRulerPoints() const UE_LIFETIMEBOUND;

        /**
        * @brief Checks if the polygon can be created and creates it if possible.
        */
        void CheckAndCreatePolygon();

        /**
        * @brief Updates the current ruler position data and its associated points.
        *
        * This function checks the validity of the ruler widget and updates the ruler positions
        * using valid point widgets. It ensures that the correct closed loop of points is generated if necessary, and recalculates the measurements relative to the
        * ellipsoid for the updated visual representation.
        */
        void UpdateRulerPosition();

        /**
        * @brief Updates the ruler distance and area measurements.
        *
        * This method calculates and sets the total length based on the current set of points.
        * If the ruler is set to display area or is in polygon mode,
        * it also calculates and updates the polygon area.
        */
        void UpdateRulerDistanceAndArea();

        /**
        * @brief Updates the ruler UI elements, checking for the correctness of the required widget before setting the updated information.
        */
        void UpdateRulerUI();

        /**
        * @brief Creates a polygon selection based on the points defined on the ellipsoid.
        *
        * This method initializes the polygon selection UI widget if the
        * number of points on the ellipsoid is sufficient. The widget is created from the specified class,
        * configured with the polygon points, and added to the viewport.
        *
        * Note: the widget will not be created if there are less than three points on the ellipsoid.
        */
        void CreatePolygonSelection();

        /**
        * @brief Gets the current ruler measurement instance.
        * @return A pointer to an ARulerMeasurement object. Returns a valid ARulerMeasurement instance if found, nullptr otherwise.
        */
        ARulerMeasurement* GetRulerMeasurement();

        /**
        * @brief Updates the ruler selection based on the current settings.
        *
        * This method checks the state of the ruler and updates the RulerSelection object
        * accordingly. If necessary, creates a new polygon selection object
        * or updates an existing one with the current polygon points.
        * If the current selection is invalid or the number of points is insufficient,
        * it removes the selection and cleans up the associated resources.
        */
        void UpdateRulerSelection();

        /**
        * @brief Sets the visibility of the ruler region.
        * @param NewShowRulerArea Specifies whether the ruler area should be shown (true) or hidden (false).
        */
        void SetShowRulerArea(bool NewShowRulerArea);

        /**
        * @brief Transforms and gets the ruler information in the ECEF coordinate system.
        *
        * @param RulerInfo A structure containing the ruler information to transform.
        * @return The transformed ruler information in the ECEF coordinate system.
        */

        FRulerInfo GetRulerInfoInEcef(FRulerInfo Info);

protected:
        ERulerUnitType RulerLengthUnitType;

        ERulerUnitType RulerAreaUnitType;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
        FRulerInfo RulerInfo;

        TArray<URulerPoint*> RulerPointWidgets;

        UPROPERTY(VisibleInstanceOnly, Category = "Ruler|Widgets")
        TObjectPtr<URulerSelection> RulerSelection = nullptr;

        UPROPERTY(EditDefaultsOnly, Category = "Ruler|Widgets")
        TSubclassOf<URulerSelection> RulerSelectionWidgetClass;

        UPROPERTY(VisibleInstanceOnly, Category = "Ruler|Widgets")
        TObjectPtr<URulerPopupWidget> RulerPopupWidget;

        UPROPERTY(EditDefaultsOnly, Category = "Ruler|Widgets")
        TSubclassOf<URulerPopupWidget> RulerPopupWidgetClass;

        bool bShowArea = false;

        bool bRulerIsPolygon = false;

        UPROPERTY(VisibleInstanceOnly, Category = "Ruler|Widgets")
        TObjectPtr<URulerLines> RulerLinesWidget;

        UPROPERTY(EditDefaultsOnly, Category = "Ruler|Widgets")
        TSubclassOf<URulerLines> RulerLinesWidgetClass;

        UPROPERTY(EditDefaultsOnly, Category = "Ruler|Widgets")
        TSubclassOf<URulerPoint> RulerPointWidgetClass;

        TArray<FVector> PointsAtEllipsoid;

        UPROPERTY(VisibleInstanceOnly, Category = "Ruler|Widgets")
        TObjectPtr<ARulerMeasurement> RulerMeasurement;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ruler")
        double PolygonArea;

        float RulerSelectionWidgetZOrder = -200;

        float RulerLinesWidgetZOrder = -100;

        float RulerPopupWidgetZOrder = 300;
};
