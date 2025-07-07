#include "Environment/GisRoadGenerator.h"

#include "CesiumGeoreference.h"
#include "GeoJsonDeserializer.h"
#include "GeoJsonDataStructures/GeoJsonFeatureCollection.h"
#include "GeoJsonDataStructures/GeoJsonObject.h"
#include "Kismet/GameplayStatics.h"
#include "PointCloud.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "System/GeoAiUtilitiesLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogGisRoadGenerator, Log, All);

AGisRoadGenerator::AGisRoadGenerator()
{
        PrimaryActorTick.bCanEverTick = false;
}

bool FindClosestWorldPosition(const FVector& TargetPosition, TArray<FGisRoadInfo>& AllIntersections, FVector& ResultVector, float Threshold = 100.0f)
{
        float MinDistance = FLT_MAX;

        FVector Direction = (ResultVector - TargetPosition).GetSafeNormal();

        for (const FGisRoadInfo& RoadInfo : AllIntersections)
        {
                for (const FVector& WorldPos : RoadInfo.WorldPositions)
                {
                        float Distance = FVector::Dist(TargetPosition, WorldPos);

                        FVector ToPoint = (WorldPos - TargetPosition).GetSafeNormal();

                        float DotProduct = FVector::DotProduct(Direction, ToPoint);

                        if (DotProduct < 0.99f)
                        {
                                continue;
                        }

                        if (Distance < Threshold)
                        {
                                ResultVector = WorldPos;
                                return true;
                        }

                        if (Distance < MinDistance)
                        {
                                MinDistance = Distance;
                                ResultVector = WorldPos;
                        }
                }
        }

        return false;
}

UGeoJsonFeatureCollection* AGisRoadGenerator::GetGeoJsonFeatureCollection() const
{
        FString FilePath =
                FPaths::ProjectDir() / (bUseTestJSON ? TEXT("Data/GisInfo/miami_roads_3d_test.geojson") : TEXT("Data/GisInfo/miami_roads_3d.geojson"));

        UGeoJsonObject* LoadedGeoInfo = UGeoJsonDeserializer::DeserializePath(FilePath);

        if (!LoadedGeoInfo)
        {
                return nullptr;
        }

        return Cast<UGeoJsonFeatureCollection>(LoadedGeoInfo);
}

void AGisRoadGenerator::GenerateRoadData()
{
        const UGeoJsonFeatureCollection* GeoMultiPolygon = GetGeoJsonFeatureCollection();
        if (!GeoMultiPolygon)
        {
                return;
        }

        const ACesiumGeoreference* const CesiumGeoreference = UGeoAiUtilitiesLibrary::GetCesiumGeoreference(this);
        check(CesiumGeoreference);

        const TArray<AActor*> ActorsToIgnore = GetActorsToIgnore();
        AllRoads.Empty();
        AllIntersections.Empty();
        LastUniqId = -1;

        CollectVehicleRoadData(GeoMultiPolygon, CesiumGeoreference, ActorsToIgnore);
}

void AGisRoadGenerator::ConstructRoad(const ACesiumGeoreference* CesiumPtr, const TArray<AActor*>& ActorsToIgnore, const FJsonObjectFeature& Feature)
{
        FJsonMultiPoint Geometry;
        if (!Feature.TryGetTemplateGeometry<EGeoJsonType::LineString>(Geometry) || Geometry.Coordinates.Num() <= 1)
        {
                return;
        }

        FGisRoadInfo NewRoad;

        for (const auto& Coord : Geometry.Coordinates)
        {
                NewRoad.WorldPositions.AddUnique(ConvertGisToUnrealPosition(CesiumPtr, Coord, ActorsToIgnore));
        }

        if (NewRoad.WorldPositions.Num() < 2)
        {
                UE_LOG(LogGisRoadGenerator, Warning, TEXT("Cant create road with 0-1 points"));
                return;
        }

        if (NewRoad.WorldPositions[0].Equals(NewRoad.WorldPositions[NewRoad.WorldPositions.Num() - 1], 50.f))
        {
                UE_LOG(LogGisRoadGenerator, Warning, TEXT("Bad road start == end"));
                return;
        }

        if (AllRoads.Contains(NewRoad))
        {
                return;
        }

        NewRoad.Id = ++LastUniqId;
        AllRoads.Add(MoveTemp(NewRoad));
}

