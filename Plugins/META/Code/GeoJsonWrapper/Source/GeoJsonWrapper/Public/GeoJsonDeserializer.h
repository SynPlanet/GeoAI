// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonDataStructures/GeoJsonObject.h"
#include "GeoJsonDeserializer.generated.h"

struct FJsonObjectWrapper;
/**
* Class for deserializing GeoJSON files
*/
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonDeserializer : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

public:
        /**
        * @brief Deserializes a GeoJSON file into engine-readable data
        *
        * @param PathToJsonSource Path to .GEOJSON file
        * @return Pointer to UGeoJsonObject - an object containing the deserialized data
        */
        UFUNCTION(BlueprintCallable, Category = "GeoJSON")
        static UGeoJsonObject* DeserializePath(const FString& PathToJsonSource);

        /**
        * @brief Deserializes the GeoJSON file loaded into the input string into engine-readable data
        *
        * @param JsonString .GEOJSON file written to the string
        * @return Pointer to UGeoJsonObject - an object containing the deserialized data
        */
        UFUNCTION(BlueprintCallable, Category = "GeoJSON")
        static UGeoJsonObject* DeserializeLoaded(const FString& JsonString);

        /**
        * @brief Deserializes the JSON string into a key-value hash table
        *
        * @param JsonString The JSON string to deserialize
        * @return The deserialized hash table
        */
        UFUNCTION(BlueprintCallable, Category = "GeoJSON")
        static TMap<FString, FString> DeserializeJson(const FString& JsonString);

        /**
        * @brief Deserializes a GeoJSON geometry
        *
        * @param ObjectInfo An FJsonObjectWrapper structure containing geometry information
        * @param Field The name of the field to deserialize
        * @param Geometry An output parameter containing the deserialized geometry
        * @return The type of the deserialized geometry on success, or None on failure
        */
        UFUNCTION(BlueprintCallable, Category = "GeoJSON")
        static EGeoJsonType DeserializeGeometry(const FJsonObjectWrapper& ObjectInfo, const FString& Field, FJsonGeometry& Geometry);

        /**
        * @brief Deserializes a GeoJSON point geometry
        *
        * @param ObjectInfo FJsonObjectWrapper structure containing geometry information
        * @param Field Name of the field to deserialize
        * @param Point Output parameter containing the deserialized point geometry
        * @return True if deserialization succeeds, false if failure
        */
        UFUNCTION(BlueprintCallable, Category = "GeoJSON")
        static bool DeserializePointGeometry(const FJsonObjectWrapper& ObjectInfo, const FString& Field, FJsonPoint& Point);

        /**
        * @brief Deserializes a JSON string into an FJsonObject
        *
        * @param JsonString JSON string to deserialize
        * @return The deserialized FJsonObject
        */
        static TSharedPtr<FJsonObject> Deserialize(const FString& JsonString);

        /**
        * @brief Deserializes an FJsonObject into engine-readable data
        *
        * @param JsonObject An FJsonObject to deserialize
        * @return A pointer to a UGeoJsonObject containing the deserialized data
        */
        static UGeoJsonObject* DeserializeObject(const TSharedPtr<FJsonObject>& JsonObject);

        /**
        * @brief Deserializes an array of coordinates representing a point.
        *
        * @param Coordinates A JSON array containing the coordinates of the point. It must contain either 2D or 3D coordinates.
        * @return An FJsonPoint containing the deserialized coordinates.
        */
        static FJsonPoint DeserializePoint(const TArray<TSharedPtr<FJsonValue>>& Coordinates);

        /**
        * @brief Deserializes an array of coordinates representing a multipoint or line.
        *
        * @param Coordinates A JSON array containing the coordinates of the multipoint or line
        * @return The deserialized FJsonMultiPoint object containing the coordinates of the multipoint or line
        */
        static FJsonMultiPoint DeserializeMultiPoint(const TArray<TSharedPtr<FJsonValue>>& Coordinates);

        /**
        * @brief Deserializes an array of coordinates representing a polygon or multiline.
        *
        * @param Coordinates A JSON array containing the coordinates of the polygon or multiline.
        * @return The deserialized coordinates of the polygon or multiline as an FJsonPolygon object.
        */
        static FJsonPolygon DeserializePolygon(const TArray<TSharedPtr<FJsonValue>>& Coordinates);

        /**
        * @brief Deserializes a coordinate array representing a multipolygon
        *
        * @param Coordinates A JSON array containing the coordinates of the multipolygon
        * @return The deserialized MultiPolygon as an FJsonMultiPolygon structure
        */
        static FJsonMultiPolygon DeserializeMultiPolygon(const TArray<TSharedPtr<FJsonValue>>& Coordinates);

        /**
        * @brief Deserializes a GeoJSON geometry object
        *
        * @param Geometry An FJsonObject containing the geometry type and coordinates
        * @return A pair containing the geometry type and the FJsonGeometry object
        */
        static TPair<EGeoJsonType, FJsonGeometry> DeserializeGeometry(const TSharedPtr<FJsonObject>& Geometry);

        /**
        * @brief Deserializes a set of geometries from a JSON array.
        *
        * @param Geometries An array of TSharedPtr<FJsonValue> containing the geometries to deserialize.
        * @return An FJsonGeometryCollection structure containing the deserialized collection of geometries.
        */
        static FJsonGeometryCollection DeserializeGeometryCollection(const TArray<TSharedPtr<FJsonValue>>& Geometries);

        /**
        * @brief Deserializes a GeoJSON feature object into an FJsonObjectFeature structure.
        *
        * @param Feature The GeoJSON feature object to deserialize.
        * @return The FJsonObjectFeature structure containing the deserialized data.
        */
        static FJsonObjectFeature DeserializeFeature(const TSharedPtr<FJsonObject>& Feature);

        /**
        * @brief Deserializes a JSON object into a FeatureCollection structure.
        *
        * @param FeatureCollection The GeoJSON feature object to deserialize.
        * @return The FJsonFeatureCollection structure containing the deserialized data.
        */
        static FJsonFeatureCollection DeserializeFeatureCollection(const TSharedPtr<FJsonObject>& FeatureCollection);

        /**
        * Deserializes all JSON object properties into string key - string value
        * The key can be simple (justKey) or multi-part (mainProperty.subProperty, first.second.third)
        * If the field is an array, the different elements are separated by the array index: (array.0.key, array.1.key, ...)
        * The value is always a string: "SomeString", "42", "3.14", "true"
        *
        * @param Values GeoJSON property container
        * @return TMap container containing the deserialized property values
        */
        static TMap<FString, FString> DeserializeProperties(const TMap<FString, TSharedPtr<FJsonValue>>& Values);

