#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SynShapeIntersectionLibrary.generated.h"

UCLASS()
class GEOAI_API USynShapeIntersectionLibrary : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

public:
        [[nodiscard]]
        static TOptional<FVector> CalculateRayPlaneIntersection(const FRay& Ray, const FPlane& Plane);
};
