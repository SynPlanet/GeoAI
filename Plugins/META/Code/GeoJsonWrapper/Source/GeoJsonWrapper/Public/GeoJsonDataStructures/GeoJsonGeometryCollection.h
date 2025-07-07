// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonGeometryCollection.generated.h"

/**
* @class UGeoJsonGeometryCollection
* @brief Class representing a GeometryCollection object in GeoJSON format.
*
* A GeometryCollection object represents a collection of multiple geometries of different types.
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonGeometryCollection : public UGeoJsonObject
{
        GENERATED_BODY()

public:
        /**
        * @brief Returns the type of the GeoJSON object.
        *
        * This method returns the type of the GeoJSON object as an EGeoJsonType enumeration value.
        *
        * @return The type of the GeoJSON object.
        */
        virtual EGeoJsonType GetType() const override;

        /**
        * @brief Attempts to retrieve a point from the given collection of geometries.
        *
        * @param Point Output parameter where the extracted point geometry will be stored.
        * @return True if the point is found and successfully extracted, false otherwise.
        */
        virtual bool TryGetPoint(FJsonPoint& Point) const override;

        /**
        * @brief Attempts to extract a line from the given collection of geometries.
        *
        * @param LineString Output parameter where the extracted line geometry will be stored.
        * @return True if the line is found and successfully extracted, false otherwise.
        */
        virtual bool TryGetLineString(FJsonMultiPoint& LineString) const override;

        /**
        * @brief Attempts to extract a polygon from the given collection of geometries.
        *
        * @param Polygon Output parameter where the extracted polygon geometry will be stored.
        * @return True if the polygon is found and successfully extracted, false otherwise.
        */
        virtual bool TryGetPolygon(FJsonPolygon& Polygon) const override;

        /**
        * @brief Attempts to extract a multipoint from the given geometry collection.
        *
        * @param MultiPoint An output parameter that will store the extracted multipoint geometry.
        * @return True if the multipoint is found and successfully extracted, false otherwise.
        */
        virtual bool TryGetMultiPoint(FJsonMultiPoint& MultiPoint) const override;

        /**
        * @brief Attempts to extract a multiline from the given geometry collection.
        *
        * @param MultiLineString An output parameter that will store the extracted multiline geometry.
        * @return True if the multiline is found and successfully extracted, false otherwise.
        */
        virtual bool TryGetMultiLineString(FJsonPolygon& MultiLineString) const override;

        /**
        * @brief Attempts to extract a multipolygon from the given collection of geometries.
        *
        * @param MultiPolygon An output parameter that will store the extracted multipolygon geometry.
        * @return True if the multipolygon is found and successfully extracted, false otherwise.
        */
        virtual bool TryGetMultiPolygon(FJsonMultiPolygon& MultiPolygon) const override;

        /**
        * @brief Attempts to extract a geometry from the given collection of geometries.
        *
        * @param Geometry An output parameter that will store the extracted geometry.
        * @return True if the multipolygon is found and successfully extracted, false otherwise.
        */
        virtual bool TryGetGeometry(FJsonGeometry& Geometry) const override;

        /**
        * @brief Returns the geometry at the specified index.
        *
        * @param Idx The index of the geometry.
        * @return A const reference to the geometry at the specified index.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        const FJsonGeometry& GetGeometry(int32 Idx) const;

        /**
        * @brief Returns the multipolygon at the specified index.
        *
        * @param Idx The index of the multipolygon to retrieve.
        * @return A const reference to the multipolygon at the specified index.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        const FJsonMultiPolygon& GetMultiPolygon(int32 Idx) const;

        /**
        * @brief Returns the polygon at the specified index.
        *
        * @param Idx The index of the polygon to retrieve.
        * @return A const reference to the polygon at the specified index.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        const FJsonPolygon& GetPolygon(int32 Idx) const;

        /**
        * @brief Returns the multiline at the specified index.
        *
        * @param Idx The index of the multiline to retrieve.
        * @return A const reference to the multiline at the specified index.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        const FJsonPolygon& GetMultiLineString(int32 Idx) const;

        /**
        * @brief Returns the line at the specified index.
        *
        * @param Idx Index of the line to retrieve.
        * @return A const reference to the line at the specified index.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        const FJsonMultiPoint& GetLineString(int32 Idx) const;

        /**
        * @brief Returns the multipoint at the specified index.
        *
        * @param Idx Index of the multipoint to retrieve.
        * @return A const reference to the multipoint at the specified index.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        const FJsonMultiPoint& GetMultiPoint(int32 Idx) const;

        /**
        * @brief Returns the point at the specified index.
        *
        * @param Idx The index of the point to retrieve.
        * @return A const reference to the point at the specified index.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        const FJsonPoint& GetPoint(int32 Idx) const;

        /**
        * @brief Checks if the geometry collection is empty.
        *
        * @return True if the geometry collection is empty, false otherwise.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        bool IsEmpty() const;

        /**
        * @brief Returns the number of geometries in the GeometryCollection.
        *
        * @return The number of geometries.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        int32 Num() const;

        /**
        * @brief Sets the geometry collection for a GeoJSON object.
        *
        * @param NewCollection The new collection of geometries to set.
        */
        void SetCollection(FJsonGeometryCollection&& NewCollection);

        /**
        * @brief Sets the collection of geometries for a GeoJSON object.
        *
        * @param NewCollection The new collection of geometries to set.
        */
        void SetCollection(const FJsonGeometryCollection& NewCollection);

private:
        FJsonGeometryCollection Geometries;

        /**
         * @brief A template getter function for geometries of various types
         *
         * @param Result An output parameter where the extracted geometry will be stored
         * @return True if the template geometry was found and successfully extracted, false otherwise.
         */
        template <EGeoJsonType FeatureGeometryType, typename TValueType>
        bool TryGetTemplateGeometry(TValueType& Result) const
        {
                for (int32 Idx = 0; Idx < Geometries.Geometries.Num(); ++Idx)
                {
                        if (Geometries.Types[Idx] == FeatureGeometryType)
                        {
                                Result = Geometries.Geometries[Idx].Geometry.Get<TValueType>();
                                return true;
                        }
                }
                return false;
        }
};