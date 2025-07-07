//
////  Copyright META. All Rights Reserved.  \\\\
//

#include "JsonRpcLibrary.h"
#include "Dom/JsonObject.h"

FJsonRpcObject UJsonRpcLibrary::MakeJsonRpcObject(const FGuid& Guid)
{
        return FJsonRpcObject(Guid);
}

FJsonRpcRequest UJsonRpcLibrary::MakeJsonRpcRequest(const FGuid& Guid, const FString& Method, const FJsonObjectWrapper& Params)
{
        const auto ParamsJsonObject = Params.JsonObject;
        return FJsonRpcRequest(Guid, Method, ParamsJsonObject ? *ParamsJsonObject : FJsonObject{});
}

FJsonRpcResponse UJsonRpcLibrary::MakeJsonRpcResponse(const FGuid& Guid, const FJsonObjectWrapper& Result, const FJsonRpcError& Error)
{
        const auto ResultJsonObject = Result.JsonObject;
        return FJsonRpcResponse(Guid, ResultJsonObject ? *ResultJsonObject : FJsonObject{}, Error);
}

FJsonRpcError UJsonRpcLibrary::MakeJsonRpcError(EJsonRpcErrorType Type, const FString& Message)
{
        return FJsonRpcError(GetErrorTypeCode(Type), Message);
}

FGuid UJsonRpcLibrary::GetGuid(const FJsonRpcObject& Object)
{
        return Object.GetGuid();
}

FString UJsonRpcLibrary::GetMethod(const FJsonRpcRequest& Request)
{
        return Request.GetMethod();
}

FJsonObjectWrapper UJsonRpcLibrary::GetParams(const FJsonRpcRequest& Request)
{
        FJsonObjectWrapper Wrapper;
        Wrapper.JsonObject = MakeShared<FJsonObject>(Request.GetParams());
        return Wrapper;
}

FJsonObjectWrapper UJsonRpcLibrary::GetResult(const FJsonRpcResponse& Response)
{
        FJsonObjectWrapper Wrapper;
        Wrapper.JsonObject = MakeShared<FJsonObject>(Response.GetResult());
        return Wrapper;
}

FJsonRpcError UJsonRpcLibrary::GetError(const FJsonRpcResponse& Response)
{
        return Response.GetError();
}

int32 UJsonRpcLibrary::GetCode(const FJsonRpcError& Error)
{
        return Error.GetCode();
}

FString UJsonRpcLibrary::GetMessage(const FJsonRpcError& Error)
{
        return Error.GetMessage();
}

int32 UJsonRpcLibrary::GetErrorTypeCode(EJsonRpcErrorType ErrorType)
{
        if (const auto* const Code = ErrorCodeLookupTable.Find(ErrorType); Code)
        {
                return StaticCast<int32>(*Code);
        }

        return 0;
}
