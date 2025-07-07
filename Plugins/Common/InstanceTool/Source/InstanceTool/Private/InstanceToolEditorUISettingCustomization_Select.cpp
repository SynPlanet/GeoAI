// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolEditorUISetting.h"
#include "InstanceToolEditorUISettingCustomization.h"
#include "InstanceToolModule.h"

#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/Selection.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "InstanceTool.UISetting"

TSharedRef<IDetailCustomization> FInstanceToolEditorUISettingCustomization_Select::MakeInstance(class FInstanceToolEdMode* InEditMode)
{
	auto Instance = MakeShareable(new FInstanceToolEditorUISettingCustomization_Select());
	EditMode = InEditMode;
	return Instance;
}

void FInstanceToolEditorUISettingCustomization_Select::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	HideCategoryByToolMode(DetailBuilder, EToolMode::Select);

	// Select
	IDetailCategoryBuilder& SelectCategory = DetailBuilder.EditCategory("Select", LOCTEXT("SelectCategory", "Select"), ECategoryPriority::Default);
	
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectNone", "Select None"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectNoneButtonClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectAll", "Select All"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectAllButtonClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(5, 0)
		.AutoWidth()
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectAllOption)))
			.ShouldDisplayName(false)
		]
	];

	// -- Select Overlapped
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectAllOverlapped", "Select All Overlapped"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectOverlappedInstancesButtonClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Tolerance", "Tolerance:"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1.f)
		.HAlign(HAlign_Left)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectInvalidToleranceLocation)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectInvalidToleranceRotation)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectInvalidToleranceScale)))
				.ShouldDisplayName(false)
			]
		]
	];

	// -- Select by Axis
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		
		+ SHorizontalBox::Slot()
		.Padding(2,0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("SelectByAxis", "Select by Axis"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("-X", "-X"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorX)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonNegXClicked)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("+X", "+X"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorX)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonXClicked)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("-Y", "-Y"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorY)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonNegYClicked)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("+Y", "+Y"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorY)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonYClicked)
			]
			+ SUniformGridPanel::Slot(4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("-Z", "-Z"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorZ)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonNegZClicked)
			]
			+ SUniformGridPanel::Slot(5, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("+Z", "+Z"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorZ)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonZClicked)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Tolerance", "Tolerance:"))
		]

		+ SHorizontalBox::Slot()
		.Padding(1, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByAxisTolerance)))
			.ShouldDisplayName(false)
		]
	];

	// -- Select by Location
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectByLocation", "Select by Loc"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByLocButtonClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SUniformGridPanel)
			.SlotPadding(0)

			+ SUniformGridPanel::Slot(0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("X", "X"))
				.ColorAndOpacity(InstanceTool::ColorX)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByLocX)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(2, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Y", "Y"))
				.ColorAndOpacity(InstanceTool::ColorY)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByLocY)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Z", "Z"))
				.ColorAndOpacity(InstanceTool::ColorZ)
			]
			+ SUniformGridPanel::Slot(5, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByLocZ)))
				.ShouldDisplayName(false)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Tolerance", "Tolerance:"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1.f)
		.HAlign(HAlign_Left)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByLocToleranceX)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByLocToleranceY)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByLocToleranceZ)))
				.ShouldDisplayName(false)
			]
		]
	];

	// -- Select by Rotation
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectByRotation", "Select by Rot"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByRotButtonClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SUniformGridPanel)
			.SlotPadding(0)

			+ SUniformGridPanel::Slot(0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("X", "X"))
				.ColorAndOpacity(InstanceTool::ColorX)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByRotX)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(2, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Y", "Y"))
				.ColorAndOpacity(InstanceTool::ColorY)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByRotY)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Z", "Z"))
				.ColorAndOpacity(InstanceTool::ColorZ)
			]
			+ SUniformGridPanel::Slot(5, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByRotZ)))
				.ShouldDisplayName(false)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Tolerance", "Tolerance:"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1.f)
		.HAlign(HAlign_Left)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByRotToleranceX)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByRotToleranceY)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByRotToleranceZ)))
				.ShouldDisplayName(false)
			]
		]
	];

	// -- Select by Scale
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectByScale", "Select by Scale"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectByScaleButtonClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SUniformGridPanel)
			.SlotPadding(0)

			+ SUniformGridPanel::Slot(0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("X", "X"))
				.ColorAndOpacity(InstanceTool::ColorX)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByScaleX)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(2, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Y", "Y"))
				.ColorAndOpacity(InstanceTool::ColorY)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByScaleY)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Z", "Z"))
				.ColorAndOpacity(InstanceTool::ColorZ)
			]
			+ SUniformGridPanel::Slot(5, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectByScaleZ)))
				.ShouldDisplayName(false)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Tolerance", "Tolerance:"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.FillWidth(1.f)
		.HAlign(HAlign_Left)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByScaleToleranceX)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByScaleToleranceY)))
				.ShouldDisplayName(false)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SelectByScaleToleranceZ)))
				.ShouldDisplayName(false)
			]
		]
	];

	// -- Duplicate and Delete
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("DuplicateSelectedInstances", "Duplicate Selected Instances"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDuplicateSelectedInstancesButtonClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("DeleteSelectedInstances", "Delete Selected Instances"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDeleteSelectedInstancesButtonClicked)
		]
	];

	// -- Select Parent Actors
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectParentActor", "Select Parent Actor"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectParentActorButtonClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Select::CanExecuteSelectParentButtons)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectParentComponent", "Select Parent Component"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSelectParentComponentButtonClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Select::CanExecuteSelectParentButtons)
		]
	];

	// Rebuild Render Data
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RebuidInstances", "Rebuild Instances"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnRebuildRenderDataButtonClicked)
		]

