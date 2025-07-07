// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "GeographicalLocation.h"
#include "GeographicCoordinates.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LocalGeoReferencingSystem/LocalGeoReferencingSystem.h"
#include "GeographicalTransforms.generated.h"

UCLASS(Category = "GeographicalMath", ClassGroup = "Custom")
class GEOGRAPHICALMATH_API UGeographicalTransforms : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

        static inline TWeakObjectPtr<ALocalGeoReferencingSystem> GeoReferencingSystem{};

protected:
        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Transforms")
        static ALocalGeoReferencingSystem* GetGeoReferencingSystem(const UObject* WorldContext);

        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Transforms")
        static FGeographicCoordinates ToGeographicCoordinates(const FGeographicalLocation& Location);

        UFUNCTION(BlueprintPure, Category = "GeographicalMath|Transforms")
        static FGeographicalLocation ToGeographicalLocation(const FGeographicCoordinates& Location);

public:
        template <typename TTo, typename TFrom>
        static TTo
        Transform(const TFrom& From, const UObject* WorldContext, void (ALocalGeoReferencingSystem::*Convert)(const std::type_identity_t<TFrom>&, TTo&));

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "WGS84 to ECEF",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector Wgs84ToEcef(const FGeographicalLocation& TargetWgs84, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "ECEF to WGS84",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FGeographicalLocation EcefToWgs84(const FVector& TargetEcef, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "ECEF to Unreal",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector EcefToUnreal(const FVector& TargetEcef, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "Unreal to ECEF",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector UnrealToEcef(const FVector& TargetUnreal, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "WGS84 to Unreal",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector Wgs84ToUnreal(const FGeographicalLocation& TargetWgs84, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "Unreal to WGS84",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FGeographicalLocation UnrealToWgs84(const FVector& TargetUnreal, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "WGS84 to MSK77",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector Wgs84ToMsk77(const FGeographicalLocation& TargetWgs84, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "MSK77 to WGS84",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FGeographicalLocation Msk77ToWgs84(const FVector& TargetMsk77, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "ECEF to MSK77",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector EcefToMsk77(const FVector& TargetEcef, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "MSK77 to ECEF",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector Msk77ToEcef(const FVector& TargetMsk77, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "Unreal to MSK77",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector UnrealToMsk77(const FVector& TargetUnreal, const UObject* WorldContext = nullptr);

        UFUNCTION(BlueprintPure,
                  Category = "GeographicalMath|Transforms",
                  DisplayName = "MSK77 to Unreal",
                  Meta = (NotBlueprintThreadSafe, WorldContext = "WorldContext"))
        static FVector Msk77ToUnreal(const FVector& TargetMsk77, const UObject* WorldContext = nullptr);
};

template <typename TTo, typename TFrom>
TTo UGeographicalTransforms::Transform(const TFrom& From,
                                       const UObject* WorldContext,
                                       void (ALocalGeoReferencingSystem::*Convert)(const std::type_identity_t<TFrom>&, TTo&))
{
        auto* const Grs = GetGeoReferencingSystem(WorldContext);
        checkf(IsValid(Grs), TEXT("Invalid Local GeoReferencing System instance during call to UGeographicalTransforms::Transform."));

        TTo Result;
        (Grs->*Convert)(From, Result);

        return Result;
}
