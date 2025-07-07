// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RulerLines.generated.h"

/**
 *
 */
UCLASS()
class METAMEASUREMENT_API URulerLines : public UUserWidget
{
        GENERATED_BODY()

public:
        /**
        * @brief Sets new ruler points.
        * @param NewPoints The new ruler points.
        */
        UFUNCTION(BlueprintCallable)
        void SetPoints(const TArray<FVector>& NewPoints);

        /**
        * @brief Gets the current ruler points.
        * @return A const reference to the array of points currently set on the ruler.
        */
        UFUNCTION(BlueprintCallable)
        const TArray<FVector>& GetPoints();

        /**
        * @brief Changes the color of the ruler lines.
        * @param NewColor The new color to apply to the ruler lines.
        */
        void ChangeLinesColor(FLinearColor NewColor);

protected:
        UPROPERTY(VisibleAnywhere, BlueprintReadWrite, BlueprintSetter = SetPoints)
        TArray<FVector> Points;

        UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RulerMeasurement|Highlight")
        FLinearColor RulerLinesColor = FColor::FromHex("FFC149FF");
};
