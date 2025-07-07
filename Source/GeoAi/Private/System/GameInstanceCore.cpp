// Fill out your copyright notice in the Description page of Project Settings.

#include "System/GameInstanceCore.h"

#include "PixelStreamingMethodHandler.h"

void UGameInstanceCore::Init()
{
        Super::Init();

        PixelStreamingClient = UPixelStreamingClient::Create(this, PixelStreamingClientClass);
        PixelStreamingMethodHandler = UPixelStreamingMethodHandler::Create(this, PixelStreamingMethodHandlerClass);
}