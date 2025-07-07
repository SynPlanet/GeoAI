// META all rights received (c)

#include "GeoJsonSerializer.h"

#include "JsonObjectWrapper.h"
#include "Algo/AnyOf.h"
#include "GeoJsonDataStructures/GeoJsonWrapperTypes.h"

namespace
{
        TMap<EGeoJsonType, FString> TypeToStringMap = { { EGeoJsonType::Point, TEXT("Point") },
                                                        { EGeoJsonType::LineString, TEXT("LineString") },
                                                        { EGeoJsonType::Polygon, TEXT("Polygon") },
                                                        { EGeoJsonType::MultiPoint, TEXT("MultiPoint") },
                                                        { EGeoJsonType::MultiPolygon, TEXT("MultiPolygon") },
                                                        { EGeoJsonType::MultiLineString, TEXT("MultiLineString") },
                                                        { EGeoJsonType::GeometryCollection, TEXT("GeometryCollection") },
                                                        { EGeoJsonType::Feature, TEXT("Feature") },
                                                        { EGeoJsonType::FeatureCollection, TEXT("FeatureCollection") } };

        TMap<FString, FString> SubMap(const TMap<FString, FString>& Map, const FString& ParentKey)
        {
                if (ParentKey.IsEmpty())
                {
                        return Map;
                }

                TMap<FString, FString> Result;
                for (const auto& [Key, Value] : Map)
                {
                        FString RightKey;
                        FString LeftKey;
                        if (Key.Split(ParentKey, &LeftKey, &RightKey) && !RightKey.IsEmpty())
                        {
                                if (!RightKey.StartsWith(".") || !LeftKey.IsEmpty())
                                {
                                        continue;
                                }
                                RightKey.RightChopInline(1);
                                Result.Emplace(RightKey, Value);
                        }
                }
                return Result;
        }

        bool StartsWithNumber(const FString& String)
        {
                const FString FirstLetter = String.LeftChop(String.Len() - 1);
                int32 Number;
                LexFromString(Number, *FirstLetter);
                return FirstLetter == LexToString(Number);
        }

        bool IsJsonArray(const TMap<FString, FString>& Map)
        {
                return Algo::AnyOf(Map, [](const TTuple<FString, FString>& Pair) { return StartsWithNumber(Pair.Key); });
        }
} // namespace

FString UGeoJsonSerializer::SerializeObjectToString(const FJsonObjectWrapper& JsonObject)
{
        return SerializeObjectToString(JsonObject.JsonObject);
}

FString UGeoJsonSerializer::SerializeObjectToString(const TSharedPtr<FJsonObject>& JsonObject)
{
        FString Result;
        const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Result);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
        return Result;
}

TSharedPtr<FJsonObject> UGeoJsonSerializer::SerializeFeatureCollection(const FJsonFeatureCollection& FeatureCollection, bool bSerializeZ)
{
        FJsonObject Result;
        Result.SetStringField(TEXT("type"), TypeToStringMap.FindChecked(EGeoJsonType::FeatureCollection));
        TArray<TSharedPtr<FJsonValue>> Features;
        for (const FJsonObjectFeature& Feature : FeatureCollection.Features)
        {
                Features.Emplace(MakeShared<FJsonValueObject>(SerializeFeature(Feature, bSerializeZ)));
        }
        Result.SetArrayField(TEXT("features"), Features);
        for (TSharedPtr<FJsonObject> OtherFields = SerializeObject(FeatureCollection.OtherFields, {}); const auto& [Key, Value] : OtherFields->Values)
        {
                Result.SetField(Key, Value);
        }
        return MakeShared<FJsonObject>(Result);
}

TSharedPtr<FJsonObject> UGeoJsonSerializer::SerializeFeature(const FJsonObjectFeature& Feature, bool bSerializeZ)
{
        FJsonObject Result;
        Result.SetStringField(TEXT("type"), TypeToStringMap.FindChecked(EGeoJsonType::Feature));
        Result.SetObjectField(TEXT("geometry"), SerializeGeometry(Feature.GeometryType, Feature.Geometry, bSerializeZ));
        if (const TSharedPtr<FJsonObject> Properties = SerializeObject(Feature.Properties, {}); Properties && !Properties->Values.IsEmpty())
        {
                Result.SetObjectField(TEXT("properties"), Properties);
        }
        return MakeShared<FJsonObject>(Result);
}

TSharedPtr<FJsonObject> UGeoJsonSerializer::SerializeGeometry(EGeoJsonType GeometryType, const FJsonGeometry& Geometry, bool bSerializeZ)
{
        FJsonObject Result;
        Result.SetStringField(TEXT("type"), TypeToStringMap.FindChecked(GeometryType));

        TArray<TSharedPtr<FJsonValue>> SerializedGeometry;
        if (const FJsonPoint* const Point = Geometry.Geometry.TryGet<FJsonPoint>(); Point)
        {
                SerializedGeometry = SerializePoint(*Point, bSerializeZ);
        }
        else if (const FJsonMultiPoint* const MultiPoint = Geometry.Geometry.TryGet<FJsonMultiPoint>(); MultiPoint)
        {
                SerializedGeometry = SerializeMultiPoint(*MultiPoint, bSerializeZ);
        }
        else if (const FJsonPolygon* const Polygon = Geometry.Geometry.TryGet<FJsonPolygon>(); Polygon)
        {
                SerializedGeometry = SerializePolygon(*Polygon, bSerializeZ);
        }
        else
        {
                SerializedGeometry = SerializeMultiPolygon(Geometry.Geometry.Get<FJsonMultiPolygon>(), bSerializeZ);
        }
        Result.SetArrayField(TEXT("coordinates"), SerializedGeometry);
        return MakeShared<FJsonObject>(Result);
}

