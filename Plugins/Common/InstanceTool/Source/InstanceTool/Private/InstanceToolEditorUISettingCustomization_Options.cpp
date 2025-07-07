// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolEditorUISetting.h"
#include "InstanceToolEditorUISettingCustomization.h"

#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "InstanceTool.UISetting"

TSharedRef<IDetailCustomization> FInstanceToolEditorUISettingCustomization_Options::MakeInstance(class FInstanceToolEdMode* InEditMode)
{
	auto Instance = MakeShareable(new FInstanceToolEditorUISettingCustomization_Options());
	EditMode = InEditMode;
	return Instance;
}

void FInstanceToolEditorUISettingCustomization_Options::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	HideCategoryByToolMode(DetailBuilder, EToolMode::Options);
}

#undef LOCTEXT_NAMESPACE