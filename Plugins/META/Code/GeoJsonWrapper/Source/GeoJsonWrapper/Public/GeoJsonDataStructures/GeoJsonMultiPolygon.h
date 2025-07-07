// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonMultiPolygon.generated.h"

/**
* @class UGeoJsonMultiPolygon
* @brief Represents a GeoJSON MultiPolygon geometry object.
*
* A MultiPolygon object represents a sequence of multiple Polygon objects
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonMultiPolygon : public UGeoJsonObject
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
        * @brief Attempts to get the multipolygon representation of the GeoJSON object.
        *
        * @param MultiPolygon An output parameter that will store the retrieved multipolygon geometry.
        * @return True if the GeoJSON object can be converted to a multipolygon, false otherwise.
        */
        virtual bool TryGetMultiPolygon(FJsonMultiPolygon& MultiPolygon) const override;

        /**
        * @brief Attempts to get the geometry of the GeoJSON object.
        *
        * This method attempts to get the geometry of the GeoJSON object and store it in the provided FJsonGeometry object.
        *
        * @param Geometry The FJsonGeometry object to store the retrieved geometry in.
        * @return Returns true if the geometry was successfully retrieved, false otherwise.
        */
        virtual bool TryGetGeometry(FJsonGeometry& Geometry) const override;

        /**
        * @brief Sets the coordinates of the MultiPolygon object.
        *
        * @param NewCoordinates The new coordinates of the multipolygon.
        */
        void SetCoordinates(FJsonMultiPolygon&& NewCoordinates);
        /**
        * @brief Sets the coordinates of the MultiPolygon object.
        *
        * @param NewCoordinates The new coordinates of the multipolygon.
        */
        void SetCoordinates(const FJsonMultiPolygon& NewCoordinates);

private:
        FJsonMultiPolygon Coordinates;
};