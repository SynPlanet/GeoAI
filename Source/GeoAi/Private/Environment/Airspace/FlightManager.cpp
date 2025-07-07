// Fill out your copyright notice in the Description page of Project Settings.

#include "Environment/Airspace/FlightManager.h"

#include "CesiumGeoreference.h"
#include "System/GeoAiUtilitiesLibrary.h"
#include "Vehicle/PlaneTrack.h"

AFlightManager::AFlightManager()
{
        PrimaryActorTick.bCanEverTick = true;
}

void AFlightManager::ClearConstructedFlightPaths()
{
        for (auto& CurrentPlan : FlightPlans)
        {
                if (CurrentPlan.GeneratedPathInfo.FlightPath)
                {
                        CurrentPlan.GeneratedPathInfo.FlightPath->DestroyComponent();
                }

                if (CurrentPlan.GeneratedPathInfo.CurrentAircraft.IsValid())
                {
                        CurrentPlan.GeneratedPathInfo.CurrentAircraft.Get()->Destroy();
                }

                CurrentPlan.GeneratedPathInfo.Speeds = TArray<FSpeedDistance>{};
        }
}

void AFlightManager::ConstructFlightPaths()
{
        CesiumGeoreference = UGeoAiUtilitiesLibrary::GetCesiumGeoreference(this);
        check(CesiumGeoreference);

        ClearConstructedFlightPaths();

        for (auto& CurrentPlan : FlightPlans)
        {
                CreateSplineForFlightPlan(CurrentPlan);
        }
}

void AFlightManager::BeginPlay()
{
        Super::BeginPlay();

        ConstructFlightPaths();

        for (auto& FlightPlan : FlightPlans)
        {
                if (!FlightPlan.GeneratedPathInfo.FlightPath)
                {
                        continue;
                }

                TSoftClassPtr<AActor> Class;
                if (Class = FlightPlan.Aircraft; !Class.IsValid())
                {
                        int32 Index = FMath::RandRange(0, Aircrafts.Num() - 1);
                        Class = Aircrafts[Index];
                }

                TSubclassOf<AActor> LoadedAircraft = Class.LoadSynchronous();
                if (!LoadedAircraft)
                {
                        continue;
                }

                FActorSpawnParameters NewParams;
                NewParams.Owner = this;
                NewParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                const FTransform SpawnTransform =
                        FlightPlan.GeneratedPathInfo.FlightPath->GetTransformAtDistanceAlongSpline(0.0, ESplineCoordinateSpace::World);

                FlightPlan.GeneratedPathInfo.CurrentAircraft = GetWorld()->SpawnActor<AActor>(LoadedAircraft, SpawnTransform, NewParams);

                FlightPlan.GeneratedPathInfo.CurrentAircraft->SetActorHiddenInGame(true);
        }
}

void AFlightManager::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);

        const float TimeAfterGameStart = GetGameTimeSinceCreation();

        for (auto& CurrentPlan : FlightPlans)
        {
                if (!CurrentPlan.GeneratedPathInfo.FlightPath)
                {
                        continue;
                }
                if (CurrentPlan.MovingInfo.StartTime >= TimeAfterGameStart)
                {
                        continue;
                }

                if (CurrentPlan.MovingInfo.WaitNextFlight)
                {
                        CurrentPlan.MovingInfo.Speed = CurrentPlan.GeneratedPathInfo.Speeds[1].Speed;
                        CurrentPlan.MovingInfo.Progress = 0.f;
                        CurrentPlan.MovingInfo.WaitNextFlight = false;
                        CurrentPlan.GeneratedPathInfo.CurrentAircraft->SetActorHiddenInGame(false);
                }

                const float MaxLength = CurrentPlan.GeneratedPathInfo.FlightPath->GetSplineLength();

                const FTransform TargetTransform = CurrentPlan.GeneratedPathInfo.FlightPath->GetTransformAtDistanceAlongSpline(CurrentPlan.MovingInfo.Progress,
                                                                                                                               ESplineCoordinateSpace::World);

                CurrentPlan.GeneratedPathInfo.CurrentAircraft->SetActorTransform(TargetTransform);

                const float OldProgress = CurrentPlan.MovingInfo.Progress;

                CurrentPlan.MovingInfo.Progress = FMath::FInterpTo(OldProgress, MaxLength, DeltaTime, CurrentPlan.MovingInfo.Speed * Speed);

                if (CurrentPlan.MovingInfo.Progress - OldProgress <= KINDA_SMALL_NUMBER)
                {
                        CurrentPlan.MovingInfo.StartTime += TimeAfterGameStart - CurrentPlan.MovingInfo.StartTime + CurrentPlan.DelayBetweenFlights;
                        CurrentPlan.MovingInfo.WaitNextFlight = true;
                        CurrentPlan.GeneratedPathInfo.CurrentAircraft->SetActorHiddenInGame(true);
                        continue;
                }

                const int32 CurrentIndex = CurrentPlan.MovingInfo.CurrentIndex;
                if (CurrentPlan.GeneratedPathInfo.Speeds.IsValidIndex(CurrentIndex))
                {
                        if (CurrentPlan.GeneratedPathInfo.Speeds[CurrentIndex].Distance > CurrentPlan.MovingInfo.Progress)
                        {
                                CurrentPlan.MovingInfo.CurrentIndex += 1;
                                CurrentPlan.MovingInfo.Speed = CurrentPlan.GeneratedPathInfo.Speeds[CurrentIndex].Speed;
                        }
                }
        }
}

