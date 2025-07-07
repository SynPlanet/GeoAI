#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SynCartographicTransformationActorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSynOnCartographicDragEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSynOnCartographicZoomEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSynOnCartographicOrbitEnd);

DECLARE_LOG_CATEGORY_EXTERN(LogSynCartographicTransformationActorComponent, All, Log);

UCLASS(ClassGroup = (Custom), Meta = (BlueprintSpawnableComponent))
class GEOAI_API USynCartographicTransformationActorComponent : public UActorComponent
{
        GENERATED_BODY()

protected:
        TOptional<FVector> CartographicPivot;
        TOptional<FRotator> PreviousControlRotation;
        TOptional<FPlane> ProjectiveCartographicDragSurfacePlane;
        TOptional<FRotator> CartographicOrbitPreviousAngularVelocity;
        TOptional<double> CartographicZoomPreviousDeltaScale;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls")
        double CartographicPivotTraceDistance = 10000000000.0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        double CartographicDragPivotDistanceMultiplier = 25.0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        double CartographicDragInterpSpeed = 10.0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        double CartographicDragMaxPivotDistance = 2500000000.0;

        UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "CartographicControls|Drag")
        bool bCartographicDragLocked = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        double CartographicDragMaxRelativeDistanceToPivot = 6.0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        double CartographicOrthogonalDragRelativeDistanceScale = 100000.0;

        UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "CartographicControls|Drag")
        FVector PreviousDesiredPivotDestination = FVector::ZeroVector;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        FVector LastCartographicDragOrthogonalOffset = FVector::ZeroVector;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        float CartographicDragMaxDeltaSeconds = 0.05;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        float CartographicOrthogonalDragMaxDeltaSeconds = 0.02;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        double CartographicDragOrthogonalPullMultiplier = 4.0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        double ProjectiveOrthogonalCartographicDragAngleCosine = 0.1;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Drag")
        double CartographicZoomScaleMultiplier = 1.5;

        UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "CartographicControls|Drag")
        bool bPerformOrthogonalDrag = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CartographicControls|Common")
        float InterpSpeed = 10.f;

        // UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (InstanceEditable), Category = "CartographicControls|Common")
        // bool bSmoothRotation = false;

public:
        USynCartographicTransformationActorComponent();

        virtual void BeginPlay() override;
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

        UPROPERTY(BlueprintAssignable, Category = "CartographicControls|Drag")
        FSynOnCartographicDragEnd OnCartographicDragEnd;

        UPROPERTY(BlueprintAssignable, Category = "CartographicControls|Zoom")
        FSynOnCartographicZoomEnd OnCartographicZoomEnd;

        UPROPERTY(BlueprintAssignable, Category = "CartographicControls|Orbit")
        FSynOnCartographicOrbitEnd OnCartographicOrbitEnd;

        bool InitializeCartographicPivot();
        void ResetCartographicPivot();

        void ApplyCartographicDrag(float DeltaSeconds, bool bForward = true);

        UFUNCTION()
        void SetCartographicDragLocked(bool bLocked);

        UFUNCTION()
        void ApplyCartographicOrbit(FRotator const& DesiredAngularVelocity);

        UFUNCTION()
        void ApplyCartographicZoom(double DesiredDeltaScale);

        UFUNCTION()
        void ApplyCartographic2xZoom();

        UFUNCTION(BlueprintCallable, Category = "CartographicControls|Drag")
        void ResetTargetRotation();

private:
        FRotator CurrentRotation = FRotator::ZeroRotator;
        FRotator TargetRotation = FRotator::ZeroRotator;
};
