// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RulerSplineWidget.generated.h"

/**
 *
 */
UCLASS()
class METAMEASUREMENT_API URulerSplineWidget : public UUserWidget
{
        GENERATED_BODY()

public:
        /**
        * @brief Sets new spline points.
        * @param Points New spline points.
        */
        UFUNCTION(BlueprintCallable)
        void SetSplinePoints(const TArray<FVector2D>& Points);

protected:
        UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RulerSplineWidget", BlueprintSetter = SetSplinePoints)
        TArray<FVector2D> SplinePoints;
};
