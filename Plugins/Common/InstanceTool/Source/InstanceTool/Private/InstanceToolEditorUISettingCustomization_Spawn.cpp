// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolEditorUISetting.h"
#include "InstanceToolEditorUISettingCustomization.h"
#include "InstanceToolEditorObject.h"
#include "InstanceToolModule.h"

#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "PropertyCustomizationHelpers.h"

#include "LevelUtils.h"
#include "EditorActorFolders.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Editor/GroupActor.h"
#include "Engine/Selection.h"
#include "Misc/ScopedSlowTask.h"
#include "Components/SplineComponent.h"
#include "Widgets/Input/SButton.h"

#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "InstanceTool.UISetting"

namespace
{
	bool SetIntPropertyByName(const UObject* InObject, const FString& InPropertyName, int32 IntValue)
	{
		bool bSuccess = false;
		for (FProperty* Property = InObject->GetClass()->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			if (CastField<FIntProperty>(Property)
//				|| Cast<UInt16Property>(Property)
// 				|| Cast<UInt64Property>(Property)
// 				|| Cast<UUInt16Property>(Property)
// 				|| Cast<UUInt32Property>(Property)
// 				|| Cast<UUInt64Property>(Property)
			)
			{

				FString Propertyname;
				Property->GetName(Propertyname);
				if (Propertyname == InPropertyName)
				{
					int32* DataPtr = const_cast<int32*>(Property->ContainerPtrToValuePtr<int32>(InObject));
					//Property->CopySingleValue(DataPtr, &IntValue);
					*DataPtr = IntValue;
					bSuccess = true;
					break;
				}
			}
		}
		return bSuccess;
	}
}

TSharedRef<IDetailCustomization> FInstanceToolEditorUISettingCustomization_Spawn::MakeInstance(class FInstanceToolEdMode* InEditMode)
{
	auto Instance = MakeShareable(new FInstanceToolEditorUISettingCustomization_Spawn());
	EditMode = InEditMode;
	return Instance;
}

void FInstanceToolEditorUISettingCustomization_Spawn::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	HideCategoryByToolMode(DetailBuilder, EToolMode::Spawn);

	// SpawnInstancesAtCurrentWidgetLocation
	IDetailCategoryBuilder& SpawnAtWidgetCategory = DetailBuilder.EditCategory("SpawnAtWidget", LOCTEXT("SpawnAtWidgetCategory", "Spawn Instance at Widget Location"), ECategoryPriority::Default);

	//SpawnAtWidgetCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SpawnSplineActor)));

	SpawnAtWidgetCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SpawnInstance", "Spawn Instance"))
			.ToolTipText(LOCTEXT("SpawnInstanceAtWidgetTooltip", "Holding Alt key with Middle Mouse to change current widget location"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Spawn::OnSpawnInstanceAtWidgetLocationClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Spawn::HasAnyActorOrInstanceSelected)
		]
	];

	// SpawnInstancesOnSpline
	IDetailCategoryBuilder& SpawnOnSplineCategory = DetailBuilder.EditCategory("SpawnOnSpline", LOCTEXT("SpawnInstancesOnSplineCategory", "Spawn Instances On Spline"), ECategoryPriority::Default);

	SpawnOnSplineCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SpawnSplineActor)));
	SpawnOnSplineCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSpawnOnSplineIgnoreRotation)));
	SpawnOnSplineCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectSpawnedInstances)));
	SpawnOnSplineCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SpawnOnSplineOffsetTransform)));

	SpawnOnSplineCategory.AddCustomRow(FText::GetEmpty())
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SpawnInstances", "Spawn Instances"))
		.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Spawn::OnSpawnInstancesOnSplineClicked)
		.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Spawn::HasAnyInstanceSelected)
		]
	];


	// SpawnBlueprintByInstances
	IDetailCategoryBuilder& SpawnBlueprintCategory = DetailBuilder.EditCategory("SpawnBlueprintByInstances", LOCTEXT("SpawnBlueprintCategory", "Spawn Blueprint Actors by Instances"), ECategoryPriority::Default);

	SpawnBlueprintCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SpawnBlueprint)));
	SpawnBlueprintCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SpawnAutoAssignInstanceIdVar)));
	SpawnBlueprintCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SpawnOffsetTransform)));
	SpawnBlueprintCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSpawnActorAlwaysUseIdentityScale)));
	SpawnBlueprintCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bPlaceSpawnedActorsInFolder)));
	SpawnBlueprintCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, SpawnedActorsFolder)));

	SpawnBlueprintCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SpawnActors", "Spawn Actors"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Spawn::OnSpawnBySelectedInstancesButtonClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Spawn::HasAnyInstanceSelected)
		]
	];

	
}

