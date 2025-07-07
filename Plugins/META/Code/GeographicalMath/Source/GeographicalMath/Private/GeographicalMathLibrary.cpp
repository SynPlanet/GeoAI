#include "GeographicalMathLibrary.h"

DEFINE_LOG_CATEGORY(LogGeographicalMathLibrary);

FVector UGeographicalMathLibrary::CalculateSurfaceNormal(const FVector& Target, const FGeographicalEllipsoid& Ellipsoid)
{
        return Ellipsoid.Rotation.RotateVector((Ellipsoid.Rotation.UnrotateVector(Target - Ellipsoid.Center) / Ellipsoid.Radii).GetSafeNormal());
}

FQuat UGeographicalMathLibrary::CalculateSurfaceNormalRotation(const FVector& Target, const FGeographicalEllipsoid& Ellipsoid)
{
        return FQuat::FindBetweenNormals(FVector::UpVector, CalculateSurfaceNormal(Target, Ellipsoid));
}

FHitResult UGeographicalMathLibrary::LineTraceAlongNormalAxis(const FVector& Target,
                                                              const FGeographicalEllipsoid& Ellipsoid,
                                                              const UObject* WorldContext,
                                                              ECollisionChannel CollisionChannel)
{
        auto const World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
        if (!IsValid(World))
        {
                return {};
        }

        const FVector VerticalOffset = Ellipsoid.Radii.GetMax() * CalculateSurfaceNormal(Target, Ellipsoid);

        if (FHitResult Hit; World->LineTraceSingleByChannel(Hit, Target, Target - VerticalOffset, CollisionChannel))
        {
                return Hit;
        }

        if (FHitResult Hit; World->LineTraceSingleByChannel(Hit, Target, Target + VerticalOffset, CollisionChannel))
        {
                return Hit;
        }

        return {};
}

FVector UGeographicalMathLibrary::FindCoaxialSurfaceLocation(
        const FVector& Target, const FGeographicalEllipsoid& Ellipsoid, const UObject* WorldContext, bool bIgnoreCollisions, ECollisionChannel CollisionChannel)
{
        if (bIgnoreCollisions)
        {
                goto MathFallback;
        }

        if (auto const World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull); IsValid(World))
        {
                if (const auto Hit = LineTraceAlongNormalAxis(Target, Ellipsoid, World, CollisionChannel); Hit.bBlockingHit)
                {
                        return Hit.ImpactPoint;
                }
        }

MathFallback:
        const FRay Ray{ Target, -CalculateSurfaceNormal(Target, Ellipsoid) };
        const auto Near = CalculateRayEllipsoidIntersections(Ray, Ellipsoid, true).Near;
        return Near.Get(Ellipsoid.Center);
}

FRayEllipsoidIntersections
UGeographicalMathLibrary::CalculateRayEllipsoidIntersections(const FRay& Ray, const FGeographicalEllipsoid& Ellipsoid, bool bAllowReverseDirection)
{
        const auto RelativePosition = Ellipsoid.Rotation.UnrotateVector(Ray.Origin - Ellipsoid.Center);
        const auto RelativePositionScaled = RelativePosition / Ellipsoid.Radii;

        const auto Direction = Ellipsoid.Rotation.UnrotateVector(Ray.Direction);
        const auto DirectionScaled = Direction / Ellipsoid.Radii;

        const auto RayParamBase = DirectionScaled.Dot(RelativePositionScaled);
        const auto DirectionScaledLengthSquared = DirectionScaled.SquaredLength();

        const auto SquaredRayParamAdjustment = RayParamBase * RayParamBase - DirectionScaledLengthSquared * (RelativePositionScaled.SquaredLength() - 1);
        if (SquaredRayParamAdjustment < 0)
        {
                return {};
        }

        const auto RayParamAdjustment = FMath::Sqrt(SquaredRayParamAdjustment);

        const auto RayParamNear = (RayParamAdjustment - RayParamBase) / DirectionScaledLengthSquared;
        const auto RayParamFar = -(RayParamAdjustment + RayParamBase) / DirectionScaledLengthSquared;

        FRayEllipsoidIntersections Intersections;

        if (bAllowReverseDirection || RayParamNear >= 0)
        {
                Intersections.Near = Ellipsoid.Center + Ellipsoid.Rotation.RotateVector(RelativePosition + RayParamNear * Direction);
        }

        if (bAllowReverseDirection || RayParamFar >= 0)
        {
                Intersections.Far = Ellipsoid.Center + Ellipsoid.Rotation.RotateVector(RelativePosition + RayParamFar * Direction);
        }

        return Intersections;
}

void UGeographicalMathLibrary::K2_CalculateRayEllipsoidIntersections(
        const FRay& Ray, const FGeographicalEllipsoid& Ellipsoid, FVector& Near, bool& bNearSet, FVector& Far, bool& bFarSet)
{
        const auto [NearOpt, FarOpt] = CalculateRayEllipsoidIntersections(Ray, Ellipsoid);

        Near = NearOpt.Get({});
        bNearSet = NearOpt.IsSet();

        Far = FarOpt.Get({});
        bFarSet = FarOpt.IsSet();
}

double UGeographicalMathLibrary::CalculateHeightAboveSurface(
        const FVector& Target, const FGeographicalEllipsoid& Ellipsoid, const UObject* WorldContext, bool bIgnoreCollisions, ECollisionChannel CollisionChannel)
{
        const auto SurfaceNormal = CalculateSurfaceNormal(Target, Ellipsoid);
        const auto CoaxialSurfaceLocation = FindCoaxialSurfaceLocation(Target, Ellipsoid, WorldContext, bIgnoreCollisions, CollisionChannel);

        return SurfaceNormal.Dot(Target - CoaxialSurfaceLocation);
}

FVector UGeographicalMathLibrary::AddConcentricOffset(const FVector& Target, const FGeographicalEllipsoid& Ellipsoid, const FVector& DenormalizedOffset)
{
        const auto CurrentNormal = CalculateSurfaceNormal(Target, Ellipsoid);
        const auto FinalNormal = CalculateSurfaceNormal(Target + DenormalizedOffset, Ellipsoid);

        const auto Axis = (FinalNormal - CurrentNormal).GetSafeNormal();
        return Target + (Axis.IsNearlyZero() ? DenormalizedOffset : DenormalizedOffset.ProjectOnTo(Axis));
}

FVector UGeographicalMathLibrary::CalculateNorthDirection(const FVector& Origin, const FGeographicalEllipsoid& Ellipsoid)
{
        const FVector Axis = Ellipsoid.Rotation.RotateVector(FVector::UpVector);
        const FVector Normal = CalculateSurfaceNormal(Origin, Ellipsoid);

        return Normal.Cross(Axis.Cross(Normal).GetSafeNormal());
}
