// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorCommands.h"

#define LOCTEXT_NAMESPACE "InstanceToolEditorCommands"

void FInstanceToolEditorCommands::RegisterCommands()
{
	UI_COMMAND(SelectMode, "Mode - Select", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Select, SelectMode);
	UI_COMMAND(ConvertMode, "Mode - Convert", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Convert, ConvertMode);
	UI_COMMAND(SpawnMode, "Mode - Spawn", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Spawn, SpawnMode);
	UI_COMMAND(PlacementMode, "Mode - Placement", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Placement, PlacementMode);
	UI_COMMAND(OptionsMode, "Mode - Options", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::Options, OptionsMode);
	UI_COMMAND(AboutMode, "Mode - About", "", EUserInterfaceActionType::RadioButton, FInputChord());
	CommandMap.Add(EToolMode::About, AboutMode);
}

#undef LOCTEXT_NAMESPACE