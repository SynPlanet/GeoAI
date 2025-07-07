// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonMultiLineString.h"

EGeoJsonType UGeoJsonMultiLineString::GetType() const
{
        return EGeoJsonType::LineString;
}

bool UGeoJsonMultiLineString::TryGetMultiLineString(FJsonPolygon& MultiLineString) const
{
        MultiLineString = Coordinates;
        return true;
}

bool UGeoJsonMultiLineString::TryGetGeometry(FJsonGeometry& Geometry) const
{
        Geometry.Geometry.Set<FJsonPolygon>(Coordinates);
        return true;
}

void UGeoJsonMultiLineString::SetCoordinates(FJsonPolygon&& NewCoordinates)
{
        Coordinates = MoveTemp(NewCoordinates);
}

void UGeoJsonMultiLineString::SetCoordinates(const FJsonPolygon& NewCoordinates)
{
        Coordinates = NewCoordinates;
}