bool AGisRoadGenerator::IsPointOccupiedByAnotherIntersection(FIntersectionInfo SourceInfo, const FVector& RefPoint) const
{
        for (const auto& [NeighboringId, NeighboringInfo] : SourceInfo.NeighboringIntersections)
        {
                if (NeighboringInfo == RefPoint)
                {
                        return true;
                }
        }

        return false;
}

void AGisRoadGenerator::CollectVehicleRoadData(const UGeoJsonFeatureCollection* GeoMultiPolygon,
                                               const ACesiumGeoreference* CesiumPtr,
                                               const TArray<AActor*>& ActorsToIgnore)
{
        for (const auto& Feature : GeoMultiPolygon->GetCollection().Features)
        {
                if (Feature.GeometryType == EGeoJsonType::Polygon)
                {
                        ConstructIntersection(CesiumPtr, ActorsToIgnore, Feature);
                }
                else if (Feature.GeometryType == EGeoJsonType::LineString)
                {
                        ConstructRoad(CesiumPtr, ActorsToIgnore, Feature);
                }
                else
                {
                        UE_LOG(LogGisRoadGenerator, Log, TEXT("GeoJson have geometry type as: %s"), *UEnum::GetValueAsString(Feature.GeometryType));
                }
        }
}

void AGisRoadGenerator::ConstructIntersection(const ACesiumGeoreference* CesiumPtr, const TArray<AActor*>& ActorsToIgnore, const FJsonObjectFeature& Feature)
{
        FJsonPolygon PolygonGeometry;
        if (!Feature.TryGetTemplateGeometry<EGeoJsonType::Polygon>(PolygonGeometry) || PolygonGeometry.Coordinates.Num() == 0)
        {
                return;
        }

        FIntersectionInfo Intersection;
        for (const FJsonMultiPoint& Coordinate : PolygonGeometry.Coordinates)
        {
                const int32 ResultCoordinateNums = Coordinate.Coordinates.Num();
                const int32 IndexToCheckCircle = ResultCoordinateNums - 1;

                for (int32 i = 0; i < ResultCoordinateNums; i++)
                {
                        FVector UeWorldPosition = ConvertGisToUnrealPosition(CesiumPtr, Coordinate.Coordinates[i], ActorsToIgnore);

                        // Skip the last index if it is the beginning of a polygon
                        if (i == IndexToCheckCircle)
                        {
                                if (Intersection.WorldPositions[0] == UeWorldPosition)
                                {
                                        continue;
                                }
                        }

                        Intersection.WorldPositions.Add(MoveTemp(UeWorldPosition));
                }
        }

        if (Intersection.WorldPositions.Num() < 3)
        {
                UE_LOG(LogGisRoadGenerator, Warning, TEXT("Can't create intersection with 0-2 points"));
                return;
        }

        Intersection.Center = FindCenter(Intersection.WorldPositions);

        for (const auto& [Id, Info] : AllIntersections)
        {
                if (Info.Center == Intersection.Center)
                {
                        return;
                }
        }

        Intersection.Id = ++LastUniqId;

        AllIntersections.Add(LastUniqId, MoveTemp(Intersection));
}

