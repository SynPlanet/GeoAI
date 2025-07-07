//
////  Copyright META. All Rights Reserved.  \\\\
//

#pragma once

#include "JsonRpcObject.h"
#include "JsonRpcRequest.generated.h"
/**
* @class FJsonRpcRequest
*
* This class represents a JSON-RPC request object.
*/
USTRUCT(BlueprintType)
struct JSONRPC_API FJsonRpcRequest : public FJsonRpcObject
{
        GENERATED_BODY()

        /**
        * @brief Default constructor for FJsonRpcRequest.
        *
        * This constructor initializes a new instance of the FJsonRpcRequest class with default values.
        */
        FJsonRpcRequest() = default;

        /**
        * FJsonRpcRequest constructor.
        * Initializes a new instance of the FJsonRpcRequest class with the specified RequestJsonObject.
        *
        * @param RequestJsonObject a JSON object representing the JSON-RPC request.
        * @return None. */
        explicit FJsonRpcRequest(const FJsonObject& RequestJsonObject);

        /**
        * Creates a new Json RPC request object.
        *
        * @param Guid GUID associated with the request.
        * @param Method Name of the request method.
        * @param Params Request parameters as a JSON object.
        */
        explicit FJsonRpcRequest(const FGuid& Guid, const FString& Method, const FJsonObject& Params);

        /**
        * Gets the name of the JSON-RPC request method.
        *
        * @return The name of the JSON-RPC request method.
        */

        FString GetMethod() const;

        /**
        * Sets the JsonRpcRequest method.
        *
        * @param Method Method to set.
        */
        void SetMethod(const FString& Method) const;

        /**
        * Retrieves the parameters object from the JSON-RPC request.
        *
        * @return The JSON-RPC request parameters object.
        */

        FJsonObject GetParams() const;

        /**
        * Sets the parameters of the JSON-RPC request.
        *
        * @param Params A JSON object representing the parameters.
        * @return None.
        */
        void SetParams(const FJsonObject& Params) const;
};
