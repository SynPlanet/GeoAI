//
////  Copyright META. All Rights Reserved.  \\\\
//

#pragma once

#include "JsonObjectWrapper.h"
#include "JsonRpcObject.generated.h"

/**
* @struct FJsonRpcObject
* @brief Represents a JSON-RPC object.
*
* This class is a wrapper around FJsonObject, providing additional functionality for working with JSON-RPC objects.
*/
USTRUCT(BlueprintType)
struct JSONRPC_API FJsonRpcObject : public FJsonObjectWrapper
{
        GENERATED_BODY()

        /**
        * FJsonRpcObject constructor.
        * Initializes a new instance of the FJsonRpcObject class.
        *
        * @return None.
        */
        FJsonRpcObject();

        /**
        * @brief Constructs a FJsonRpcObject from the given FJsonObject.
        *
        * @param JsonObject The FJsonObject from which to construct the FJsonRpcObject.
        */
        explicit FJsonRpcObject(const FJsonObject& JsonObject);

        /**
        * Constructor for FJsonRpcObject that takes a Guid parameter.
        *
        * @param Guid The GUID to assign to the object.
        */
        explicit FJsonRpcObject(const FGuid& Guid);

        /**
        * @brief Gets the Guid from the JsonRpcObject.
        *
        * @return the Guid associated with the JsonRpcObject.
        */

        FGuid GetGuid() const;

        /**
        * Sets the Guid for the JsonRpcObject.
        *
        * @param Guid The Guid to set.
        */
        void SetGuid(const FGuid& Guid) const;

        /**
        * @brief Resets the protocol version of the JSON-RPC object to "2.0".
        *
        * This method sets the value of the "jsonrpc" field in the underlying JsonObject to "2.0".
        * This ensures that the JSON-RPC object is compatible with the JSON-RPC 2.0 protocol.
        */
        void ResetProtocolVersion() const;
};