void AGisRoadGenerator::UploadCloudPoints()
{
        if (!IsValid(PointCloud))
        {
                return;
        }

        TArray<FPointCloudPoint> Points;
        int RoadID = -1;

        for (const FGisRoadInfo& Info : AllRoads)
        {
                ++RoadID;
                int32 Order = -1;
                for (const auto& CurrentInfo : Info.WorldPositions)
                {
                        FPointCloudPoint NewPoint;
                        ++Order;
                        NewPoint.Transform = FTransform(CurrentInfo);
                        NewPoint.Attributes.Add("ShapeType", "Spline");
                        NewPoint.Attributes.Add("ID", FString::FromInt(RoadID));
                        NewPoint.Attributes.Add("SequenceNumber", FString::FromInt(Order));

                        NewPoint.Attributes.Add("NumberOfLanes", FString::FromInt(2));
                        NewPoint.Attributes.Add("Unidirectional", FString::FromInt(0));
                        NewPoint.Attributes.Add("CanSupportLongVehicles", FString::FromInt(1));
                        NewPoint.Attributes.Add("IsFreeway", FString::FromInt(0));

                        NewPoint.Attributes.Add("UpVectorX", FString::FromInt(CurrentInfo.UpVector.X));
                        NewPoint.Attributes.Add("UpVectorY", FString::FromInt(CurrentInfo.UpVector.Y));
                        NewPoint.Attributes.Add("UpVectorZ", FString::FromInt(CurrentInfo.UpVector.Z));
                        NewPoint.Attributes.Add("ForwardVectorX", FString::FromInt(CurrentInfo.ForwardVector.X));
                        NewPoint.Attributes.Add("ForwardVectorY", FString::FromInt(CurrentInfo.ForwardVector.Y));
                        NewPoint.Attributes.Add("ForwardVectorZ", FString::FromInt(CurrentInfo.ForwardVector.Z));
                        NewPoint.Attributes.Add("TrafficForwardVectorX", FString::FromInt(CurrentInfo.ForwardVector.X));
                        NewPoint.Attributes.Add("TrafficForwardVectorY", FString::FromInt(CurrentInfo.ForwardVector.Y));
                        NewPoint.Attributes.Add("TrafficForwardVectorZ", FString::FromInt(CurrentInfo.ForwardVector.Z));

                        NewPoint.Attributes.Add("IsFreewayOfframp", FString::FromInt(0));
                        NewPoint.Attributes.Add("IsFreewayOnramp", FString::FromInt(0));

                        Points.Add(MoveTemp(NewPoint));
                }
        }

        for (const auto& [id, Inter] : AllIntersections)
        {
                int32 Seq = 0;
                for (const FVector& Pos : Inter.WorldPositions)
                {
                        FPointCloudPoint InterPoint;
                        InterPoint.Transform = FTransform(Pos);
                        InterPoint.Attributes.Add("ShapeType", "Polygon");
                        InterPoint.Attributes.Add("ID", FString::FromInt(id));
                        InterPoint.Attributes.Add("SequenceNumber", FString::FromInt(Seq++));

                        InterPoint.Attributes.Add("NumberOfLanes", FString::FromInt(2));
                        InterPoint.Attributes.Add("Unidirectional", FString::FromInt(0));
                        InterPoint.Attributes.Add("CanSupportLongVehicles", FString::FromInt(1));
                        InterPoint.Attributes.Add("IsFreeway", FString::FromInt(0));

                        InterPoint.Attributes.Add("UpVectorX", FString::FromInt(Pos.UpVector.X));
                        InterPoint.Attributes.Add("UpVectorY", FString::FromInt(Pos.UpVector.Y));
                        InterPoint.Attributes.Add("UpVectorZ", FString::FromInt(Pos.UpVector.Z));
                        InterPoint.Attributes.Add("ForwardVectorX", FString::FromInt(Pos.ForwardVector.X));
                        InterPoint.Attributes.Add("ForwardVectorY", FString::FromInt(Pos.ForwardVector.Y));
                        InterPoint.Attributes.Add("ForwardVectorZ", FString::FromInt(Pos.ForwardVector.Z));
                        InterPoint.Attributes.Add("TrafficForwardVectorX", FString::FromInt(Pos.ForwardVector.X));
                        InterPoint.Attributes.Add("TrafficForwardVectorY", FString::FromInt(Pos.ForwardVector.Y));
                        InterPoint.Attributes.Add("TrafficForwardVectorZ", FString::FromInt(Pos.ForwardVector.Z));

                        InterPoint.Attributes.Add("IsAdapterPolygonPoint", FString::FromInt(1));
                        InterPoint.Attributes.Add("IsFreewayRampConnection", FString::FromInt(0));
                        InterPoint.Attributes.Add("IsFreewayOfframp", FString::FromInt(0));
                        InterPoint.Attributes.Add("IsFreewayOnramp", FString::FromInt(0));

                        Points.Add(MoveTemp(InterPoint));
                }
        }

        PointCloud->LoadFromPoints(Points);
}

