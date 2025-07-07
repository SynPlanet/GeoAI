// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonLineString.h"

EGeoJsonType UGeoJsonLineString::GetType() const
{
        return EGeoJsonType::LineString;
}

bool UGeoJsonLineString::TryGetLineString(FJsonMultiPoint& LineString) const
{
        LineString = Coordinates;
        return true;
}

bool UGeoJsonLineString::TryGetGeometry(FJsonGeometry& Geometry) const
{
        Geometry.Geometry.Set<FJsonMultiPoint>(Coordinates);
        return true;
}

void UGeoJsonLineString::SetCoordinates(FJsonMultiPoint&& NewCoordinates)
{
        Coordinates = MoveTemp(NewCoordinates);
}

void UGeoJsonLineString::SetCoordinates(const FJsonMultiPoint& NewCoordinates)
{
        Coordinates = NewCoordinates;
}