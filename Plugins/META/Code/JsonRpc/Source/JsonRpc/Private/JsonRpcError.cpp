//
////  Copyright META. All Rights Reserved.  \\\\
//

#include "JsonRpcError.h"

FJsonRpcError::FJsonRpcError(const FJsonObject& ErrorJsonObject)
{
        JsonObject = MakeShared<FJsonObject>(ErrorJsonObject);
}

FJsonRpcError::FJsonRpcError(int32 Code, const FString& Message)
{
        SetCode(Code);
        SetMessage(Message);
}

FJsonRpcError::FJsonRpcError(EJsonRpcErrorCode Code, const FString& Message) : FJsonRpcError(StaticCast<int32>(Code), Message) {}

int32 FJsonRpcError::GetCode() const
{
        if (int32 Code; JsonObject->TryGetNumberField(TEXT("code"), Code))
        {
                return Code;
        }

        return {};
}

void FJsonRpcError::SetCode(int32 ErrorCode) const
{
        return JsonObject->SetNumberField("code", ErrorCode);
}

FString FJsonRpcError::GetMessage() const
{
        if (FString Message; JsonObject->TryGetStringField(TEXT("message"), Message))
        {
                return Message;
        }

        return {};
}

void FJsonRpcError::SetMessage(const FString& ErrorMessage) const
{
        return JsonObject->SetStringField("message", ErrorMessage);
}

bool FJsonRpcError::operator==(const FJsonRpcError& Other) const
{
        return GetCode() == Other.GetCode() && GetMessage() == Other.GetMessage();
}

bool FJsonRpcError::operator!=(const FJsonRpcError& Other) const
{
        return !(*this == Other);
}

FJsonRpcError::operator bool() const noexcept
{
        return *this != FJsonRpcError{};
}