#if 0 // WIP, candidate for debug category
		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("InValidate Light Cache", "Invalidate Light Cache"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnInvalidateLightingCacheButtonClicked)
		]
#endif

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bRebuildRenderDataApplyToAllActors)))
			.ShouldDisplayName(true)
		]
	]; 

	// Reset to Blueprint Default
	SelectCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("Apply Changes to Blueprint", "Apply Changes to Blueprint"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnApplyChangesToBlueprintClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Select::CanExecuteApplyChangesToBlueprint)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("Reset to Blueprint Default", "Reset to Blueprint Default"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnResetToBlueprintDefaultsClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Select::HasBlueprintActorOrInstanceSelected)
		]
	];

	SelectCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, MarqueeSelectOption)));
	SelectCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bMarqueeSelectSameActorOnly)));
	SelectCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bMarqueeSelectSubtractMode)));
	
	//DetailBuilder.HideProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bLockSelection)));

	// Align
	IDetailCategoryBuilder& AlignCategory = DetailBuilder.EditCategory("Align", LOCTEXT("AlignCategory", "Align"), ECategoryPriority::Default);

	AlignCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, AlignCoord)));

	// -- Align Location
	AlignCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Location", "Location"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[

			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0,0)
			[
				SNew(SButton)
				.Text(LOCTEXT("X","X"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorX)
				//.ButtonStyle(FInstanceToolStyle::Get(), "InstanceTool.AxisButton.X")
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignLocButtonXClicked)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Y", "Y"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorY)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignLocButtonYClicked)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Z", "Z"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorZ)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignLocButtonZClicked)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("All", "All"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignLocButtonAllClicked)
			]
		]
	];

	// -- Align Rotation
	AlignCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Rotation", "Rotation"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[

			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("X", "X"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorX)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignRotButtonXClicked)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Y", "Y"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorY)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignRotButtonYClicked)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Z", "Z"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorZ)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignRotButtonZClicked)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("All", "All"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignRotButtonAllClicked)
			]
		]
	];

	// -- Align to Grid
	AlignCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("AlignToGrid", "Align to Grid"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[

			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Location", "Location"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignToGridLocButtonClicked)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Rotation", "Rotation"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignToGridRotButtonClicked)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Scale", "Scale"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignToGridScaleButtonClicked)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("All", "All"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnAlignToGridAllButtonClicked)
			]
		]
	];

	// -- Force Scale Align
