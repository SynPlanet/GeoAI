// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeoPin.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageTags.h"
#include "GeoPinManager.generated.h"

class AGeoPin;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GEOAI_API UGeoPinManager : public UActorComponent
{
        GENERATED_BODY()

public:
        // Sets default values for this component's properties
        UGeoPinManager();

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSubclassOf<AGeoPin> PinActorClass = AGeoPin::StaticClass();

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<TObjectPtr<AGeoPin>> Pins;

        UFUNCTION(BlueprintCallable)
        void ConstructPins(TArray<FGeoPinInfo> TakenInfo);
};
