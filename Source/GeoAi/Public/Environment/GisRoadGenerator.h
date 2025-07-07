// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CesiumGeoreference.h"
#include "GameFramework/Actor.h"
#include "GeoJsonDataStructures/GeoJsonFeatureCollection.h"
#include "GeoJsonDataStructures/GeoJsonWrapperTypes.h"
#include "GisRoadGenerator.generated.h"

class UPointCloud;

USTRUCT(BlueprintType)
struct FIntersectionInfo
{
        GENERATED_BODY()

        UPROPERTY(BlueprintReadWrite)
        TArray<FVector> WorldPositions;

        FVector Center = FVector::ZeroVector;
        int32 Id = -1;

        TMap<int32, FVector> NeighboringIntersections;
};

USTRUCT(BlueprintType)
struct FGisRoadInfo
{
        GENERATED_BODY()

        UPROPERTY(BlueprintReadWrite)
        TArray<FVector> WorldPositions;

        int32 Id = -1;

        bool operator==(const FGisRoadInfo& Other) const
        {
                if (WorldPositions.IsEmpty() || Other.WorldPositions.IsEmpty())
                {
                        return false;
                }

                constexpr float Tolerance = 50.f;
                const bool StartEqualStart = WorldPositions[0].Equals(Other.WorldPositions[0], Tolerance);
                const bool EndEqualEnd = WorldPositions[WorldPositions.Num() - 1].Equals(Other.WorldPositions[Other.WorldPositions.Num() - 1], Tolerance);

                if (StartEqualStart && EndEqualEnd)
                {
                        return true;
                }

                const bool StartEqualEnd = WorldPositions[0].Equals(Other.WorldPositions[Other.WorldPositions.Num() - 1], Tolerance);
                const bool EndEqualStart = WorldPositions[WorldPositions.Num() - 1].Equals(Other.WorldPositions[0], Tolerance);

                if (StartEqualEnd && EndEqualStart)
                {
                        return true;
                }

                return false;
        }
};

UCLASS(HideCategories = (Rendering, Collision, Input, Replication, Actor, LOD, Cooking))
class GEOAI_API AGisRoadGenerator : public AActor
{
        GENERATED_BODY()

public:
        AGisRoadGenerator();

        /** 1. Generate basic data based on geojson
         * If you are satisfied with the result, move on to SetNeighboringIntersections()
         */
        UFUNCTION(BlueprintCallable, CallInEditor)
        void GenerateRoadData();

        /** 2. We set up dependencies between intersections in order to correctly design and adjust the roads between them
         * If you are satisfied with the result, move on to SetNeighboringIntersections()
         */
        UFUNCTION(BlueprintCallable, CallInEditor)
        void SetNeighboringIntersections();

        /** 3. We place roads between intersections where there are none, or we move the ends of the existing road to the edges of the intersection
         * If you are satisfied with the result, move on to GenerateSegments()
         */
        UFUNCTION(BlueprintCallable, CallInEditor)
        void GenerateSegments();

        /** 4. Forming the resulting point cloud for ZoneGraph generation
         * If you are satisfied with the result, start the road generator at the level
         */
        UFUNCTION(BlueprintCallable, CallInEditor)
        void UploadCloudPoints();

        /** Show current road net
        * Just debug
        */
        UFUNCTION(BlueprintCallable, CallInEditor)
        void ShowRoads();

        UGeoJsonFeatureCollection* GetGeoJsonFeatureCollection() const;

        void CollectVehicleRoadData(const UGeoJsonFeatureCollection* GeoMultiPolygon,
                                    const ACesiumGeoreference* CesiumPtr,
                                    const TArray<AActor*>& ActorsToIgnore);
        void ConstructIntersection(const ACesiumGeoreference* CesiumPtr, const TArray<AActor*>& ActorsToIgnore, const FJsonObjectFeature& Feature);
        void ConstructRoad(const ACesiumGeoreference* CesiumPtr, const TArray<AActor*>& ActorsToIgnore, const FJsonObjectFeature& Feature);

        bool IsPointOccupiedByAnotherIntersection(FIntersectionInfo SourceInfo, const FVector& RefPoint) const;

        const FIntersectionInfo* FindIntersectionByID(int32 IntersectionID);

#pragma region Helpers
        FVector ConvertGisToUnrealPosition(const ACesiumGeoreference* CesiumPtr, const FJsonPoint& GisCoord, const TArray<AActor*>& ActorsToIgnore) const;
        static FVector FindCenter(const TArray<FVector>& Points);
        TArray<AActor*> GetActorsToIgnore() const;
#pragma endregion Helpers

protected:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
        TObjectPtr<UPointCloud> PointCloud;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        double DistanceToReplacePointInRoad = 350.0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        double OffsetFromRoad = 50.0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float DisplayDataPointsTime = 5.0;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Information")
        int32 IntersectionNums = 5.0;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Information")
        int32 RoadNums = 5.0;

        UPROPERTY(BlueprintReadOnly)
        TArray<FGisRoadInfo> AllRoads;

        UPROPERTY(BlueprintReadOnly)
        TMap<int32, FIntersectionInfo> AllIntersections;

        int32 LastUniqId = -1;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bUseTestJSON = true;

        TMultiMap<FVector, int32> RoadEndpoints;
};