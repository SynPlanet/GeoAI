// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/GeoPinManager.h"
#include "GameFramework/GameplayMessageTags.h"
// Sets default values for this component's properties
UGeoPinManager::UGeoPinManager()
{
        // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
        // off to improve performance if you don't need them.
        PrimaryComponentTick.bCanEverTick = false;
}

void UGeoPinManager::ConstructPins(TArray<FGeoPinInfo> TakenInfo)
{
        for (const auto& CurrentPin : Pins)
        {
                if (IsValid(CurrentPin))
                {
                        CurrentPin->Destroy();
                }
        }

        Pins.Empty();

        for (const auto& pin : TakenInfo)
        {
                auto CreatedPin = GetWorld()->SpawnActorDeferred<AGeoPin>(PinActorClass, FTransform::Identity, GetOwner());

                if (!CreatedPin)
                {
                        continue;
                }

                CreatedPin->PinInfo = pin;

                CreatedPin->FinishSpawning(FTransform(pin.WorldPositions));

                Pins.Add(MoveTemp(CreatedPin));
        }
}
