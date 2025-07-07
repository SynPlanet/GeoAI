// META all rights received (c)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PolygonWidget.h"
#include "RulerSelection.generated.h"

/**
 *
 */
UCLASS()
class METAMEASUREMENT_API URulerSelection : public UUserWidget
{
        GENERATED_BODY()

public:
        /**
        * Creates a polygon selection widget with the specified fill color.
        *
        * This function initializes and adds a custom Slate widget `SPolygonWidget`
        * to the game viewport. The widget is assigned the fill color passed in via the parameter.
        * If the widget is created successfully, it is added to the viewport with a Z-order of -200.
        *
        * @param Color The fill color for the polygon widget.
        */

        UFUNCTION(BlueprintCallable, Category = "PolygonSelection")
        void CreatePolygonSelection(FLinearColor Color);

        /**
        * Updates the polygon selection with a new set of points.
        *
        * This method updates the polygon selection widget by replacing the
        * current points defining the polygon with a new array of points. * The points are expected to define the vertices of the polygon in either clockwise or counterclockwise
        * order.
        *
        * @param CurrentPoints A new array of 2D points defining the polygon.
        */
        UFUNCTION(BlueprintCallable, Category = "PolygonSelection")
        void PolygonSelectionUpdatePoints(TArray<FVector> Points) const;

        /**
        * @brief Sets the visibility flag of the selection polygon.
        * @param bIsVisible The visibility flag of the polygon.
        */
        UFUNCTION(BlueprintCallable)
        void SetPolygonVisibility(bool bIsVisible);

        UFUNCTION(BlueprintPure, Category = "PolygonSelection")
        const TArray<FVector>& GetPolygonPoints();

        UFUNCTION(BlueprintCallable, Category = "PolygonSelection")
        void SetPolygonPoints(const TArray<FVector>& NewPoints);

protected:
        virtual void NativeConstruct() override;

        virtual void NativeDestruct() override;

        UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn = "true"))
        TArray<FVector> PolygonPoints;

        UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
        FLinearColor PolygonColor = FColor::FromHex("FFFFFFB3");

private:
        TSharedPtr<SPolygonWidget> PolygonWidget;
};