void AFlightManager::CreateSplineForFlightPlan(FFlightPlan& ModifiedPlan)
{
        if (!IsValid(CesiumGeoreference))
        {
                return;
        }

        const UDataTable* LoadedInfo = ModifiedPlan.FlightData.LoadSynchronous();

        if (!IsValid(LoadedInfo))
        {
                return;
        }

        auto* NewSpline = AddComponentByClass(USplineComponent::StaticClass(), true, FTransform::Identity, false);
        USplineComponent* NewPath = StaticCast<USplineComponent*>(NewSpline);
        NewPath->SetOverrideConstructionScript(true);
        NewPath->bInputSplinePointsToConstructionScript = true;
        NewPath->ClearSplinePoints(false);
        NewPath->SetDrawDebug(bEnableDebug);

        int32 PointIndex = 0;
        for (auto& row : LoadedInfo->GetRowMap())
        {
                if (!row.Value)
                {
                        continue;
                }

                FAircraftRawData* Point = (FAircraftRawData*)row.Value;

                if (!Point)
                {
                        continue;
                }
                // Get row data point in lat/long/alt and transform it into points

                FString PointPosition = Point->Position;

                double PointLatitude = 0.0;
                double PointLongitude = 0.0;

                ParsePosition(Point->Position, PointLatitude, PointLongitude);

                double PointHeight = Point->Altitude;

                const FVector GeoPosition = FVector(PointLongitude, PointLatitude, PointHeight);

                // Compute the position in UE coordinates
                FVector SplinePointPosition = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(GeoPosition);

                // Get the up vector at the position to orient the aircraft

                const FMatrix EarthOrientationForPoint = CesiumGeoreference->ComputeEastSouthUpToUnrealTransformation(SplinePointPosition);
                const FVector Up = EarthOrientationForPoint.GetUnitAxis(EAxis::Z);

                NewPath->AddSplinePointAtIndex(SplinePointPosition, PointIndex, ESplineCoordinateSpace::World, false);
                // Compute the up vector at each point to correctly orient the plane
                NewPath->SetUpVectorAtSplinePoint(PointIndex, Up, ESplineCoordinateSpace::World, false);

                FSpeedDistance SpeedDistance;
                SpeedDistance.Speed = (Point->Speed / 1.944);

                ModifiedPlan.GeneratedPathInfo.Speeds.Add(SpeedDistance);

                PointIndex++;
        }

        NewPath->UpdateSpline();

        const int32 MaxPointIndex = NewPath->GetNumberOfSplinePoints() - 1;
        PointIndex = 0;
        while (PointIndex <= MaxPointIndex)
        {
                if (!ModifiedPlan.GeneratedPathInfo.Speeds.IsValidIndex(PointIndex))
                {
                        break;
                }

                ModifiedPlan.GeneratedPathInfo.Speeds[PointIndex].Distance = NewPath->GetDistanceAlongSplineAtSplinePoint(PointIndex);
                PointIndex++;
        }

        ModifiedPlan.GeneratedPathInfo.FlightPath = MoveTemp(NewPath);

        // Set first timestamp
        ModifiedPlan.MovingInfo.StartTime = ModifiedPlan.DelayBeforeStartFlight;
}

void AFlightManager::ParsePosition(const FString& Position, double& OutLatitude, double& OutLongitude)
{
        FString CleanedPosition = Position;
        CleanedPosition = CleanedPosition.Replace(TEXT("\""), TEXT(""));

        TArray<FString> Components;
        CleanedPosition.ParseIntoArray(Components, TEXT(","), true);

        if (Components.Num() == 2)
        {
                OutLatitude = FCString::Atod(*Components[0]);
                OutLongitude = FCString::Atod(*Components[1]);
        }
        else
        {
                UE_LOG(LogTemp, Warning, TEXT("Invalid Position format: %s"), *Position);
                OutLatitude = 0.0;
                OutLongitude = 0.0;
        }
}
