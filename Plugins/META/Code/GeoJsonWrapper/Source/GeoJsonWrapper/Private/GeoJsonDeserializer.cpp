// META all rights received (c)

#include "GeoJsonDeserializer.h"

#include "JsonObjectWrapper.h"
#include "GeoJsonDataStructures/GeoJsonFeatureCollection.h"
#include "GeoJsonDataStructures/GeoJsonGeometryCollection.h"
#include "GeoJsonDataStructures/GeoJsonLineString.h"
#include "GeoJsonDataStructures/GeoJsonMultiLineString.h"
#include "GeoJsonDataStructures/GeoJsonMultiPoint.h"
#include "GeoJsonDataStructures/GeoJsonMultiPolygon.h"
#include "GeoJsonDataStructures/GeoJsonObjectFeature.h"
#include "GeoJsonDataStructures/GeoJsonPoint.h"
#include "GeoJsonDataStructures/GeoJsonPolygon.h"
#include "Serialization/JsonReader.h"

namespace
{
        const TMap<FString, EGeoJsonType> StringToTypeMap{ { TEXT("Point"), EGeoJsonType::Point },
                                                           { TEXT("LineString"), EGeoJsonType::LineString },
                                                           { TEXT("Polygon"), EGeoJsonType::Polygon },
                                                           { TEXT("MultiPoint"), EGeoJsonType::MultiPoint },
                                                           { TEXT("MultiPolygon"), EGeoJsonType::MultiPolygon },
                                                           { TEXT("MultiLineString"), EGeoJsonType::MultiLineString },
                                                           { TEXT("GeometryCollection"), EGeoJsonType::GeometryCollection },
                                                           { TEXT("ObjectFeature"), EGeoJsonType::Feature },
                                                           { TEXT("Feature"), EGeoJsonType::Feature },
                                                           { TEXT("FeatureCollection"), EGeoJsonType::FeatureCollection } };

        EGeoJsonType StringToType(const FString& String)
        {
                if (const EGeoJsonType* const Type = StringToTypeMap.Find(String); Type)
                {
                        return *Type;
                }
                return EGeoJsonType::None;
        }

        template <typename TGeoJsonObject, typename TDeserializer>
        TGeoJsonObject* DeserializeCoordinates(const TDeserializer& Deserializer, const TSharedPtr<FJsonObject>& Object)
        {
                TGeoJsonObject* const GeoJsonObject = NewObject<TGeoJsonObject>();
                GeoJsonObject->SetCoordinates(Deserializer(Object->GetArrayField(TEXT("coordinates"))));
                return GeoJsonObject;
        }

        template <typename TGeoJsonObject, typename TDeserializer>
        TGeoJsonObject* DeserializeCollection(const TDeserializer& Deserializer, const TSharedPtr<FJsonObject>& Object, const FString& CollectionName)
        {
                TGeoJsonObject* const GeoJsonObject = NewObject<TGeoJsonObject>();
                GeoJsonObject->SetCollection(Deserializer(Object->GetArrayField(CollectionName)));
                return GeoJsonObject;
        }
} // namespace

UGeoJsonObject* UGeoJsonDeserializer::DeserializePath(const FString& PathToJsonSource)
{
        FString JsonString;
        FFileHelper::LoadFileToString(JsonString, *PathToJsonSource);
        return DeserializeLoaded(JsonString);
}

UGeoJsonObject* UGeoJsonDeserializer::DeserializeLoaded(const FString& JsonString)
{
        if (const TSharedPtr<FJsonObject> JsonObject = Deserialize(JsonString); JsonObject)
        {
                return DeserializeObject(JsonObject);
        }

        return nullptr;
}

TMap<FString, FString> UGeoJsonDeserializer::DeserializeJson(const FString& JsonString)
{
        if (const TSharedPtr<FJsonObject> JsonObject = Deserialize(JsonString); JsonObject)
        {
                return DeserializeProperties(JsonObject->Values);
        }

        return {};
}

EGeoJsonType UGeoJsonDeserializer::DeserializeGeometry(const FJsonObjectWrapper& ObjectInfo, const FString& Field, FJsonGeometry& Geometry)
{
        EGeoJsonType Result = EGeoJsonType::None;
        FJsonObjectWrapper ObjectWrapper;
        if (ObjectWrapper.JsonObjectFromString(ObjectInfo.JsonObject->GetStringField(Field)))
        {
                Tie(Result, Geometry) = DeserializeGeometry(ObjectWrapper.JsonObject);
        }
        return Result;
}

bool UGeoJsonDeserializer::DeserializePointGeometry(const FJsonObjectWrapper& ObjectInfo, const FString& Field, FJsonPoint& Point)
{
        FJsonObjectWrapper ObjectWrapper;
        if (ObjectWrapper.JsonObjectFromString(ObjectInfo.JsonObject->GetStringField(Field)))
        {
                Point = DeserializePoint(ObjectWrapper.JsonObject->GetArrayField(TEXT("coordinates")));
                return true;
        }
        return false;
}

