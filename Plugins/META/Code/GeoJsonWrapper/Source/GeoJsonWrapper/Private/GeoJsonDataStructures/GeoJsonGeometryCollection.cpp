// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonGeometryCollection.h"

EGeoJsonType UGeoJsonGeometryCollection::GetType() const
{
        return EGeoJsonType::GeometryCollection;
}

bool UGeoJsonGeometryCollection::TryGetPoint(FJsonPoint& Point) const
{
        return TryGetTemplateGeometry<EGeoJsonType::Point>(Point);
}

bool UGeoJsonGeometryCollection::TryGetLineString(FJsonMultiPoint& LineString) const
{
        return TryGetTemplateGeometry<EGeoJsonType::LineString>(LineString);
}

bool UGeoJsonGeometryCollection::TryGetPolygon(FJsonPolygon& Polygon) const
{
        return TryGetTemplateGeometry<EGeoJsonType::Polygon>(Polygon);
}

bool UGeoJsonGeometryCollection::TryGetMultiPoint(FJsonMultiPoint& MultiPoint) const
{
        return TryGetTemplateGeometry<EGeoJsonType::MultiPoint>(MultiPoint);
}

bool UGeoJsonGeometryCollection::TryGetMultiLineString(FJsonPolygon& MultiLineString) const
{
        return TryGetTemplateGeometry<EGeoJsonType::MultiLineString>(MultiLineString);
}

bool UGeoJsonGeometryCollection::TryGetMultiPolygon(FJsonMultiPolygon& MultiPolygon) const
{
        return TryGetTemplateGeometry<EGeoJsonType::MultiPolygon>(MultiPolygon);
}

bool UGeoJsonGeometryCollection::TryGetGeometry(FJsonGeometry& Geometry) const
{
        if (IsEmpty())
        {
                return false;
        }
        Geometry = Geometries.Geometries.Top();
        return true;
}

const FJsonGeometry& UGeoJsonGeometryCollection::GetGeometry(int32 Idx) const
{
        return Geometries.Geometries[Idx];
}

const FJsonMultiPolygon& UGeoJsonGeometryCollection::GetMultiPolygon(int32 Idx) const
{
        return Geometries.Geometries[Idx].Geometry.Get<FJsonMultiPolygon>();
}

const FJsonPolygon& UGeoJsonGeometryCollection::GetPolygon(int32 Idx) const
{
        return Geometries.Geometries[Idx].Geometry.Get<FJsonPolygon>();
}

const FJsonPolygon& UGeoJsonGeometryCollection::GetMultiLineString(int32 Idx) const
{
        return Geometries.Geometries[Idx].Geometry.Get<FJsonPolygon>();
}

const FJsonMultiPoint& UGeoJsonGeometryCollection::GetLineString(int32 Idx) const
{
        return Geometries.Geometries[Idx].Geometry.Get<FJsonMultiPoint>();
}

const FJsonMultiPoint& UGeoJsonGeometryCollection::GetMultiPoint(int32 Idx) const
{
        return Geometries.Geometries[Idx].Geometry.Get<FJsonMultiPoint>();
}

const FJsonPoint& UGeoJsonGeometryCollection::GetPoint(int32 Idx) const
{
        return Geometries.Geometries[Idx].Geometry.Get<FJsonPoint>();
}

bool UGeoJsonGeometryCollection::IsEmpty() const
{
        return Geometries.Geometries.IsEmpty();
}

int32 UGeoJsonGeometryCollection::Num() const
{
        return Geometries.Geometries.Num();
}

void UGeoJsonGeometryCollection::SetCollection(FJsonGeometryCollection&& NewCollection)
{
        Geometries = MoveTemp(NewCollection);
}

void UGeoJsonGeometryCollection::SetCollection(const FJsonGeometryCollection& NewCollection)
{
        Geometries = NewCollection;
}