// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonFeatureCollection.generated.h"

/**
* @class UGeoJsonFeatureCollection
* @brief Class representing a GeoJSON FeatureCollection object.
*
* A FeatureCollection object represents a collection of multiple ObjectFeatures.
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonFeatureCollection : public UGeoJsonObject
{
        GENERATED_BODY()

public:
        /**
         * @brief Returns the GeoJSON feature type.
         *
         * This method returns the GeoJSON feature type as an EGeoJsonType enumeration value.
         *
         * @return The GeoJSON feature type.
         */
        virtual EGeoJsonType GetType() const override;

        /**
        * @brief Gets or creates a GeoJsonObjectFeature at the specified index.
        *
        * This method retrieves the UGeoJsonObjectFeature at the specified index if it already exists.
        * If it does not exist, a new UGeoJsonObjectFeature is created, initialized with the function at the specified index.
        *
        * @param Idx The index of the ObjectFeature.
        * @return The GeoJsonObjectFeature at the specified index, or nullptr if the index is out of range.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        UGeoJsonObjectFeature* GetOrCreateFeatureObject(int32 Idx);

        /**
        * @brief Checks if the feature collection is empty.
        *
        * @return True if the feature collection is empty, false otherwise.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        bool IsEmpty() const;

        /**
        * @brief Returns the number of features in the GeoJsonFeatureCollection.
        *
        * @return The number of features in the collection.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        int32 Num() const;

        /**
        * @brief Gets the ObjectFeature collection.
        *
        * @return A reference to the ObjectFeature collection.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        const FJsonFeatureCollection& GetCollection() const;

        /**
        * @brief Sets a feature collection to GeoJsonFeatureCollection.
        *
        * @param NewCollection A new feature collection.
        */
        void SetCollection(FJsonFeatureCollection&& NewCollection);

        /**
        * @brief Sets a feature collection to GeoJsonFeatureCollection.
        *
        * @param NewCollection A new feature collection.
        */
        void SetCollection(const FJsonFeatureCollection& NewCollection);

private:
        FJsonFeatureCollection Collection;
        TMap<int32, UGeoJsonObjectFeature*> CreatedFeatures;
};