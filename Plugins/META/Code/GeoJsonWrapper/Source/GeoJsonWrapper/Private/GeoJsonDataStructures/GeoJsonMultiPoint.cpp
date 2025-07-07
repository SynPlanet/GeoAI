// META all rights received (c)

#include "GeoJsonDataStructures/GeoJsonMultiPoint.h"

EGeoJsonType UGeoJsonMultiPoint::GetType() const
{
        return EGeoJsonType::MultiPoint;
}

bool UGeoJsonMultiPoint::TryGetMultiPoint(FJsonMultiPoint& MultiPoint) const
{
        MultiPoint = Coordinates;
        return true;
}

bool UGeoJsonMultiPoint::TryGetGeometry(FJsonGeometry& Geometry) const
{
        Geometry.Geometry.Set<FJsonMultiPoint>(Coordinates);
        return true;
}

void UGeoJsonMultiPoint::SetCoordinates(FJsonMultiPoint&& NewCoordinates)
{
        Coordinates = MoveTemp(NewCoordinates);
}

void UGeoJsonMultiPoint::SetCoordinates(const FJsonMultiPoint& NewCoordinates)
{
        Coordinates = NewCoordinates;
}