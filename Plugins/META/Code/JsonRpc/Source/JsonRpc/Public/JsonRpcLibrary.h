//
////  Copyright META. All Rights Reserved.  \\\\
//

#pragma once

#include "JsonRpcError.h"
#include "JsonRpcRequest.h"
#include "JsonRpcResponse.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonRpcLibrary.generated.h"

/**
* @brief An enumeration defining the various JSON-RPC error types.
*/
UENUM(BlueprintType)
enum class EJsonRpcErrorType : uint8
{
        None,
        ParseError,
        InvalidRequest,
        MethodNotFound,
        InvalidParams,
        InternalError,
};

/**
* Creates a new JsonRpcObject with the specified GUID.
*
* @param Guid The GUID for the JsonRpcObject.
* @return The created JsonRpcObject.
*/
UCLASS()
class JSONRPC_API UJsonRpcLibrary : public UBlueprintFunctionLibrary
{
        GENERATED_BODY()

        /**
        * @brief This lookup table maps JSON-RPC error types to error codes.
        */
        static inline const TMap<EJsonRpcErrorType, EJsonRpcErrorCode> ErrorCodeLookupTable = {
                { EJsonRpcErrorType::None, EJsonRpcErrorCode::None },
                { EJsonRpcErrorType::ParseError, EJsonRpcErrorCode::ParseError },
                { EJsonRpcErrorType::InvalidRequest, EJsonRpcErrorCode::InvalidRequest },
                { EJsonRpcErrorType::MethodNotFound, EJsonRpcErrorCode::MethodNotFound },
                { EJsonRpcErrorType::InvalidParams, EJsonRpcErrorCode::InvalidParams },
                { EJsonRpcErrorType::InternalError, EJsonRpcErrorCode::InternalError },
        };

public:
        /**
        * @brief Creates a JSON-RPC object with the specified GUID.
        *
        * @param Guid The GUID to set for the JSON-RPC object.
        *
        * @return The created JSON-RPC object.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc", Meta = (AutoCreateRefTerm = "Guid"))
        static FJsonRpcObject MakeJsonRpcObject(const FGuid& Guid);

        /**
        * @brief Creates a JSON-RPC request object.
        *
        * @param Guid The unique identifier for the request.
        * @param Method The name of the JSON-RPC method.
        * @param Params The parameters of the JSON-RPC request.
        * @return The created JSON-RPC request object.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc", Meta = (AutoCreateRefTerm = "Guid, Params"))
        static FJsonRpcRequest MakeJsonRpcRequest(const FGuid& Guid, const FString& Method, const FJsonObjectWrapper& Params);

        /**
        * @brief Creates a JSON-RPC response object.
        *
        * This method creates a JSON-RPC response object with the specified parameters.
        *
        * @param Guid Unique identifier of the response.
        * @param Result The result of the JSON-RPC request. Must be of type FJsonObjectWrapper.
        * @param Error JSON-RPC error object. Must be of type FJsonRpcError.
        *
        * @return FJsonRpcResponse The created JSON-RPC response object.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc", Meta = (AutoCreateRefTerm = "Guid, Result, Error"))
        static FJsonRpcResponse MakeJsonRpcResponse(const FGuid& Guid, const FJsonObjectWrapper& Result, const FJsonRpcError& Error);

        /**
        * @brief Creates a JSON-RPC error object with the specified error type and message.
        *
        * This method creates an FJsonRpcError object with the specified error type and message.
        *
        * @param Type The error type.
        * @param Message The error message.
        * @return An FJsonRpcError object representing the JSON-RPC error.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc")
        static FJsonRpcError MakeJsonRpcError(EJsonRpcErrorType Type, const FString& Message);

        /**
        * @brief Retrieves the unique identifier (GUID) associated with the JSON-RPC object.
        *
        * @param Object The JSON-RPC object.
        *
        * @return The unique identifier (GUID) associated with the JSON-RPC object.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc")
        static FGuid GetGuid(const FJsonRpcObject& Object);

        /**
        * Retrieves the method from this JSON-RPC request.
        *
        * @param Request The JSON-RPC request object.
        * @return The request method.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc|Request")
        static FString GetMethod(const FJsonRpcRequest& Request);

        /**
        * @brief Retrieves the JSON-RPC request parameters.
        *
        * This function takes a JSON-RPC request object and returns the request parameters
        * as an `FJsonObjectWrapper
        *
        * @param Request The JSON-RPC request object.
        * @return The request parameters as an `FJsonObjectWrapper` object.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc|Request")
        static FJsonObjectWrapper GetParams(const FJsonRpcRequest& Request);

        /**
        * Extracts the result object from this JsonRpcResponse.
        *
        * @param Response The JsonRpcResponse object.
        * @return The result object as a JsonObjectWrapper.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc|Response")
        static FJsonObjectWrapper GetResult(const FJsonRpcResponse& Response);

        /**
        * @brief Extracts the error object from the JSON-RPC response.
        *
        * This function extracts the error object from the specified JSON-RPC response.
        *
        * @param Response The JSON-RPC response object.
        *
        * @return The error object from the JSON-RPC response.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc|Response")
        static FJsonRpcError GetError(const FJsonRpcResponse& Response);

        /**
        * @brief Retrieves the error code from this FJsonRpcError object.
        *
        * This method retrieves the error code for the JSON-RPC error object.
        *
        * @param Error The FJsonRpcError object from which to retrieve the error code.
        * @return The error code as an integer.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc|Error")
        static int32 GetCode(const FJsonRpcError& Error);

        /**
        * @brief Retrieves the error message from this FJsonRpcError object.
        *
        * This method returns the error message associated with the JSON-RPC error.
        *
        * @param Error The FJsonRpcError object from which to retrieve the error message.
        *
        * @return The error message as an FString.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc|Error")
        static FString GetMessage(const FJsonRpcError& Error);

        /**
        * Gets the error type code for the specified error type.
        *
        * @param ErrorType The error type.
        * @return The error type code.
        */
        UFUNCTION(BlueprintPure, Category = "JsonRpc|Error")
        static int32 GetErrorTypeCode(EJsonRpcErrorType ErrorType);
};
