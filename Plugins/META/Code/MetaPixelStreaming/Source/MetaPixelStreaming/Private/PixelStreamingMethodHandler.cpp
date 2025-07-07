#include "PixelStreamingMethodHandler.h"

#include "CesiumGeoreference.h"
#include "JsonObjectConverter.h"
#include "JsonRpcLibrary.h"
#include "PixelStreamingClient.h"
#include "PixelStreamingMethod.h"
#include "PixelStreamingMethodContext.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameplayMessageTags.h"
#include "Kismet/GameplayStatics.h"
#include "System/GeoAiUtilitiesLibrary.h"

DEFINE_LOG_CATEGORY(LogPixelStreamingMethodHandler);

bool UPixelStreamingMethodHandler::IsAvailable()
{
        return IsValid(Singleton.Get());
}

UGameInstance* UPixelStreamingMethodHandler::GetGameInstance() const noexcept
{
        return GameInstance;
}

UWorld* UPixelStreamingMethodHandler::GetWorld() const
{
        return !IsValid(GameInstance) ? nullptr : GameInstance->GetWorld();
}

void UPixelStreamingMethodHandler::SendTimerPolygonCoordinates(APlayerController* Instigator, UPARAM(ref) const FVector& Checkpoint, const float Radius)
{
        verify(IsAvailable());

        FJsonObject AreaCoordinates;

        TArray<TSharedPtr<FJsonValue>> CoordinatesArray;
        CoordinatesArray.Add(MakeShared<FJsonValueNumber>(Checkpoint.X));
        CoordinatesArray.Add(MakeShared<FJsonValueNumber>(Checkpoint.Y));

        AreaCoordinates.SetArrayField("coordinates", CoordinatesArray);

        AreaCoordinates.SetNumberField("radius", Radius);

        if (const UPixelStreamingClient* const Client = UPixelStreamingClient::Get(); IsValid(Client))
        {
                Client->SendRpcObject(FJsonRpcRequest(FGuid::NewGuid(), FPixelStreamingMethod::SendTimerPolygonCoordinates, AreaCoordinates), Instigator);
        }
}

void UPixelStreamingMethodHandler::SendScreenTouchAsWGS(APlayerController* Instigator, const FVector& Checkpoint)
{
        verify(IsAvailable());

        FJsonObject AreaCoordinates;

        TArray<TSharedPtr<FJsonValue>> Coordinate;

        Coordinate.Add(MakeShared<FJsonValueNumber>(Checkpoint.X));
        Coordinate.Add(MakeShared<FJsonValueNumber>(Checkpoint.Y));

        AreaCoordinates.SetArrayField("coordinate", Coordinate);

        if (const UPixelStreamingClient* const Client = UPixelStreamingClient::Get(); IsValid(Client))
        {
                Client->SendRpcObject(FJsonRpcRequest(FGuid::NewGuid(), FPixelStreamingMethod::SendScreenTouchAsWGS, AreaCoordinates), Instigator);
        }
}

void UPixelStreamingMethodHandler::SendSearchAreaStartScale(APlayerController* Instigator, const float InScale)
{
        verify(IsAvailable());

        FJsonObject ScaleJsonObject;

        ScaleJsonObject.SetNumberField("scale", InScale);

        if (const UPixelStreamingClient* const Client = UPixelStreamingClient::Get(); IsValid(Client))
        {
                Client->SendRpcObject(FJsonRpcRequest(FGuid::NewGuid(), FPixelStreamingMethod::SendSearchAreaStartScale, ScaleJsonObject), Instigator);
        }
}

void UPixelStreamingMethodHandler::SendGeoPinClick(APlayerController* Instigator, const FString& Id)
{
        verify(IsAvailable());

        FJsonObject ScaleJsonObject;

        ScaleJsonObject.SetStringField("id", Id);

        if (const UPixelStreamingClient* const Client = UPixelStreamingClient::Get(); IsValid(Client))
        {
                Client->SendRpcObject(FJsonRpcRequest(FGuid::NewGuid(), FPixelStreamingMethod::SendGeoPinClick, ScaleJsonObject), Instigator);
        }
}

