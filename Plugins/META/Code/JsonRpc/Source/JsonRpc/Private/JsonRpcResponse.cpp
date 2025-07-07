//
////  Copyright META. All Rights Reserved.  \\\\
//

#include "JsonRpcResponse.h"

FJsonRpcResponse::FJsonRpcResponse(const FJsonObject& ResponseJsonObject) : FJsonRpcObject(ResponseJsonObject) {}

FJsonRpcResponse::FJsonRpcResponse(const FGuid& Guid, const FJsonObject& Result, const FJsonRpcError& Error) : FJsonRpcObject(Guid)
{
        if (!Result.Values.IsEmpty())
        {
                SetResult(Result);
        }

        if (Error)
        {
                SetError(Error);
        }
}

FJsonRpcResponse::FJsonRpcResponse(const FGuid& Guid, const FJsonRpcError& Error) : FJsonRpcResponse(Guid, {}, Error) {}

FJsonObject FJsonRpcResponse::GetResult() const
{
        if (const TSharedPtr<FJsonObject>* ResultJsonObject; JsonObject->TryGetObjectField(TEXT("result"), ResultJsonObject))
        {
                return **ResultJsonObject;
        }

        return {};
}

void FJsonRpcResponse::SetResult(const FJsonObject& Result) const
{
        JsonObject->SetObjectField("result", MakeShared<FJsonObject>(Result));
}

void FJsonRpcResponse::SetResult(const FString& Message) const
{
        JsonObject->SetStringField("result", *Message);
}

void FJsonRpcResponse::UnsetResult() const
{
        JsonObject->RemoveField(TEXT("result"));
}

void FJsonRpcResponse::UnsetError() const
{
        JsonObject->RemoveField(TEXT("error"));
}

void FJsonRpcResponse::SetError(const FJsonRpcError& Error) const
{
        JsonObject->SetObjectField("error", Error.JsonObject);
}

FJsonRpcError FJsonRpcResponse::GetError() const
{
        if (const TSharedPtr<FJsonObject>* ErrorJsonObject; JsonObject->TryGetObjectField(TEXT("error"), ErrorJsonObject))
        {
                return FJsonRpcError{ **ErrorJsonObject };
        }

        return {};
}
