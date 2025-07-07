// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonMultiLineString.generated.h"

/**
* @class UGeoJsonMultiLineString
* @brief Represents a GeoJSON MultiLineString geometry object.
*
* A MultiLineString object represents a sequence of multiple LineString objects
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonMultiLineString : public UGeoJsonObject
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
        * @brief Attempts to get the multiline representation of the GeoJSON object.
        *
        * @param MultiLineString An output parameter that will store the extracted multiline geometry.
        * @return True if the GeoJSON object can be converted to a multiline, false otherwise.
        */
        virtual bool TryGetMultiLineString(FJsonPolygon& MultiLineString) const override;

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
        * @brief Sets the coordinates of the MultiLineString object.
        *
        * @param NewCoordinates The new coordinates of the multiline.
        */
        void SetCoordinates(FJsonPolygon&& NewCoordinates);

        /**
        * @brief Sets the coordinates of the MultiLineString object.
        *
        * @param NewCoordinates The new coordinates of the multiline.
        */
        void SetCoordinates(const FJsonPolygon& NewCoordinates);

private:
        FJsonPolygon Coordinates;
};