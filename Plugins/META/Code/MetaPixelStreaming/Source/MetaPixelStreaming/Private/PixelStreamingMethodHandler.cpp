#include "PixelStreamingMethodHandler.h"

#include "CesiumGeoreference.h"
#include "JsonObjectConverter.h"
#include "JsonRpcLibrary.h"
#include "PixelStreamingClient.h"
#include "PixelStreamingMethod.h"
#include "PixelStreamingMethodContext.h"
#include "Components/CameraTranslationComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameplayMessageTags.h"
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

FJsonRpcResponse UPixelStreamingMethodHandler::CreateSimpleJsonResponse(FGuid RequestGuid, const EJsonRpcErrorCode& ErrorCode, const FString& Message)
{
        TSharedRef<FJsonObject> ResponseObject = MakeShared<FJsonObject>();
        FJsonRpcError JsonRpcError;

        const bool bHaveError = !Message.IsEmpty();

        if (bHaveError)
        {
                JsonRpcError.SetCode(StaticCast<int32>(ErrorCode));
                JsonRpcError.SetMessage(Message);
        }

        FJsonObjectWrapper Result;

        ResponseObject->SetBoolField(TEXT("success"), !bHaveError);
        Result.JsonObject = ResponseObject;

        return UJsonRpcLibrary::MakeJsonRpcResponse(RequestGuid, Result, JsonRpcError);
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
        META_REGISTER_DISPATCHER(OnTranslateCameraLocationReceived);

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
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidRequest, TEXT("Caller doesn't existed!"));
        }

        FPlayerControllerEventMessage NewMessage;
        NewMessage.InstigatingController = Caller;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnUserChanged, MoveTemp(NewMessage));

        return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::None, FString());
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnBackToOrigin(const FPixelStreamingMethodContext& Ctx)
{
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        const auto Guid = Ctx.Request.GetGuid();
        if (!IsValid(Caller))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidRequest, TEXT("Caller doesn't existed!"));
        }

        FPlayerControllerEventMessage NewMessage;
        NewMessage.InstigatingController = Caller;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnBackToOrigin, MoveTemp(NewMessage));

        return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::None, FString());
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnGetSearchAreaScale(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        if (!IsValid(Ctx.Caller))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidRequest, TEXT("Caller doesn't existed!"));
        }

        float ResultScale;
        if (!Ctx.Request.GetParams().TryGetNumberField(TEXT("scale"), ResultScale))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidParams, TEXT("Invalid `scale` parameter!"));
        }

        FScaleObjectMessage NewMessage;
        NewMessage.InstigatingController = Caller;
        NewMessage.ScaleValue = ResultScale;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnGetSearchAreaScale, MoveTemp(NewMessage));

        return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::None, FString());
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnChangeActiveStateSearchArea(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        if (!IsValid(Ctx.Caller))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidRequest, TEXT("Caller doesn't existed!"));
        }

        bool bEnableArea;
        if (!Ctx.Request.GetParams().TryGetBoolField(TEXT("active"), bEnableArea))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidParams, "Invalid `active` parameter.");
        }

        FChangeObjectStateMessage NewMessage;
        NewMessage.InstigatingController = Caller;
        NewMessage.bState = bEnableArea;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnChangeActiveStateSearchArea,
                                                                                               MoveTemp(NewMessage));
        return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::None, FString());
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnScanResponseReceived(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        if (!IsValid(Ctx.Caller))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InternalError, "Invalid caller.");
        }

        FPlayerControllerEventMessage NewMessage;
        NewMessage.InstigatingController = Caller;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnScanResponseReceived,
                                                                                               MoveTemp(NewMessage));
        return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::None, FString());
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnGeoPinsReceived(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        if (!IsValid(Ctx.Caller))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidRequest, TEXT("Caller doesn't existed!"));
        }

        const FJsonObject Params = Ctx.Request.GetParams();

        const ACesiumGeoreference* const CesiumGeoreference = UGeoAiUtilitiesLibrary::GetCesiumGeoreference(Caller);
        check(CesiumGeoreference);

        const TArray<TSharedPtr<FJsonValue>>* Pins{};
        if (!Params.TryGetArrayField(TEXT("pins"), Pins))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidParams, "Invalid `pins` parameter.");
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
                        CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidParams, "OnGeoPinsReceived: Invalid `coordinates` parameter.");
                        continue;
                }

                FString PinId;
                if (!Object->TryGetStringField(TEXT("id"), PinId))
                {
                        CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidParams, "OnGeoPinsReceived: Invalid `id` parameter.");
                        continue;
                }

                NewPin.GeoPositions = FVector((*Coordinates)[1]->AsNumber(), (*Coordinates)[0]->AsNumber(), 0.0);
                NewPin.WorldPositions = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(NewPin.GeoPositions);
                NewPin.WorldPositions.Z = 500.0;
                NewPin.Id = PinId;

                NewMessage.PinsInfo.Add(MoveTemp(NewPin));
        }

        if (NewMessage.PinsInfo.IsEmpty())
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidRequest, "OnGeoPinsReceived: Nothing to show.");
        }

        NewMessage.InstigatingController = Caller;

        Caller->GetGameInstance()->GetSubsystem<UGameplayMessageSubsystem>()->BroadcastMessage(GameplayTagsRouter::OnGeoPinsReceived, MoveTemp(NewMessage));

        return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::None, FString());
}

