//
////  Copyright META. All Rights Reserved.  \\\\
//

#include "JsonRpcRequest.h"

FJsonRpcRequest::FJsonRpcRequest(const FJsonObject& RequestJsonObject) : FJsonRpcObject(RequestJsonObject) {}

FJsonRpcRequest::FJsonRpcRequest(const FGuid& Guid, const FString& Method, const FJsonObject& Params) : FJsonRpcObject(Guid)
{
        SetMethod(Method);
        SetParams(Params);
}

FString FJsonRpcRequest::GetMethod() const
{
        if (FString Method; JsonObject->TryGetStringField(TEXT("method"), Method))
        {
                return Method;
        }

        return {};
}

void FJsonRpcRequest::SetMethod(const FString& Method) const
{
        return JsonObject->SetStringField("method", Method);
}

FJsonObject FJsonRpcRequest::GetParams() const
{
        if (const TSharedPtr<FJsonObject>* ParamsJsonObject; JsonObject->TryGetObjectField(TEXT("params"), ParamsJsonObject))
        {
                return **ParamsJsonObject;
        }

        return {};
}

void FJsonRpcRequest::SetParams(const FJsonObject& Params) const
{
        return JsonObject->SetObjectField("params", MakeShared<FJsonObject>(Params));
}
