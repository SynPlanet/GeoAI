//
////  Copyright META. All Rights Reserved.  \\\\
//

#include "JsonRpcObject.h"

FJsonRpcObject::FJsonRpcObject() : FJsonRpcObject(FJsonObject{}) {}

FJsonRpcObject::FJsonRpcObject(const FJsonObject& ShardJsonObject)
{
        JsonObject = MakeShared<FJsonObject>(ShardJsonObject);
        ResetProtocolVersion();
}

FJsonRpcObject::FJsonRpcObject(const FGuid& Guid) : FJsonRpcObject()
{
        SetGuid(Guid);
}

FGuid FJsonRpcObject::GetGuid() const
{
        FGuid Guid;
        if (FString GuidStr; JsonObject->TryGetStringField(TEXT("guid"), GuidStr))
        {
                FGuid::Parse(JsonObject->GetStringField(TEXT("guid")), Guid);
        }

        return Guid;
}

void FJsonRpcObject::SetGuid(const FGuid& Guid) const
{
        return JsonObject->SetStringField("guid", Guid.ToString());
}

void FJsonRpcObject::ResetProtocolVersion() const
{
        JsonObject->SetStringField("jsonrpc", "2.0");
}
