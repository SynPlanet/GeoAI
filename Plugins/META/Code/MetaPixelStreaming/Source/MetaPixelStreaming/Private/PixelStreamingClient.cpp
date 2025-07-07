//
////  Copyright META. All Rights Reserved.  \\\\
//

#include "PixelStreamingClient.h"

#include "PixelStreamingMethodHandler.h"

#include <PixelStreamingInputComponent.h>

DEFINE_LOG_CATEGORY(LogPixelStreamingClient);

UPixelStreamingClient* UPixelStreamingClient::Get()
{
        return IsAvailable() ? Singleton : nullptr;
}

UPixelStreamingClient* UPixelStreamingClient::Create(UGameInstance* InGameInstance, TSubclassOf<UPixelStreamingClient> Class)
{
        Singleton = NewObject<UPixelStreamingClient>(InGameInstance, Class);
        Singleton->GameInstance = InGameInstance;
        return Singleton;
}

bool UPixelStreamingClient::IsAvailable()
{
        return IsValid(Singleton);
}

UGameInstance* UPixelStreamingClient::GetGameInstance() const noexcept
{
        return GameInstance;
}

UWorld* UPixelStreamingClient::GetWorld() const
{
        return !IsValid(GameInstance) ? nullptr : GameInstance->GetWorld();
}

void UPixelStreamingClient::HandleIncomingInputMessage(const FString& Descriptor, APlayerController* Sender)
{
        UE_LOG(LogPixelStreamingClient,
               Log,
               TEXT("Received an input message from %s: (%s)"),
               IsValid(Sender) ? *Sender->GetName() : TEXT("<unknown>"),
               *Descriptor);

        if (!UPixelStreamingMethodHandler::IsAvailable())
        {
                UE_LOG(LogPixelStreamingClient, Error, TEXT("Pixel Streaming Method Handler is not initialized."));
                return;
        }

        int32 DescriptorEndIdx = Descriptor.Len() - 1;
        Descriptor.FindChar(0, DescriptorEndIdx);

        FJsonObjectWrapper JsonObjectWrapper;
        if (!JsonObjectWrapper.JsonObjectFromString(Descriptor.Mid(0, DescriptorEndIdx)))
        {
                return;
        }

        SendRpcObject(UPixelStreamingMethodHandler::Get()->HandleRequest({ FJsonRpcRequest{ *JsonObjectWrapper.JsonObject.Get() }, Sender }), Sender);
}

void UPixelStreamingClient::SendRpcObject(const FJsonRpcObject& Object, APlayerController* Recipient) const
{
        FString JsonString;
        Object.JsonObjectToString(JsonString);

        if (!IsValid(Recipient))
        {
                UE_LOG(LogPixelStreamingClient, Warning, TEXT("Invalid Recipient."));
                return;
        }

        const auto PixelStreamingInput = Recipient->FindComponentByClass<UPixelStreamingInput>();
        if (!IsValid(PixelStreamingInput))
        {
                UE_LOG(LogPixelStreamingClient, Warning, TEXT("Invalid Pixel Streaming Input component on the Recipient."));
                return;
        }

        UE_LOG(LogPixelStreamingClient, Log, TEXT("Sending RPC Object to %s: (%s)"), *Recipient->GetName(), *JsonString);

        PixelStreamingInput->SendPixelStreamingResponse(JsonString);
}
