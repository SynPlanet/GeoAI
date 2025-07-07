//
////  Copyright META. All Rights Reserved.  \\\\
//

#pragma once

#include "JsonObjectWrapper.h"
#include "JsonRpcError.generated.h"

/**
* @brief An enumerator defining the possible error codes for JSON-RPC.
*
* The EJsonRpcErrorCode enumerator provides a list of possible error codes
* that may occur when processing JSON-RPC.
*
* The possible error codes are:
* - None: Represents no error.
* - ParseError: Represents a parse error that occurred when processing JSON-RPC.
* - InvalidRequest: Represents an error about an invalid request made to JSON-RPC.
* - MethodNotFound: Represents a "method not found" error that occurred when processing JSON-RPC.
* - InvalidParams: Represents an error about invalid parameters passed to a JSON-RPC method.
* - InternalError: Represents an internal error that occurred when processing JSON-RPC.
*/
enum class EJsonRpcErrorCode : int32
{
        None = 0,
        ParseError = -32700,
        InvalidRequest = -32600,
        MethodNotFound = -32601,
        InvalidParams = -32602,
        InternalError = -32603,
};

/**
* @struct FJsonRpcError
*
* This structure represents a JSON-RPC error. It inherits from FJsonObjectWrapper.
*
* @see FJsonObjectWrapper
*/
USTRUCT(BlueprintType)
struct JSONRPC_API FJsonRpcError : public FJsonObjectWrapper
{
        GENERATED_BODY()

        /**
        * @brief Default constructor for FJsonRpcError.
        */
        FJsonRpcError() = default;

        /**
        * Constructs a FJsonRpcError object from the given FJsonObject.
        *
        * @param ErrorJsonObject FJsonObject containing information about the error.
        */
        explicit FJsonRpcError(const FJsonObject& ErrorJsonObject);

        /**
        * @brief Constructs a new FJsonRpcError object with the given code and message.
        *
        * @param Code The error code.
        * @param Message The error message.
        */
        explicit FJsonRpcError(int32 Code, const FString& Message);

        /**
        * Creates a new FJsonRpcError instance with the given error code and message.
        *
        * @param Code The error code to set.
        * @param Message The error message to set.
        */
        explicit FJsonRpcError(EJsonRpcErrorCode Code, const FString& Message);

        /**
        * @brief Gets the error code.
        *
        * This method retrieves the error code for a JSON-RPC error.
        *
        * @return The error code as an integer.
        */

        int32 GetCode() const;

        /**
        * @brief Sets the error code.
        *
        * This method sets the error code for a JSON-RPC error object.
        *
        * @param ErrorCode The error code to set.
        */
        void SetCode(int32 ErrorCode) const;

        /**
        * Gets the error message associated with this JSON-RPC error.
        *
        * @return The error message.
        */

        FString GetMessage() const;

        /**
        * @brief Sets the error message for the JSON-RPC error.
        *
        * @param ErrorMessage The error message to set.
        */
        void SetMessage(const FString& ErrorMessage) const;

        /**
        * @brief An overloaded equality operator for comparing two FJsonRpcError objects.
        *
        * @param Other The FJsonRpcError object to compare.
        * @return true if the FJsonRpcError objects have the same code and message, false otherwise.
        */

        bool operator==(const FJsonRpcError& Other) const;

        /**
        * Checks if the current FJsonRpcError object is equal to another FJsonRpcError object.
        *
        * @param Other The FJsonRpcError object to compare.
        *
        * @return true if the FJsonRpcError objects are not equal, false otherwise.
        */

        bool operator!=(const FJsonRpcError& Other) const;

        /**
        * @brief Converts an FJsonRpcError object to a bool value.
        *
        * @details This method overrides the bool() operator and provides an implicit bool conversion for FJsonRpcError objects. Returns true if the
        * FJsonRpcError object is not equal to the default FJsonRpcError object, false otherwise.
        *
        * @return A boolean value indicating whether an error exists.
        *
        * @see FJsonRpcError
        */
        explicit operator bool() const noexcept;
};

template <>
struct TStructOpsTypeTraits<FJsonRpcError> : TStructOpsTypeTraitsBase2<FJsonRpcError>
{
        enum
        {
                WithIdenticalViaEquality = true,
        };
};