private:
        /**
        * @brief Deserializes a JSON array of values into a FeatureCollection structure.
        *
        * @param Features An array of JSON values representing the FeatureCollection.
        * @return A FeatureCollection structure containing the deserialized data.
        */
        static TArray<FJsonObjectFeature> DeserializeFeatures(const TArray<TSharedPtr<FJsonValue>>& Features);

        /**
        * Deserializes the feature and adds it to the provided container.
        *
        * @param Container The container to add the deserialized property to.
        * @param Key The key that will be associated with the deserialized property in the container.
        * @param Value A pointer to an FJsonValue representing the value of the property to be deserialized.
        */
        static void DeserializeProperty(TMap<FString, FString>& Container, const FString& Key, const TSharedPtr<FJsonValue>& Value);

        /**
        * Deserializes an array property and adds it to the specified container.
        * Each element in the array is deserialized using the DeserializeProperty method.
        *
        * @param Container The container to store the deserialized properties in.
        * @param PrevKey The key of the previous property in the container.
        * @param Array An array of JSON values to deserialize.
        */
        static void DeserializeArrayProperty(TMap<FString, FString>& Container, const FString& PrevKey, const TArray<TSharedPtr<FJsonValue>>& Array);

        /**
        * Deserializes an object property from a GeoJSON file.
        *
        * @param Container The Map container to store the deserialized properties in.
        * @param PrevKey The key to use as the base for the deserialized properties. New keys will be formed by concatenating
        * PrevKey and the current property key.
        * @param Object JSON object containing the properties to deserialize.
        */
        static void DeserializeObjectProperty(TMap<FString, FString>& Container, const FString& PrevKey, const TSharedPtr<FJsonObject>& Object);
};