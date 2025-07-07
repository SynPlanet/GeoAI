//
//// Copyright META. All Rights Reserved. \\\\
//

#pragma once

#include "JsonRpcError.h"
#include "JsonRpcObject.h"
#include "JsonRpcResponse.generated.h"

/**
* FJsonRpcResponse is a struct representing a JSON-RPC response. It inherits from FJsonRpcObject.
*/
USTRUCT(BlueprintType)
struct JSONRPC_API FJsonRpcResponse : public FJsonRpcObject
{
        GENERATED_BODY()

        /**
        * @brief Default constructor for FJsonRpcResponse.
        *
        * This constructor initializes a new instance of the FJsonRpcResponse class with default values.
        *
        * @return None.
        */
        FJsonRpcResponse() = default;

        /**
        * @brief Constructor for a new FJsonRpcResponse instance from the given FJsonObject.
        *
        * This constructor initializes a new FJsonRpcResponse instance by calling the constructor of its base class, FJsonRpcObject,
        * with the provided ResponseJsonObject.
        *
        * @param ResponseJsonObject The FJsonObject to create the FJsonRpcResponse.
        */
        explicit FJsonRpcResponse(const FJsonObject& ResponseJsonObject);

        /**
        * @brief Constructor for FJsonRpcResponse.
        *
        * This constructor creates a new FJsonRpcResponse instance with the given Guid, Result, and Error objects.
        *
        * @param Guid The unique identifier of the JSON-RPC response object.
        * @param Result The JSON object containing the result data.
        * @param Error The JSON-RPC error object.
        */
        explicit FJsonRpcResponse(const FGuid& Guid, const FJsonObject& Result, const FJsonRpcError& Error = {});

        /**
        * @brief Construct a new FJsonRpcResponse object with the given Guid and Error.
        *
        * @param Guid The Guid to set.
        * @param Error The error information to set.
        */
        explicit FJsonRpcResponse(const FGuid& Guid, const FJsonRpcError& Error);

        /**
        * @brief Extracts the result from the JSON-RPC response.
        *
        * This method returns the JSON-RPC response result as an FJsonObject.
        * If the response does not contain a result field, an empty FJsonObject is returned.
        *
        * @return The JSON-RPC response result as an FJsonObject.
        */

        FJsonObject GetResult() const;

        /**
        * Sets the result field of the JSON-RPC response object.
        * This method sets the value of the "result" field of the underlying JsonObject to the provided result object.
        *
        * @param Result The FJsonObject representing the result.
        *
        * @return None.
        */
        void SetResult(const FJsonObject& Result) const;

        /**
        * @brief Sets the result field of the JSON-RPC response object.
        *
        * This method sets the "result" field of the corresponding JsonObject as a string.
        *
        * @param Message The message to be set as the result.
        *
        * @return void.
        */
        void SetResult(const FString& Message) const;

        /**
        * @brief Resets the result field of the JSON-RPC response object.
        *
        * This method removes the "result" field from the JSON object representing the JSON-RPC response.
        * Any existing value associated with the "result" field will be lost.
        */
        void UnsetResult() const;

        /**
        * @brief Retrieves the error object from the JSON-RPC response.
        *
        * This method retrieves the error object from the JSON-RPC response. If the JSON-RPC response contains an "error" field, it is retrieved and returned as an
        * FJsonRpcError object. If "error" is not found, an empty FJsonRpcError object is returned.
        *
        * @return An FJsonRpcError object representing the error, or an empty FJsonRpcError object if no errors were found.
        *
        * @see FJsonRpcError
        */
        FJsonRpcError GetError() const;

        /**
        * @brief Sets the error object for the JSON-RPC response.
        *
        * This method sets the error object for the JSON-RPC response using the provided FJsonRpcError object.
        * An error object is created by setting the "error" field on the underlying JsonObject.
        *
        * @param Error An FJsonRpcError object containing the error information.
        *
        * @see FJsonRpcError
        */
        void SetError(const FJsonRpcError& Error) const;

        /**
        * @brief Removes the "error" field from the JSON-RPC response object.
        *
        * This method removes the "error" field from the underlying JsonObject of the JSON-RPC response.
        * After calling this method, any error information associated with the response will be cleared.
        */
        void UnsetError() const;
};