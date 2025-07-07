#include "GeographicalTransforms.h"

#include "LocalGeoReferencingSystem/LocalGeoReferencingSystem.h"

ALocalGeoReferencingSystem* UGeographicalTransforms::GetGeoReferencingSystem(const UObject* WorldContext)
{
        if (!GeoReferencingSystem.IsValid())
        {
                checkf(IsValid(WorldContext), TEXT("Invalid World Context in UGeographicalCrsLibrary::GetCachedGrs"));
                GeoReferencingSystem = ALocalGeoReferencingSystem::Get(WorldContext);
        }

        return GeoReferencingSystem.Get();
}

FGeographicCoordinates UGeographicalTransforms::ToGeographicCoordinates(const FGeographicalLocation& Location)
{
        return { Location.Longitude, Location.Latitude, Location.Altitude };
}

FGeographicalLocation UGeographicalTransforms::ToGeographicalLocation(const FGeographicCoordinates& Location)
{
        return { Location.Longitude, Location.Latitude, Location.Altitude };
}

FVector UGeographicalTransforms::Wgs84ToEcef(const FGeographicalLocation& TargetWgs84, const UObject* WorldContext)
{
        return Transform<FVector>(ToGeographicCoordinates(TargetWgs84), WorldContext, &ALocalGeoReferencingSystem::GeographicToECEF);
}

FGeographicalLocation UGeographicalTransforms::EcefToWgs84(const FVector& TargetEcef, const UObject* WorldContext)
{
        return ToGeographicalLocation(Transform<FGeographicCoordinates>(TargetEcef, WorldContext, &ALocalGeoReferencingSystem::ECEFToGeographic));
}

FVector UGeographicalTransforms::EcefToUnreal(const FVector& TargetEcef, const UObject* WorldContext)
{
        return Transform<FVector>(TargetEcef, WorldContext, &ALocalGeoReferencingSystem::ECEFToEngine);
}

FVector UGeographicalTransforms::UnrealToEcef(const FVector& TargetUnreal, const UObject* WorldContext)
{
        return Transform<FVector>(TargetUnreal, WorldContext, &ALocalGeoReferencingSystem::EngineToECEF);
}

FVector UGeographicalTransforms::Wgs84ToUnreal(const FGeographicalLocation& TargetWgs84, const UObject* WorldContext)
{
        return Transform<FVector>(ToGeographicCoordinates(TargetWgs84), WorldContext, &ALocalGeoReferencingSystem::GeographicToEngine);
}

FGeographicalLocation UGeographicalTransforms::UnrealToWgs84(const FVector& TargetUnreal, const UObject* WorldContext)
{
        return ToGeographicalLocation(Transform<FGeographicCoordinates>(TargetUnreal, WorldContext, &ALocalGeoReferencingSystem::EngineToGeographic));
}

FVector UGeographicalTransforms::Wgs84ToMsk77(const FGeographicalLocation& TargetWgs84, const UObject* WorldContext)
{
        return Transform<FVector>(ToGeographicCoordinates(TargetWgs84), WorldContext, &ALocalGeoReferencingSystem::GeographicToProjected);
}

FGeographicalLocation UGeographicalTransforms::Msk77ToWgs84(const FVector& TargetMsk77, const UObject* WorldContext)
{
        return ToGeographicalLocation(Transform<FGeographicCoordinates>(TargetMsk77, WorldContext, &ALocalGeoReferencingSystem::ProjectedToGeographic));
}

FVector UGeographicalTransforms::EcefToMsk77(const FVector& TargetEcef, const UObject* WorldContext)
{
        return Transform<FVector>(TargetEcef, WorldContext, &ALocalGeoReferencingSystem::ECEFToProjected);
}

FVector UGeographicalTransforms::Msk77ToEcef(const FVector& TargetMsk77, const UObject* WorldContext)
{
        return Transform<FVector>(TargetMsk77, WorldContext, &ALocalGeoReferencingSystem::ProjectedToECEF);
}

FVector UGeographicalTransforms::UnrealToMsk77(const FVector& TargetUnreal, const UObject* WorldContext)
{
        return Transform<FVector>(TargetUnreal, WorldContext, &ALocalGeoReferencingSystem::EngineToProjected);
}

FVector UGeographicalTransforms::Msk77ToUnreal(const FVector& TargetMsk77, const UObject* WorldContext)
{
        return Transform<FVector>(TargetMsk77, WorldContext, &ALocalGeoReferencingSystem::ProjectedToEngine);
}