// 	AlignCategory.AddCustomRow(FText::GetEmpty())
// 	[
// 		SNew(SHorizontalBox)
// 
// 		+ SHorizontalBox::Slot()
// 		.Padding(2, 0)
// 		.AutoWidth()
// 		.VAlign(VAlign_Center)
// 		[
// 			SNew(SButton).
// 			Text(LOCTEXT("ForceScaleAlign", "Force Scale Align"))
// 			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnForceScaleAlignButtonClicked)
// 			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Select::CanExecuteForceScaleAlignButtons)
// 		]
// 	];

	// Distribute
	IDetailCategoryBuilder& DistributeCategory = DetailBuilder.EditCategory("Distribute", LOCTEXT("DistributeCategory", "Distribute"));

	DistributeCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, DistributeCoord)));

	// Distribute - Distribute by Axis
	DistributeCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Distribute", "Distribute"))
		]

		+SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("X", "X"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorX)
				//.ButtonStyle(FInstanceToolStyle::Get(), "InstanceTool.AxisButton.X")
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDistributeButtonXClicked)
			]

			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Y", "Y"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorY)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDistributeButtonYClicked)
			]

			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Z", "Z"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorZ)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDistributeButtonZClicked)
			]

			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("All", "All"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDistributeButtonAllClicked)
			]
		]
	];

	// Distribute - Distribute by Axis Options
	DistributeCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("DistributeIgnoreSameLoc", "Distribute Ignore Same Location?"))
		]

		+ SHorizontalBox::Slot()
		.Padding(0, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bDistributeIgnoreSameLoc)))
			.ShouldDisplayName(false)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Tolerance", "Tolerance:"))
		]

		+ SHorizontalBox::Slot()
		.Padding(0, 0)
		.AutoWidth()
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, DistributeIgnoreSameLocTolerance)))
			.ShouldDisplayName(false)
		]
	];

	// Distribute - Line Up by Axis
	DistributeCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("LineUp", "Line Up"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SUniformGridPanel)
			.SlotPadding(2)

			+ SUniformGridPanel::Slot(0, 0)
			[
			SNew(SButton)
			.Text(LOCTEXT("X", "X"))
			.ForegroundColor(FLinearColor::White)
			.ButtonColorAndOpacity(InstanceTool::ColorX)
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnLineUpButtonXClicked)
			]

		+ SUniformGridPanel::Slot(1, 0)
		[
			SNew(SButton)
			.Text(LOCTEXT("Y", "Y"))
			.ForegroundColor(FLinearColor::White)
			.ButtonColorAndOpacity(InstanceTool::ColorY)
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnLineUpButtonYClicked)
		]

		+ SUniformGridPanel::Slot(2, 0)
		[
			SNew(SButton)
			.Text(LOCTEXT("Z", "Z"))
			.ForegroundColor(FLinearColor::White)
			.ButtonColorAndOpacity(InstanceTool::ColorZ)
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnLineUpButtonZClicked)
		]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("LineUpBetweenFirstLast", "First to Last"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnLineUpButtonFirstLastClicked)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("AutoAlignRotation", "Align Rotation"))
		]

		+ SHorizontalBox::Slot()
		.Padding(0, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bLineUpAlignRotation)))
			.ShouldDisplayName(false)
		]
	];

	// Snap
	IDetailCategoryBuilder& SnapCategory = DetailBuilder.EditCategory("Snap", LOCTEXT("SnapCategory", "Snap"), ECategoryPriority::Default);

	auto SnapCoordPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SnapCoord));
	SnapCategory.AddProperty(SnapCoordPropertyHandle);

	SnapCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Snap", "Snap"))
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[

			SNew(SUniformGridPanel)
			.SlotPadding(2)
		
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("-X", "-X"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorX)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonNegXClicked)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("+X", "+X"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorX)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonXClicked)
			]
			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("-Y", "-Y"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorY)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonNegYClicked)
			]
			+ SUniformGridPanel::Slot(3, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("+Y", "+Y"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorY)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonYClicked)
			]
			+ SUniformGridPanel::Slot(4, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("-Z", "-Z"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorZ)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonNegZClicked)
			]
			+ SUniformGridPanel::Slot(5, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("+Z", "+Z"))
				.ForegroundColor(FLinearColor::White)
				.ButtonColorAndOpacity(InstanceTool::ColorZ)
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonZClicked)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Font(DetailBuilder.GetDetailFont())
			.Text(LOCTEXT("Offset", "Offset:"))
		]

		+ SHorizontalBox::Slot()
		.Padding(1, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SnapOffset)))
			.ShouldDisplayName(false)
		]
	];
	SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bUseOwnSnapDir)));
	SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bAlwaysSnapToGrid)));
	SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bRotateToHitNormal)));
	SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSnapFollowLastSelected)));
	SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SnapTraceOption)));
	//SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bAlwaysUseLocalBoundForTracing)));
	SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSnapIgnoreStartPenetrating)));
	SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SnapTraceDistance)));
	SnapCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSnapDrawDebug)));

	// Delta Transform
	IDetailCategoryBuilder& DeltaTransformCategory = DetailBuilder.EditCategory("DeltaTransform", LOCTEXT("DeltaTransformCategory", "DeltaTransform"), ECategoryPriority::Default);

	DeltaTransformCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, DeltaTransformLoc)));
	DeltaTransformCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, DeltaTransformRotation)));
	DeltaTransformCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, DeltaTransformScale)));
	DeltaTransformCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, DeltaTransformRotationOrder)));

	DeltaTransformCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[

			SNew(SUniformGridPanel)
			.SlotPadding(5)

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("Reset", "Reset"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDeltaTransformButtonResetClicked)
			]

			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("Sub", "Sub"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDeltaTransformButtonSubClicked)
			]

			+ SUniformGridPanel::Slot(2, 0)
			[
				SNew(SButton).
				Text(LOCTEXT("Add", "Add"))
				.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Select::OnDeltaTransformButtonAddClicked)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Duplicate?", "Duplicate?"))
			]

			+ SHorizontalBox::Slot()
			.Padding(0, 0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bDeltaTransformDuplicate)))
				.ShouldDisplayName(false)
			]

			+ SHorizontalBox::Slot()
			.Padding(2, 0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text(LOCTEXT("Copies", "Copies"))
			]

			+ SHorizontalBox::Slot()
			.Padding(0, 0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, DeltaTransformDuplicateCopies)))
				.ShouldDisplayName(false)
			]
		]
	];
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectAllButtonClicked()
{
	if (EditMode)
	{
		EditMode->ForceRebuildRenderData();

		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		if (UISetting.bLockSelection)
		{
			EditMode->NotifyMessage(LOCTEXT("SelectionLocked", "Selection was locked."));
			return FReply::Handled();
		}

		if ((UISetting.bCanSelectActorInWorldOutliner || UISetting.bCanSelectActorInViewport) && GEditor->GetSelectedActorCount() > 0)
		{
			TArray<AActor*> Selected;
			GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(Selected);
			EditMode->SelectAllInstancesOfActors(Selected, /*bEnableUndo=*/ true);
			GEditor->SelectNone(/*bNoteSelectionChange=*/ true, /*bDeselectBSPSurfs=*/ true);
		}
		else
		{
			bool bSelectSameComponentOnly = UISetting.SelectAllOption == ESelectAllOption::SameComponentAsSelected;
			EditMode->SelectAllInstances(/*bSelectSameComponentOnly=*/ bSelectSameComponentOnly, /*bEnableUndo=*/ true);
		}

// 		FText Message = FText::Format(LOCTEXT("SelectedInstances", "{0} instances selected."), EditMode->GetSelectedInstanceCount());
// 		EditMode->NotifyMessage(Message);
	}
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectOverlappedInstancesButtonClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		if (UISetting.bLockSelection)
		{
			EditMode->NotifyMessage(LOCTEXT("SelectionLocked", "Selection was locked."));
			return FReply::Handled();
		}

		if ((UISetting.bCanSelectActorInWorldOutliner || UISetting.bCanSelectActorInViewport) && GEditor->GetSelectedActorCount() > 0)
		{
			TArray<AActor*> Selected;
			GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(Selected);
			EditMode->SelectAllInstancesOfActors(Selected);			
		}

		FText Message;
		if (!EditMode->HasAnyInstanceSelected())
		{ 
			Message = LOCTEXT("SelectInstanceFirst", "Please select an instance first!");
		}
		else
		{
			bool bSelectSameComponentOnly = UISetting.SelectAllOption == ESelectAllOption::SameComponentAsSelected;
			int32 OverlappedCount = EditMode->SelectAllOverlappedInstances(/*bSelectSameComponentOnly=*/ bSelectSameComponentOnly);

			if ((UISetting.bCanSelectActorInWorldOutliner || UISetting.bCanSelectActorInViewport) && GEditor->GetSelectedActorCount() > 0
				&& OverlappedCount > 0)
			{
				GEditor->SelectNone(/*bNoteSelectionChange=*/ true, /*bDeselectBSPSurfs=*/ true);
			}

			if (OverlappedCount > 0)
			{
				Message = FText::Format(LOCTEXT("SelectedOverlappedInstance", "Selected {0} overlapped instances."), OverlappedCount);
			}
			else
			{
				Message = LOCTEXT("SelectedNoOverlappedInstance", "No overlapped instances found.");
			}
		}

		EditMode->NotifyMessage(Message);
	}
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnDuplicateSelectedInstancesButtonClicked()
{
	if (EditMode)
	{
		EditMode->DuplicateSelectedInstances();
		
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnDeleteSelectedInstancesButtonClicked()
{
	if (EditMode)
	{
		EditMode->DeleteSelectedInstances();
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectParentActorButtonClicked()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		EditMode->NotifyMessage(LOCTEXT("SelectionLocked", "Selection was locked."));
		return FReply::Handled();
	}

	FText Message;
	if (!EditMode->HasAnyInstanceSelected())
	{
		Message = LOCTEXT("SelectInstanceFirst", "Please select an instance first!");
		return FReply::Handled();
	}

	EditMode->SelectParentActors();

	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectParentComponentButtonClicked()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		EditMode->NotifyMessage(LOCTEXT("SelectionLocked", "Selection was locked."));
		return FReply::Handled();
	}

	FText Message;
	if (!EditMode->HasAnyInstanceSelected())
	{
		Message = LOCTEXT("SelectInstanceFirst", "Please select an instance first!");
		return FReply::Handled();
	}

	EditMode->SelectParentComponents();

	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnRebuildRenderDataButtonClicked()
{
	EditMode->ForceRebuildRenderData();
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnInvalidateLightingCacheButtonClicked()
{
	EditMode->InvalidateLightingCache();
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnResetToBlueprintDefaultsClicked()
{
	EditMode->ResetToBlueprintDefaults();
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnApplyChangesToBlueprintClicked()
{
	EditMode->ApplyChangesToBlueprint();
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonXClicked()
{
	if (EditMode)
	{
		EditMode->SelectByAxis(EAxis::X, /*bNegative=*/ false, /*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonNegXClicked()
{
	if (EditMode)
	{
		EditMode->SelectByAxis(EAxis::X, true, /*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonYClicked()
{
	if (EditMode)
	{
		EditMode->SelectByAxis(EAxis::Y, /*bNegative=*/ false, /*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonNegYClicked()
{
	if (EditMode)
	{
		EditMode->SelectByAxis(EAxis::Y, true, /*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonZClicked()
{
	if (EditMode)
	{
		EditMode->SelectByAxis(EAxis::Z, /*bNegative=*/ false, /*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByAxisButtonNegZClicked()
{
	if (EditMode)
	{
		EditMode->SelectByAxis(EAxis::Z, true, /*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByLocButtonClicked()
{
	if (EditMode)
	{
		EditMode->SelectByLoc(/*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByRotButtonClicked()
{
	if (EditMode)
	{
		EditMode->SelectByRot(/*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectByScaleButtonClicked()
{
	if (EditMode)
	{
		EditMode->SelectByScale(/*bShowMessage=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSelectNoneButtonClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		if (UISetting.bLockSelection)
		{
			EditMode->NotifyMessage(LOCTEXT("SelectionLocked", "Selection was locked."));
			return FReply::Handled();
		}

		if ((UISetting.bCanSelectActorInWorldOutliner || UISetting.bCanSelectActorInViewport) && GEditor->GetSelectedActorCount() > 0)
		{
			GEditor->SelectNone(/*bNoteSelectionChange=*/ true, /*bDeselectBSPSurfs=*/ true);
		}

		EditMode->SetSelectNone(/*bEnableUndo=*/ true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignLocButtonXClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		ECoordSystem CoordSystem = UISetting.AlignCoord == EAlignSnapCoord::Auto
			? GLevelEditorModeTools().GetCoordSystem()
			: (UISetting.AlignCoord == EAlignSnapCoord::Local) ? COORD_Local : COORD_World;
		EditMode->AlignSelectionLocation(CoordSystem, EAxis::X);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignLocButtonYClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		ECoordSystem CoordSystem = UISetting.AlignCoord == EAlignSnapCoord::Auto
			? GLevelEditorModeTools().GetCoordSystem()
			: (UISetting.AlignCoord == EAlignSnapCoord::Local) ? COORD_Local : COORD_World;
		EditMode->AlignSelectionLocation(CoordSystem, EAxis::Y);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignLocButtonZClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		ECoordSystem CoordSystem = UISetting.AlignCoord == EAlignSnapCoord::Auto
			? GLevelEditorModeTools().GetCoordSystem()
			: (UISetting.AlignCoord == EAlignSnapCoord::Local) ? COORD_Local : COORD_World;
		EditMode->AlignSelectionLocation(CoordSystem, EAxis::Z);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignLocButtonAllClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		ECoordSystem CoordSystem = UISetting.AlignCoord == EAlignSnapCoord::Auto
			? GLevelEditorModeTools().GetCoordSystem()
			: (UISetting.AlignCoord == EAlignSnapCoord::Local) ? COORD_Local : COORD_World;

		EditMode->AlignSelectionLocation(CoordSystem, EAxis::None);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignRotButtonXClicked()
{
	if (EditMode)
	{
		EditMode->AlignSelectionRotation(EAxis::X);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignRotButtonYClicked()
{
	if (EditMode)
	{
		EditMode->AlignSelectionRotation(EAxis::Y);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignRotButtonZClicked()
{
	if (EditMode)
	{
		EditMode->AlignSelectionRotation(EAxis::Z);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignRotButtonAllClicked()
{
	if (EditMode)
	{
		EditMode->AlignSelectionRotation(EAxis::None);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignToGridLocButtonClicked()
{
	if (EditMode)
	{
		EditMode->AlignSelectionToGrid(UE::Widget::WM_Translate);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignToGridRotButtonClicked()
{
	if (EditMode)
	{
		EditMode->AlignSelectionToGrid(UE::Widget::WM_Rotate);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignToGridScaleButtonClicked()
{
	if (EditMode)
	{
		EditMode->AlignSelectionToGrid(UE::Widget::WM_Scale);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnAlignToGridAllButtonClicked()
{
	if (EditMode)
	{
		EditMode->AlignSelectionToGrid(UE::Widget::WM_None);
	}
	return FReply::Handled();
}

// FReply FInstanceToolEditorUISettingCustomization_Select::OnForceScaleAlignButtonClicked()
// {
// 	if (EditMode)
// 	{
// 		EditMode->ForceScaleAlign();
// 	}
// 	return FReply::Handled();
// }

// bool FInstanceToolEditorUISettingCustomization_Select::CanExecuteForceScaleAlignButtons() const
// {
// 	return UISetting.ForceScaleAlignOption != EForceScaleSnapOption::None;
// }

FReply FInstanceToolEditorUISettingCustomization_Select::OnDistributeButtonXClicked()
{
	if (EditMode)
	{
		EditMode->DistributeSelectionLocation(EAxis::X);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnDistributeButtonYClicked()
{
	if (EditMode)
	{
		EditMode->DistributeSelectionLocation(EAxis::Y);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnDistributeButtonZClicked()
{
	if (EditMode)
	{
		EditMode->DistributeSelectionLocation(EAxis::Z);
	}
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnDistributeButtonAllClicked()
{
	if (EditMode)
	{
		EditMode->DistributeSelectionLocation(EAxis::None);
	}
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnLineUpButtonFirstLastClicked()
{
	if (EditMode)
	{
		EditMode->LineUpSelectionLocationFirstLast();
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnLineUpButtonXClicked()
{
	if (EditMode)
	{
		EditMode->LineUpSelectionLocation(EAxis::X);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnLineUpButtonYClicked()
{
	if (EditMode)
	{
		EditMode->LineUpSelectionLocation(EAxis::Y);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnLineUpButtonZClicked()
{
	if (EditMode)
	{
		EditMode->LineUpSelectionLocation(EAxis::Z);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonXClicked()
{
	if (EditMode)
	{
		EditMode->SnapSelection(EAxis::X);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonNegXClicked()
{
	if (EditMode)
	{
		EditMode->SnapSelection(EAxis::X, true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonYClicked()
{
	if (EditMode)
	{
		EditMode->SnapSelection(EAxis::Y);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonNegYClicked()
{
	if (EditMode)
	{
		EditMode->SnapSelection(EAxis::Y, true);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonZClicked()
{
	if (EditMode)
	{
		EditMode->SnapSelection(EAxis::Z);
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Select::OnSnapButtonNegZClicked()
{
	if (EditMode)
	{
		EditMode->SnapSelection(EAxis::Z, true);
	}
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnDeltaTransformButtonResetClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();
		UISetting.DeltaTransformLoc = FVector::ZeroVector;
		UISetting.DeltaTransformRotation = FRotator::ZeroRotator;
		UISetting.DeltaTransformScale = FVector::ZeroVector;
	}
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnDeltaTransformButtonSubClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		FTransform DeltaTransform(-1 * UISetting.DeltaTransformRotation, -1 * UISetting.DeltaTransformLoc, -1 * UISetting.DeltaTransformScale);
		EditMode->DeltaTransformSelection(DeltaTransform);
	}
	return FReply::Handled();
}


FReply FInstanceToolEditorUISettingCustomization_Select::OnDeltaTransformButtonAddClicked()
{
	if (EditMode)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		FTransform DeltaTransform(UISetting.DeltaTransformRotation, UISetting.DeltaTransformLoc, UISetting.DeltaTransformScale);
		EditMode->DeltaTransformSelection(DeltaTransform);
	}
	return FReply::Handled();
}

bool FInstanceToolEditorUISettingCustomization_Select::CanExecuteSelectParentButtons() const
{
	return HasAnyInstanceSelected() && CanSelectActor();
}

bool FInstanceToolEditorUISettingCustomization_Select::CanExecuteResetToBlueprintDefaults() const
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bAllowBlueprintEditing)
	{
		return false;
	}
	return HasBlueprintActorOrInstanceSelected();
}

bool FInstanceToolEditorUISettingCustomization_Select::CanExecuteApplyChangesToBlueprint() const
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bAllowBlueprintEditing || UISetting.bAlwaysApplyInstanceChangesToBlueprint)
	{
		return false;
	}
	return HasBlueprintActorOrInstanceSelected();
}

#undef LOCTEXT_NAMESPACE