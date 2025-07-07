// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonMultiPolygon.h"

EGeoJsonType UGeoJsonMultiPolygon::GetType() const
{
        return EGeoJsonType::MultiPolygon;
}

bool UGeoJsonMultiPolygon::TryGetMultiPolygon(FJsonMultiPolygon& MultiPolygon) const
{
        MultiPolygon = Coordinates;
        return true;
}

bool UGeoJsonMultiPolygon::TryGetGeometry(FJsonGeometry& Geometry) const
{
        Geometry.Geometry.Set<FJsonMultiPolygon>(Coordinates);
        return true;
}

void UGeoJsonMultiPolygon::SetCoordinates(FJsonMultiPolygon&& NewCoordinates)
{
        Coordinates = MoveTemp(NewCoordinates);
}

void UGeoJsonMultiPolygon::SetCoordinates(const FJsonMultiPolygon& NewCoordinates)
{
        Coordinates = NewCoordinates;
}