void AGisRoadGenerator::SetNeighboringIntersections()
{
        TArray<int32> FullConnectedIntersections;
        FCollisionQueryParams Params;
        Params.bTraceComplex = true;

        for (auto& [CurrentIntersectionId, CurrentIntersection] : AllIntersections)
        {
                for (const FVector& RefPoint : CurrentIntersection.WorldPositions)
                {
                        if (IsPointOccupiedByAnotherIntersection(CurrentIntersection, RefPoint))
                        {
                                continue;
                        }

                        FVector Direction = (RefPoint - CurrentIntersection.Center).GetSafeNormal();

                        if (Direction.Length() <= KINDA_SMALL_NUMBER)
                        {
                                continue;
                        }

                        float MinDistance = FLT_MAX;
                        FIntersectionInfo* BestIntersectionInfo = nullptr;
                        FVector BestOtherPoint = FVector::ZeroVector;

                        for (auto& [OtherId, OtherIntersection] : AllIntersections)
                        {
                                // Skip self
                                if (OtherIntersection.Center == CurrentIntersection.Center)
                                {
                                        continue;
                                }

                                // Skip all connected Intersections
                                if (FullConnectedIntersections.Contains(OtherId))
                                {
                                        continue;
                                }

                                for (const FVector& OtherRefPoint : OtherIntersection.WorldPositions)
                                {
                                        float DotProduct = FVector::DotProduct(Direction, (OtherRefPoint - CurrentIntersection.Center).GetSafeNormal());

                                        if (DotProduct <= 0.9f)
                                        {
                                                continue;
                                        }

                                        float Distance = FVector::Dist(OtherRefPoint, RefPoint);

                                        if (Distance > MinDistance)
                                        {
                                                continue;
                                        }

                                        // We check before the trace whether the point was already occupied
                                        if (IsPointOccupiedByAnotherIntersection(OtherIntersection, OtherRefPoint))
                                        {
                                                continue;
                                        }

                                        FHitResult Hit;
                                        if (GetWorld()->LineTraceSingleByChannel(Hit, RefPoint, OtherRefPoint, ECC_Visibility, Params))
                                        {
                                                continue;
                                        }

                                        MinDistance = Distance;
                                        BestOtherPoint = OtherRefPoint;
                                        BestIntersectionInfo = &OtherIntersection;
                                }
                        }

                        if (!BestIntersectionInfo)
                        {
                                continue;
                        }

                        CurrentIntersection.NeighboringIntersections.Add(BestIntersectionInfo->Id, RefPoint);
                        BestIntersectionInfo->NeighboringIntersections.Add(CurrentIntersection.Id, BestOtherPoint);

                        if (CurrentIntersection.NeighboringIntersections.Num() == CurrentIntersection.WorldPositions.Num())
                        {
                                FullConnectedIntersections.Add(CurrentIntersection.Id);
                        }

                        if (BestIntersectionInfo->NeighboringIntersections.Num() == BestIntersectionInfo->WorldPositions.Num())
                        {
                                FullConnectedIntersections.Add(BestIntersectionInfo->Id);
                        }
                }
        }
}

