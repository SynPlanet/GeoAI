//
////  Copyright META. All Rights Reserved.  \\\\
//

#pragma once
#include "JsonRpcObject.h"
#include "PixelStreamingClient.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPixelStreamingClient, Log, Log);

class APlayerController;
class UPixelStreamingInput;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInputReceived, const FString&, Descriptor);

/**
* @class UPixelStreamingClient
*
* @brief Class representing the Pixel Streaming client.
*
* This class is responsible for managing communication between the Pixel Streaming server and the MetaPlayerController.
*/
UCLASS(BlueprintType, Blueprintable)
class METAPIXELSTREAMING_API UPixelStreamingClient : public UObject
{
        GENERATED_BODY()

        /**
        * @brief Pointer to the current game instance.
        */
        UPROPERTY()
        TObjectPtr<UGameInstance> GameInstance{};

        static inline TObjectPtr<UPixelStreamingClient> Singleton{};

public:
        /**
        * @brief Handles the incoming input message.
        *
        * This method is responsible for handling incoming input messages from MetaPixelStreamingInput.
        * It parses the input message, checks that the MethodHandler is valid, and then calls the
        * handleRequest method from the MethodHandler with the parsed JSON object.
        *
        * @param Descriptor The incoming input message handle string.
        * @param Sender The sender.
        */
        UFUNCTION(BlueprintCallable, Category = "PixelStreaming|Client")
        void HandleIncomingInputMessage(const FString& Descriptor, APlayerController* Sender);

        /**
        * @brief Gets an instance of UPixelStreamingClient.
        *
        * This method returns an instance of UPixelStreamingClient if available, otherwise nullptr.
        * To check if UPixelStreamingClient is available, use the static IsAvailable() function.
        *
        * @return An instance of UPixelStreamingClient if available, otherwise nullptr.
        */

        UFUNCTION(BlueprintPure, Category = "PixelStreaming|Client", DisplayName = "Get Pixel Streaming Client")
        static UPixelStreamingClient* Get();

        /**
        * @brief Creates an instance of UPixelStreamingClient.
        *
        * This method creates a new instance of UPixelStreamingClient using the provided UGameInstance and Class.
        *
        * @param InGameInstance a pointer to the current game instance.
        * @param Class The UClass of UPixelStreamingClient to create the instance of.
        *
        * @return a pointer to the created UPixelStreamingClient instance.
        */
        UFUNCTION(BlueprintCallable, Category = "PixelStreaming|Client", DisplayName = "Create Pixel Streaming Client")
        static UPixelStreamingClient* Create(UGameInstance* InGameInstance, TSubclassOf<UPixelStreamingClient> Class);

        /**
         * @brief Checks if UPixelStreamingClient is available.
         *
         * This method checks if UPixelStreamingClient is available by checking if its singleton instance is valid.
         *
         * @return True if UPixelStreamingClient is available, false otherwise.
         */

        UFUNCTION(BlueprintPure, Category = "PixelStreaming|Client", DisplayName = "Is Pixel Streaming Client Available")
        static bool IsAvailable();

        /**
        * @brief Gets the game instance associated with UPixelStreamingClient.
        *
        * This method returns the game instance that is associated with UPixelStreamingClient.
        *
        * @return The game instance associated with UPixelStreamingClient.
        */

        UFUNCTION(BlueprintPure, Category = "PixelStreaming|Client")
        UGameInstance* GetGameInstance() const noexcept;

        /**
        * @brief Get the UWorld object associated with the current game instance.
        *
        * This method returns the UWorld object associated with the current game instance.
        *
        * @return The UWorld object associated with the current game instance.
        */
        virtual UWorld* GetWorld() const override;

        /**
        * Sends an RPC object.
        *
        * @param Object The JSON-RPC object to send.
        * @param Recipient The recipient.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|Client")
        void SendRpcObject(const FJsonRpcObject& Object, APlayerController* Recipient) const;
};
