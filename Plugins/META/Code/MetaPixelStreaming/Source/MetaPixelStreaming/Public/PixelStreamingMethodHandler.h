#pragma once

#include "JsonRpcResponse.h"
#include "PixelStreamingMethodContext.h"

#include "PixelStreamingMethodHandler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPixelStreamingMethodHandler, Log, Log);

struct FPixelStreamingMethodContext;

struct FObjectProperties;
class UGameInstance;
class UPTWidget;
class UPixelStreamingClient;

using FPixelStreamingMethodDispatcher = TFunction<FJsonRpcResponse(const FPixelStreamingMethodContext&)>;

/**
* @class UPixelStreamingMethodHandler
*
* @brief The APixelStreamingMethodHandler class is responsible for handling Pixel Streaming method requests.
*/
UCLASS(BlueprintType, Blueprintable)
class METAPIXELSTREAMING_API UPixelStreamingMethodHandler : public UObject
{
        GENERATED_BODY()

        /**
        * @brief A pointer to the current game instance.
        */
        UPROPERTY()
        TObjectPtr<UGameInstance> GameInstance{};

        /**
        * @brief A map of dispatchers for Pixel Streaming methods.
        *
        * This map maps a method name (FString) to a method dispatcher (FPixelStreamingMethodDispatcher).
        * The method dispatcher is responsible for handling requests and creating responses for the corresponding method name.
        */
        TMap<FString, FPixelStreamingMethodDispatcher> Dispatchers;

        static inline TObjectPtr<UPixelStreamingMethodHandler> Singleton{};

        /**
        * @brief RegisterDefaultDispatchers is a method of the UPixelStreamingMethodHandler class.
        *
        * This method registers default dispatchers for various pixel streaming methods.
        */
        UFUNCTION(BlueprintCallable, Category = "PixelStreaming|MethodHandler")
        void RegisterDefaultDispatchers();

protected:
        /**
        * @brief Register a method dispatcher for the given method name.
        *
        * @param MethodName The name of the method.
        * @param Dispatcher The method dispatcher.
        */
        void RegisterDispatcher(const FString& MethodName, const FPixelStreamingMethodDispatcher& Dispatcher);

        /**
        * Find a dispatcher based on the given method name.
        *
        * @param MethodName The name of the method to find the dispatcher for.
        * @return The dispatcher associated with the given method name, or nullptr if not found.
        */
        const FPixelStreamingMethodDispatcher* FindDispatcher(const FString& MethodName);

public:
        /**
        * @fn UPixelStreamingMethodHandler::Get()
        *
        * @brief Gets an instance of UPixelStreamingMethodHandler, if available.
        *
        * @return UPixelStreamingMethodHandler* - A pointer to the UPixelStreamingMethodHandler instance, if available, otherwise nullptr.
        */

        UFUNCTION(BlueprintPure, Category = "PixelStreaming|MethodHandler", DisplayName = "Get Pixel Streaming Method Handler")
        static UPixelStreamingMethodHandler* Get();

        /**
        * @brief Creates a new UPixelStreamingMethodHandler instance.
        *
        * @param InGameInstance The game instance.
        * @param Class The UPixelStreamingMethodHandler class to create.
        * @return A pointer to the created UPixelStreamingMethodHandler instance.
        */
        UFUNCTION(BlueprintCallable, Category = "PixelStreaming|MethodHandler", DisplayName = "Create Pixel Streaming Method Handler")
        static UPixelStreamingMethodHandler* Create(UGameInstance* InGameInstance, TSubclassOf<UPixelStreamingMethodHandler> Class);

        /**
        * @brief Checks if the pixel streaming method is available.
        *
        * @return True if the method is available, false otherwise.
        */

        UFUNCTION(BlueprintPure, Category = "PixelStreaming|MethodHandler", DisplayName = "Is Pixel Streaming Method Handler Available")
        static bool IsAvailable();

        /**
        * @brief Retrieves the game instance from UPixelStreamingMethodHandler.
        *
        * This method retrieves the game instance from UPixelStreamingMethodHandler and returns it.
        * The game instance is of type UGameInstance.
        *
        * @return The game instance.
        */

        UFUNCTION(BlueprintPure, Category = "PixelStreaming|MethodHandler")
        UGameInstance* GetGameInstance() const noexcept;
        /**
        * @brief Returns the world object associated with the PixelStreaming method handler.
        *
        * This method is used to obtain a reference to the world object associated with the PixelStreaming method handler. The world object represents the game
        * world in which the PixelStreaming method handler operates.
        *
        * @return A pointer to a UWorld object representing the game world, or nullptr if no running instance of the game is available.
        */
        virtual UWorld* GetWorld() const override;

        /**
        * @brief Handles the request made to the PixelStreaming method handler.
        * @param Ctx The context of the method.
        * @return The JSON-RPC response object.
        */
        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler")
        FJsonRpcResponse HandleRequest(const FPixelStreamingMethodContext& Ctx);

        static FJsonRpcResponse CreateSimpleJsonResponse(FGuid RequestGuid, const EJsonRpcErrorCode& ErrorCode, const FString& Message);

        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Instigators")
        static void SendTimerPolygonCoordinates(APlayerController* Instigator, UPARAM(ref) const FVector& Checkpoint, const float Radius);

        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Instigators")
        static void SendScreenTouchAsWGS(APlayerController* Instigator, UPARAM(ref) const FVector& Checkpoint);

        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Instigators")
        static void SendSearchAreaStartScale(APlayerController* Instigator, const float InScale);

        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Instigators")
        static void SendGeoPinClick(APlayerController* Instigator, const FString& Id);

        /**
        * @brief Sends a user connection event.
        * @param Ctx The method context.
        * @return A JSON-RPC response object indicating the success or failure of the method call
        */
        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Dispatchers")
        static FJsonRpcResponse OnUserChanged(const FPixelStreamingMethodContext& Ctx);

        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Dispatchers")
        static FJsonRpcResponse OnBackToOrigin(const FPixelStreamingMethodContext& Ctx);

        /**
        * @brief Dispatches a user connection event.
        * @param Ctx The method context.
        * @return A JSON-RPC response object indicating the success or failure of the method call
        */
        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Dispatchers")
        static FJsonRpcResponse OnGetSearchAreaScale(const FPixelStreamingMethodContext& Ctx);

        /**
        * @brief Dispatches a user connection event.
        * @param Ctx The method context.
        * @return A JSON-RPC response object indicating the success or failure of the method call
        */
        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Dispatchers")
        static FJsonRpcResponse OnChangeActiveStateSearchArea(const FPixelStreamingMethodContext& Ctx);

        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Dispatchers")
        static FJsonRpcResponse OnScanResponseReceived(const FPixelStreamingMethodContext& Ctx);

        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Dispatchers")
        static FJsonRpcResponse OnGeoPinsReceived(const FPixelStreamingMethodContext& Ctx);

        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "PixelStreaming|MethodHandler|Dispatchers")
        static FJsonRpcResponse OnTranslateCameraLocationReceived(const FPixelStreamingMethodContext& Ctx);

        friend class FPixelStreamingMethodHandlerRegularTests;
        friend class FPixelStreamingMethodHandlerDegenerateTests;
};