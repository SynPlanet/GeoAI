// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonObject.h"
#include "GeoJsonObjectFeature.generated.h"

/**
* @class UGeoJsonObjectFeature
* @brief Class representing a GeoJSON ObjectFeature.
*
* An ObjectFeature represents a union of a geometry's coordinates with a set of various properties of that geometry.
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonObjectFeature : public UGeoJsonObject
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
        * @brief This method attempts to obtain the ObjectFeature as a corresponding structure.
        *
        * @param ObjectFeature The FJsonObjectFeature object that will hold the retrieved ObjectFeature.
        * @return Returns true if the ObjectFeature was successfully retrieved, false otherwise.
        */
        UFUNCTION(BlueprintCallable)
        bool TryGetObjectFeature(FJsonObjectFeature& ObjectFeature) const;

        /**
        * @brief Attempts to get a point from the GeoJsonObjectFeature.
        *
        * @param Point The FJsonPoint object to populate with the point coordinates.
        * @return True if the feature's geometry type is a point and the FJsonPoint object was successfully populated, false otherwise.
        */
        virtual bool TryGetPoint(FJsonPoint& Point) const override;

        /**
        * @brief Attempts to get a line from the GeoJsonObjectFeature.
        *
        * @param LineString The FJsonMultiPoint object that will contain the retrieved line geometry.
        * @return Returns true if the feature's geometry type is a line and the FJsonMultiPoint object was successfully populated, otherwise false.
        */
        virtual bool TryGetLineString(FJsonMultiPoint& LineString) const override;

        /**
        * @brief Attempts to get a polygon from the GeoJsonObjectFeature.
        *
        * @param Polygon An FJsonPolygon object to store the polygon geometry.
        * @return Returns true if the feature's geometry type is a polygon and the FJsonPolygon object was successfully retrieved, otherwise false.
        */
        virtual bool TryGetPolygon(FJsonPolygon& Polygon) const override;

        /**
        * @brief Attempts to get a multipoint from the GeoJsonObjectFeature.
        *
        * @param MultiPoint An FJsonMultiPoint object to store the multipoint geometry.
        * @return Returns true if the element's geometry type is multipoint and the FJsonMultiPoint object was successfully retrieved, false otherwise.
        */
        virtual bool TryGetMultiPoint(FJsonMultiPoint& MultiPoint) const override;

        /**
        * @brief Attempts to get the geometry of a multiline from a GeoJsonObjectFeature.
        *
        * @param MultiLineString An FJsonPolygon object to store the geometry of the multiline.
        * @return Returns true if the geometry of the multiline was successfully retrieved, false otherwise.
        */
        virtual bool TryGetMultiLineString(FJsonPolygon& MultiLineString) const override;

        /**
        * @brief Attempts to get the geometry of a multipolygon from a GeoJsonObjectFeature.
        *
        * @param MultiPolygon An FJsonMultiPolygon object to store the geometry of the multipolygon.
        * @return True if the geometry of the multipolygon was successfully retrieved, false otherwise.
        */
        virtual bool TryGetMultiPolygon(FJsonMultiPolygon& MultiPolygon) const override;

        /**
        * @brief Attempts to retrieve a geometry from a GeoJsonObjectFeature.
        *
        * @param Geometry The FJsonGeometry object to store the geometry in.
        * @return True if the geometry was successfully retrieved, false otherwise.
        */
        virtual bool TryGetGeometry(FJsonGeometry& Geometry) const override;

        /**
        * @brief Retrieves the string value of the property associated with the specified key from the GeoJsonObjectFeature properties.
        *
        * @param Key The key of the property to retrieve.
        * @return The string value of the property associated with the specified key. If the key is not found, an empty string is returned.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        FString GetStringProperty(const FString& Key) const;

        /**
        * @brief Looks up a property with the given key and returns it as a string, or a default value if the key is not found.
        *
        * @param Key The key of the value to look up.
        * @param Default The default value to return if there is no value with the given key.
        * @return The string corresponding to the given key, or the default value if the key is not found.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        FString GetStringPropertyWithDefault(const FString& Key, const FString& Default) const;

        /**
        * @brief Retrieves the boolean value of the property associated with the given key from the GeoJsonObjectFeature properties.
        *
        * @param Key The key of the property to retrieve.
        * @return The boolean value associated with the given key if found, false otherwise.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        bool GetBoolProperty(const FString& Key) const;

        /**
        * @brief Looks up a property with the given key and returns it as a boolean flag, or a default value if the key is not found.
        *
        * @param Key The key of the value to look up.
        * @param bDefault The default value to return if there is no value with the given key.
        * @return The boolean flag corresponding to the given key, or the default value if the key is not found.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        bool GetBoolPropertyWithDefault(const FString& Key, bool bDefault) const;

        /**
        * @brief Retrieves the numeric value of the property associated with the specified key from the GeoJsonObjectFeature properties.
        *
        * @param Key The key of the property to get.
        * @return The value of the numeric property, or DBL_MAX if the property does not exist or cannot be converted to a number.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        double GetNumberProperty(const FString& Key) const;

        /**
        * @brief Looks up the property with the specified key and returns it as a float, or a default value if the key is not found.
        *
        * @param Key The key of the value to look up.
        * @param Default The default value to return if there is no value with the given key.
        * @return The string corresponding to the given key, or the default value if the key is not found.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        double GetNumberPropertyWithDefault(const FString& Key, double Default) const;

        /**
        * @brief Retrieves the integer value of the property associated with the specified key from the GeoJsonObjectFeature properties.
        *
        * @param Key The key of the property to retrieve.
        * @return The integer value of the specified property, or INT_MAX if the property is not found or cannot be converted to an integer.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        int32 GetIntProperty(const FString& Key) const;

        /**
        * @brief Searches for a property with the specified key and returns it as an integer, or the default value if the key is not found.
        *
        * @param Key The key of the value to search for.
        * @param Default The default value to return if there is no value with the given key.
        * @return The string corresponding to the given key, or the default value if the key is not found.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure)
        int32 GetIntPropertyWithDefault(const FString& Key, int32 Default) const;

        /**
        * @brief Sets the value of the GeoJsonObjectFeature.
        *
        * @param Element The FJsonObjectFeature instance to set as the value.
        */
        void SetFeature(FJsonObjectFeature&& Element);

        /**
        * @brief Sets the value of the GeoJsonObjectFeature.
        *
        * @param Element The FJsonObjectFeature instance to set as the value.
        */
        void SetFeature(const FJsonObjectFeature& Element);

private:
        FJsonObjectFeature Feature;
};