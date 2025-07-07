//
////  Copyright META. All Rights Reserved.  \\\\
//

#pragma once

#include "CoreMinimal.h"
#include "JsonRpcRequest.h"
#include "PixelStreamingMethodContext.generated.h"

class APlayerController;

USTRUCT(Blueprintable, BlueprintType, Category = "PixelStreaming")
struct METAPIXELSTREAMING_API FPixelStreamingMethodContext
{
        GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PixelStreaming|MethodContext")
        FJsonRpcRequest Request{};

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PixelStreaming|MethodContext")
        TObjectPtr<APlayerController> Caller{};
};
