#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GeographicalLocation.generated.h"

USTRUCT(Blueprintable, BlueprintType, Category = "GeographicalMath")
struct GEOGRAPHICALMATH_API FGeographicalLocation
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeographicalMath|Location")
        double Longitude = 0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeographicalMath|Location")
        double Latitude = 0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeographicalMath|Location")
        double Altitude = 0;

        explicit operator FVector() const;
};

FORCEINLINE FGeographicalLocation::operator FVector() const
{
        return { Longitude, Latitude, Altitude };
}
