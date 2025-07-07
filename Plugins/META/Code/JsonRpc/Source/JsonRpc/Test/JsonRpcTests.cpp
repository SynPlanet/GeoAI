#include "JsonRpcTests.h"
#include "Dom/JsonObject.h"

#if WITH_DEV_AUTOMATION_TESTS

TSharedPtr<FJsonObject> CreateSampleJsonObject()
{
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
        JsonObject->SetStringField(TEXT("exampleField"), TEXT("exampleValue"));
        return JsonObject;
}

bool FJsonRpcErrorTest::RunTest(const FString& Parameters)
{
        // Test the constructor that accepts FJsonObject
        FJsonRpcError JsonRpcError(EJsonRpcErrorCode::InvalidRequest, "Invalid Request");

        TestEqual("Error code should match", JsonRpcError.GetCode(), static_cast<int32>(EJsonRpcErrorCode::InvalidRequest));
        TestEqual("Error message should match", JsonRpcError.GetMessage(), "Invalid Request");

        JsonRpcError = FJsonRpcError(EJsonRpcErrorCode::MethodNotFound, "Method not found");

        TestEqual("Error code should match", JsonRpcError.GetCode(), static_cast<int32>(EJsonRpcErrorCode::MethodNotFound));
        TestEqual("Error message should match", JsonRpcError.GetMessage(), "Method not found");

        // Test setting and getting code
        JsonRpcError = FJsonRpcError(EJsonRpcErrorCode::InternalError, "Invalid params");
        JsonRpcError.SetCode(static_cast<int32>(EJsonRpcErrorCode::InvalidParams));

        TestEqual("Error code should match after set", JsonRpcError.GetCode(), static_cast<int32>(EJsonRpcErrorCode::InvalidParams));

        // Test setting and getting message
        JsonRpcError = FJsonRpcError(EJsonRpcErrorCode::InternalError, "Internal error");
        JsonRpcError.SetMessage("Server error");

        TestEqual("Error message should match after set", JsonRpcError.GetMessage(), "Server error");
        // Test equality operator
        JsonRpcError = FJsonRpcError(EJsonRpcErrorCode::ParseError, "Parse error");
        FJsonRpcError JsonRpcError2(EJsonRpcErrorCode::ParseError, "Parse error");
        TestTrue("Errors should be equal", JsonRpcError == JsonRpcError2);

        // Test inequality operator
        JsonRpcError = FJsonRpcError(EJsonRpcErrorCode::ParseError, "Parse error");
        JsonRpcError2 = FJsonRpcError(EJsonRpcErrorCode::InvalidRequest, "Invalid request");

        TestTrue("Errors should not be equal", JsonRpcError != JsonRpcError2);

        // Test boolean operator
        JsonRpcError = FJsonRpcError(-32608, "Some error");
        TestTrue("Error should evaluate to true", static_cast<bool>(JsonRpcError));

        FJsonRpcError DefaultJsonRpcError;
        TestFalse("Default error should evaluate to false", static_cast<bool>(DefaultJsonRpcError));

        return true;
}

