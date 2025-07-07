// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonPoint.h"

EGeoJsonType UGeoJsonPoint::GetType() const
{
        return EGeoJsonType::Point;
}

bool UGeoJsonPoint::TryGetPoint(FJsonPoint& Point) const
{
        Point = Coordinates;
        return true;
}

bool UGeoJsonPoint::TryGetGeometry(FJsonGeometry& Geometry) const
{
        Geometry.Geometry.Set<FJsonPoint>(Coordinates);
        return true;
}

void UGeoJsonPoint::SetCoordinates(FJsonPoint&& Point)
{
        Coordinates = MoveTemp(Point);
}

void UGeoJsonPoint::SetCoordinates(const FJsonPoint& Point)
{
        Coordinates = Point;
}