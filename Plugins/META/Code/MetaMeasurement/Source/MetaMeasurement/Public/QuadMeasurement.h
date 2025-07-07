// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuadMeasurement.generated.h"

USTRUCT(BlueprintType)
struct FQuadAreaInfo
{
        GENERATED_BODY()

        UPROPERTY(BlueprintReadOnly)
        TArray<FVector2D> ScreenPoints;

        UPROPERTY(BlueprintReadOnly)
        TArray<FVector> WorldPoints;
};

UCLASS()
class METAMEASUREMENT_API AQuadMeasurement : public AActor
{
        GENERATED_BODY()

public:
        // Sets default values for this actor's properties
        AQuadMeasurement();

        UFUNCTION(BlueprintNativeEvent)
        void UpdateSize(FVector2D OriginRealSize = FVector2D());

        UFUNCTION(BlueprintImplementableEvent)
        void OnScaleUpdated(float NewScale);

        UFUNCTION(BlueprintCallable, BlueprintPure)
        float GetCurrentNormalizedAreaSizeScale() const;

        UFUNCTION(BlueprintCallable)
        void UpdateMovementAvailabilityState(bool bIsAvailable);

        void UpdateLocationFromPlayerViewPoint();

        void ChangeScale(float NewScale);

        FQuadAreaInfo GetMeasurementInfo();

protected:
        // Called when the game starts or when spawned
        virtual void BeginPlay() override;
        void InterpActorScale(float DeltaTime);
        virtual void Tick(float DeltaTime) override;

        bool UpdateLocationFromPlayerView(const FCollisionQueryParams& NewParams, const FVector& WorldLocation, const FVector& WorldDirection);

protected:
        UPROPERTY(EditDefaultsOnly)
        bool bUpdateSizeOnBeginPlay = true;

        // Current screen size usage percentage
        UPROPERTY(EditDefaultsOnly)
        float UsableScreenArea = 0.75f;

        UPROPERTY(EditDefaultsOnly)
        bool bDrawDebug = false;

        float TargetScale = 1.0f;
        float CurrentScale = 1.0f;

        UPROPERTY(EditDefaultsOnly, meta = (ForceUnits = "Kilometers"))
        FVector2D SizeLimit{ 20.f, 100.f };

        FVector2D CalculatedLimit{ 1.f, 20.f };

        UPROPERTY(EditDefaultsOnly)
        float ScalingSpeed = 10.f;

        UPROPERTY(EditDefaultsOnly)
        bool bMoveAlongPlayerViewpoint = false;

private:
        UPROPERTY()
        TWeakObjectPtr<APlayerController> CurrentController;
        FQuadAreaInfo QuadAreaInfo;

        bool bMoveAvailable = false;
};
