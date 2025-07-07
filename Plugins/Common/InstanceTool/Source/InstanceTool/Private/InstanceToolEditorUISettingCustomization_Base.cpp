// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolEdMode.h"
#include "InstanceToolModule.h"
#include "InstanceToolEditorUISetting.h"
#include "InstanceToolEditorUISettingCustomization.h"

#include "Engine/Selection.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "InstanceTool.UISetting"

FInstanceToolEdMode* FInstanceToolEditorUISettingCustomization_Base::EditMode = nullptr;

void FInstanceToolEditorUISettingCustomization_Base::HideCategoryByToolMode(IDetailLayoutBuilder& DetailBuilder, EToolMode ToolMode)
{
	TArray<FName> AllCategories;

	TArray<FName> SelectCategories({ "Select", "Align", "Distribute", "Snap", "DeltaTransform" });
	AllCategories.Append(SelectCategories);

	TArray<FName> ConvertCategories({ "ConvertTo", "ConvertFrom" });
	AllCategories.Append(ConvertCategories);
	
	TArray<FName> SpawnCategories({ "SpawnBlueprintByInstances", "SpawnInstancesOnSpline" });
	AllCategories.Append(SpawnCategories);

	TArray<FName> PlacementCategories({ "Placement" });
	AllCategories.Append(PlacementCategories);

	TArray<FName> OptionsCategories({ "Options" });
	AllCategories.Append(OptionsCategories);
		
	TArray<FName> AboutCategories({ "About" });
	AllCategories.Append(AboutCategories);

	TArray<FName>* CurrentCategory = nullptr;
	switch (ToolMode)
	{
	case EToolMode::Select:
		CurrentCategory = &SelectCategories;
		break;
	case EToolMode::Convert:
		CurrentCategory = &ConvertCategories;
		break;
	case EToolMode::Spawn:
		CurrentCategory = &SpawnCategories;
		break;
	case EToolMode::Placement:
		CurrentCategory = &PlacementCategories;
		break;
	case EToolMode::Options:
		CurrentCategory = &OptionsCategories;
		break;
	case EToolMode::About:
		CurrentCategory = &AboutCategories;
		break;
	default:
		break;
	}

	if (CurrentCategory)
	{
		for (auto& Cat : AllCategories)
		{
			if (!CurrentCategory->Contains(Cat))
			{
				DetailBuilder.HideCategory(Cat);
			}
		}
	}
}

bool FInstanceToolEditorUISettingCustomization_Base::HasAnyActorSelected() const
{
	return GEditor->GetSelectedActorCount() > 0;
}

bool FInstanceToolEditorUISettingCustomization_Base::HasAnyInstanceSelected() const
{
	if (EditMode)
	{
		return EditMode->HasAnyInstanceSelected();
	}
	return false;
}

bool FInstanceToolEditorUISettingCustomization_Base::HasAnyActorOrInstanceSelected() const
{
	if (EditMode)
	{
		return EditMode->HasAnyInstanceSelected() || GEditor->GetSelectedActorCount() > 0;
	}
	return false;
}

bool FInstanceToolEditorUISettingCustomization_Base::HasBlueprintActorOrInstanceSelected() const
{
	if (EditMode && HasAnyActorOrInstanceSelected())
	{
		TArray<AActor*> SelectedActors;
		if (GEditor->GetSelectedActorCount() > 0)
		{
			GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);
		}
		else
		{
			SelectedActors = EditMode->Selection.GetSelectedActors();
		}

		if (SelectedActors.Num() > 0)
		{
			for (auto& Actor : SelectedActors)
			{
				UBlueprint* Blueprint = (Actor != nullptr) ? Cast<UBlueprint>(Actor->GetClass()->ClassGeneratedBy) : nullptr;

				if ((Actor != NULL) && (Blueprint != NULL) && (Actor->GetClass()->ClassGeneratedBy == Blueprint))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool FInstanceToolEditorUISettingCustomization_Base::CanSelectActor() const
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();
	return EditMode && (UISetting.bCanSelectActorInViewport || UISetting.bCanSelectActorInWorldOutliner);
}

#undef LOCTEXT_NAMESPACE