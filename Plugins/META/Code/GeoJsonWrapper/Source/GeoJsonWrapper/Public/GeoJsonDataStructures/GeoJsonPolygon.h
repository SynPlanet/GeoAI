// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonPolygon.generated.h"

/**
* @class UGeoJsonPolygon
* @brief Represents a GeoJSON Polygon geometry object.
*
* A Polygon object represents a sequence of one or more closed polylines.
* The first polyline defines the outer boundary of the polygon.
* The second and subsequent polylines, if any, define the inner boundaries (holes) of the polygon.
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonPolygon : public UGeoJsonObject
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
        * @brief Attempts to get the polygon representation of the GeoJSON object.
        *
        * @param Polygon An output parameter that will store the retrieved polygon geometry.
        * @return True if the GeoJSON object can be converted to a polygon, false otherwise.
        */
        virtual bool TryGetPolygon(FJsonPolygon& Polygon) const override;

        /**
        * @brief Attempts to get the geometry of a GeoJSON object.
        *
        * This method attempts to get the geometry of a GeoJSON object and store it in the provided FJsonGeometry object.
        *
        * @param Geometry An FJsonGeometry object that will store the retrieved geometry.
        * @return Returns true if the geometry was successfully retrieved, false otherwise.
        */
        virtual bool TryGetGeometry(FJsonGeometry& Geometry) const override;

        /**
        * @brief Sets the coordinates of the Polygon object.
        *
        * @param NewCoordinates New coordinates of the polygon.
        */
        void SetCoordinates(FJsonPolygon&& NewCoordinates);

        /**
        * @brief Sets the coordinates of the Polygon object.
        *
        * @param NewCoordinates New coordinates of the polygon.
        */
        void SetCoordinates(const FJsonPolygon& NewCoordinates);

private:
        FJsonPolygon Coordinates;
};