const FIntersectionInfo* AGisRoadGenerator::FindIntersectionByID(int32 IntersectionID)
{
        return AllIntersections.Find(IntersectionID);
}

bool ArePointsBetween(const FVector& Start, const FVector& End, const TArray<FVector>& TestPoints, float Tolerance = 10.f)
{
        for (const FVector& TestPoint : TestPoints)
        {

                if (FVector::Dist(TestPoint, Start) + FVector::Dist(TestPoint, End) - FVector::Dist(Start, End) > Tolerance)
                {

                        return false;
                }
        }

        return true;
}

bool IsPointXBetweenAAndBWithWidth(const FVector& A, const FVector& B, const FVector& X, float Width)
{

        FVector AB = B - A;

        FVector AX = X - A;

        FVector CrossProduct = FVector::CrossProduct(AB, AX);

        float Distance = CrossProduct.Size() / AB.Size();

        if (FVector::DotProduct(AB, AX) >= 0 && FVector::DotProduct(AB, AX) <= AB.Size() * AB.Size() && Distance <= Width)
        {
                return true;
        }

        return false;
}

void AGisRoadGenerator::GenerateSegments()
{
        for (const auto& [CurrentId, Info] : AllIntersections)
        {
                for (auto& [OtherId, OtherInfo] : Info.NeighboringIntersections)
                {
                        const auto& OtherIntersection = AllIntersections.Find(OtherId);

                        if (!OtherIntersection)
                        {
                                continue;
                        }

                        const auto& TargetLocation = OtherIntersection->NeighboringIntersections.Find(CurrentId);
                        if (!TargetLocation)
                        {
                                continue;
                        }

                        DrawDebugDirectionalArrow(GetWorld(), OtherInfo, *TargetLocation, 500.f, FColor::Red, false, DisplayDataPointsTime);

                        bool bIsRoadBetweenIntersections = false;

                        TArray<FGisRoadInfo> RoadsBetweenIntersections;
                        for (FGisRoadInfo& Road : AllRoads)
                        {
                                for (const FVector& Position : Road.WorldPositions)
                                {
                                        if (IsPointXBetweenAAndBWithWidth(*TargetLocation, OtherInfo, Position, 600.f))
                                        {
                                                RoadsBetweenIntersections.Add(Road);
                                                break;
                                        }
                                }

                                // if (bIsRoadBetweenIntersections)
                                // {
                                //         const float D1 = FVector::Distance(OtherInfo, Road.WorldPositions[0]);
                                //         const float D2 = FVector::Distance(OtherInfo, Road.WorldPositions[Road.WorldPositions.Num() - 1]);
                                //
                                //         if (D1 < D2)
                                //         {
                                //                 Road.WorldPositions[0] = OtherInfo;
                                //                 Road.WorldPositions[Road.WorldPositions.Num() - 1] = *TargetLocation;
                                //         }
                                //         else
                                //         {
                                //                 Road.WorldPositions[0] = *TargetLocation;
                                //                 Road.WorldPositions[Road.WorldPositions.Num() - 1] = OtherInfo;
                                //         }
                                //
                                //         break;
                                // }
                        }

                        UE_LOG(LogGisRoadGenerator, Warning, TEXT("Roads before clear %i"), AllRoads.Num());

                        for (const auto& Road : RoadsBetweenIntersections)
                        {
                                AllRoads.Remove(Road);
                        }

                        UE_LOG(LogGisRoadGenerator, Warning, TEXT("Roads after clear %i"), AllRoads.Num());

                        if (!bIsRoadBetweenIntersections)
                        {
                                FGisRoadInfo NewRoad;
                                NewRoad.WorldPositions.Add(OtherInfo);
                                NewRoad.WorldPositions.Add(*TargetLocation);

                                if (AllRoads.Contains(NewRoad))
                                {
                                        continue;
                                }

                                NewRoad.Id = ++LastUniqId;

                                AllRoads.Add(MoveTemp(NewRoad));

                                DrawDebugPoint(GetWorld(), OtherInfo, 20.0f, FColor::Yellow, false, DisplayDataPointsTime);
                                DrawDebugPoint(GetWorld(), *TargetLocation, 20.0f, FColor::Cyan, false, DisplayDataPointsTime);
                        }
                }
        }
}

