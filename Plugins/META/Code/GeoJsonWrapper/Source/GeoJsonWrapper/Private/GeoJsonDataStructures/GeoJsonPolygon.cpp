// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonPolygon.h"

EGeoJsonType UGeoJsonPolygon::GetType() const
{
        return EGeoJsonType::Polygon;
}

bool UGeoJsonPolygon::TryGetPolygon(FJsonPolygon& Polygon) const
{
        Polygon = Coordinates;
        return true;
}

bool UGeoJsonPolygon::TryGetGeometry(FJsonGeometry& Geometry) const
{
        Geometry.Geometry.Set<FJsonPolygon>(Coordinates);
        return true;
}

void UGeoJsonPolygon::SetCoordinates(FJsonPolygon&& NewCoordinates)
{
        Coordinates = MoveTemp(NewCoordinates);
}

void UGeoJsonPolygon::SetCoordinates(const FJsonPolygon& NewCoordinates)
{
        Coordinates = NewCoordinates;
}