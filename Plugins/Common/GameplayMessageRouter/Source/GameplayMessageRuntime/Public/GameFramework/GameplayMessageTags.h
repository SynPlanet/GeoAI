// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "GameplayMessageTags.generated.h"

#define UE_API GAMEPLAYMESSAGERUNTIME_API

class APlayerController;
class UObject;

USTRUCT(BlueprintType)
struct FPlayerControllerEventMessage
{
        GENERATED_BODY()
        
        UPROPERTY(BlueprintReadWrite)
        TObjectPtr<APlayerController> InstigatingController = nullptr;
};

USTRUCT(BlueprintType)
struct FScaleObjectMessage
{
        GENERATED_BODY()
        
        UPROPERTY(BlueprintReadWrite)
        TObjectPtr<APlayerController> InstigatingController = nullptr;

        UPROPERTY(BlueprintReadWrite)
        float ScaleValue = 0;
};

USTRUCT(BlueprintType)
struct FChangeObjectStateMessage
{
        GENERATED_BODY()
        
        UPROPERTY(BlueprintReadWrite)
        TObjectPtr<APlayerController> InstigatingController = nullptr;

        UPROPERTY(BlueprintReadWrite)
        bool bState = false;
};

USTRUCT(BlueprintType)
struct FMovementControllMessage
{
        GENERATED_BODY()
        
        UPROPERTY(BlueprintReadWrite)
        TObjectPtr<APlayerController> InstigatingController = nullptr;

        UPROPERTY(BlueprintReadWrite)
        bool bMovementAvailable = true;

        UPROPERTY(BlueprintReadWrite)
        bool bForwardMoving = true;
};


USTRUCT(BlueprintType)
struct FVisualizationScreenTouchInfo
{
        GENERATED_BODY()
        
        UPROPERTY(BlueprintReadWrite)
        TObjectPtr<APlayerController> InstigatingController = nullptr;

        UPROPERTY(BlueprintReadWrite)
        FVector2D ScreenPosition = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct FGeoPinInfo
{
        GENERATED_BODY()
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Id;

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector GeoPositions;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FVector WorldPositions;
};

USTRUCT(BlueprintType)
struct FGeoPinsInfo
{
        GENERATED_BODY()

        UPROPERTY(BlueprintReadWrite)
        TObjectPtr<APlayerController> InstigatingController = nullptr;

        UPROPERTY(BlueprintReadWrite)
        TArray<FGeoPinInfo> PinsInfo;
};

namespace GameplayTagsRouter
{
        UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnUserChanged);
        UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnChangeActiveStateSearchArea);
        UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnGetSearchAreaScale);
        UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnBackToOrigin);
        UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnScanResponseReceived);
        UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(OnGeoPinsReceived);

        
        UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ShowScreenTouchInfo);

}

#undef UE_API
