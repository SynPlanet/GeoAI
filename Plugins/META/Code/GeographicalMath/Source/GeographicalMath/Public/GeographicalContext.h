#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GeographicalMathLibrary.h"
#include "GeographicalLocation.h"
#include "GeographicalContext.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGeographicalContext, Log, All);

UCLASS(Blueprintable, Category = "GeographicalMath", ClassGroup = "Custom", Meta = (BlueprintThreadSafe))
class GEOGRAPHICALMATH_API UGeographicalContext : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

        static inline FGeographicalEllipsoid Ellipsoid{};
        static inline TWeakObjectPtr<UWorld> World{};

public:
        /**
         * @brief Initializes the context.
         */
        UFUNCTION(BlueprintCallable,
                  Category = "GeographicalMath|Context|State",
                  DisplayName = "Initialize Geographical Context",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static void Initialize(const FGeographicalEllipsoid& InEllipsoid, const UObject* WorldContext);

        /**
        * @brief Checks if the context is initialized.
        */
        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Context|State",
                  DisplayName = "Is Geographical Context Initialized",
                  Meta = (NotBlueprintThreadSafe))
        static bool IsInitialized() noexcept;

        /**
        * @brief Resets the context parameters.
        */
        UFUNCTION(BlueprintCallable, Category = "GeographicalMath|Context|State", DisplayName = "Reset Geographical Context", Meta = (NotBlueprintThreadSafe))
        static void Reset() noexcept;

        /**
        * @brief Returns the internal representation of the ellipsoid.
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Context|State", DisplayName = "Get Ellipsoid from Geographical Context")
        static const FGeographicalEllipsoid& GetEllipsoid() noexcept;

        /**
        * @brief Calculate the tangent surface normal of the ellipsoid.
        *
        * @param Target The target location.
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Context|Orientation", Meta = (CompactNodeTitle = "Surface Normal"))
        static FVector CalculateSurfaceNormal(const FVector& Target);

        /**
        * @brief Calculate the rotation from the world yaw vector to the tangent surface normal vector.
        *
        * @param Target The target location.
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Context|Orientation", Meta = (CompactNodeTitle = "Surface Normal Rotation"))
        static FQuat CalculateSurfaceNormalRotation(const FVector& Target);

        /**
        * @brief Perform a ray trace along the normal.
        *
        * This does the following:
        * 1. Perform a ray trace toward the center of the sphere. If an obstacle is found,
        * stops and returns the result.
        * 2. Perform a ray trace outward from the given location with a length equal to
        * the radius of the sphere.
        *
        * @param Target The target location.
        * @param CollisionChannel The collision channel to test.
        *
        * @return The intersection result of the ray trace.
        */
        UFUNCTION(BlueprintCallable, Category = "GeographicalMath|Context|Surfaces", Meta = (NotBlueprintThreadSafe, AdvancedDisplay = "CollisionChannel"))
        static FHitResult LineTraceAlongNormalAxis(const FVector& Target, ECollisionChannel CollisionChannel = ECC_Visibility);

        /**
          * @brief Find the location of the surface coaxial with the target location.
          *
          * Effectively sets the target's height above the ground to zero.
          *
          * @param Target The target location.
          * @param bIgnoreCollisions Whether to ignore collisions.
          * @param CollisionChannel The collision channel to test if @c bIgnoreCollisions is @c false.
          */
        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Context|Surfaces",
                  Meta = (NotBlueprintThreadSafe, AdvancedDisplay = "bIgnoreCollisions, CollisionChannel"))
        static FVector FindCoaxialSurfaceLocation(const FVector& Target, bool bIgnoreCollisions = false, ECollisionChannel CollisionChannel = ECC_Visibility);

        /**
        * @brief Calculate ray-ellipsoid intersections.
        *
        * If @c bAllowReverseDirection is @c false, then intersection points
        * that correspond to negative ray parameters will not be returned,
        * instead @c NullOpt will be returned.
        *
        * @param Ray The ray to intersect with the ellipsoid.
        * @param bAllowReverseDirection Whether or not to return intersection points
        * that correspond to negative ray parameters.
        *
        * @return Ray-ellipsoid intersections.
        *
        * @note @c Ray.Direction is assumed to be normalized.
        *
        * @sa FRayEllipsoidIntersections
        */
        static FRayEllipsoidIntersections CalculateRayEllipsoidIntersections(const FRay& Ray, bool bAllowReverseDirection = false);

        /**
        * @brief Calculate the intersections of the given ray with the ellipsoid.
        *
        * @param Ray The ray to intersect with the ellipsoid.
        * @param[out] Near The intersection point closer to the ray start.
        * @param bNearSet Whether the near point is set or not.
        * @param[out] Far The intersection points farther from the ray start.
        * @param bFarSet Whether the far point is set or not.
        *
        * @return The intersections of the ray and the ellipsoid.
        *
        * @note @c Ray.Direction is assumed to be normalized.
        *
        * @sa UGeographicalMathFunctionLibrary::CalculateRayEllipsoidIntersections
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Context|Surfaces", DisplayName = "Calculate Ray-Ellipsoid Intersections")
        static void K2_CalculateRayEllipsoidIntersections(const FRay& Ray, FVector& Near, bool& bNearSet, FVector& Far, bool& bFarSet);

        /**
          * @brief Calculate the height above the surface at the given point.
          *
          * @param Target The target location.
          * @param bIgnoreCollisions Whether to ignore collisions.
          * @param CollisionChannel The collision channel to test if @c bIgnoreCollisions is @c false.
          */
        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Context|Heights",
                  Meta = (NotBlueprintThreadSafe, AdvancedDisplay = "bIgnoreCollisions, CollisionChannel"))
        static double CalculateHeightAboveSurface(const FVector& Target, bool bIgnoreCollisions = false, ECollisionChannel CollisionChannel = ECC_Visibility);

        /**
        * @brief Add a concentric offset to the given location.
        *
        * This adds the given offset while maintaining the target's height above the surface.
        *
        * @param Target The target location.
        * @param DenormalizedOffset The initial denormalized offset value.
        */
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Context|Transforms")
        static FVector AddConcentricOffset(const FVector& Target, const FVector& DenormalizedOffset);

        // Calculates the North direction (the upper pole of the ellipsoid) at the given point.
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Context|Transforms")
        static FVector CalculateNorthDirection(const FVector& Origin);
};
