// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeoJsonDataStructures/GeoJsonWrapperTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeoJsonSerializer.generated.h"

struct FJsonObjectFeature;
struct FJsonFeatureCollection;
struct FJsonObjectWrapper;
struct FJsonPoint;
struct FJsonMultiPoint;
struct FJsonPolygon;
struct FJsonMultiPolygon;
/**
 *
 */
UCLASS()
class GEOJSONWRAPPER_API UGeoJsonSerializer : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

public:
        /**
        * @brief Serializes this JSON object to a string.
        *
        * @param JsonObject The JSON object to be serialized.
        * @return The serialized JSON object as a string.
        */
        UFUNCTION(BlueprintCallable, Category = "GeoJSON")
        static FString SerializeObjectToString(const FJsonObjectWrapper& JsonObject);

        /**
        * @brief Serializes this JSON object to a string.
        *
        * @param JsonObject The JSON object to be serialized.
        * @return The serialized JSON object as a string.
        */
        static FString SerializeObjectToString(const TSharedPtr<FJsonObject>& JsonObject);

        /**
        * @brief Serializes the feature collection structure to a JSON object.
        * @param FeatureCollection The feature collection structure to serialize.
        * @param bSerializeZ If true, serialize 3D geometry, otherwise 2D.
        * @return The serialized JSON object.
        */
        static TSharedPtr<FJsonObject> SerializeFeatureCollection(const FJsonFeatureCollection& FeatureCollection, bool bSerializeZ = true);

        /**
        * @brief Serialize the feature structure to a JSON object.
        * @param Feature The feature structure to serialize.
        * @param bSerializeZ If true, serialize 3D geometry, otherwise 2D.
        * @return The serialized JSON object.
        */
        static TSharedPtr<FJsonObject> SerializeFeature(const FJsonObjectFeature& Feature, bool bSerializeZ = true);

        /**
        * @brief Serializes a GeoJSON geometry to a JSON object.
        * @param GeometryType The type of geometry to serialize.
        * @param Geometry The geometry to serialize.
        * @param bSerializeZ If true, serializes a 3D geometry, otherwise a 2D geometry.
        * @return The serialized JSON object.
        */
        static TSharedPtr<FJsonObject> SerializeGeometry(EGeoJsonType GeometryType, const FJsonGeometry& Geometry, bool bSerializeZ = true);

        /**
        * @brief Serializes an FVector object to a JSON array.
        *
        * @param Vector The FVector object to serialize.
        * @param bSerializeZ If true, serialize 3D geometry, otherwise 2D
        * @return An array of JSON values representing the serialized FVector object.
        */
        static TArray<TSharedPtr<FJsonValue>> SerializeVector(const FVector& Vector, bool bSerializeZ = true);

        /**
        * @brief Serialize the FRotator object to an array of FJsonValue objects.
        *
        * @param Rotation The FRotator object to serialize.
        * @return An array of JSON values representing the serialized FRotator.
        */
        static TArray<TSharedPtr<FJsonValue>> SerializeRotator(const FRotator& Rotation);

        /**
        * @brief Serialize a GeoJSON point.
        *
        * @param Point The point to serialize.
        * @param bSerializeZ If true, serialize a 3D geometry, otherwise a 2D geometry
        * @return An array of JSON values representing the point.
        */
        static TArray<TSharedPtr<FJsonValue>> SerializePoint(const FJsonPoint& Point, bool bSerializeZ = true);

        /**
        * @brief Serialize a GeoJSON multipoint.
        *
        * @param MultiPoint The multipoint to serialize.
        * @param bSerializeZ If true, serialize a 3D geometry, otherwise a 2D geometry
        * @return An array of JSON values representing the multipoint.
        */
        static TArray<TSharedPtr<FJsonValue>> SerializeMultiPoint(const FJsonMultiPoint& MultiPoint, bool bSerializeZ = true);

        /**
        * @brief Serializes a GeoJSON polygon.
        *
        * @param Polygon The polygon to serialize.
        * @param bSerializeZ If true, serializes a 3D geometry, otherwise a 2D geometry
        * @return An array of JSON values representing the polygon.
        */
        static TArray<TSharedPtr<FJsonValue>> SerializePolygon(const FJsonPolygon& Polygon, bool bSerializeZ = true);

        /**
        * @brief Serializes a GeoJSON multipolygon.
        *
        * @param MultiPolygon The multipolygon to serialize.
        * @param bSerializeZ If true, serializes a 3D geometry, otherwise a 2D geometry.
        * @return An array of JSON values representing the multipolygon.
        */
        static TArray<TSharedPtr<FJsonValue>> SerializeMultiPolygon(const FJsonMultiPolygon& MultiPolygon, bool bSerializeZ = true);

        /**
        * @brief Serializes key-value properties to a JSON array
        *
        * @param Properties The properties to serialize
        * @param Prefix The prefix of the key to serialize
        * @return The serialized JSON array
        */
        static TArray<TSharedPtr<FJsonValue>> SerializeArray(const TMap<FString, FString>& Properties, const FString& Prefix);

        /**
        * @brief Serializes key-value properties to a JSON object
        *
        * @param Properties The properties to serialize
        * @param Prefix The prefix of the key to serialize
        * @return The serialized JSON object
        */
        static TSharedPtr<FJsonObject> SerializeObject(const TMap<FString, FString>& Properties, const FString& Prefix);

private:
        /**
        * @brief Serializes the given geometry coordinates using the provided sub-geometry serialization function.
        *
        * @param Coords The coordinates of the geometry to serialize.
        * @param SubGeomSerializer The sub-geometry serialization function.
        * @param bSerializeZ If true, serializes a 3D geometry, otherwise a 2D geometry.
        *
        * @return An array of shared pointers pointing to JSON values representing the serialized geometry.
        */
        template <typename FJsonType, typename FSerializer>
        static TArray<TSharedPtr<FJsonValue>> SerializeGeometry(const FJsonType& Coords, const FSerializer& SubGeomSerializer, bool bSerializeZ = true);
};

template <typename FJsonType, typename FSerializer>
TArray<TSharedPtr<FJsonValue>> UGeoJsonSerializer::SerializeGeometry(const FJsonType& Coords, const FSerializer& SubGeomSerializer, bool bSerializeZ)
{
        TArray<TSharedPtr<FJsonValue>> Result;
        for (const auto& SubCoordinates : Coords.Coordinates)
        {
                Result.Add(MakeShared<FJsonValueArray>(SubGeomSerializer(SubCoordinates, bSerializeZ)));
        }
        return Result;
}