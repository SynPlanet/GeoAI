#include "GeographicalContext.h"

DEFINE_LOG_CATEGORY(LogGeographicalContext);

void UGeographicalContext::Initialize(const FGeographicalEllipsoid& InEllipsoid, const UObject* WorldContext)
{
        if (IsInitialized())
        {
                UE_LOG(LogGeographicalContext, Warning, TEXT("Ignored double initialization attempt for Geographical Context."));
                return;
        }

        Ellipsoid = InEllipsoid;
        World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);

        UE_LOG(LogGeographicalContext,
               Log,
               TEXT("Initialized Geographical Context with Ellipsoid = (Center = (%s) Radii = (%s))"),
               *Ellipsoid.Center.ToString(),
               *Ellipsoid.Radii.ToString());
}

bool UGeographicalContext::IsInitialized() noexcept
{
        return !Ellipsoid.Radii.IsZero() && World.IsValid();
}

void UGeographicalContext::Reset() noexcept
{
        Ellipsoid = {};
        World = {};
}

const FGeographicalEllipsoid& UGeographicalContext::GetEllipsoid() noexcept
{
        return Ellipsoid;
}

FVector UGeographicalContext::CalculateSurfaceNormal(const FVector& Target)
{
        return UGeographicalMathLibrary::CalculateSurfaceNormal(Target, Ellipsoid);
}

FQuat UGeographicalContext::CalculateSurfaceNormalRotation(const FVector& Target)
{
        return UGeographicalMathLibrary::CalculateSurfaceNormalRotation(Target, Ellipsoid);
}

FHitResult UGeographicalContext::LineTraceAlongNormalAxis(const FVector& Target, ECollisionChannel CollisionChannel)
{
        return UGeographicalMathLibrary::LineTraceAlongNormalAxis(Target, Ellipsoid, World.Get(), CollisionChannel);
}

FVector UGeographicalContext::FindCoaxialSurfaceLocation(const FVector& Target, bool bIgnoreCollisions, ECollisionChannel CollisionChannel)
{
        return UGeographicalMathLibrary::FindCoaxialSurfaceLocation(Target, Ellipsoid, World.Get(), bIgnoreCollisions, CollisionChannel);
}

FRayEllipsoidIntersections UGeographicalContext::CalculateRayEllipsoidIntersections(const FRay& Ray, bool bAllowReverseDirection)
{
        return UGeographicalMathLibrary::CalculateRayEllipsoidIntersections(Ray, Ellipsoid, bAllowReverseDirection);
}

void UGeographicalContext::K2_CalculateRayEllipsoidIntersections(const FRay& Ray, FVector& Near, bool& bNearSet, FVector& Far, bool& bFarSet)
{
        return UGeographicalMathLibrary::K2_CalculateRayEllipsoidIntersections(Ray, Ellipsoid, Near, bNearSet, Far, bFarSet);
}

double UGeographicalContext::CalculateHeightAboveSurface(const FVector& Target, bool bIgnoreCollisions, ECollisionChannel CollisionChannel)
{
        return UGeographicalMathLibrary::CalculateHeightAboveSurface(Target, Ellipsoid, World.Get(), bIgnoreCollisions, CollisionChannel);
}

FVector UGeographicalContext::AddConcentricOffset(const FVector& Target, const FVector& DenormalizedOffset)
{
        return UGeographicalMathLibrary::AddConcentricOffset(Target, Ellipsoid, DenormalizedOffset);
}

FVector UGeographicalContext::CalculateNorthDirection(const FVector& Origin)
{
        return UGeographicalMathLibrary::CalculateNorthDirection(Origin, Ellipsoid);
}
