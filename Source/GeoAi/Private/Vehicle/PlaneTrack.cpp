#include "Vehicle/PlaneTrack.h"

#include "CesiumGeoreference.h"

// Sets default values
APlaneTrack::APlaneTrack()
{
        // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
        PrimaryActorTick.bCanEverTick = true;

        // Initialize the track
        SplineTrack = CreateDefaultSubobject<USplineComponent>(TEXT("SplineTrack"));
        // This lets us visualize the spline in Play mode
        SplineTrack->SetDrawDebug(true);
        // Set the color of the spline
        SplineTrack->SetUnselectedSplineSegmentColor(FLinearColor(1.f, 0.f, 0.f));
}

// Called when the game starts or when spawned
void APlaneTrack::BeginPlay()
{
        Super::BeginPlay();
}

// Called every frame
void APlaneTrack::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
}

void APlaneTrack::LoadSplineTrackPoints(UDataTable* AircraftsRawDataTable)
{
        if (AircraftsRawDataTable != nullptr && this->GeoReferencingSystem != nullptr)
        {
                int32 PointIndex = 0;
                for (auto& row : AircraftsRawDataTable->GetRowMap())
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
                        FVector SplinePointPosition = GeoReferencingSystem->TransformLongitudeLatitudeHeightPositionToUnreal(GeoPosition);
                        SplineTrack->AddSplinePointAtIndex(SplinePointPosition, PointIndex, ESplineCoordinateSpace::World, false);

                        FSpeedDistance SpeedDistance;
                        SpeedDistance.Speed = (Point->Speed / 1.944);
                        Speeds.Add(SpeedDistance);

                        // Get the up vector at the position to orient the aircraft

                        const FMatrix EarthOrientationForPoint = GeoReferencingSystem->ComputeEastSouthUpToUnrealTransformation(SplinePointPosition);
                        const FVector Up = EarthOrientationForPoint.GetUnitAxis(EAxis::Z);

                        // Compute the up vector at each point to correctly orient the plane
                        SplineTrack->SetUpVectorAtSplinePoint(PointIndex, Up, ESplineCoordinateSpace::World, false);

                        PointIndex++;
                }
                SplineTrack->UpdateSpline();

                int32 MaxPointIndex = (SplineTrack->GetNumberOfSplinePoints() - 1);
                PointIndex = 0;
                while (PointIndex <= MaxPointIndex)
                {
                        Speeds[PointIndex].Distance = SplineTrack->GetDistanceAlongSplineAtSplinePoint(PointIndex);
                        PointIndex++;
                }
        }
}

void APlaneTrack::ParsePosition(const FString& Position, double& OutLatitude, double& OutLongitude)
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