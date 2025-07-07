// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeoAiUtilitiesLibrary.generated.h"

class ACesiumGeoreference;
/**
 *
 */
UCLASS()
class GEOAI_API UGeoAiUtilitiesLibrary : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

public:
        UFUNCTION(BlueprintCallable, BlueprintPure)
        static bool IsEditor();

        UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext = "WorldContextObject"))
        static ACesiumGeoreference* GetCesiumGeoreference(UObject* WorldContextObject);
};
