// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Engine/DataTable.h"
#include "PlaneTrack.generated.h"

class ACesiumGeoreference;

USTRUCT(BlueprintType)
struct FAircraftRawData : public FTableRowBase
{
        GENERATED_USTRUCT_BODY()

public:
        FAircraftRawData() : UTC(""), Callsign(""), Position(""), Altitude(0.0), Speed(0.0), Direction(0.0) {}
        UPROPERTY(EditAnywhere, Category = "FlightTracker")
        FString UTC{};
        UPROPERTY(EditAnywhere, Category = "FlightTracker")
        FString Callsign{};
        UPROPERTY(EditAnywhere, Category = "FlightTracker")
        FString Position{};
        UPROPERTY(EditAnywhere, Category = "FlightTracker")
        double Altitude{};
        UPROPERTY(EditAnywhere, Category = "FlightTracker")
        double Speed{};
        UPROPERTY(EditAnywhere, Category = "FlightTracker")
        double Direction{};
};

USTRUCT(BlueprintType)
struct FSpeedDistance
{
        GENERATED_USTRUCT_BODY()

public:
        FSpeedDistance() : Speed(0.0), Distance(0.0) {}
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker")
        double Speed;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker")
        double Distance;
};

USTRUCT(BlueprintType)
struct FMinMaxXY
{
        GENERATED_USTRUCT_BODY()

public:
        FMinMaxXY() : MinX(0.0), MinY(0.0), MaxX(0.0), MaxY(0.0) {}
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker")
        double MinX;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker")
        double MinY;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker")
        double MaxX;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker")
        double MaxY;
};

UCLASS()
class GEOAI_API APlaneTrack : public AActor
{
        GENERATED_BODY()

public:
        // Sets default values for this actor's properties
        APlaneTrack();

protected:
        // Called when the game starts or when spawned
        virtual void BeginPlay() override;

public:
        // Called every frame
        virtual void Tick(float DeltaTime) override;

public:
        // Spline variable to represent the plane track
        UPROPERTY(BlueprintReadOnly, Category = "FlightTracker")
        USplineComponent* SplineTrack;

        // Cesium class that contain many useful  coordinate conversion functions
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker", Meta = (ExposeOnSpawn = "true"))
        ACesiumGeoreference* GeoReferencingSystem;

        UPROPERTY(BlueprintReadWrite, Category = "FlightTracker")
        TArray<FSpeedDistance> Speeds;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker", Meta = (ExposeOnSpawn = "true"))
        bool EnableBorders = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FlightTracker", Meta = (ExposeOnSpawn = "true"))
        FMinMaxXY MinMaxXY;

        UFUNCTION(BlueprintCallable, Category = "FlightTracker")
        void LoadSplineTrackPoints(UDataTable* AircraftsRawDataTable);

        UFUNCTION(BlueprintCallable, Category = "FlightTracker")
        void ParsePosition(const FString& Position, double& OutLatitude, double& OutLongitude);
};