FJsonRpcResponse UPixelStreamingMethodHandler::HandleRequest(const FPixelStreamingMethodContext& Ctx)
{
        if (const auto Dispatcher = FindDispatcher(Ctx.Request.GetMethod()); Dispatcher)
        {
                return (*Dispatcher)(Ctx);
        }

        return FJsonRpcResponse(Ctx.Request.GetGuid(), FJsonRpcError(EJsonRpcErrorCode::MethodNotFound, "Unknown method."));
}

void UPixelStreamingMethodHandler::RegisterDefaultDispatchers()
{
#define META_REGISTER_DISPATCHER(Method) \
        RegisterDispatcher(FPixelStreamingMethod::Method, [this](const FPixelStreamingMethodContext& Ctx) { return Method(Ctx); })

        META_REGISTER_DISPATCHER(OnUserChanged);
        META_REGISTER_DISPATCHER(OnGetSearchAreaScale);
        META_REGISTER_DISPATCHER(OnChangeActiveStateSearchArea);
        META_REGISTER_DISPATCHER(OnBackToOrigin);
        META_REGISTER_DISPATCHER(OnScanResponseReceived);
        META_REGISTER_DISPATCHER(OnGeoPinsReceived);
#undef META_REGISTER_DISPATCHER
}

void UPixelStreamingMethodHandler::RegisterDispatcher(const FString& MethodName, const FPixelStreamingMethodDispatcher& Dispatcher)
{
        Dispatchers.Add(MethodName, Dispatcher);
}

const FPixelStreamingMethodDispatcher* UPixelStreamingMethodHandler::FindDispatcher(const FString& MethodName)
{
        return Dispatchers.Find(MethodName);
}

UPixelStreamingMethodHandler* UPixelStreamingMethodHandler::Get()
{
        return IsAvailable() ? Singleton.Get() : nullptr;
}

