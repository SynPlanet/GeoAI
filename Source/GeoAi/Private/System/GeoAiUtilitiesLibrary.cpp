// Fill out your copyright notice in the Description page of Project Settings.

#include "System/GeoAiUtilitiesLibrary.h"

#include "CesiumGeoreference.h"

#include <Kismet/GameplayStatics.h>

bool UGeoAiUtilitiesLibrary::IsEditor()
{
#if WITH_EDITOR
        return true;
#else
        return false;
#endif
}

ACesiumGeoreference* UGeoAiUtilitiesLibrary::GetCesiumGeoreference(UObject* WorldContextObject)
{
        TArray<AActor*> CesiumRef;
        UGameplayStatics::GetAllActorsOfClass(WorldContextObject, ACesiumGeoreference::StaticClass(), CesiumRef);
        if (CesiumRef.IsEmpty())
        {
                return nullptr;
        }

        return Cast<ACesiumGeoreference>(CesiumRef[0]);
}