void AGisRoadGenerator::ShowRoads()
{
#if ENABLE_DRAW_DEBUG
        for (const auto& Road : AllRoads)
        {
                for (const auto& Point : Road.WorldPositions)
                {
                        DrawDebugPoint(GetWorld(), Point, 20.0f, FColor::Blue, false, DisplayDataPointsTime);
                }
        }

        for (const auto& [CurrentId, Info] : AllIntersections)
        {
                for (const auto& Point : Info.WorldPositions)
                {
                        DrawDebugPoint(GetWorld(), Point, 20.0f, FColor::Red, false, DisplayDataPointsTime);
                }

                DrawDebugPoint(GetWorld(), Info.Center, 20.0f, FColor::Green, false, DisplayDataPointsTime);

                for (const auto& [OtherId, OtherInfo] : Info.NeighboringIntersections)
                {
                        const auto& OtherIntersection = AllIntersections.Find(OtherId);

                        if (!OtherIntersection)
                        {
                                continue;
                        }

                        const auto& TargetLocation = OtherIntersection->NeighboringIntersections.Find(CurrentId);
                        if (!TargetLocation)
                        {
                                continue;
                        }
                        DrawDebugDirectionalArrow(GetWorld(), OtherInfo, *TargetLocation, 500.f, FColor::Red, false, DisplayDataPointsTime);
                }
        }
#endif
}

#pragma region Helpers

FVector AGisRoadGenerator::ConvertGisToUnrealPosition(const ACesiumGeoreference* CesiumPtr,
                                                      const FJsonPoint& GisCoord,
                                                      const TArray<AActor*>& ActorsToIgnore) const
{
        const FVector EcefCoord(GisCoord.Coordinates.X, GisCoord.Coordinates.Y, GisCoord.Coordinates.Z);
        FVector UnrealCoord = CesiumPtr->TransformLongitudeLatitudeHeightPositionToUnreal(EcefCoord);

        const FMatrix EarthOrientationForPoint = CesiumPtr->ComputeEastSouthUpToUnrealTransformation(UnrealCoord);
        const FVector UpVector = EarthOrientationForPoint.GetUnitAxis(EAxis::Z);

        FHitResult Hit;
        if (UKismetSystemLibrary::LineTraceSingleByProfile(
                    this, UnrealCoord, UnrealCoord + UpVector * -UE_MAX_FLT, "RoadGeneration", true, ActorsToIgnore, EDrawDebugTrace::None, Hit, true))
        {
                UnrealCoord = Hit.ImpactPoint + Hit.ImpactNormal * OffsetFromRoad;
        }

        return MoveTemp(UnrealCoord);
}

FVector AGisRoadGenerator::FindCenter(const TArray<FVector>& Points)
{
        float SumX = 0.0f;
        float SumY = 0.0f;
        float SumZ = 0.0f;

        for (const FVector& Point : Points)
        {
                SumX += Point.X;
                SumY += Point.Y;
                SumZ += Point.Z;
        }

        return FVector(SumX / Points.Num(), SumY / Points.Num(), SumZ / Points.Num());
}

TArray<AActor*> AGisRoadGenerator::GetActorsToIgnore() const
{
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

        TArray<AActor*> ResultActors;
        for (const auto& Actor : AllActors)
        {
                if (!Actor)
                {
                        continue;
                        ;
                }

                if (Actor->FindComponentByClass<UHierarchicalInstancedStaticMeshComponent>())
                {
                        ResultActors.Add(Actor);
                }
        }

        return ResultActors;
}

#pragma endregion Helpers