UPixelStreamingMethodHandler* UPixelStreamingMethodHandler::Create(UGameInstance* InGameInstance, TSubclassOf<UPixelStreamingMethodHandler> Class)
{
        Singleton = NewObject<UPixelStreamingMethodHandler>(InGameInstance, Class);
        Singleton->GameInstance = InGameInstance;
        Singleton->RegisterDefaultDispatchers();
        return Singleton.Get();
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnUserChanged(const FPixelStreamingMethodContext& Ctx)
{
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        const auto Guid = Ctx.Request.GetGuid();
        if (!IsValid(Caller))
        {
                FJsonRpcError NewError = FJsonRpcError();
                NewError.SetCode(405);
                NewError.SetMessage(TEXT("not existed caller"));

                return FJsonRpcResponse(Guid, FJsonObject{}, NewError);
        }

        FPlayerControllerEventMessage NewMessage;
        NewMessage.InstigatingController = Caller;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnUserChanged, MoveTemp(NewMessage));

        return FJsonRpcResponse(Guid, FJsonObject{});
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnBackToOrigin(const FPixelStreamingMethodContext& Ctx)
{
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        const auto Guid = Ctx.Request.GetGuid();
        if (!IsValid(Caller))
        {
                return FJsonRpcResponse(Guid, FJsonObject{});
        }

        FPlayerControllerEventMessage NewMessage;
        NewMessage.InstigatingController = Caller;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnBackToOrigin, MoveTemp(NewMessage));

        return FJsonRpcResponse(Guid, FJsonObject{});
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnGetSearchAreaScale(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        if (!IsValid(Ctx.Caller))
        {
                return FJsonRpcResponse(Guid, FJsonRpcError(EJsonRpcErrorCode::InternalError, "Invalid caller."));
        }

        float ResultScale;
        if (!Ctx.Request.GetParams().TryGetNumberField(TEXT("scale"), ResultScale))
        {
                return FJsonRpcResponse(Guid, FJsonRpcError(EJsonRpcErrorCode::InvalidParams, "Invalid `value` parameter."));
        }

        FScaleObjectMessage NewMessage;
        NewMessage.InstigatingController = Caller;
        NewMessage.ScaleValue = ResultScale;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnGetSearchAreaScale, MoveTemp(NewMessage));

        return FJsonRpcResponse(Guid, FJsonObject{});
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnChangeActiveStateSearchArea(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        bool bEnableArea;
        if (!Ctx.Request.GetParams().TryGetBoolField(TEXT("active"), bEnableArea))
        {
                return FJsonRpcResponse(Guid, FJsonRpcError(EJsonRpcErrorCode::InvalidParams, "Invalid `value` parameter."));
        }

        if (!IsValid(Ctx.Caller))
        {
                return FJsonRpcResponse(Guid, FJsonRpcError(EJsonRpcErrorCode::InternalError, "Invalid caller."));
        }

        FChangeObjectStateMessage NewMessage;
        NewMessage.InstigatingController = Caller;
        NewMessage.bState = bEnableArea;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnChangeActiveStateSearchArea,
                                                                                               MoveTemp(NewMessage));
        return FJsonRpcResponse(Guid, FJsonObject{});
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnScanResponseReceived(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        if (!IsValid(Ctx.Caller))
        {
                return FJsonRpcResponse(Guid, FJsonRpcError(EJsonRpcErrorCode::InternalError, "Invalid caller."));
        }

        FPlayerControllerEventMessage NewMessage;
        NewMessage.InstigatingController = Caller;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnScanResponseReceived,
                                                                                               MoveTemp(NewMessage));
        return FJsonRpcResponse(Guid, FJsonObject{});
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnGeoPinsReceived(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        if (!IsValid(Ctx.Caller))
        {
                return FJsonRpcResponse(Guid, FJsonRpcError(EJsonRpcErrorCode::InternalError, "Invalid caller."));
        }

        const FJsonObject Params = Ctx.Request.GetParams();

        const ACesiumGeoreference* const CesiumGeoreference = UGeoAiUtilitiesLibrary::GetCesiumGeoreference(Caller);
        check(CesiumGeoreference);

        const TArray<TSharedPtr<FJsonValue>>* Pins{};
        if (!Params.TryGetArrayField(TEXT("pins"), Pins))
        {

                return FJsonRpcResponse(Guid, FJsonRpcError(EJsonRpcErrorCode::InternalError, "pins is empty"));
        }

        FGeoPinsInfo NewMessage;

        for (const auto& Pin : *Pins)
        {
                if (!Pin)
                {
                        continue;
                }

                FGeoPinInfo NewPin;

                const auto Object = Pin->AsObject();

                const TArray<TSharedPtr<FJsonValue>>* Coordinates{};
                if (!Object->TryGetArrayField(TEXT("coordinates"), Coordinates))
                {
                        continue;
                }

                FString PinId;
                if (!Object->TryGetStringField(TEXT("id"), PinId))
                {
                        continue;
                }

                NewPin.GeoPositions = FVector((*Coordinates)[1]->AsNumber(), (*Coordinates)[0]->AsNumber(), 0.0);
                NewPin.WorldPositions = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(NewPin.GeoPositions);
                NewPin.WorldPositions.Z = 500.0;

                NewMessage.PinsInfo.Add(MoveTemp(NewPin));
        }

        if (NewMessage.PinsInfo.IsEmpty())
        {
                return FJsonRpcResponse(Guid, FJsonRpcError(EJsonRpcErrorCode::InternalError, "Nothing to spawn"));
        }

        NewMessage.InstigatingController = Caller;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnGeoPinsReceived, MoveTemp(NewMessage));
        return FJsonRpcResponse(Guid, FJsonObject{});
}
