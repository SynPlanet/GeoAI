// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonDataStructures/GeoJsonWrapperTypes.h"
#include "GeoJsonObject.generated.h"

class UGeoJsonPoint;
class UGeoJsonLineString;
class UGeoJsonPolygon;
class UGeoJsonMultiPoint;
class UGeoJsonMultiLineString;
class UGeoJsonMultiPolygon;
class UGeoJsonGeometryCollection;
class UGeoJsonObjectFeature;
class UGeoJsonFeatureCollection;

/**
* @class UGeoJsonObject
* @brief Base class representing a GeoJSON object.
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonObject : public UObject
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
        UFUNCTION(BlueprintCallable)
        virtual EGeoJsonType GetType() const;

        /**
        * @brief Attempts to extract a point from a GeoJSON object.
        *
        * This method attempts to extract a point from a GeoJSON object and stores it in the given FJsonPoint structure.
        *
        * @param Point A structure to store the extracted point coordinates.
        * @return True if the point was successfully extracted, false otherwise.
        */
        UFUNCTION(BlueprintCallable)
        virtual bool TryGetPoint(FJsonPoint& Point) const;

        /**
        * @brief Attempts to get a line representation of a GeoJSON object.
        *
        * @param Line An output parameter that will store the extracted line geometry.
        * @return True if the GeoJSON object can be converted to a line, false otherwise.
        */
        UFUNCTION(BlueprintCallable)
        virtual bool TryGetLineString(FJsonMultiPoint& Line) const;

        /**
        * @brief Attempts to get a polygon from a GeoJSON object.
        *
        * This method attempts to extract a polygon from a GeoJSON object.
        *
        * @param Polygon An output parameter that will store the extracted polygon geometry.
        * @return True if the polygon was successfully extracted, false otherwise.
        */
        UFUNCTION(BlueprintCallable)
        virtual bool TryGetPolygon(FJsonPolygon& Polygon) const;

        /**
        * @brief This method attempts to extract the multipoint geometry from a GeoJSON object.
        *
        * @param MultiPoint The output parameter where the extracted multipoint geometry will be stored.
        * @return Returns true if the extraction was successful, false otherwise.
        */
        UFUNCTION(BlueprintCallable)
        virtual bool TryGetMultiPoint(FJsonMultiPoint& MultiPoint) const;

        /**
        * \brief Attempts to obtain the multiline representation from a GeoJSON object.
        *
        * This method attempts to obtain the MultiLineString representation of a GeoJSON object and assigns it to the parameter.
        *
        * \param MultiLine The FJsonPolygon variable to store the result in.
        * \return Returns true if the MultiLineString representation was successfully retrieved, false otherwise.
        */
        UFUNCTION(BlueprintCallable)
        virtual bool TryGetMultiLineString(FJsonPolygon& MultiLine) const;

        /**
        * @brief Attempts to get the multipolygon representation from the GeoJSON object.
        *
        * This method attempts to get the multipolygon representation of the GeoJSON object and assigns it to the parameter.
        *
        * @param MultiPolygon A reference to the FJsonMultiPolygon object to store the result in.
        * @return True if the GeoJSON object has a multipolygon representation, false otherwise.
        */
        UFUNCTION(BlueprintCallable)
        virtual bool TryGetMultiPolygon(FJsonMultiPolygon& MultiPolygon) const;

        /**
        * @brief Attempts to get the geometry of the GeoJSON object.
        *
        * This method attempts to retrieve the GeoJSON object's geometry and store it in the provided FJsonGeometry object.
        *
        * @param Geometry The FJsonGeometry object in which to store the retrieved geometry.
        * @return Returns true if the geometry was retrieved successfully, false otherwise.
        */
        UFUNCTION(BlueprintCallable)
        virtual bool TryGetGeometry(FJsonGeometry& Geometry) const;
};