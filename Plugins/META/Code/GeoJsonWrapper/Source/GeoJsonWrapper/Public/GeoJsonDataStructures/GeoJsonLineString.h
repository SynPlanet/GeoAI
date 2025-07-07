// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonLineString.generated.h"

/**
* @class UGeoJsonLineString
* @brief Represents a GeoJSON LineString geometry object.
*
* A LineString object represents a sequence of two or more points, where each point is represented by a set of two or three coordinates.
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonLineString : public UGeoJsonObject
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
        * @brief Attempts to get the line representation of the GeoJSON object.
        *
        * @param LineString An output parameter in which to store the extracted line geometry.
        * @return True if the GeoJSON object can be converted to a line, false otherwise.
        */
        virtual bool TryGetLineString(FJsonMultiPoint& LineString) const override;

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
        * @brief Sets the coordinates of the LineString object.
        *
        * @param NewCoordinates The new coordinates of the line.
        */
        void SetCoordinates(FJsonMultiPoint&& NewCoordinates);

        /**
        * @brief Sets the coordinates of the LineString object.
        *
        * @param NewCoordinates The new coordinates of the line.
        */
        void SetCoordinates(const FJsonMultiPoint& NewCoordinates);

private:
        FJsonMultiPoint Coordinates;
};