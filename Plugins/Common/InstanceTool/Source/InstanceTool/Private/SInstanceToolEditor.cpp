// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "SInstanceToolEditor.h"
#include "InstanceToolEditorObject.h"
#include "InstanceToolEditorObjectCustomization.h"
#include "InstanceToolEditorUISetting.h"
#include "InstanceToolEditorUISettingCustomization.h"
#include "InstanceToolEdMode.h"
#include "InstanceToolModule.h"
#include "InstanceToolEdModeToolkit.h"

#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Interfaces/IPluginManager.h"
#include "EditorModeManager.h"
#include "SlateOptMacros.h"
#include "Framework/MultiBox/MultiBoxDefs.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "InstanceToolEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInstanceToolEditor::Construct(const FArguments& InArgs, TSharedRef<FInstanceToolEdModeToolkit> InParentToolkit)
{
	FText VersionName = FText::FromString(IPluginManager::Get().FindPlugin(TEXT("InstanceTool"))->GetDescriptor().VersionName);
	FText CreatedBy = FText::FromString(TEXT("by " ) + IPluginManager::Get().FindPlugin(TEXT("InstanceTool"))->GetDescriptor().CreatedBy);
	FText FriendlyName = FText::FromString(IPluginManager::Get().FindPlugin(TEXT("InstanceTool"))->GetDescriptor().FriendlyName);

	// Todo InstanceTool: Read from text file
	FText ChangeLog = LOCTEXT("InstanceToolChangeLog",
"1.2.9.7\n\
------------------\n\
* Add 5.1 support\n\
\n\
1.2.9.6\n\
------------------\n\
* Add 5.0 support\n\
\n\
1.2.9.5\n\
------------------\n\
* Add option to force positive scale on all converted instances when converting\n\
\n\
1.2.9.4\n\
------------------\n\
* Add option to disable Always Instance Changes to Blueprint\n\
* Add Apply Changes to Blueprint and Reset to Blueprint Default\n\
* Fix crash when exiting Instance Tool edit mode\n\
\n\
1.2.9.3\n\
------------------\n\
* Fix editor crash when editing blueprint instances\n\
\n\
1.2.9.2\n\
------------------\n\
* Fix ObjectProperty FInstancesSnapshot::Component is not initialized properly warning\n\
\n\
1.2.9.1\n\
------------------\n\
* Add 4.23 support\n\
\n\
1.2.9\n\
------------------\n\
* Fix lightmap issue in editor by add option to rebuild instances when map opened\n\
* Add option to select all instances after converted\n\
* Add option to Rebuild Instances to be able to appy to all ISM / HISM actors in level\n\
* Now can use Alt + middle mouse drag to change widget location, useful to set temporary rotation pivot\n\
* Add Sapwn Instance at Current Widget Location in Spawn tab\n\
* Now Rebuild Instances also create hitproxies\n\
* Fix bug that undo is broken if component is not transactional\n\
* Fix bug that converted HISMA's instance can't be selected by clicking\n\
\n\
1.2.8\n\
------------------\n\
* Add Rebuild Instances button to fix instances disappeared after actor duplicated or moved to other level\n\
* Change defautl value of minimum instances needed to be converted from 2 to 1\n\
\n\
1.2.7\n\
------------------\n\
* Fix editor crash when editing blueprint instances\n\
* Fix a bug that first instance selection not been registered for undo\n\
* Refactoring undo system\n\
\n\
1.2.6\n\
------------------\n\
* Add option of minum instances needed to be converted from static meshes into instanced static meshes\n\
* Add option to delete source (H)ISMA after all instaces been converted\n\
* Now delete instances and ISMAs after converted by default\n\
\n\
1.2.5\n\
------------------\n\
* Add option to frame viewport to indicate actor or instance selection\n\
* Fix potential editor crash when convert/add instances in 4.17.2\n\
\n\
1.2.4\n\
------------------\n\
* Add option to convert actors with negative scale to instances\n\
* Add option to customize converted actor's label\n\
* Now source actors will be deleted by default after conversion\n\
* Support non-unity build; Drop 4.14 support\n\
\n\
1.2.3\n\
------------------\n\
* Add option to mark material 'Used with Instanced Static Mesh' usage flag when converting actors into ISM/HISM instances\n\
* Change plugin loading phase from 'Default' to 'PostEngineInit' to avoid potential crash reported by users when starting up engine\n\
\n\
1.2.2\n\
------------------\n\
* Now 4.16 ready; drop 4.13 support\n\
* Add spawn instances on spline\n\
\n\
1.2.1\n\
------------------\n\
* Update for 4.15 IWYU header changes\n\
\n\
1.2.0\n\
------------------\n\
* Add new Spawn panel\n\
* Now can spawn blueprint instances base on ISM/HISM instances and auto assign instance id to blueprint instances\n\
* Convert to new ISM/HISM actor base on staic mesh materia (thanks Emanuel May from Subiculum Interactive)\n\
* Set default value of bUseAsOccluder and bDisallowMeshPaintPerInstance for converted ISMA/HISMA, \n\
  to make it consistent with manual created ISMA/HISMA\n\
\n\
1.1.5\n\
------------------\n\
* Now 4.15 ready\n\
* Fixed Mac compile error (thanks jeff_lamarche)\n\
\n\
1.1.4\n\
------------------\n\
* Add pivot option for converting instance actor\n\
* Fixed potential ui setting object name collision\n\
\n\
1.1.3.1\n\
------------------\n\
* Fix bug enabling actor viewport select might cause editor out of memory due to early cancel of undo transaction\n\
\n\
1.1.3\n\
------------------\n\
* Add new Delta Transform panel with duplicate option\n\
* Now can duplicate / delect instances in Edit menu / Select panel or using standardard Ctrl + W / Del hotkeys\n\
* Add Use Own Snap Dir option in Snap panel\n\
* Improve Align to Grid function to get rid of gap between 90 / 180 degree rotated instances\n\
* Remove Use Local Bound For World Space Box Trace option in Snap panel\n\
* Set SnapOffset default to 0.f\n\
* Fix bug PlaceInstancesInto setting not read from save correctly\n\
* Fix bug setting scale in Transform panel when parent component has scale was wrong\n\
* Fix bug marquee select should not select invisible instances\n\
* Fix bug PlaceSourceActorsInFolder not working properly when converting actors to instances\n\
\n\
1.1.2\n\
------------------\n\
* Add new Distribute panel, now can distribute / line-up instances\n\
* Add Auto Align Scale to Grid option to workaround instances scale drifting issue\n\
* Avoid applying unnecessary rotation to instances when converting instances between ISMC and HISMC\n\
* Fixed a bug that undo snapshot not been updated after converting actors to instances\n\
\n\
1.1.1\n\
------------------\n\
* Adding full undo support\n\
* All properties of static mesh component will be transfered when converting\n\
* Add Disable Undo option\n\
* Fixed a bug editing instance transfrom in transform panel alwas use relative transform even parent component set to use absolute transform\n\
* Change Name of Select Invalid to Select Overlapped\n\
* Change Name of Snap Use Last Selected to Snap Follw Last Selected Instance\n\
\n\
1.1.0\n\
------------------\n\
* Add new Convert Tool, support flexible conversion between Instances to Static Mesh Actors\n\
* Add experimental Blueprint Instances Editing support\n\
* Add Snap Offset option to avoid seams between snapped instances\n\
* Add options to enable select normal actors in InstanceTool editmode\n\
* Add tolerance for Select By Axis\n\
* Add Rotate To Hit Normal option for snapping instances\n\
* Add Select Parent Acotr / Component option\n\
* Add more notification message for vary operations\n\
* New Icons\n\
* Minor bug fixes\n\
\n\
1.0.2\n\
------------------\n\
* Fixed instances not translate correctly if parent component has rotation applied\n\
\n\
1.0.1\n\
------------------\n\
* Fixed Mac compile warning\n\
\n\
1.0\n\
------------------\n\
* Initial Release");

	TSharedRef<FUICommandList> CommandList = InParentToolkit->GetToolkitCommands();
	FToolBarBuilder ModeToolBar(CommandList, FMultiBoxCustomization::None);
	{
		ModeToolBar.AddToolBarButton(FInstanceToolEditorCommands::Get().SelectMode, NAME_None, LOCTEXT("Mode.Select", "Select"), LOCTEXT("Mode.Select.Tooltip", "Select Instance"));
 		ModeToolBar.AddToolBarButton(FInstanceToolEditorCommands::Get().ConvertMode, NAME_None, LOCTEXT("Mode.Convert", "Convert"), LOCTEXT("Mode.Convert.Tooltip", "Convert"));
		ModeToolBar.AddToolBarButton(FInstanceToolEditorCommands::Get().SpawnMode, NAME_None, LOCTEXT("Mode.Spawn", "Spawn"), LOCTEXT("Mode.Spawn.Tooltip", "Spawn"));
// 		ModeToolBar.AddToolBarButton(FInstanceToolEditorCommands::Get().PlacementMode, NAME_None, LOCTEXT("Mode.Placement", "Placement"), LOCTEXT("Mode.Placement.Tooltip", "Instance Placement"));
 		ModeToolBar.AddToolBarButton(FInstanceToolEditorCommands::Get().OptionsMode, NAME_None, LOCTEXT("Mode.Options", "Options"), LOCTEXT("Mode.Options.Tooltip", "General Options"));
		ModeToolBar.AddToolBarButton(FInstanceToolEditorCommands::Get().AboutMode, NAME_None, LOCTEXT("Mode.About", "About"), LOCTEXT("Mode.About.Tooltip", "About Instance Tool"));
	}

	FPropertyEditorModule& PropetyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ENameAreaSettings::HideNameArea;

	FInstanceToolEdMode* EditMode = GetEditorMode();

	SelectionDetailView = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	SelectionDetailView->RegisterInstancedCustomPropertyLayout(UInstanceToolEditorObject::StaticClass(), 
		FOnGetDetailCustomizationInstance::CreateStatic(&FInstanceToolEditorObjectCustomization::MakeInstance, EditMode)
		);

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	TSharedPtr<IDetailsView> ToolDetailView_Select = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailView_Select->RegisterInstancedCustomPropertyLayout(UInstanceToolEditorUISetting::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FInstanceToolEditorUISettingCustomization_Select::MakeInstance, EditMode)
		);
	ToolDetailView_Select->SetObject(&UISetting);

	TSharedPtr<IDetailsView> ToolDetailView_Convert = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailView_Convert->RegisterInstancedCustomPropertyLayout(UInstanceToolEditorUISetting::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FInstanceToolEditorUISettingCustomization_Convert::MakeInstance, EditMode)
	);
	ToolDetailView_Convert->SetObject(&UISetting);

	TSharedPtr<IDetailsView> ToolDetailView_Spawn = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailView_Spawn->RegisterInstancedCustomPropertyLayout(UInstanceToolEditorUISetting::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FInstanceToolEditorUISettingCustomization_Spawn::MakeInstance, EditMode)
	);
	ToolDetailView_Spawn->SetObject(&UISetting);

	TSharedPtr<IDetailsView> ToolDetailView_Options = PropetyEditorModule.CreateDetailView(DetailsViewArgs);
	ToolDetailView_Options->RegisterInstancedCustomPropertyLayout(UInstanceToolEditorUISetting::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FInstanceToolEditorUISettingCustomization_Options::MakeInstance, EditMode)
	);
	ToolDetailView_Options->SetObject(&UISetting);

	
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 5)
		[
			SAssignNew(ErrorText, SErrorText)
		]

		// Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 0, 4, 5)
		[
			// Mode ToolBar
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.HAlign(HAlign_Center)
				[
					ModeToolBar.MakeWidget()
				]
			]
		]
		
		// Main
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot().Padding(0, 5)
			[
				SNew(SVerticalBox)

				// Selection
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0)
				[
					SNew(SHorizontalBox)
					.Visibility_Static(&GetVisibility, (int32)(EToolMode::Select) | (int32)(EToolMode::Convert))
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0)
					[
					SelectionDetailView.ToSharedRef()
					]
				]

				// Tools - Select
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0)
				[
					SNew(SHorizontalBox)
					.Visibility_Static(&GetVisibility, EToolMode::Select)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0)
					[
						ToolDetailView_Select.ToSharedRef()
					]
				]

				// Tools - Convert
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0)
				[
					SNew(SHorizontalBox)
					.Visibility_Static(&GetVisibility, EToolMode::Convert)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0)
					[
						ToolDetailView_Convert.ToSharedRef()
					]
				]

				// Tools - Spawn
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0)
				[
					SNew(SHorizontalBox)
					.Visibility_Static(&GetVisibility, EToolMode::Spawn)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0)
					[
						ToolDetailView_Spawn.ToSharedRef()
					]
				]

				// Tools - Options
				+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0)
					[
						SNew(SHorizontalBox)
						.Visibility_Static(&GetVisibility, EToolMode::Options)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(0)
					[
						ToolDetailView_Options.ToSharedRef()
					]
				]

				// About
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0)
				[
					SNew(SHorizontalBox)
					.Visibility_Static(&GetVisibility, EToolMode::About)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.Padding(0)
							.AutoWidth()
							[
								SNew(SRichTextBlock)
								.Text(FriendlyName)
								.TextStyle(FAppStyle::Get(), "Credits.Strong")
								.DecoratorStyleSet(&FAppStyle::Get())
								.Justification(ETextJustify::Center)
								.AutoWrapText(false)
							]
						]

						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.Padding(2, 2)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.Padding(0)
							.AutoWidth()
							[
								SNew(SRichTextBlock)
								.Text(VersionName)
								.TextStyle(FAppStyle::Get(), "Credits.Normal")
								.DecoratorStyleSet(&FAppStyle::Get())
								.Justification(ETextJustify::Center)
								.AutoWrapText(false)
							]
						]

						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.Padding(2, 2)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.Padding(0)
							.AutoWidth()
							[
								SNew(SRichTextBlock)
								.Text(CreatedBy)
								//.TextStyle(FAppStyle::Get(), "Credits.Normal")
								.DecoratorStyleSet(&FAppStyle::Get())
								.Justification(ETextJustify::Center)
								.AutoWrapText(false)
							]
						]

						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.Padding(2,5)
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.Padding(0)
							.AutoWidth()
							[
								SNew(SMultiLineEditableTextBox)
								.IsReadOnly(true)
								.Text(ChangeLog)
								.Style(FInstanceToolStyle::Get(), "InstanceTool.ChangeLogText")
								.Margin(4)
								//.DecoratorStyleSet(&FAppStyle::Get())
								.Justification(ETextJustify::Left)
								.AutoWrapText(true)
							]
						]
					]
				]
			]
		]
		
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SInstanceToolEditor::NotifySelectionChanged()
{	
	if (FInstanceToolEdMode* EditMode = GetEditorMode())
	{
 		TArray<UObject*> Selection = EditMode->GetSelectedObjects();
 		SelectionDetailView->SetObjects(Selection, true);

// 		if (Selection.Num() > 0)
// 		{
// 			ToolDetailView->SetObject(EditMode->UISetting);
// 		}
// 		else 
// 		{
// 			ToolDetailView->SetObject(nullptr, true);
// 		}

// 		if (Selection.Num() == 0)
// 		{
// 			ErrorText->SetError(TEXT("No instance selected"));
// 			ErrorText->SetBorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f));
// 		}
// 		else
// 		{
// 			ErrorText->SetError(TEXT(""));
// 		}
	}
}

FInstanceToolEdMode* SInstanceToolEditor::GetEditorMode()
{
	return (FInstanceToolEdMode*)GLevelEditorModeTools().GetActiveMode(FInstanceToolEdMode::EM_InstanceToolEdModeId);
}

bool SInstanceToolEditor::GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const
{
	return true;
}

EVisibility SInstanceToolEditor::GetVisibility(EToolMode InModes)
{
	FInstanceToolEdMode* EditMode = GetEditorMode();
	if (EditMode && (EditMode->GetCurrentToolMode() == InModes))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SInstanceToolEditor::GetVisibility(int32 InMode)
{
	FInstanceToolEdMode* EditMode = GetEditorMode();
	if (EditMode && ((uint8)EditMode->GetCurrentToolMode() & InMode))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
