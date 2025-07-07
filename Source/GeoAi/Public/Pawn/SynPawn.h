#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SynPawn.generated.h"

struct FInputActionValue;
class USynCartographicTransformationActorComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class GEOAI_API ASynPawn : public APawn
{
        GENERATED_BODY()

protected:
        UPROPERTY(EditAnywhere, Category = "Input|Default")
        TObjectPtr<USynCartographicTransformationActorComponent> CartographicTransformationComponent;

        UPROPERTY(EditAnywhere, Category = "Input|Default")
        TObjectPtr<UInputMappingContext> CartographicMappingContext;

        UPROPERTY(EditAnywhere, Category = "Input|Default")
        TObjectPtr<UInputAction> CartographicDragPreconditionInputAction;

        UPROPERTY(EditAnywhere, Category = "Input|Default")
        TObjectPtr<UInputAction> CartographicDragInputAction;

        UPROPERTY(EditAnywhere, Category = "Input|Default")
        TObjectPtr<UInputAction> CartographicOrbitPreconditionInputAction;

        UPROPERTY(EditAnywhere, Category = "Input|Default")
        TObjectPtr<UInputAction> CartographicOrbitInputAction;

        UPROPERTY(EditAnywhere, Category = "Input|Default")
        TObjectPtr<UInputAction> CartographicZoomInputAction;

        TOptional<TPair<FVector2f, FVector2f>> FingerPositions;

        UPROPERTY(EditAnywhere, Category = "Input|Default")
        float CartographicOrbitVerticalRotationSensitivity = 0.05f;

        TOptional<bool> bZoomBlock = false;
        TWeakObjectPtr<AActor> InteractableActor = nullptr;

        // These two are better off as a single enum, but I don't have a lot of time to properly think about it, so I don't care.
        TOptional<bool> bCartographicOrbitVerticalRotationActive = false;
        TOptional<bool> bCartographicZoomActive = false;

        // Percentages of screen size
        UPROPERTY(EditAnywhere, Category = "Input|Default")
        float CartographicOrbitFingerOffsetTolerance = 1.f;

        UPROPERTY(EditAnywhere, Category = "Input|Default")
        float ToleranceFromViewportSize = 10.f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bIsForwardMoving = true;

        UFUNCTION()
        void User_CartographicDrag_Initialize();

        UFUNCTION()
        void User_CartographicDrag_Apply();

        UFUNCTION()
        void User_CartographicDrag_Reset();

        UFUNCTION()
        void User_CartographicOrbit_Initialize();

        UFUNCTION()
        void User_CartographicOrbit_Apply(FInputActionValue const& InputActionValue);

        UFUNCTION()
        void User_CartographicOrbit_Reset();

        UFUNCTION()
        void User_CartographicZoom_Initialize();

        UFUNCTION()
        void User_CartographicZoom_Apply(FInputActionValue const& InputActionValue);

        UFUNCTION()
        void User_CartographicZoom_Reset();

        UFUNCTION(BlueprintCallable)
        void ApplyCartographicZoom2x();

        TPair<FVector2f, FVector2f> RetrieveFingerPositions() const;

public:
        ASynPawn();

        virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

        FHitResult SweepAlongVector(FVector const& DeltaLocation, double Scale = 1.0);

        UFUNCTION(BlueprintCallable)
        void UpdateMovementAvailabilityState(bool bIsAvailable);
        void SendTouchScreenVisualizationMessage(APlayerController* PC);

private:
        bool bMovementAvailable = false;
};
