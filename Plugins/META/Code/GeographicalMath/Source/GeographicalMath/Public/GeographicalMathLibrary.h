#pragma once

#include "CoreMinimal.h"
#include "GeographicalEllipsoid.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeographicalMathLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGeographicalMathLibrary, Log, Log);

struct FRayEllipsoidIntersections
{
        /**
         * @brief The point closest to the start of the ray.
         */
        TOptional<FVector> Near;

        /**
        * @brief The point furthest from the start of the ray.
        */
        TOptional<FVector> Far;
};

UCLASS(Category = "GeographicalMath", ClassGroup = "Custom", Meta = (BlueprintThreadSafe, RestrictedToClasses = "UGeographicalContext"))
class GEOGRAPHICALMATH_API UGeographicalMathLibrary : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

public:
        /**
        * @brief Calculate the tangent surface normal vector.
        *
        * @param Target The target location.
        * @param Ellipsoid The ellipsoid.
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Orientation")
        static FVector CalculateSurfaceNormal(const FVector& Target, const FGeographicalEllipsoid& Ellipsoid);

        /**
        * Calculate the rotation from the world-up axis vector to the tangent surface normal vector.
        *
        * @param Target The target location.
        * @param Ellipsoid The ellipsoid.
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Orientation")
        static FQuat CalculateSurfaceNormalRotation(const FVector& Target, const FGeographicalEllipsoid& Ellipsoid);

        /**
        * @brief Perform a line trace along the normal axis.
        *
        * This does the following:
        * 1. Perform a line trace toward the center of the sphere. If a blocking collision is found,
        * stop executing and return the result.
        * 2. Perform a line trace toward the given location with a length equal to the radius of the sphere.
        *
        * @param Target The target location.
        * @param Ellipsoid The corresponding ellipsoid.
        * @param WorldContext The world context in which to perform the line trace.
        * @param CollisionChannel The collision channel to test.
        *
        * @return The result of the line trace.
        */
        UFUNCTION(BlueprintCallable, Category = "GeographicalMath|Surfaces", Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FHitResult LineTraceAlongNormalAxis(const FVector& Target,
                                                   const FGeographicalEllipsoid& Ellipsoid,
                                                   const UObject* WorldContext,
                                                   ECollisionChannel CollisionChannel = ECC_Visibility);

        /**
        * @brief Find the location of the surface coaxial with the target location.
        *
        * This effectively sets the target's height above the surface to zero.
        *
        * @param Target The target location.
        * @param Ellipsoid The corresponding ellipsoid.
        * @param WorldContext The world context to test for collisions.
        * @param bIgnoreCollisions Whether to ignore collisions or not.
        * @param CollisionChannel The collision channel to test if @c bIgnoreCollisions is @c false.
        */
        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Surfaces",
                  Meta = (NotBlueprintThreadSafe, CallableWithoutWorldContext, WorldContext = "WorldContext"))
        static FVector FindCoaxialSurfaceLocation(const FVector& Target,
                                                  const FGeographicalEllipsoid& Ellipsoid,
                                                  const UObject* WorldContext,
                                                  bool bIgnoreCollisions = false,
                                                  ECollisionChannel CollisionChannel = ECC_Visibility);

        /**
        * @brief Calculate ray-ellipsoid intersections.
        *
        * If @c bAllowReverseDirection is set to @c false, then intersection points
        * that correspond to negative ray parameter values will not be returned,
        * instead the @c NullOpt value will be returned in their place. *
        * If the ray touches the ellipsoid surface, the two returned points will be identical.
        *
        * If the ray does not intersect the ellipsoid, the pair will contain only @c NullOpt.
        *
        * @param Ray The ray to intersect with the sphere.
        * @param Ellipsoid The ellipsoid to intersect with the ray.
        * @param bAllowReverseDirection Whether or not to return intersection points corresponding to
        * negative values of the ray parameters.
        *
        * @return The intersections of the ray and the ellipsoid.
        *
        * @note This assumes that @c Ray.Direction is normalized.
        *
        * @sa FRayEllipsoidIntersections
        */
        static FRayEllipsoidIntersections CalculateRayEllipsoidIntersections(const FRay& Ray,
                                                                             const FGeographicalEllipsoid& Ellipsoid,
                                                                             bool bAllowReverseDirection = false);

        /**
        * @brief Calculate the intersections of this ray with the ellipsoid.
        *
        * @param Ray The ray to intersect with the sphere.
        * @param Ellipsoid The ellipsoid to intersect with the ray.
        * @param[out] Near The intersection point closer to the start of the ray.
        * @param bNearSet Whether the near point was set.
        * @param[out] Far The intersection points farther from the start of the ray.
        * @param bFarSet Whether the far point was set.
        *
        * @return The intersections of the ray and the ellipsoid.
        *
        * @note This assumes that @c Ray.Direction is normalized.
        *
        * @sa UGeographicalMathFunctionLibrary::CalculateRayEllipsoidIntersections
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Surfaces", DisplayName = "Calculate Ray-Ellipsoid Intersections")
        static void K2_CalculateRayEllipsoidIntersections(
                const FRay& Ray, const FGeographicalEllipsoid& Ellipsoid, FVector& Near, bool& bNearSet, FVector& Far, bool& bFarSet);

        /**
        * @brief Calculate the height above the surface at the given point.
        *
        * @param Target The target location.
        * @param Ellipsoid The corresponding ellipsoid.
        * @param WorldContext The world context to calculate the target height in.
        * @param bIgnoreCollisions Whether to ignore collisions or not.
        * @param CollisionChannel The collision channel to test if @c bIgnoreCollisions is @c false.
        */
        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Heights",
                  Meta = (NotBlueprintThreadSafe, CallableWithoutWorldContext, WorldContext = "WorldContext"))
        static double CalculateHeightAboveSurface(const FVector& Target,
                                                  const FGeographicalEllipsoid& Ellipsoid,
                                                  const UObject* WorldContext,
                                                  bool bIgnoreCollisions = false,
                                                  ECollisionChannel CollisionChannel = ECC_Visibility);

        /**
        * @brief Add a concentric offset to the given location.
        *
        * This adds the given offset while maintaining the target's height above the surface.
        *
        * @param Target The target location.
        * @param Ellipsoid The corresponding ellipsoid.
        * @param DenormalizedOffset The initial denormalized offset value.
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Transforms")
        static FVector AddConcentricOffset(const FVector& Target, const FGeographicalEllipsoid& Ellipsoid, const FVector& DenormalizedOffset);

        // Calculates the direction to North (the upper pole of the ellipsoid) at a given point.
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Transforms", Meta = (WorldContext = "WorldContext"))
        static FVector CalculateNorthDirection(const FVector& Origin, const FGeographicalEllipsoid& Ellipsoid);
};