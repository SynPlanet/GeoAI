// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PixelStreamingClient.h"
#include "PixelStreamingMethodHandler.h"
#include "CitySample/Game/CitySampleGameInstanceBase.h"
#include "GameInstanceCore.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class GEOAI_API UGameInstanceCore : public UCitySampleGameInstanceBase
{
        GENERATED_BODY()

protected:
        virtual void Init() override;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PixelStreaming")
        TObjectPtr<UPixelStreamingClient> PixelStreamingClient{};

        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PixelStreaming")
        TSubclassOf<UPixelStreamingClient> PixelStreamingClientClass = UPixelStreamingClient::StaticClass();

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PixelStreaming")
        TObjectPtr<UPixelStreamingMethodHandler> PixelStreamingMethodHandler{};

        UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PixelStreaming")
        TSubclassOf<UPixelStreamingMethodHandler> PixelStreamingMethodHandlerClass = UPixelStreamingMethodHandler::StaticClass();
};