bool FJsonRpcLibraryTest::RunTest(const FString& Parameters)
{
        // Test for MakeJsonRpcObject
        FGuid TestGuid = FGuid::NewGuid();
        FJsonRpcObject JsonRpcObject = UJsonRpcLibrary::MakeJsonRpcObject(TestGuid);
        TestEqual("Guid should match", JsonRpcObject.GetGuid(), TestGuid);

        // Test for MakeJsonRpcRequest
        TestGuid = FGuid::NewGuid();
        FString TestMethod = TEXT("testMethod");
        FJsonObjectWrapper ParamsWrapper;
        ParamsWrapper.JsonObject = CreateSampleJsonObject();

        FJsonRpcRequest JsonRpcRequest = UJsonRpcLibrary::MakeJsonRpcRequest(TestGuid, TestMethod, ParamsWrapper);
        TestEqual("Guid should match", JsonRpcRequest.GetGuid(), TestGuid);
        TestEqual("Method should match", JsonRpcRequest.GetMethod(), TestMethod);

        // Test for MakeJsonRpcResponse
        TestGuid = FGuid::NewGuid();
        FJsonObjectWrapper ResultWrapper;
        ResultWrapper.JsonObject = CreateSampleJsonObject();
        FJsonRpcError TestError(EJsonRpcErrorCode::InvalidRequest, TEXT("Test Error"));

        FJsonRpcResponse JsonRpcResponse = UJsonRpcLibrary::MakeJsonRpcResponse(TestGuid, ResultWrapper, TestError);
        TestEqual("Guid should match", JsonRpcResponse.GetGuid(), TestGuid);
        TestEqual("Error message should match", JsonRpcResponse.GetError().GetMessage(), TEXT("Test Error"));

        // Test for MakeJsonRpcError
        EJsonRpcErrorType TestErrorType = EJsonRpcErrorType::InvalidRequest;
        FString TestErrorMessage = TEXT("Invalid Request");

        FJsonRpcError JsonRpcError = UJsonRpcLibrary::MakeJsonRpcError(TestErrorType, TestErrorMessage);
        TestEqual("Error code should match", JsonRpcError.GetCode(), -32600);
        TestEqual("Error message should match", JsonRpcError.GetMessage(), TestErrorMessage);

        // Test for GetGuid
        TestGuid = FGuid::NewGuid();
        JsonRpcObject = FJsonRpcObject(TestGuid);

        FGuid RetrievedGuid = UJsonRpcLibrary::GetGuid(JsonRpcObject);
        TestEqual("Guid should match", RetrievedGuid, TestGuid);

        // Test for GetMethod
        TestGuid = FGuid::NewGuid();
        TestMethod = TEXT("testMethod");
        JsonRpcRequest = FJsonRpcRequest(TestGuid, TestMethod, *CreateSampleJsonObject());

        FString RetrievedMethod = UJsonRpcLibrary::GetMethod(JsonRpcRequest);
        TestEqual("Method should match", RetrievedMethod, TestMethod);

        // Test for GetParams
        TestGuid = FGuid::NewGuid();
        TSharedPtr<FJsonObject> ParamsJsonObject = CreateSampleJsonObject();
        JsonRpcRequest = FJsonRpcRequest(TestGuid, TEXT("testMethod"), *ParamsJsonObject);

        FJsonObjectWrapper RetrievedParams = UJsonRpcLibrary::GetParams(JsonRpcRequest);
        TestEqual("Params should match",
                  RetrievedParams.JsonObject->GetStringField(TEXT("exampleField")),
                  ParamsJsonObject->GetStringField(TEXT("exampleField")));

        // Test for GetResult
        TestGuid = FGuid::NewGuid();
        TSharedPtr<FJsonObject> ResultJsonObject = CreateSampleJsonObject();
        FJsonRpcError TestErrorClear;
        JsonRpcResponse = FJsonRpcResponse(TestGuid, *ResultJsonObject, TestErrorClear);

        FJsonObjectWrapper RetrievedResult = UJsonRpcLibrary::GetResult(JsonRpcResponse);
        TestEqual("Result should match",
                  RetrievedResult.JsonObject->GetStringField(TEXT("exampleField")),
                  ResultJsonObject->GetStringField(TEXT("exampleField")));

        // Test for GetError
        TestGuid = FGuid::NewGuid();
        ResultJsonObject = CreateSampleJsonObject();
        TestError = FJsonRpcError(EJsonRpcErrorCode::InvalidRequest, TEXT("Test Error"));
        JsonRpcResponse = FJsonRpcResponse(TestGuid, *ResultJsonObject, TestError);

        FJsonRpcError RetrievedError = UJsonRpcLibrary::GetError(JsonRpcResponse);
        TestEqual("Error code should match", RetrievedError.GetCode(), TestError.GetCode());
        TestEqual("Error message should match", RetrievedError.GetMessage(), TestError.GetMessage());

        // Test for GetCode
        TestError = FJsonRpcError(EJsonRpcErrorCode::InvalidRequest, TEXT("Test Error"));
        int32 RetrievedCode = UJsonRpcLibrary::GetCode(TestError);
        TestEqual("Error code should match", RetrievedCode, TestError.GetCode());

        // Test for GetMessage
        TestError = FJsonRpcError(EJsonRpcErrorCode::InvalidRequest, TEXT("Test Error"));
        FString RetrievedMessage = UJsonRpcLibrary::GetMessage(TestError);
        TestEqual("Error message should match", RetrievedMessage, TestError.GetMessage());

        // Test for GetErrorTypeCode
        int32 InvalidRequestCode = UJsonRpcLibrary::GetErrorTypeCode(EJsonRpcErrorType::InvalidRequest);
        TestEqual("Invalid Request error code should match", InvalidRequestCode, -32600);

        int32 MethodNotFoundCode = UJsonRpcLibrary::GetErrorTypeCode(EJsonRpcErrorType::MethodNotFound);
        TestEqual("Method Not Found error code should match", MethodNotFoundCode, -32601);

        int32 InvalidParamsCode = UJsonRpcLibrary::GetErrorTypeCode(EJsonRpcErrorType::InvalidParams);
        TestEqual("Invalid Params error code should match", InvalidParamsCode, -32602);

        int32 InternalErrorCode = UJsonRpcLibrary::GetErrorTypeCode(EJsonRpcErrorType::InternalError);
        TestEqual("Internal Error error code should match", InternalErrorCode, -32603);
        return true;
}

bool FJsonRpcObjectTest::RunTest(const FString& Parameters)
{
        // Test for the default constructor
        FJsonRpcObject JsonRpcObject;
        TestEqual("Protocol version should be 2.0", JsonRpcObject.JsonObject->GetStringField(TEXT("jsonrpc")), TEXT("2.0"));

        // Test for the constructor with FJsonObject parameter
        TSharedPtr<FJsonObject> SampleJsonObject = CreateSampleJsonObject();
        JsonRpcObject = FJsonRpcObject(*SampleJsonObject);
        TestEqual("Example field should match", JsonRpcObject.JsonObject->GetStringField(TEXT("exampleField")), TEXT("exampleValue"));
        TestEqual("Protocol version should be 2.0", JsonRpcObject.JsonObject->GetStringField(TEXT("jsonrpc")), TEXT("2.0"));

        // Test for the constructor with FGuid parameter
        FGuid TestGuid = FGuid::NewGuid();
        JsonRpcObject = FJsonRpcObject(TestGuid);
        TestEqual("Guid should match", JsonRpcObject.GetGuid(), TestGuid);

        // Test for GetGuid method
        TestGuid = FGuid::NewGuid();
        SampleJsonObject = MakeShared<FJsonObject>();
        SampleJsonObject->SetStringField("guid", TestGuid.ToString());
        JsonRpcObject = FJsonRpcObject(*SampleJsonObject);
        TestEqual("Guid should match", JsonRpcObject.GetGuid(), TestGuid);

        // Test for SetGuid method
        TestGuid = FGuid::NewGuid();
        JsonRpcObject = FJsonRpcObject();
        JsonRpcObject.SetGuid(TestGuid);
        TestEqual("Guid should match", JsonRpcObject.GetGuid(), TestGuid);

        return true;
}

bool FJsonRpcRequestTest::RunTest(const FString& Parameters)
{
        // Test for the constructor with FJsonObject parameter
        TSharedPtr<FJsonObject> SampleJsonObject = CreateSampleJsonObject();
        FJsonRpcRequest JsonRpcRequest(*SampleJsonObject);
        TestEqual("Example field should match", JsonRpcRequest.JsonObject->GetStringField(TEXT("exampleField")), TEXT("exampleValue"));

        // Test for the constructor with FGuid, FString, and FJsonObject parameters
        FGuid TestGuid = FGuid::NewGuid();
        FString TestMethod = TEXT("testMethod");
        FJsonObject Params = *CreateSampleJsonObject();
        JsonRpcRequest = FJsonRpcRequest(TestGuid, TestMethod, Params);
        TestEqual("Guid should match", JsonRpcRequest.GetGuid(), TestGuid);
        TestEqual("Method should match", JsonRpcRequest.GetMethod(), TestMethod);
        TestEqual("Params should match", JsonRpcRequest.GetParams().GetStringField(TEXT("exampleField")), TEXT("exampleValue"));

        // Test for GetMethod method
        SampleJsonObject = MakeShared<FJsonObject>();
        SampleJsonObject->SetStringField("method", TEXT("testMethod"));
        JsonRpcRequest = FJsonRpcRequest(*SampleJsonObject);
        TestEqual("Method should match", JsonRpcRequest.GetMethod(), TEXT("testMethod"));

        // Test for SetMethod method
        JsonRpcRequest = FJsonRpcRequest();
        JsonRpcRequest.SetMethod(TEXT("newMethod"));
        TestEqual("Method should match", JsonRpcRequest.GetMethod(), TEXT("newMethod"));

        // Test for GetParams method
        SampleJsonObject = MakeShared<FJsonObject>();
        SampleJsonObject->SetObjectField("params", CreateSampleJsonObject());
        JsonRpcRequest = FJsonRpcRequest(*SampleJsonObject);
        TestEqual("Params should match", JsonRpcRequest.GetParams().GetStringField(TEXT("exampleField")), TEXT("exampleValue"));

        // Test for SetParams method
        JsonRpcRequest = FJsonRpcRequest();
        Params = *CreateSampleJsonObject();
        JsonRpcRequest.SetParams(Params);
        TestEqual("Params should match", JsonRpcRequest.GetParams().GetStringField(TEXT("exampleField")), TEXT("exampleValue"));
        return true;
}

bool FJsonRpcResponseTest::RunTest(const FString& Parameters)
{
        // Test for the constructor with FJsonObject parameter
        TSharedPtr<FJsonObject> SampleJsonObject = CreateSampleJsonObject();
        FJsonRpcResponse JsonRpcResponse(*SampleJsonObject);
        TestEqual("Example field should match", JsonRpcResponse.JsonObject->GetStringField(TEXT("exampleField")), TEXT("exampleValue"));

        // Test for the constructor with FGuid, FJsonObject, and FJsonRpcError parameters
        FGuid TestGuid = FGuid::NewGuid();
        FJsonObject Result = *CreateSampleJsonObject();
        FJsonRpcError Error(404, TEXT("Not Found"));

        JsonRpcResponse = FJsonRpcResponse(TestGuid, Result, Error);
        TestEqual("Guid should match", JsonRpcResponse.GetGuid(), TestGuid);
        TestEqual("Result should match", JsonRpcResponse.GetResult().GetStringField(TEXT("exampleField")), TEXT("exampleValue"));
        TestEqual("Error code should match", JsonRpcResponse.GetError().GetCode(), 404);
        TestEqual("Error message should match", JsonRpcResponse.GetError().GetMessage(), TEXT("Not Found"));

        // Test for the constructor with FGuid and FJsonRpcError parameters
        TestGuid = FGuid::NewGuid();
        Error = FJsonRpcError(404, TEXT("Not Found"));

        JsonRpcResponse = FJsonRpcResponse(TestGuid, Error);
        TestEqual("Guid should match", JsonRpcResponse.GetGuid(), TestGuid);
        TestEqual("Error code should match", JsonRpcResponse.GetError().GetCode(), 404);
        TestEqual("Error message should match", JsonRpcResponse.GetError().GetMessage(), TEXT("Not Found"));

        // Test for GetResult method
        SampleJsonObject = MakeShared<FJsonObject>();
        SampleJsonObject->SetObjectField("result", CreateSampleJsonObject());
        JsonRpcResponse = FJsonRpcResponse(*SampleJsonObject);
        TestEqual("Result should match", JsonRpcResponse.GetResult().GetStringField(TEXT("exampleField")), TEXT("exampleValue"));

        // Test for SetResult method (FJsonObject version)
        JsonRpcResponse = FJsonRpcResponse();
        Result = *CreateSampleJsonObject();
        JsonRpcResponse.SetResult(Result);
        TestEqual("Result should match", JsonRpcResponse.GetResult().GetStringField(TEXT("exampleField")), TEXT("exampleValue"));

        // Test for SetResult method (FString version)
        JsonRpcResponse = FJsonRpcResponse();
        JsonRpcResponse.SetResult(TEXT("TestResult"));
        TestEqual("Result should match", JsonRpcResponse.JsonObject->GetStringField(TEXT("result")), TEXT("TestResult"));

        // Test for UnsetResult method
        JsonRpcResponse = FJsonRpcResponse();
        Result = *CreateSampleJsonObject();
        JsonRpcResponse.SetResult(Result);
        JsonRpcResponse.UnsetResult();
        TestFalse("Result should be unset", JsonRpcResponse.JsonObject->HasField(TEXT("result")));

        // Test for UnsetError method
        JsonRpcResponse = FJsonRpcResponse();
        Error = FJsonRpcError(404, TEXT("Not Found"));
        JsonRpcResponse.SetError(Error);
        JsonRpcResponse.UnsetError();
        TestFalse("Error should be unset", JsonRpcResponse.JsonObject->HasField(TEXT("error")));

        // Test for SetError method
        JsonRpcResponse = FJsonRpcResponse();
        Error = FJsonRpcError(404, TEXT("Not Found"));
        JsonRpcResponse.SetError(Error);
        TestEqual("Error code should match", JsonRpcResponse.GetError().GetCode(), 404);
        TestEqual("Error message should match", JsonRpcResponse.GetError().GetMessage(), TEXT("Not Found"));

        // Test for GetError method
        SampleJsonObject = MakeShared<FJsonObject>();
        TSharedPtr<FJsonObject> ErrorJsonObject = MakeShared<FJsonObject>();
        ErrorJsonObject->SetNumberField("code", 404);
        ErrorJsonObject->SetStringField("message", TEXT("Not Found"));
        SampleJsonObject->SetObjectField("error", ErrorJsonObject);

        JsonRpcResponse = FJsonRpcResponse(*SampleJsonObject);
        Error = JsonRpcResponse.GetError();
        TestEqual("Error code should match", Error.GetCode(), 404);
        TestEqual("Error message should match", Error.GetMessage(), TEXT("Not Found"));
        return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS