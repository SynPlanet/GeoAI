#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CartographicTransformsLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCartographicTransforms, All, All);

UCLASS()
class CARTOGRAPHICTRANSFORMS_API UCartographicTransformsLibrary : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

public:
        /**
         * @brief Applies an atomic map move.
         *
         * @param Target The target point.
         * @param Pivot The pivot point.
         * @param PivotDestination The pivot point destination.
         *
         * @return The point setting.
         */
        UFUNCTION(BlueprintCallable, Category = "CartographicTransforms")
        static FVector Drag(const FVector& Target, const FVector& Pivot, const FVector& PivotDestination);

        /**
        * @brief Applies atomic map scaling.
        *
        * @param Target The target point.
        * @param Pivot The pivot point.
        * @param Scale The scale amount.
        *
        * @return The destination of the target point.
        */
        UFUNCTION(BlueprintCallable, Category = "CartographicTransforms")
        static FVector Zoom(const FVector& Target, const FVector& Pivot, double Scale);

        /**
        * @brief Applies an atomic map rotation.
        *
        * @param Target The target point.
        * @param Pivot The pivot point.
        * @param TargetDirection The rotation of the target point.
        * @param AngularVelocity The angular velocity of the target point relative to the pivot point.
        *
        * @return The destination of the target point.
        */
        UFUNCTION(BlueprintCallable, Category = "CartographicTransforms")
        static FVector Orbit(const FVector& Target, const FVector& Pivot, const FRotator& AngularVelocity, const FVector& TargetDirection);
};
