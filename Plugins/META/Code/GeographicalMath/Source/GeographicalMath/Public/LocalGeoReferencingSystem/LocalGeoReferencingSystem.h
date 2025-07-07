#pragma once

#include "CoreMinimal.h"
#include "GeoReferencingSystem.h"
#include "LocalGeoReferencingSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLocalGeoReferencingSystem, Log, Log);

UCLASS(Category = "GeographicalMath", ClassGroup = "Custom")
class GEOGRAPHICALMATH_API ALocalGeoReferencingSystem : public AGeoReferencingSystem
{
        GENERATED_BODY()

        static inline TWeakObjectPtr<ALocalGeoReferencingSystem> Singleton{};

public:
        ALocalGeoReferencingSystem();

        /**
        * @brief Gets an instance of the ALocalGeoReferencingSystem class.
        *
        * This method returns an instance of the ALocalGeoReferencingSystem class. If an instance has already been created,
        * it will be returned. Otherwise, it will create a new instance using the provided WorldContext.
        *
        * @param WorldContext The WorldContext object used to determine the world in which the actor should be created.
        * @return An instance of the ALocalGeoReferencingSystem class.
        */
        UFUNCTION(BlueprintPure, Category = "LocalGeoReferencing", Meta = (WorldContext = "WorldContext"))
        static ALocalGeoReferencingSystem* Get(const UObject* WorldContext);
};
