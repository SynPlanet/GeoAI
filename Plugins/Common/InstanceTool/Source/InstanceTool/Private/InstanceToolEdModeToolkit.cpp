// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolEdModeToolkit.h"
#include "InstanceToolEdMode.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FInstanceToolEdModeToolkit"

void FInstanceToolEdModeToolkit::Init(const TSharedPtr<class IToolkitHost>& InitToolkitHost)
{
	InstanceToolEditorWidget = SNew(SInstanceToolEditor, SharedThis(this));

	FInstanceToolEdMode* EditorMode = GetInstanceToolEdMode();
	if (EditorMode)
	{
		EditorMode->SetOnSelectionChangedDelegate(::FOnSelectionChanged::CreateRaw(InstanceToolEditorWidget.Get() , &SInstanceToolEditor::NotifySelectionChanged));
	}

	auto NameToCommandMap = FInstanceToolEditorCommands::Get().CommandMap;
	TSharedRef<FUICommandList> CommandList = GetToolkitCommands();

#define MAP_MODE(ModeName) CommandList->MapAction(NameToCommandMap.FindChecked(ModeName), FUIAction(FExecuteAction::CreateSP(this, &FInstanceToolEdModeToolkit::OnChangeMode, ModeName), FCanExecuteAction::CreateSP(this, &FInstanceToolEdModeToolkit::IsModeEnabled, ModeName), FIsActionChecked::CreateSP(this, &FInstanceToolEdModeToolkit::IsModeActive, ModeName)));
	MAP_MODE(EToolMode::Select);
	MAP_MODE(EToolMode::Convert);
	MAP_MODE(EToolMode::Spawn);
	MAP_MODE(EToolMode::Options);
	MAP_MODE(EToolMode::About);
#undef MAP_MODE



	FModeToolkit::Init(InitToolkitHost);
}

FInstanceToolEdModeToolkit::FInstanceToolEdModeToolkit()
{
	
}

FName FInstanceToolEdModeToolkit::GetToolkitFName() const
{
	return FName("InstanceToolEditor");
}

FText FInstanceToolEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("InstanceToolEditor", "DisplayName", "Instance Tool");
}

FEdMode* FInstanceToolEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FInstanceToolEdMode::EM_InstanceToolEdModeId);
}

FInstanceToolEdMode* FInstanceToolEdModeToolkit::GetInstanceToolEdMode() const
{
	return (FInstanceToolEdMode*)GLevelEditorModeTools().GetActiveMode(FInstanceToolEdMode::EM_InstanceToolEdModeId);
}

void FInstanceToolEdModeToolkit::OnChangeMode(EToolMode InMode)
{
	GetInstanceToolEdMode()->SetCurrentToolMode(InMode);
	InstanceToolEditorWidget->NotifySelectionChanged();
}

bool FInstanceToolEdModeToolkit::IsModeEnabled(EToolMode InMode) const
{
	return true;
}

bool FInstanceToolEdModeToolkit::IsModeActive(EToolMode InMode) const
{
	return GetInstanceToolEdMode()->GetCurrentToolMode() == InMode;
}


#undef LOCTEXT_NAMESPACE
