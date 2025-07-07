// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonObject.h"

EGeoJsonType UGeoJsonObject::GetType() const
{
        return EGeoJsonType::None;
}

bool UGeoJsonObject::TryGetPoint(FJsonPoint& Point) const
{
        checkf(false, TEXT("Use of unimplemented function UGeoJsonObject::TryGetPoint")) return false;
}

bool UGeoJsonObject::TryGetLineString(FJsonMultiPoint& Line) const
{
        checkf(false, TEXT("Use of unimplemented function UGeoJsonObject::TryGetLineString")) return false;
}

bool UGeoJsonObject::TryGetPolygon(FJsonPolygon& Polygon) const
{
        checkf(false, TEXT("Use of unimplemented function UGeoJsonObject::TryGetPolygon")) return false;
}

bool UGeoJsonObject::TryGetMultiPoint(FJsonMultiPoint& MultiPoint) const
{
        checkf(false, TEXT("Use of unimplemented function UGeoJsonObject::TryGetMultiPoint")) return false;
}

bool UGeoJsonObject::TryGetMultiLineString(FJsonPolygon& MultiLine) const
{
        checkf(false, TEXT("Use of unimplemented function UGeoJsonObject::TryGetMultiLineString")) return false;
}

bool UGeoJsonObject::TryGetMultiPolygon(FJsonMultiPolygon& MultiPolygon) const
{
        checkf(false, TEXT("Use of unimplemented function UGeoJsonObject::TryGetMultiPolygon")) return false;
}

bool UGeoJsonObject::TryGetGeometry(FJsonGeometry& Geometry) const
{
        checkf(false, TEXT("Use of unimplemented function UGeoJsonObject::TryGetGeometry")) return false;
}