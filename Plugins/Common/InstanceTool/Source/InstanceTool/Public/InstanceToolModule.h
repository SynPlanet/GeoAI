// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define ENABLE_INSTANCETOOL_LOG 0

DECLARE_LOG_CATEGORY_EXTERN(LogInstanceTool, Verbose, All);

#if ENABLE_INSTANCETOOL_LOG
#define INSTANCETOOL_LOG(Verbosity, Format, ...) \
	UE_LOG(LogInstanceTool, Verbosity, Format, ##__VA_ARGS__);
#else
#define INSTANCETOOL_LOG(Verbosity, Format, ...)
#endif

class FInstanceToolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnLevelAddedToWorld(ULevel* Level, UWorld* World);
	void OnMapOpened(const FString& Filename, bool bLoadAsTemplate);

	static class UInstanceToolEditorUISetting& GetInstanceToolSetting();
	static void ReleaseInstanceToolSetting();

private:
	void RegisterEditorDelegates();
	void UnregisterEditorDelegates();

private:
	static TWeakObjectPtr<UInstanceToolEditorUISetting> UISetting;
};