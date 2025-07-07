// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonPoint.generated.h"

/**
* @class UGeoJsonPoint
* @brief Class representing a GeoJSON Point object.
*
* A Point object represents a point represented by a set of two or three coordinates.
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonPoint : public UGeoJsonObject
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
        * @brief Attempts to get the point representation of the GeoJSON object.
        *
        * @param Point A structure to hold the extracted coordinates of the point.
        * @return True if the point was successfully extracted, false otherwise.
        */
        virtual bool TryGetPoint(FJsonPoint& Point) const override;

        /**
        * @brief Tries to get the GeoJSON object's geometry.
        *
        * @param Geometry The FJsonGeometry object to store the retrieved geometry in.
        * @return Returns true if the geometry was successfully retrieved, false otherwise.
        */
        virtual bool TryGetGeometry(FJsonGeometry& Geometry) const override;

        /**
        * @brief Sets the coordinates of the GeoJsonPoint object.
        *
        * @param Point The new coordinates of the point.
        */
        void SetCoordinates(FJsonPoint&& Point);
        /**
        * @brief Sets the coordinates of the GeoJsonPoint object.
        *
        * @param Point The new coordinates of the point.
        */
        void SetCoordinates(const FJsonPoint& Point);

private:
        FJsonPoint Coordinates;
};