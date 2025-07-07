// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonMultiPoint.generated.h"

/**
* @class UGeoJsonMultiPoint
* @brief A class representing a GeoJSON MultiPoint object.
*
* A MultiPoint object represents a set of points, each represented by a set of two or three coordinates.
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonMultiPoint : public UGeoJsonObject
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
        * @brief Attempts to extract a multipoint from a GeoJSON object.
        *
        * @param MultiPoint A structure to store the extracted coordinates of the multipoint.
        * @return True if the multipoint was successfully retrieved, false otherwise.
        */
        virtual bool TryGetMultiPoint(FJsonMultiPoint& MultiPoint) const override;

        /**
        * @brief Attempts to get the geometry of a GeoJSON object.
        *
        * @param Geometry The FJsonGeometry object to store the retrieved geometry in.
        * @return Returns true if the geometry was successfully retrieved, false otherwise.
        */
        virtual bool TryGetGeometry(FJsonGeometry& Geometry) const override;

        /**
        * @brief Sets the coordinates of a GeoJsonMultiPoint object.
        *
        * @param NewCoordinates The new coordinates of the multipoint.
        */
        void SetCoordinates(FJsonMultiPoint&& NewCoordinates);

        /**
        * @brief Sets the coordinates of the GeoJsonMultiPoint object.
        *
        * @param NewCoordinates The new coordinates of the multipoint.
        */
        void SetCoordinates(const FJsonMultiPoint& NewCoordinates);

private:
        FJsonMultiPoint Coordinates;
};