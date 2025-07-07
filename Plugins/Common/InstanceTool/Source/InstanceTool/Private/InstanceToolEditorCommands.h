// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolStyle.h"
#include "Framework/Commands/Commands.h"

enum class EToolMode
{
	Select = 1,
	Convert = 2,
	Placement = 4,
	Spawn = 8,
	Options = 16,
	About = 32
};

class FInstanceToolEditorCommands : public TCommands<FInstanceToolEditorCommands>
{
public:
	FInstanceToolEditorCommands() : TCommands<FInstanceToolEditorCommands>
	(
		"InstanceTool",
		NSLOCTEXT("Contexts", "InstanceTool", "Instance Tool"),
		NAME_None,
		FInstanceToolStyle::Get()->GetStyleSetName()
	)
	{
	}

	virtual void RegisterCommands() override;

public:
	// Mode
	TSharedPtr<FUICommandInfo> SelectMode;
	TSharedPtr<FUICommandInfo> ConvertMode;
	TSharedPtr<FUICommandInfo> SpawnMode;
	TSharedPtr<FUICommandInfo> PlacementMode;
	TSharedPtr<FUICommandInfo> OptionsMode;
	TSharedPtr<FUICommandInfo> AboutMode;

	// Map
	TMap < EToolMode, TSharedPtr<FUICommandInfo>> CommandMap;
};
