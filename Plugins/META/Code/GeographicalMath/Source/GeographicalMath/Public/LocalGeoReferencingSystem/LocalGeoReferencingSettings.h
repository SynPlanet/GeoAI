// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeographicCoordinates.h"
#include "GeoReferencingSystem.h"
#include "Engine/DeveloperSettings.h"
#include "LocalGeoReferencingSettings.generated.h"

UCLASS(Config = "Game", DefaultConfig, Meta = (DisplayName = "Local Geo Referencing"), Category = "GeographicalMath", ClassGroup = "Custom")
class GEOGRAPHICALMATH_API ULocalGeoReferencingSettings : public UDeveloperSettings
{
        GENERATED_BODY()

public:
        /**
* @brief Gets the default ULocalGeoReferencingSettings instance.
*
* @return The default ULocalGeoReferencingSettings instance.
*/
        UFUNCTION(BlueprintPure, Category = "LocalGeoReferencing")
        static ULocalGeoReferencingSettings* Get();

        UPROPERTY(Config, EditAnywhere, Category = "LocalGeoReferencing")
        EPlanetShape PlanetShape = EPlanetShape::FlatPlanet;

        UPROPERTY(Config, EditAnywhere, Category = "LocalGeoReferencing|CRS", DisplayName = "Projected CRS")
        FString ProjectedCrs = TEXT("EPSG:3857");

        UPROPERTY(Config, EditAnywhere, Category = "LocalGeoReferencing|CRS", DisplayName = "Geographic CRS")
        FString GeographicCrs = TEXT("EPSG:4326");

        UPROPERTY(Config, EditAnywhere, Category = "LocalGeoReferencing|Origin", DisplayName = "Origin in Projected CRS")
        bool bOriginInProjectedCrs = false;

        UPROPERTY(Config, EditAnywhere, Category = "LocalGeoReferencing|Origin")
        FGeographicCoordinates Origin;
};
