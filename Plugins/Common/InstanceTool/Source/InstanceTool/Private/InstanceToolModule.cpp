// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolModule.h"
#include "InstanceToolEdMode.h"
#include "InstanceToolEditorUISetting.h"

#include "InstanceToolStyle.h"
#include "InstanceToolEditorCommands.h"

#define LOCTEXT_NAMESPACE "FInstanceToolModule"

void FInstanceToolModule::StartupModule()
{
	FInstanceToolStyle::Initialize();

	FInstanceToolEditorCommands::Register();

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FInstanceToolEdMode>(
		FInstanceToolEdMode::EM_InstanceToolEdModeId, 
		LOCTEXT("InstanceToolEdModeName", "Instance Tool"), 
		FSlateIcon(FInstanceToolStyle::GetStyleSetName(), "InstanceTool.InstanceToolMode", "InstanceTool.InstanceToolMode.Small"), 
		true
		);

	RegisterEditorDelegates();
}

void FInstanceToolModule::ShutdownModule()
{
	ReleaseInstanceToolSetting();

	UnregisterEditorDelegates();

	FInstanceToolEditorCommands::Unregister();

	FInstanceToolStyle::Shutdown();

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FInstanceToolEdMode::EM_InstanceToolEdModeId);
}

void FInstanceToolModule::OnLevelAddedToWorld(ULevel* Level, UWorld* World)
{
	//INSTANCETOOL_LOG(Display, TEXT("[FInstanceToolModule::OnLevelAddedToWorld]: Add %s to %s"), *LevelWorld->GetName(), *World->GetName());
	FInstanceToolUtil::ForceRebuildRenderDataInLevel(World, Level);
}

void FInstanceToolModule::OnMapOpened(const FString& Filename, bool bLoadAsTemplate)
{
	//INSTANCETOOL_LOG(Display, TEXT("[FInstanceToolModule::OnMapOpened]: %s"), *Filename);

	UInstanceToolEditorUISetting& ToolSetting = FInstanceToolModule::GetInstanceToolSetting();
	
	const bool bRebuildInstancesWhenMapOpned = ToolSetting.bRebuildInstancesWhenMapOpned;
	if (bRebuildInstancesWhenMapOpned)
	{
		if (UWorld* World = GEditor->GetEditorWorldContext().World())
		{
			FInstanceToolUtil::ForceRebuildRenderDataInLevel(World);
		}
	}
}

TWeakObjectPtr<UInstanceToolEditorUISetting> FInstanceToolModule::UISetting = NULL;

UInstanceToolEditorUISetting& FInstanceToolModule::GetInstanceToolSetting()
{
	if (!UISetting.IsValid())
	{
		UISetting = NewObject<UInstanceToolEditorUISetting>(GetTransientPackage(), NAME_None, RF_Standalone);
		UISetting->ClearFlags(RF_Transactional);
		UISetting->Load();
	}

	check(UISetting.IsValid());
	return *UISetting.Get();
}

void FInstanceToolModule::ReleaseInstanceToolSetting()
{
	if (IsValid(UISetting.Get()))
	{
		UISetting->ClearFlags(RF_Standalone);
		UISetting.Reset();
	}
}

void FInstanceToolModule::RegisterEditorDelegates()
{
	FWorldDelegates::LevelAddedToWorld.AddRaw(this, &FInstanceToolModule::OnLevelAddedToWorld);
	FEditorDelegates::OnMapOpened.AddRaw(this, &FInstanceToolModule::OnMapOpened);
}

void FInstanceToolModule::UnregisterEditorDelegates()
{
	FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
	FEditorDelegates::OnMapOpened.RemoveAll(this);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FInstanceToolModule, InstanceTool)
DEFINE_LOG_CATEGORY(LogInstanceTool);