TSharedPtr<FJsonObject> UGeoJsonDeserializer::Deserialize(const FString& JsonString)
{
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
        TSharedPtr<FJsonObject> JsonObject;
        return FJsonSerializer::Deserialize(Reader, JsonObject) ? JsonObject : nullptr;
}

UGeoJsonObject* UGeoJsonDeserializer::DeserializeObject(const TSharedPtr<FJsonObject>& JsonObject)
{
        switch (StringToType(JsonObject->GetStringField(TEXT("type"))))
        {
                case EGeoJsonType::Point:
                        {
                                return DeserializeCoordinates<UGeoJsonPoint>(&UGeoJsonDeserializer::DeserializePoint, JsonObject);
                        }
                case EGeoJsonType::LineString:
                        {
                                return DeserializeCoordinates<UGeoJsonLineString>(&UGeoJsonDeserializer::DeserializeMultiPoint, JsonObject);
                        }
                case EGeoJsonType::Polygon:
                        {
                                return DeserializeCoordinates<UGeoJsonPolygon>(&UGeoJsonDeserializer::DeserializePolygon, JsonObject);
                        }
                case EGeoJsonType::MultiPoint:
                        {
                                return DeserializeCoordinates<UGeoJsonMultiPoint>(&UGeoJsonDeserializer::DeserializeMultiPoint, JsonObject);
                        }
                case EGeoJsonType::MultiLineString:
                        {
                                return DeserializeCoordinates<UGeoJsonMultiLineString>(&UGeoJsonDeserializer::DeserializePolygon, JsonObject);
                        }
                case EGeoJsonType::MultiPolygon:
                        {
                                return DeserializeCoordinates<UGeoJsonMultiPolygon>(&UGeoJsonDeserializer::DeserializeMultiPolygon, JsonObject);
                        }
                case EGeoJsonType::GeometryCollection:
                        {
                                return DeserializeCollection<UGeoJsonGeometryCollection>(
                                        &UGeoJsonDeserializer::DeserializeGeometryCollection, JsonObject, "geometries");
                        }
                case EGeoJsonType::Feature:
                        {
                                UGeoJsonObjectFeature* const Feature = NewObject<UGeoJsonObjectFeature>();
                                Feature->SetFeature(DeserializeFeature(JsonObject));
                                return Feature;
                        }
                case EGeoJsonType::FeatureCollection:
                        {
                                UGeoJsonFeatureCollection* const FeatureCollection = NewObject<UGeoJsonFeatureCollection>();
                                FeatureCollection->SetCollection(DeserializeFeatureCollection(JsonObject));
                                return FeatureCollection;
                        }
                default:
                        return nullptr;
        }
}

FJsonPoint UGeoJsonDeserializer::DeserializePoint(const TArray<TSharedPtr<FJsonValue>>& Coordinates)
{
        FJsonPoint Result{ FVector::ZeroVector };
        check(Coordinates.Num() <= 3) // expect only 2D or 3D points
                for (int32 Idx = 0; Idx < Coordinates.Num(); ++Idx)
        {
                Result.Coordinates[Idx] = Coordinates[Idx]->AsNumber();
        }
        return Result;
}

FJsonMultiPoint UGeoJsonDeserializer::DeserializeMultiPoint(const TArray<TSharedPtr<FJsonValue>>& Coordinates)
{
        FJsonMultiPoint Result;
        for (const auto& PointCoordinates : Coordinates)
        {
                Result.Coordinates.Add(DeserializePoint(PointCoordinates->AsArray()));
        }
        return Result;
}

FJsonPolygon UGeoJsonDeserializer::DeserializePolygon(const TArray<TSharedPtr<FJsonValue>>& Coordinates)
{
        FJsonPolygon Result;
        for (const auto& PolylineCoordinates : Coordinates)
        {
                Result.Coordinates.Add(DeserializeMultiPoint(PolylineCoordinates->AsArray()));
        }
        return Result;
}

FJsonMultiPolygon UGeoJsonDeserializer::DeserializeMultiPolygon(const TArray<TSharedPtr<FJsonValue>>& Coordinates)
{
        FJsonMultiPolygon Result;
        for (const auto& PolygonCoordinates : Coordinates)
        {
                Result.Coordinates.Add(DeserializePolygon(PolygonCoordinates->AsArray()));
        }
        return Result;
}

