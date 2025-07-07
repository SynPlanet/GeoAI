#include "Math/SynShapeIntersectionLibrary.h"

TOptional<FVector> USynShapeIntersectionLibrary::CalculateRayPlaneIntersection(FRay const& Ray, FPlane const& Plane)
{
        FVector const& PlaneNormal = Plane.GetNormal();

        double const Nd = PlaneNormal.Dot(Ray.Direction);
        double const Nv = PlaneNormal.Dot(Plane.GetOrigin() - Ray.Origin);

        if (FMath::IsNearlyZero(Nv))
        {
                return Ray.Origin;
        }

        if (!FMath::IsNearlyZero(Nd))
        {
                return Ray.Origin + Ray.Direction * Nv / Nd;
        }

        return NullOpt;
}