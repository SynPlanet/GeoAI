// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeographicalLocation.h"
#include "Components/ActorComponent.h"
#include "GeographicalSurfaceAnchor.generated.h"

UCLASS(Category = "GeographicalMath", ClassGroup = "Component", Meta = (BlueprintSpawnableComponent))
class GEOGRAPHICALMATH_API UGeographicalSurfaceAnchor : public USceneComponent
{
        GENERATED_BODY()

protected:
        /**
        * Default geographic coordinates of the object.
        */
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeographicalMath|SurfaceAnchor")
        FGeographicalLocation DefaultCoordinates{};

        /**
        * Default local rotation of the object (rotation in object space).
        */
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeographicalMath|SurfaceAnchor")
        FRotator DefaultLocalRotation = FRotator::ZeroRotator;

        /**
        * Default local rotation of the object (rotation in object space).
        */
        UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GeographicalMath|SurfaceAnchor")
        bool bAlignWithSurfaceNormalOnTick = false;

        virtual void BeginPlay() override;

        /**
        * @brief Updates this component.
        *
        * This function is called every frame to update the component.
        *
        * @param DeltaTime Time since the last update.
        * @param TickType The update type (world, level, etc.).
        * @param ThisTickFunction The update function for this component.
        */
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
        UGeographicalSurfaceAnchor();

        /**
        * @brief Sets the default coordinates to the current position of the object.
        */
        UFUNCTION(BlueprintCallable, CallInEditor, Category = "GeographicalMath|SurfaceAnchor")
        void ResetDefaultCoordinates();

        /**
        * @brief Aligns the vertical axis of the object with the surface normal of the ellipsoid at the current point.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure = false, CallInEditor, Category = "GeographicalMath|SurfaceAnchor")
        void AlignWithSurfaceNormal() const;

        /**
        * @brief Moves the object to the default coordinates.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure = false, CallInEditor, Category = "GeographicalMath|SurfaceAnchor")
        void TranslateToDefaultCoordinates() const;

        /**
        * @brief Moves the object to the default coordinates and aligns the vertical axis with the normal.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure = false, CallInEditor, Category = "GeographicalMath|SurfaceAnchor")
        void ApplyDefaultTransform() const;
};
