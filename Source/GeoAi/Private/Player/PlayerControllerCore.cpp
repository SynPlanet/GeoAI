// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerControllerCore.h"
#include "PixelStreamingInputComponent.h"

#include "PixelStreamingClient.h"

APlayerControllerCore::APlayerControllerCore()
{
        PixelStreamingInput = CreateDefaultSubobject<UPixelStreamingInput>(TEXT("PixelStreamingInput"));

        PixelStreamingInput->OnInputEvent.AddDynamic(this, &APlayerControllerCore::PassPixelStreamingMessageToClient);
}

void APlayerControllerCore::PassPixelStreamingMessageToClient(const FString& Descriptor)
{
        const auto Client = UPixelStreamingClient::Get();
        if (!IsValid(Client))
        {
                return;
        }

        Client->HandleIncomingInputMessage(Descriptor, this);
}