TArray<TSharedPtr<FJsonValue>> UGeoJsonSerializer::SerializeVector(const FVector& Vector, bool bSerializeZ)
{
        if (bSerializeZ)
        {
                return { MakeShared<FJsonValueNumber>(Vector.X), MakeShared<FJsonValueNumber>(Vector.Y), MakeShared<FJsonValueNumber>(Vector.Z) };
        }
        return { MakeShared<FJsonValueNumber>(Vector.X), MakeShared<FJsonValueNumber>(Vector.Y) };
}

TArray<TSharedPtr<FJsonValue>> UGeoJsonSerializer::SerializeRotator(const FRotator& Rotation)
{
        return SerializeVector({ Rotation.Roll, Rotation.Pitch, Rotation.Yaw });
}

TArray<TSharedPtr<FJsonValue>> UGeoJsonSerializer::SerializePoint(const FJsonPoint& Point, bool bSerializeZ)
{
        return SerializeVector(Point.Coordinates, bSerializeZ);
}

TArray<TSharedPtr<FJsonValue>> UGeoJsonSerializer::SerializeMultiPoint(const FJsonMultiPoint& MultiPoint, bool bSerializeZ)
{
        return SerializeGeometry(MultiPoint, &UGeoJsonSerializer::SerializePoint, bSerializeZ);
}

TArray<TSharedPtr<FJsonValue>> UGeoJsonSerializer::SerializePolygon(const FJsonPolygon& Polygon, bool bSerializeZ)
{
        return SerializeGeometry(Polygon, &UGeoJsonSerializer::SerializeMultiPoint, bSerializeZ);
}

TArray<TSharedPtr<FJsonValue>> UGeoJsonSerializer::SerializeMultiPolygon(const FJsonMultiPolygon& MultiPolygon, bool bSerializeZ)
{
        return SerializeGeometry(MultiPolygon, &UGeoJsonSerializer::SerializePolygon, bSerializeZ);
}

TArray<TSharedPtr<FJsonValue>> UGeoJsonSerializer::SerializeArray(const TMap<FString, FString>& Properties, const FString& Prefix)
{
        TArray<TSharedPtr<FJsonValue>> Result;
        for (int32 Idx = 0;; ++Idx)
        {
                const FString ParentKey = Prefix.IsEmpty() ? FString::Printf(TEXT("%d"), Idx) : FString::Printf(TEXT("%ls.%d"), *Prefix, Idx);
                if (const FString* const ExactValue = Properties.Find(ParentKey); ExactValue)
                {
                        if (ExactValue->IsNumeric())
                        {
                                Result.Emplace(MakeShared<FJsonValueNumberString>(*ExactValue));
                        }
                        else
                        {
                                Result.Emplace(MakeShared<FJsonValueString>(*ExactValue));
                        }
                }
                else if (const TMap<FString, FString> ChildMap = SubMap(Properties, ParentKey); !ChildMap.IsEmpty())
                {
                        if (IsJsonArray(ChildMap))
                        {
                                Result.Emplace(MakeShared<FJsonValueArray>(SerializeArray(ChildMap, "")));
                        }
                        else
                        {
                                Result.Emplace(MakeShared<FJsonValueObject>(SerializeObject(ChildMap, "")));
                        }
                }
                else
                {
                        break;
                }
        }
        return Result;
}

TSharedPtr<FJsonObject> UGeoJsonSerializer::SerializeObject(const TMap<FString, FString>& Properties, const FString& Prefix)
{
        FJsonObject Result;
        const TMap<FString, FString> ChildMap = SubMap(Properties, Prefix);
        for (const auto& [Key, Value] : ChildMap)
        {
                FString LeftStr;
                FString RightStr;
                if (Key.Split(".", &LeftStr, &RightStr))
                {
                        if (RightStr.StartsWith("."))
                        {
                                RightStr.RightChopInline(1);
                        }
                        if (Result.HasField(LeftStr))
                        {
                                continue;
                        }

                        if (StartsWithNumber(RightStr))
                        {
                                Result.SetArrayField(LeftStr, SerializeArray(ChildMap, LeftStr));
                        }
                        else
                        {
                                Result.SetObjectField(LeftStr, SerializeObject(ChildMap, LeftStr));
                        }
                }
                else
                {
                        Result.SetField(Key, MakeShared<FJsonValueString>(Value));
                }
        }
        return MakeShared<FJsonObject>(Result);
}