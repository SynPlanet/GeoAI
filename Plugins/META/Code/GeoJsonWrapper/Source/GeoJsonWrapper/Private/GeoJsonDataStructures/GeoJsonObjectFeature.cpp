// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonObjectFeature.h"

EGeoJsonType UGeoJsonObjectFeature::GetType() const
{
        return EGeoJsonType::Feature;
}

bool UGeoJsonObjectFeature::TryGetObjectFeature(FJsonObjectFeature& ObjectFeature) const
{
        ObjectFeature = Feature;
        return true;
}

bool UGeoJsonObjectFeature::TryGetPoint(FJsonPoint& Point) const
{
        return Feature.TryGetTemplateGeometry<EGeoJsonType::Point>(Point);
}

bool UGeoJsonObjectFeature::TryGetLineString(FJsonMultiPoint& LineString) const
{
        return Feature.TryGetTemplateGeometry<EGeoJsonType::LineString>(LineString);
}

bool UGeoJsonObjectFeature::TryGetPolygon(FJsonPolygon& Polygon) const
{
        return Feature.TryGetTemplateGeometry<EGeoJsonType::Polygon>(Polygon);
}

bool UGeoJsonObjectFeature::TryGetMultiPoint(FJsonMultiPoint& MultiPoint) const
{
        return Feature.TryGetTemplateGeometry<EGeoJsonType::MultiPoint>(MultiPoint);
}

bool UGeoJsonObjectFeature::TryGetMultiLineString(FJsonPolygon& MultiLineString) const
{
        return Feature.TryGetTemplateGeometry<EGeoJsonType::MultiLineString>(MultiLineString);
}

bool UGeoJsonObjectFeature::TryGetMultiPolygon(FJsonMultiPolygon& MultiPolygon) const
{
        return Feature.TryGetTemplateGeometry<EGeoJsonType::MultiPolygon>(MultiPolygon);
}

bool UGeoJsonObjectFeature::TryGetGeometry(FJsonGeometry& Geometry) const
{
        Geometry = Feature.Geometry;
        return true;
}

FString UGeoJsonObjectFeature::GetStringProperty(const FString& Key) const
{
        return GetStringPropertyWithDefault(Key, "");
}

FString UGeoJsonObjectFeature::GetStringPropertyWithDefault(const FString& Key, const FString& Default) const
{
        return Feature.GetPropertyWithDefault(Key, Default);
}

bool UGeoJsonObjectFeature::GetBoolProperty(const FString& Key) const
{
        return GetBoolPropertyWithDefault(Key, false);
}

bool UGeoJsonObjectFeature::GetBoolPropertyWithDefault(const FString& Key, bool bDefault) const
{
        return Feature.GetPropertyWithDefault(Key, bDefault);
}

double UGeoJsonObjectFeature::GetNumberProperty(const FString& Key) const
{
        return GetNumberPropertyWithDefault(Key, DBL_MAX);
}

double UGeoJsonObjectFeature::GetNumberPropertyWithDefault(const FString& Key, double Default) const
{
        return Feature.GetPropertyWithDefault(Key, Default);
}

int32 UGeoJsonObjectFeature::GetIntProperty(const FString& Key) const
{
        return GetIntPropertyWithDefault(Key, INT_MAX);
}

int32 UGeoJsonObjectFeature::GetIntPropertyWithDefault(const FString& Key, int32 Default) const
{
        return Feature.GetPropertyWithDefault(Key, Default);
}

void UGeoJsonObjectFeature::SetFeature(FJsonObjectFeature&& Element)
{
        Feature = MoveTemp(Element);
}

void UGeoJsonObjectFeature::SetFeature(const FJsonObjectFeature& Element)
{
        Feature = Element;
}