TPair<EGeoJsonType, FJsonGeometry> UGeoJsonDeserializer::DeserializeGeometry(const TSharedPtr<FJsonObject>& Geometry)
{
        const EGeoJsonType GeometryType = StringToType(Geometry->GetStringField(TEXT("type")));
        const TArray<TSharedPtr<FJsonValue>>& Coordinates = Geometry->GetArrayField(TEXT("coordinates"));
        switch (GeometryType)
        {
                case EGeoJsonType::Point:
                        return { GeometryType, FJsonGeometry{ FGeometryType(TInPlaceType<FJsonPoint>(), DeserializePoint(Coordinates)) } };
                case EGeoJsonType::LineString:
                case EGeoJsonType::MultiPoint:
                        return { GeometryType, FJsonGeometry{ FGeometryType(TInPlaceType<FJsonMultiPoint>(), DeserializeMultiPoint(Coordinates)) } };
                case EGeoJsonType::Polygon:
                case EGeoJsonType::MultiLineString:
                        return { GeometryType, FJsonGeometry{ FGeometryType(TInPlaceType<FJsonPolygon>(), DeserializePolygon(Coordinates)) } };
                case EGeoJsonType::MultiPolygon:
                        return { GeometryType, FJsonGeometry{ FGeometryType(TInPlaceType<FJsonMultiPolygon>(), DeserializeMultiPolygon(Coordinates)) } };
                default:
                        checkf(false, TEXT("Wrong geometry type"))
        }
        return { EGeoJsonType::None, FJsonGeometry() };
}

FJsonGeometryCollection UGeoJsonDeserializer::DeserializeGeometryCollection(const TArray<TSharedPtr<FJsonValue>>& Geometries)
{
        FJsonGeometryCollection Result;
        for (const auto& Geometry : Geometries)
        {
                const auto [Type, Geom] = DeserializeGeometry(Geometry->AsObject());
                Result.Geometries.Add(Geom);
                Result.Types.Add(Type);
        }
        return Result;
}

FJsonObjectFeature UGeoJsonDeserializer::DeserializeFeature(const TSharedPtr<FJsonObject>& Feature)
{
        FJsonObjectFeature Result;
        for (const auto& [Key, JsonValue] : Feature->Values)
        {
                if (Key == "geometry")
                {
                        Tie(Result.GeometryType, Result.Geometry) = DeserializeGeometry(JsonValue->AsObject());
                }
                else if (Key == "properties")
                {
                        Result.Properties = DeserializeProperties(JsonValue->AsObject()->Values);
                }
                else
                {
                        DeserializeProperty(Result.OtherFields, Key, JsonValue);
                }
        }

        return Result;
}

FJsonFeatureCollection UGeoJsonDeserializer::DeserializeFeatureCollection(const TSharedPtr<FJsonObject>& FeatureCollection)
{
        FJsonFeatureCollection Result;
        for (const auto& [Key, JsonValue] : FeatureCollection->Values)
        {
                if (Key == "features")
                {
                        Result.Features = DeserializeFeatures(JsonValue->AsArray());
                }
                else
                {
                        DeserializeProperty(Result.OtherFields, Key, JsonValue);
                }
        }
        return Result;
}

TMap<FString, FString> UGeoJsonDeserializer::DeserializeProperties(const TMap<FString, TSharedPtr<FJsonValue>>& Values)
{
        TMap<FString, FString> Result;
        for (const auto& [Key, Value] : Values)
        {
                DeserializeProperty(Result, Key, Value);
        }
        return Result;
}

TArray<FJsonObjectFeature> UGeoJsonDeserializer::DeserializeFeatures(const TArray<TSharedPtr<FJsonValue>>& Features)
{
        TArray<FJsonObjectFeature> Result;
        for (const auto& Feature : Features)
        {
                Result.Add(DeserializeFeature(Feature->AsObject()));
        }
        return Result;
}

void UGeoJsonDeserializer::DeserializeProperty(TMap<FString, FString>& Container, const FString& Key, const TSharedPtr<FJsonValue>& Value)
{
        switch (Value->Type)
        {
                case EJson::Array:
                        DeserializeArrayProperty(Container, Key, Value->AsArray());
                        break;
                case EJson::Object:
                        DeserializeObjectProperty(Container, Key, Value->AsObject());
                        break;
                case EJson::Boolean:
                case EJson::Number:
                case EJson::String:
                        Container.Add(Key, Value->AsString());
                        break;
                default:
                        break;
        }
}

void UGeoJsonDeserializer::DeserializeArrayProperty(TMap<FString, FString>& Container, const FString& PrevKey, const TArray<TSharedPtr<FJsonValue>>& Array)
{
        for (int32 Idx = 0; Idx < Array.Num(); ++Idx)
        {
                const FString Key = PrevKey + "." + FString::FormatAsNumber(Idx);
                DeserializeProperty(Container, Key, Array[Idx]);
        }
}

void UGeoJsonDeserializer::DeserializeObjectProperty(TMap<FString, FString>& Container, const FString& PrevKey, const TSharedPtr<FJsonObject>& Object)
{
        for (const auto& [Key, Value] : Object->Values)
        {
                const FString NewKey = PrevKey + "." + Key;
                DeserializeProperty(Container, NewKey, Value);
        }
}