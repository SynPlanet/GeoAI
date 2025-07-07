// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vehicle/PlaneTrack.h"

#include "FlightManager.generated.h"

class ACesiumGeoreference;

USTRUCT(BlueprintType)
struct FFlightPathInfo
{
        GENERATED_BODY()

        UPROPERTY(BlueprintReadOnly)
        TObjectPtr<USplineComponent> FlightPath;

        UPROPERTY(BlueprintReadOnly)
        TArray<FSpeedDistance> Speeds;

        UPROPERTY(BlueprintReadOnly)
        TWeakObjectPtr<AActor> CurrentAircraft;
};

USTRUCT(BlueprintType)
struct FAircraftMovingInfo
{
        GENERATED_BODY()

        UPROPERTY(BlueprintReadOnly)
        float StartTime;

        UPROPERTY(BlueprintReadOnly)
        float Progress;

        bool WaitNextFlight = true;

        int32 CurrentIndex = 1;

        double Speed = 1;
};

USTRUCT(BlueprintType)
struct FFlightPlan
{
        GENERATED_USTRUCT_BODY()

        UPROPERTY(EditAnywhere)
        TSoftObjectPtr<UDataTable> FlightData = nullptr;

        UPROPERTY(EditAnywhere)
        TSoftClassPtr<AActor> Aircraft = nullptr;

        UPROPERTY(EditAnywhere)
        double DelayBeforeStartFlight = 0.0;

        UPROPERTY(EditAnywhere)
        double DelayBetweenFlights = 10.0;

        UPROPERTY(BlueprintReadOnly)
        FFlightPathInfo GeneratedPathInfo;

        UPROPERTY(BlueprintReadOnly)
        FAircraftMovingInfo MovingInfo;
};

UCLASS(Blueprintable, HideCategories = (Rendering, Collision, Input, Replication, Actor, LOD, Cooking))
class GEOAI_API AFlightManager : public AActor
{
        GENERATED_BODY()

public:
        // Sets default values for this actor's properties
        AFlightManager();

        void ClearConstructedFlightPaths();
        void ConstructFlightPaths();

protected:
        // Called when the game starts or when spawned
        virtual void BeginPlay() override;

        /** Use only tables based on FAircraftRawData
         * 
         */
        void CreateSplineForFlightPlan(FFlightPlan& ModifiedPlan);

        void ParsePosition(const FString& Position, double& OutLatitude, double& OutLongitude);

public:
        // Called every frame
        virtual void Tick(float DeltaTime) override;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FFlightPlan> FlightPlans;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bEnableDebug = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Speed = 0.1;

        UPROPERTY(EditAnywhere)
        TArray<TSoftClassPtr<AActor>> Aircrafts;

private:
        TObjectPtr<ACesiumGeoreference> CesiumGeoreference;
};
