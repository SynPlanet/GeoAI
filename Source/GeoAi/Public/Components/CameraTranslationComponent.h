#pragma once

#include "CoreMinimal.h"
#include "GeographicalLocation.h"
#include "Components/ActorComponent.h"
#include "CameraTranslationComponent.generated.h"

/** Intended use for moving the player's camera to a given point.
* Can send notifications when approaching a target
*/
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GEOAI_API UCameraTranslationComponent : public UActorComponent
{
        GENERATED_BODY()

public:
        /** Default constructor*/
        UCameraTranslationComponent();

        /** Attempt to send notification about approaching checkpoint or about movement completion*/
        bool TrySendNotifications();

        /** Set a new target point for the camera, keeping the original rotation
        * @param NewTargetPosition points to the point where the camera should move
        */
        UFUNCTION(BlueprintCallable)
        void SetTargetLocation(const FVector& NewTargetPosition);

        /** Completely redefine the camera position and rotation based on the received data
        * @param NewTargetTransform data for offsetting and rotating the camera to the specified coordinates
        */
        UFUNCTION(BlueprintCallable)
        void SetTargetTransform(const FTransform& NewTargetTransform);

protected:
        /** Global function called every UEapp frame*/
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
        /** Send notification about whether the camera is approaching the target point
        *
        * @param bIsUpClose reflects the state that the camera has approached the frontend's preliminary notification distance
        */
        void SendNotification(const bool bIsUpClose) const;

        /** Try to start the shift and calculate the necessary parameters
        *
        * @return true if the shift was successfully started
        */
        bool TryEnableCameraTranslation();
        
protected:
        /** The destination point of the movement*/
        UPROPERTY(BlueprintReadOnly)
        FTransform TargetTransform = FTransform::Identity;

        UPROPERTY(BlueprintReadOnly)
        FTransform InitialTransform = FTransform::Identity;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraTranslation")
        FVector2D InRange = { 0.f, 12742000.f };

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraTranslation")
        FVector2D OutRangeDuration = { 2.f, 5.5f };

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraTranslation")
        FVector2D OutRangeElevation = { 0.1f, 1.f };
        
private:
        /** The identifier of the current movement operation for interaction with the frontend*/
        FGuid CurrentTranslationId{};

        /** Should be in WGS84*/
        FGeographicalLocation TargetGeoPosition{};

        /** Calculated value based on those specified in .ini. Used for distance calculations.*/
        double CalculatedDistanceTriggerSqr = 0.2;

        /** Calculated value based on those specified in .ini. Used for distance calculations.*/
        double CalculatedMinApproachNotificationThreshold = 500.0;

        /** Maximum distance squared value for distance calculations*/
        double MaxDistanceSqr = 0.f;

        // TODO: counter by checkpoints?
        /** Indicates whether the first message about approaching the target point has been sent*/
        bool bNotifySend = false;

        /** Object from which the current world position is tracked for calculating approaching the target*/
        TWeakObjectPtr<APawn> TargetPawn = nullptr;
        
        float CurrentDuration = 0.f;
        float TotalDuration = 0.f;
        float ElevationFactor = 0.f;
};