// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CitySample/Game/CitySamplePlayerController.h"
#include "PlayerControllerCore.generated.h"

class UPixelStreamingInput;
/**
 *
 */
UCLASS(Blueprintable)
class GEOAI_API APlayerControllerCore : public ACitySamplePlayerController
{
        GENERATED_BODY()

public:
        APlayerControllerCore();

        UPROPERTY()
        TObjectPtr<UPixelStreamingInput> PixelStreamingInput{};

        UFUNCTION(BlueprintCallable)
        void PassPixelStreamingMessageToClient(const FString& Descriptor);
};