FJsonRpcResponse UPixelStreamingMethodHandler::OnTranslateCameraLocationReceived(const FPixelStreamingMethodContext& Ctx)
{
        verify(IsAvailable());

        const auto Guid = Ctx.Request.GetGuid();
        const TObjectPtr<APlayerController> Caller = Ctx.Caller;

        if (!IsValid(Ctx.Caller))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidRequest, TEXT("Caller doesn't existed!"));
        }

        const FJsonObject Params = Ctx.Request.GetParams();

        const ACesiumGeoreference* const CesiumGeoreference = UGeoAiUtilitiesLibrary::GetCesiumGeoreference(Caller);
        check(CesiumGeoreference);

        const TArray<TSharedPtr<FJsonValue>>* Coordinates{};
        if (!Params.TryGetArrayField(TEXT("coordinates"), Coordinates))
        {
                return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InvalidParams, "Invalid `coordinates` parameter.");
        }

        const TArray<TSharedPtr<FJsonValue>>* Rotate{};
        Params.TryGetArrayField(TEXT("rotate"), Rotate);

        const FVector ResultGeoLocation = FVector((*Coordinates)[1]->AsNumber(), (*Coordinates)[0]->AsNumber(), 0.0);
        const FVector ResultWorldLocation = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(ResultGeoLocation);

        if (auto* TranslationComponent = Caller->FindComponentByClass<UCameraTranslationComponent>(); IsValid(TranslationComponent))
        {
                if (Rotate->IsEmpty())
                {
                        TranslationComponent->SetTargetLocation(ResultGeoLocation);
                }
                else
                {
                        const FRotator ResultingRotation = FRotator((*Rotate)[0]->AsNumber(), (*Rotate)[1]->AsNumber(), 0.0);
                        TranslationComponent->SetTargetTransform(FTransform(FQuat(ResultingRotation), ResultWorldLocation));
                }
        }
        else
        {
                TranslationComponent = StaticCast<UCameraTranslationComponent*>(
                        Caller->AddComponentByClass(UCameraTranslationComponent::StaticClass(), false, FTransform::Identity, true));

                if (!IsValid(TranslationComponent))
                {
                        return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::InternalError, TEXT("Translation component can't be create"));
                }

                if (Rotate->IsEmpty())
                {
                        TranslationComponent->SetTargetLocation(ResultGeoLocation);
                }
                else
                {
                        const FRotator ResultingRotation = FRotator((*Rotate)[0]->AsNumber(), (*Rotate)[1]->AsNumber(), 0.0);
                        TranslationComponent->SetTargetTransform(FTransform(FQuat(ResultingRotation), ResultWorldLocation));
                }

                Caller->FinishAddComponent(TranslationComponent, false, FTransform::Identity);
        }

        return CreateSimpleJsonResponse(Guid, EJsonRpcErrorCode::None, FString());
}
