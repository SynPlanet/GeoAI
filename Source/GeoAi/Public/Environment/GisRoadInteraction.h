// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GisRoadInteraction.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGisRoadInteraction : public UInterface
{
        GENERATED_BODY()
};

/**
 *
 */
class GEOAI_API IGisRoadInteraction
{
        GENERATED_BODY()

        // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
        UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
        void OnTakePoints(const TArray<FVector>& Points);
};
