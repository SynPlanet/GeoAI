// META all rights received (c)

#pragma once

#include "GeoJsonWrapperTypes.generated.h"
/**
* @enum EGeoJsonType
* @brief Enumeration representing different types of GeoJSON objects.
* @details This enumeration is used to represent different types of GeoJSON objects, such as Point, LineString, Polygon, etc.
* Each enumeration value corresponds to a specific GeoJSON type and can be used to determine the type of GeoJSON object.
*/
UENUM(BlueprintType)
enum class EGeoJsonType : uint8
{
        None,
        Point,
        LineString,
        Polygon,
        MultiPoint,
        MultiLineString,
        MultiPolygon,
        GeometryCollection,
        Feature,
        FeatureCollection
};

/**
* @struct FJsonPoint
* @brief Structure representing a point in GeoJSON.
*/
USTRUCT(BlueprintType)
struct FJsonPoint
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        FVector Coordinates = FVector::ZeroVector;

        /**
        * @brief Comparison operator
        * @return True if coordinates match, false otherwise
        */
        bool operator==(const FJsonPoint& Other) const { return Coordinates == Other.Coordinates; }
};

/**
* @struct FJsonMultiPoint
* @brief Structure representing a multipoint or line in GeoJSON.
*/
USTRUCT(BlueprintType)
struct FJsonMultiPoint
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TArray<FJsonPoint> Coordinates;
};

/**
* @struct FJsonPolygon
* @brief A structure representing a GeoJSON multiline or polygon.
*/
USTRUCT(BlueprintType)
struct FJsonPolygon
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TArray<FJsonMultiPoint> Coordinates;
};

/**
* @struct FJsonMultiPolygon
* @brief A structure representing a GeoJSON multipolygon.
*/
USTRUCT(BlueprintType)
struct FJsonMultiPolygon
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TArray<FJsonPolygon> Coordinates;
};

using FGeometryType = TVariant<FJsonPoint, FJsonMultiPoint, FJsonPolygon, FJsonMultiPolygon>;

/**
* @struct FJsonGeometry
* @brief A structure representing a generic GeoJSON geometry type.
*/
USTRUCT(BlueprintType)
struct FJsonGeometry
{
        GENERATED_BODY()

        FGeometryType Geometry;
};

/**
* @struct FJsonGeometryCollection
* @brief A structure representing a GeoJSON collection of geometries.
*/
USTRUCT(BlueprintType)
struct FJsonGeometryCollection
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TArray<EGeoJsonType> Types;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TArray<FJsonGeometry> Geometries;
};

/**
* @struct FJsonObjectFeature
* @brief Structure representing a GeoJSON Feature.
*
* A Feature object consists of a geometry, its type, and additional geometry properties
*/
USTRUCT(BlueprintType)
struct FJsonObjectFeature
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        EGeoJsonType GeometryType = EGeoJsonType::None;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        FJsonGeometry Geometry;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TMap<FString, FString> Properties;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TMap<FString, FString> OtherFields;

        /**
        * @brief A template getter function for geometries of various types
        *
        * @param Result An output parameter where the retrieved geometry will be stored
        * @return True if the template geometry was successfully retrieved, false otherwise.
        */
        template <EGeoJsonType FeatureGeometryType, typename TValueType>
        bool TryGetTemplateGeometry(TValueType& Result) const;

        /**
        * @brief Looks up a value with the specified key in properties and additional parameters and casts it to the specified type.
        *
        * @tparam TValueType The type of the value to look up.
        * @param Key The key of the value to look up.
        * @param Default The default value to return if the specified key is not present in the container.
        * @return The value corresponding to the given key and cast to the given type if the key is found, otherwise the default value.
        */
        template <typename TValueType>
        TValueType GetPropertyWithDefault(const FString& Key, const TValueType& Default) const;

        /**
        * @brief Looks up a value with the specified key in the properties and additional parameters and casts it to the specified type.
        *
        * @tparam TValueType The type of the value to look up.
        * @param Key The key of the value to look up.
        * @param Result The object to write the result to.
        * @return True if the object with the specified key is found, otherwise false.
        */
        template <typename TValueType>
        bool TryGetProperty(const FString& Key, TValueType& Result) const;
};

/**
* @struct FJsonFeatureCollection
* @brief A structure representing a collection of GeoJSON Feature objects.
*/
USTRUCT(BlueprintType)
struct FJsonFeatureCollection
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TArray<FJsonObjectFeature> Features;

        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeoJSON")
        TMap<FString, FString> OtherFields;

        template <typename TValueType>
        TValueType GetFieldWithDefault(const FString& Key, const TValueType& Default) const;

        template <typename TValueType>
        bool TryGetField(const FString& Key, TValueType& Result) const;
};

/**
* @brief Looks up a value with the specified key in the specified container and casts it to the specified type.
*
* @tparam TValueType The type of the value to search for.
* @param Map The container in which the value with the specified key is searched for.
* @param Key The key of the value to search for.
* @param Result The object to which the result will be written.
* @return True if the object with the specified key is found, otherwise - false.
*/
template <typename TValueType>
bool GetValue(const TMap<FString, FString>& Map, const FString& Key, TValueType& Result)
{
        if (const FString* const Value = Map.Find(Key); Value)
        {
                LexFromString(Result, **Value);
                return true;
        }
        return false;
}

/**
* @brief Searches the container for a value with the specified key and casts it to the specified type. If the specified key is missing, returns the default value.
*
* @tparam TValueType The type of the value to search for.
* @param Map The container in which to search for the value with the specified key.
* @param Key The key of the value to search for.
* @param Default The default value returned if the specified key is not found in the container.
* @return The value corresponding to the specified key and cast to the specified type if the key is found, otherwise the default value.
*/
template <typename TValueType>
TValueType GetValueWithDefault(const TMap<FString, FString>& Map, const FString& Key, const TValueType& Default)
{
        if (TValueType Result; GetValue(Map, Key, Result))
        {
                return Result;
        }
        return Default;
}

template <EGeoJsonType FeatureGeometryType, typename TValueType>
bool FJsonObjectFeature::TryGetTemplateGeometry(TValueType& Result) const
{
        if (GeometryType != FeatureGeometryType)
        {
                return false;
        }

        Result = Geometry.Geometry.Get<TValueType>();
        return true;
}

template <typename TValueType>
TValueType FJsonObjectFeature::GetPropertyWithDefault(const FString& Key, const TValueType& Default) const
{
        if (TValueType Result; GetValue(Properties, Key, Result) || GetValue(OtherFields, Key, Result))
        {
                return Result;
        }

        return Default;
}

template <typename TValueType>
bool FJsonObjectFeature::TryGetProperty(const FString& Key, TValueType& Result) const
{
        return GetValue(Properties, Key, Result) || GetValue(OtherFields, Key, Result);
}

template <typename TValueType>
TValueType FJsonFeatureCollection::GetFieldWithDefault(const FString& Key, const TValueType& Default) const
{
        return GetValueWithDefault(OtherFields, Key, Default);
}

template <typename TValueType>
bool FJsonFeatureCollection::TryGetField(const FString& Key, TValueType& Result) const
{
        return GetValue(OtherFields, Key, Result);
}