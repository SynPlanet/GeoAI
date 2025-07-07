#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GeographicalEllipsoid.generated.h"

USTRUCT(Blueprintable, BlueprintType, Category = "GeographicalMath")
struct GEOGRAPHICALMATH_API FGeographicalEllipsoid
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeographicalMath|Ellipsoid")
        FVector Center = FVector::ZeroVector;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeographicalMath|Ellipsoid")
        FVector Radii = FVector::ZeroVector;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeographicalMath|Ellipsoid")
        FQuat Rotation = FQuat::Identity;
};
