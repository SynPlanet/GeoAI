// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEdMode.h"
#include "InstanceToolEditorCommands.h"
#include "IDetailCustomization.h"

class IPropertyHandle;
class FInstanceToolEdMode;

namespace InstanceTool
{
	const FLinearColor ColorX(0.594f, 0.0197f, 0.0f);
	const FLinearColor ColorY(0.1349f, 0.3959f, 0.0f);
	const FLinearColor ColorZ(0.0251f, 0.207f, 0.85f);
}

class FInstanceToolEditorUISettingCustomization_Base : public  IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override = 0;

protected:	
	void HideCategoryByToolMode(IDetailLayoutBuilder& DetailBuilder, EToolMode ToolMode);
	static FInstanceToolEdMode* EditMode;

	bool HasAnyActorSelected() const;
	bool HasAnyInstanceSelected() const;
	bool HasAnyActorOrInstanceSelected() const;
	bool HasBlueprintActorOrInstanceSelected() const;

	bool CanSelectActor() const;
};

class FInstanceToolEditorUISettingCustomization_Select : public FInstanceToolEditorUISettingCustomization_Base
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FInstanceToolEdMode* InEditMode);

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	
	// Select
	FReply OnSelectAllButtonClicked();
	FReply OnSelectNoneButtonClicked(); 
	FReply OnSelectOverlappedInstancesButtonClicked();

	FReply OnDuplicateSelectedInstancesButtonClicked();
	FReply OnDeleteSelectedInstancesButtonClicked();

	FReply OnSelectParentActorButtonClicked();
	bool CanExecuteSelectParentButtons() const;
	FReply OnSelectParentComponentButtonClicked();
	FReply OnSelectByAxisButtonXClicked();
	FReply OnSelectByAxisButtonNegXClicked();
	FReply OnSelectByAxisButtonYClicked();
	FReply OnSelectByAxisButtonNegYClicked();
	FReply OnSelectByAxisButtonZClicked();
	FReply OnSelectByAxisButtonNegZClicked();

	FReply OnSelectByLocButtonClicked();
	FReply OnSelectByRotButtonClicked();
	FReply OnSelectByScaleButtonClicked();

	FReply OnRebuildRenderDataButtonClicked();
	FReply OnInvalidateLightingCacheButtonClicked();

	// Blueprint
	FReply OnResetToBlueprintDefaultsClicked();
	FReply OnApplyChangesToBlueprintClicked();
	bool CanExecuteResetToBlueprintDefaults() const;
	bool CanExecuteApplyChangesToBlueprint() const;

	// Align
	FReply OnAlignLocButtonXClicked();
	FReply OnAlignLocButtonYClicked();
	FReply OnAlignLocButtonZClicked();
	FReply OnAlignLocButtonAllClicked();

	FReply OnAlignRotButtonXClicked();
	FReply OnAlignRotButtonYClicked();
	FReply OnAlignRotButtonZClicked();
	FReply OnAlignRotButtonAllClicked();

	FReply OnAlignToGridLocButtonClicked();
	FReply OnAlignToGridRotButtonClicked();
	FReply OnAlignToGridScaleButtonClicked();
	FReply OnAlignToGridAllButtonClicked();

// 	FReply OnForceScaleAlignButtonClicked();
// 	bool CanExecuteForceScaleAlignButtons() const;

	// Distribute
	FReply OnDistributeButtonXClicked();
	FReply OnDistributeButtonYClicked();
	FReply OnDistributeButtonZClicked();
	FReply OnDistributeButtonAllClicked();
	FReply OnLineUpButtonFirstLastClicked();

	FReply OnLineUpButtonXClicked();
	FReply OnLineUpButtonYClicked();
	FReply OnLineUpButtonZClicked();

	// Snap
	FReply OnSnapButtonXClicked();
	FReply OnSnapButtonNegXClicked();
	FReply OnSnapButtonYClicked();
	FReply OnSnapButtonNegYClicked();
	FReply OnSnapButtonZClicked();
	FReply OnSnapButtonNegZClicked();

	// Delta Transform
	FReply OnDeltaTransformButtonResetClicked();
	FReply OnDeltaTransformButtonSubClicked();
	FReply OnDeltaTransformButtonAddClicked();

//	ECoordSystem GetAlignCoordSystem();
};

class FInstanceToolEditorUISettingCustomization_Convert : public FInstanceToolEditorUISettingCustomization_Base
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FInstanceToolEdMode* InEditMode);

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// Convert
	FReply OnConvertInstanceToActorButtonClicked();
	FReply OnConvertAllInstanceOfActorButtonClicked();
	FReply OnConvertActorToInstanceButtonClicked();
};

class FInstanceToolEditorUISettingCustomization_Spawn : public FInstanceToolEditorUISettingCustomization_Base
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FInstanceToolEdMode* InEditMode);

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// Convert
	FReply OnSpawnInstanceAtWidgetLocationClicked();
	FReply OnSpawnInstancesOnSplineClicked(); 
	FReply OnSpawnBySelectedInstancesButtonClicked();	

	FReply OnConnectButtonClicked();
};


class FInstanceToolEditorUISettingCustomization_Options : public FInstanceToolEditorUISettingCustomization_Base
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FInstanceToolEdMode* InEditMode);

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