FReply FInstanceToolEditorUISettingCustomization_Spawn::OnSpawnBySelectedInstancesButtonClicked()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (EditMode && UISetting.SpawnBlueprint.Get())
	{
		const TArray<UObject*>& SelectedObjects = EditMode->GetSelectedObjects();
		ULevel* ActorLevel = nullptr;
		UWorld* World = nullptr;
		bool bLevelLocked = false;

		int32 InstancesCount = SelectedObjects.Num();
		for (int32 ObjIndex = 0; ObjIndex < InstancesCount; ++ObjIndex)
		{
			UInstanceToolEditorObject* Proxy = Cast<UInstanceToolEditorObject>(SelectedObjects[ObjIndex]);
			if (Proxy && Proxy->Component && Proxy->Component->GetOwner())
			{
				if (!ActorLevel)
				{
					ActorLevel = Proxy->Component->GetOwner()->GetLevel();
				}
				else if (FLevelUtils::IsLevelLocked(ActorLevel))
				{
					bLevelLocked = true;
					break;
				}
			}
		}

		if (bLevelLocked)
		{
			EditMode->NotifyMessage(LOCTEXT("LevelLocked", "The level is locked, unlocked it first!"));
			return FReply::Handled();
		}

		UClass* BlueprintClass = UISetting.SpawnBlueprint->GeneratedClass;
		{
			const FScopedTransaction Transaction(LOCTEXT("Transaction_SpawnBlueprintsByInstances", "Spawn Actors by Instances"));

			int32 TaskSteps = InstancesCount;
			FScopedSlowTask SlowTask(TaskSteps, FText::Format(LOCTEXT("SpawnBlueprintsSlowTask", "Spawning {0} actors..."), InstancesCount));
			SlowTask.MakeDialog();

			TArray<AActor*> InstanceActors;
			for (int32 ObjIndex = 0; ObjIndex < InstancesCount; ++ObjIndex)
			{
				SlowTask.EnterProgressFrame(1.f);

				UInstanceToolEditorObject* Proxy = Cast<UInstanceToolEditorObject>(SelectedObjects[ObjIndex]);
				if (Proxy && Proxy->Component && Proxy->Component->GetOwner())
				{
					AActor* ProxyActor = Proxy->Component->GetOwner();

					FActorSpawnParameters SpawnInfo;
					SpawnInfo.OverrideLevel = ActorLevel;
					SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					World = ActorLevel->GetWorld();
					AActor* NewActor = World->SpawnActor<AActor>(BlueprintClass, SpawnInfo);
					if (NewActor)
					{
						SetIntPropertyByName(NewActor, UISetting.SpawnAutoAssignInstanceIdVar, Proxy->InstanceIndex);
						FTransform InstanceTransform = Proxy->InstanceToWorld;
						FTransform Offset = UISetting.SpawnOffsetTransform;

						FVector FinalLoc = InstanceTransform.GetLocation() + Offset.GetLocation();
						FVector FinalRot = InstanceTransform.GetRotation().Euler() + Offset.GetRotation().Euler();
						FVector FinalScale = UISetting.bSpawnActorAlwaysUseIdentityScale ? FVector(1) : InstanceTransform.GetScale3D() + Offset.GetScale3D();
						FTransform NewActorTransform(FQuat::MakeFromEuler(FinalRot), FinalLoc, FinalScale);
						NewActor->SetActorTransform(NewActorTransform);

						NewActor->PostEditMove(true);
 						NewActor->PostEditChange();

						InstanceActors.Add(NewActor);

						if (UISetting.bPlaceSpawnedActorsInFolder)
						{
							NewActor->SetFolderPath(FName(*UISetting.SpawnedActorsFolder));
						}
					}
				}
			}
		}
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Spawn::OnSpawnInstanceAtWidgetLocationClicked()
{
	if (EditMode)
	{
		EditMode->SpawnInstnceAtWidgetLocation();
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Spawn::OnSpawnInstancesOnSplineClicked()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (EditMode && UISetting.SpawnSplineActor.Get())
	{
		AActor* SplineActor = UISetting.SpawnSplineActor.Get();
		USplineComponent* SplineComponent = SplineActor->FindComponentByClass<USplineComponent>();

		if (SplineComponent)
		{
			EditMode->SpawnInstncesOnSpline(SplineComponent, UISetting.SpawnOnSplineOffsetTransform
				, UISetting.bSpawnOnSplineIgnoreRotation
				, UISetting.bSelectSpawnedInstances);
		}
		else
		{
			EditMode->NotifyMessage(LOCTEXT("SplineActorNeeded", "Actor with spline component needed!"));
		}
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Spawn::OnConnectButtonClicked()
{
	if (EditMode)
	{	
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE