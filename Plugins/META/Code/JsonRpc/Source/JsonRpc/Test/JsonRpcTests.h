#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "JsonRpc/Public/JsonRpcError.h"
#include "JsonRpc/Public/JsonRpcLibrary.h"
#include "JsonRpc/Public/JsonRpcObject.h"
#include "JsonRpc/Public/JsonRpcRequest.h"
#include "JsonRpc/Public/JsonRpcResponse.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJsonRpcErrorTest,
                                 "Meta.JsonRpc.JsonRpcMethods.JsonRpcError",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJsonRpcLibraryTest,
                                 "Meta.JsonRpc.JsonRpcMethods.JsonRpcLibrary",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJsonRpcObjectTest,
                                 "Meta.JsonRpc.JsonRpcMethods.JsonRpcObject",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJsonRpcRequestTest,
                                 "Meta.JsonRpc.JsonRpcMethods.JsonRpcRequest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJsonRpcResponseTest,
                                 "Meta.JsonRpc.JsonRpcMethods.JsonRpcResponse",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

#endif // WITH_DEV_AUTOMATION_TESTS
