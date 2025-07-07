// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolEditorUISetting.h"
#include "InstanceToolEditorUISettingCustomization.h"
#include "InstanceToolEditorObject.h"
#include "InstanceToolEdMode.h"
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
#include "Components/StaticMeshComponent.h"
#include "Editor/GroupActor.h"
#include "Engine/Selection.h"
#include "ActorGroupingUtils.h"

#include "ScopedTransaction.h"
#include "Misc/ScopedSlowTask.h"
#include "Engine/StaticMeshActor.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Widgets/Input/SButton.h"

#include "Materials/Material.h"
#include "Layers/LayersSubsystem.h"

#define LOCTEXT_NAMESPACE "InstanceTool.UISetting"

namespace
{
	FVector GetSelectionCenter()
	{
		FVector Center(0, 0, 0);

		int32 NumActors = 0;
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			AActor* Actor = CastChecked<AActor>(*It);
			NumActors == 0 ? Center = Actor->GetActorLocation() : Center += Actor->GetActorLocation();
			++NumActors;
		}
		if (NumActors > 0)
		{
			Center /= NumActors;
		}
		return Center;
	}
}

TSharedRef<IDetailCustomization> FInstanceToolEditorUISettingCustomization_Convert::MakeInstance(class FInstanceToolEdMode* InEditMode)
{
	auto Instance = MakeShareable(new FInstanceToolEditorUISettingCustomization_Convert());
	EditMode = InEditMode;
	return Instance;
}

void FInstanceToolEditorUISettingCustomization_Convert::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	HideCategoryByToolMode(DetailBuilder, EToolMode::Convert);

	// Convert To
	IDetailCategoryBuilder& ConvertToCategory = DetailBuilder.EditCategory("ConvertTo", LOCTEXT("ConvertToCategory", "Convert Instances To Actors"), ECategoryPriority::Default);

	ConvertToCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ConvertToActor", "Convert Selected Instances"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Convert::OnConvertInstanceToActorButtonClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Convert::HasAnyInstanceSelected)
		]
	];

	ConvertToCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(4, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ConvertAllToActor", "Convert All Instances of Selected Actors"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Convert::OnConvertAllInstanceOfActorButtonClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Convert::HasAnyActorOrInstanceSelected)
		]
	];

	ConvertToCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bDeleteInstancesAfterConvert)));
	ConvertToCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bDeleteSourceActorsIfAllInstancesDeleted)));
	ConvertToCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bPlaceConvertedActorsInFolder)));
	ConvertToCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bGroupConvertedActors)));
	ConvertToCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSelectConvertedActors)));

	// Convert From
	IDetailCategoryBuilder& ConvertFromCategory = DetailBuilder.EditCategory("ConvertFrom", LOCTEXT("ConvertFromCategory", "Convert Actors To Instances"), ECategoryPriority::Default);

	ConvertFromCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ConvertFromActor", "Convert to Instances"))
			.OnClicked(this, &FInstanceToolEditorUISettingCustomization_Convert::OnConvertActorToInstanceButtonClicked)
			.IsEnabled(this, &FInstanceToolEditorUISettingCustomization_Convert::HasAnyActorSelected)
		]

		+ SHorizontalBox::Slot()
		.Padding(2, 0)
		.AutoWidth()
		[
			SNew(SProperty, DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, PlaceInstancesInto)))
			.ShouldDisplayName(false)
		]
	];

	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, ConvertFromActorLabelOption)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, ConvertFromActorCustomLabel)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bConvertFromActorLabelAddSurfix)));

	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bCreateNewActorByMaterial)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bCreateNewActorForNegativeScaledActors)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bForcePositiveInstanceScale)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bCheckMaterialUsedWithInstancedStaticMeshesFlag)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, ConvertFromActorPivotOption)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, ExistingActor)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bTryReuseComponentInActor)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bSetSourceActorsNotRender)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bPlaceSourceActorsInFolder)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bGroupSourceActors)));
	ConvertFromCategory.AddProperty(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, bDeleteSourceActors)));
}

FReply FInstanceToolEditorUISettingCustomization_Convert::OnConvertInstanceToActorButtonClicked()
{
	if (EditMode)
	{
		if (!EditMode->HasAnyInstanceSelected())
		{
			EditMode->NotifyMessage( LOCTEXT("SelectInstanceFirst", "Please select instances first!") );
			return FReply::Handled();
		}

		const TArray<UObject*>& SelectedObjects = EditMode->GetSelectedObjects();

		ULevel* ActorLevel = nullptr;
		UWorld* World = nullptr;
		bool bActorsInSameLevel = true;
		bool bLevelLocked = false;
		FName LastSelectedActorFolderName;
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
				else if (ActorLevel != Proxy->Component->GetOwner()->GetLevel())
				{
					bActorsInSameLevel = false;
					break;
				}
				else if (FLevelUtils::IsLevelLocked(ActorLevel))
				{
					bLevelLocked = true;
					break;
				}

				if (ObjIndex == InstancesCount - 1)
				{
					LastSelectedActorFolderName = Proxy->Component->GetOwner()->GetFolderPath();
				}
			}
		}

		if (!bActorsInSameLevel)
		{
			EditMode->NotifyMessage(LOCTEXT("CantConvertMulitpleLevels", "Selected instances should belong to same level!"));
			return FReply::Handled();
		}
		
		if (bLevelLocked)
		{
			EditMode->NotifyMessage(LOCTEXT("LevelLocked", "The level is locked, unlocked it first!"));
			return FReply::Handled();
		}

		World = ActorLevel->GetWorld();
		TArray<AActor*> SourceActors;
		TArray<AActor*> ConvertedActors;
		
		// Converting..
		{
			const FScopedTransaction Transaction(LOCTEXT("Transaction_ConvertInstanceToActor", "Convert Instances To Static Mesh Actors"));

			int32 TaskSteps = 1;

			UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

			if (UISetting.bDeleteInstancesAfterConvert)
			{
				++TaskSteps;
			}

			if (UISetting.bSelectConvertedActors)
			{
				++TaskSteps;
			}

			if (UISetting.bGroupConvertedActors)
			{
				++TaskSteps;
			}

			FScopedSlowTask SlowTask(TaskSteps, FText::Format(LOCTEXT("ConvertInstancesSlowTask", "Converting {0} instances..."), InstancesCount));
			SlowTask.MakeDialog();

			SlowTask.EnterProgressFrame(1.f);

			TArray<AActor*> InstanceActors;
			for (int32 ObjIndex = 0; ObjIndex < InstancesCount; ++ ObjIndex)
			{
				UInstanceToolEditorObject* Proxy = Cast<UInstanceToolEditorObject>(SelectedObjects[ObjIndex]);
				if (Proxy && Proxy->Component && Proxy->Component->GetOwner())
				{
					AActor* ProxyActor = Proxy->Component->GetOwner();
					SourceActors.AddUnique(ProxyActor);
					
					FActorSpawnParameters SpawnInfo;
					SpawnInfo.OverrideLevel = ActorLevel;
					SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Proxy->InstanceToWorld, SpawnInfo);

					if (Actor)
					{
						InstanceActors.AddUnique(ProxyActor);

						FString NewActorLabel = ProxyActor->GetActorLabel() + "_" + FString::FromInt(Proxy->InstanceIndex);
						Actor->SetActorLabel(NewActorLabel);
						Actor->Layers = ProxyActor->Layers;

						if (UISetting.bPlaceConvertedActorsInFolder)
						{
							const FName NewFolderName = FName(*(ProxyActor->GetFolderPath().ToString() / ProxyActor->GetActorLabel() + " Converted (Instance Tool)"));
							//const FName NewFolderName = FActorFolders::Get().GetDefaultFolderName(*World, LastSelectedActorFolderName);
							Actor->SetFolderPath(NewFolderName);
						}
						else
						{
							Actor->SetFolderPath(ProxyActor->GetFolderPath());
						}

						if (ObjIndex == InstancesCount - 1)
						{
							LastSelectedActorFolderName = ProxyActor->GetFolderPath();
						}

						// Copy over properties
 						UStaticMeshComponent* StaticMeshComponent = Actor->GetStaticMeshComponent();
						StaticMeshComponent->SetStaticMesh(Proxy->Component->GetStaticMesh());
						InstanceToolUtilities::CopyComponentProperty<UStaticMeshComponent>(Proxy->Component, StaticMeshComponent);
						
						// Hack: UpdateBounds to trigger KnowStaticMesh update in NotifyIfStaticMeshChanged
						StaticMeshComponent->UpdateBounds();

						StaticMeshComponent->MarkRenderStateDirty();

						Actor->InvalidateLightingCache();
						Actor->PostEditMove(true);
						Actor->MarkPackageDirty();

						ConvertedActors.Add(Actor);
					}
				}
			}

			if (UISetting.bDeleteInstancesAfterConvert)
			{
				SlowTask.EnterProgressFrame(1.f, FText::Format(LOCTEXT("ConvertInstancesSlowTask_DeleteInstancs", "Deleting {0} converted instances..."), InstancesCount));
				EditMode->DeleteSelectedInstances();

				if (UISetting.bDeleteSourceActorsIfAllInstancesDeleted)
				{
					for (AActor* Actor : SourceActors)
					{
						if (Actor && !Actor->IsPendingKillPending())
						{
							if (FInstanceToolUtil::GetActorInstanceCount(Actor) == 0)
							{
								GEditor->GetEditorSubsystem<ULayersSubsystem>()->DisassociateActorFromLayers(Actor);
								Actor->GetWorld()->EditorDestroyActor(Actor, true);
							}
						}
					}
				}
			}

			if (UISetting.bSelectConvertedActors)
			{
				SlowTask.EnterProgressFrame(1.f, FText::Format(LOCTEXT("ConvertInstancesSlowTask_SelectActors", "Selecting {0} converted actors..."), ConvertedActors.Num()));
				GEditor->GetSelectedActors()->BeginBatchSelectOperation();
				for (auto& Actor : ConvertedActors)
				{
					GEditor->SelectActor(Actor, /*bInSelected=*/true, /*bNotify=*/false);
				}
				GEditor->GetSelectedActors()->EndBatchSelectOperation();
				GEditor->NoteSelectionChange();
			}

			if (UISetting.bGroupConvertedActors)
			{
				SlowTask.EnterProgressFrame(1.f, LOCTEXT("ConvertInstancesSlowTask_GroupActors", "Grouping actors..."));
				if (UISetting.bSelectConvertedActors)
				{
					UActorGroupingUtils::Get()->GroupSelected();
				}
				else
				{
					FActorSpawnParameters SpawnInfo;
					SpawnInfo.OverrideLevel = ActorLevel;
					AGroupActor* SpawnedGroupActor = ActorLevel->OwningWorld->SpawnActor<AGroupActor>(SpawnInfo);
					for (auto Actor : ConvertedActors)
					{
						SpawnedGroupActor->Add(*Actor);
					}
					SpawnedGroupActor->CenterGroupLocation();
					//SpawnedGroupActor->Lock();
				}
			}
		}

		ULevel::LevelDirtiedEvent.Broadcast();

		EditMode->NotifyMessage(FText::Format(LOCTEXT("ConvertInstancesFinished", "{0} instances have been successfully converted!"), InstancesCount));
	}
	return FReply::Handled();
}

FReply FInstanceToolEditorUISettingCustomization_Convert::OnConvertAllInstanceOfActorButtonClicked()
{
	if (EditMode)
	{
		if (GEditor->GetSelectedActorCount() <= 0)
		{
			EditMode->SelectAllInstances(/*bSelectSameComponentOnly=*/ false);
		} 
		else //if (UISetting.bCanSelectActorInWorldOutliner)
		{
			TArray<AActor*> Selected;
			GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(Selected);

			ULevel* ActorLevel = nullptr;
			bool bActorsInSameLevel = true;
			bool bLevelLocked = false;
			for (auto Actor : Selected)
			{
				if (!ActorLevel)
				{
					ActorLevel = Actor->GetLevel();
				}
				else if (ActorLevel != Actor->GetLevel())
				{
					bActorsInSameLevel = false;
					break;
				}
				else if (FLevelUtils::IsLevelLocked(ActorLevel))
				{
					bLevelLocked = true;
					break;
				}
			}

			if (!bActorsInSameLevel)
			{
				EditMode->NotifyMessage(LOCTEXT("CantConvertMulitpleLevels", "Selected instances should belong to same level!"));
				return FReply::Handled();
			}

			if (bLevelLocked)
			{
				EditMode->NotifyMessage(LOCTEXT("LevelLocked", "The level is locked, unlocked it first!"));
				return FReply::Handled();
			}

			EditMode->SelectAllInstancesOfActors(Selected);
			GEditor->SelectNone(/*bNoteSelectionChange=*/true, /*bDeselectBSPSurfs*/ true);
		}
		return OnConvertInstanceToActorButtonClicked();
	}

	return FReply::Handled();
}

struct FUniqueStaticMesh
{
public:
	static bool bUseMaterial;
	static bool bUseScale;

	FUniqueStaticMesh()
		: StaticMesh(nullptr)
		, Scale(FVector(1.f))
	{
	}

	FUniqueStaticMesh(UStaticMeshComponent* InComponent)
	{
		StaticMesh = InComponent->GetStaticMesh();

		if (bUseMaterial)
		{
			for (int32 i = 0; i < InComponent->GetNumMaterials(); ++i)
			{
				Materials.Add(InComponent->GetMaterial(i));
			}
		}

		if (bUseScale)
		{
			Scale = InComponent->GetComponentToWorld().GetScale3D().GetSignVector();
		}
	}

	bool operator== (const FUniqueStaticMesh& Other) const
	{
		if (StaticMesh != Other.StaticMesh)
			return false;

		if (Materials.Num() != Other.Materials.Num())
			return false;

		for (int32 i = 0; i < Materials.Num(); ++i)
		{
			if (Materials[i] != Other.Materials[i])
				return false;
		}

		if (!Scale.Equals(Other.Scale))
		{
			return false;
		}

		return true;
	}

	FORCEINLINE FVector GetScale() const { return Scale; }

	friend uint32 GetTypeHash(const FUniqueStaticMesh& Key)
	{
		if (!bUseMaterial)
		{
			return GetTypeHash(Key.StaticMesh);
		}

		uint32 MatHash = 0;

		for (int32 i = 0; i < Key.Materials.Num(); ++i)
		{
			MatHash += GetTypeHash(Key.Materials[i]);
		}

		return GetTypeHash(Key.StaticMesh) + (MatHash * 23);
	}

protected:
	UStaticMesh* StaticMesh;
	TArray<UMaterialInterface*> Materials;
	FVector Scale;
};

bool FUniqueStaticMesh::bUseMaterial = false;
bool FUniqueStaticMesh::bUseScale = false;

FReply FInstanceToolEditorUISettingCustomization_Convert::OnConvertActorToInstanceButtonClicked()
{
	struct Local
	{
		static void EnsureMaterialHasUsedWithInstancedStaticMeshesFlag(UMaterial* InputMaterialToCheck)
		{
			if (InputMaterialToCheck)
			{
				if (!InputMaterialToCheck->bUsedWithInstancedStaticMeshes)
				{
					InputMaterialToCheck->Modify();
					InputMaterialToCheck->CheckMaterialUsage(EMaterialUsage::MATUSAGE_InstancedStaticMeshes);
				}
			}
		}
	};

	if (EditMode)
	{
		if (GEditor->GetSelectedActorCount() <= 0)
		{
			EditMode->NotifyMessage(LOCTEXT("SelectActorFirst", "Please select source actors to convert from!"));
			return FReply::Handled();
		}

		TArray<AActor*> SelectedActors;
		GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);

		bool TargetLevelIsLocked = false;

		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		AActor* ExistingTargetActor = UISetting.ExistingActor.Get();
		if (ExistingTargetActor)
		{
			if (SelectedActors.Contains(ExistingTargetActor))
			{
				EditMode->NotifyMessage(LOCTEXT("CanConvertIntoSelf", "Can't convert into self, please select another actor!"));
				return FReply::Handled();
			}
 			if (ExistingTargetActor->GetClass()->ClassGeneratedBy && !UISetting.bAllowBlueprintEditing)
			{
				EditMode->NotifyMessage(LOCTEXT("CanConvertIntoBlueprint", "Can't convert into Blueprint actor, please select another one!"));
				return FReply::Handled();
			}

			if (FLevelUtils::IsLevelLocked(ExistingTargetActor->GetLevel()))
			{
				TargetLevelIsLocked = true;
			}
		}
		else
		{
			for (auto Actor : SelectedActors)
			{
				if (FLevelUtils::IsLevelLocked(Actor->GetLevel()))
				{
					TargetLevelIsLocked = true;
					break;
				}
			}
		}

		if (TargetLevelIsLocked)
		{
			EditMode->NotifyMessage(LOCTEXT("TargetLevelLocked", "Target level is locked, unlocked it first!"));
			return FReply::Handled();
		}

		AActor* LastSelectedActor = SelectedActors.Last();

		const int32 SourceActorsCount = SelectedActors.Num();
		int32 ConvertedInstancesCount = 0;

		TArray<AActor*> ConvertingActors;
		TArray<AActor*> ActualUsedSourceActors;
		TArray<AActor*> TargetActors;
		if (ExistingTargetActor)
		{
			TargetActors.Add(ExistingTargetActor);
		}
		//Converting...
		{

			const FScopedTransaction Transaction(LOCTEXT("Transaction_ConvertActorToInstance", "Convert Actors To Instances"));

			FScopedSlowTask SlowTask(0, FText::Format(LOCTEXT("ConvertActorssSlowTask", "Converting {0} actors..."), SourceActorsCount));
			SlowTask.MakeDialog();

			TMap<FUniqueStaticMesh, UInstancedStaticMeshComponent*> ConvertMap;

			const bool bAddSurfix = UISetting.bConvertFromActorLabelAddSurfix;
			FString ActorLabelSubfix(TEXT(""));
			if (bAddSurfix)
			{
				if (UISetting.PlaceInstancesInto == EConvertFromActorOption::HierarchicalInstancedStaticMeshActor)
				{
					ActorLabelSubfix = TEXT(" (HISMA)");
				}
				else
				{
					ActorLabelSubfix = TEXT(" (ISMA)");
				}
			}

			if (ExistingTargetActor)
			{
				ExistingTargetActor->Modify();
			}

			const bool bCreateNewActorByMaterial = UISetting.bCreateNewActorByMaterial;
			FUniqueStaticMesh::bUseMaterial = bCreateNewActorByMaterial;

			const bool bCreateNewActorByScale = UISetting.bCreateNewActorForNegativeScaledActors;
			FUniqueStaticMesh::bUseScale = bCreateNewActorByScale;

			const bool bForcePositiveScale = UISetting.bForcePositiveInstanceScale;

			const bool bCheckMaterialUsedWithInstancedStaticMeshesFlag = UISetting.bCheckMaterialUsedWithInstancedStaticMeshesFlag;

			EConvertFromActorLabelOption ActorLabelOption = UISetting.ConvertFromActorLabelOption;
			FString ActorCustomLabel = UISetting.ConvertFromActorCustomLabel;
			if (ActorLabelOption == EConvertFromActorLabelOption::Custom && ActorCustomLabel.TrimEnd().IsEmpty())
			{
				ActorLabelOption = EConvertFromActorLabelOption::FromStaticMesh;
			}

			// Filter Selected Actors base on MinmumNumConvertedInstances
			if (ExistingTargetActor)
			{
				ConvertingActors = SelectedActors;
			}
			else
			{
				TMap<UStaticMesh*, TArray<AActor*>> MeshActorMap;
				for (int32 ActorIndex = SourceActorsCount - 1; ActorIndex >= 0; --ActorIndex)
				{
					AActor* SelectedActor = SelectedActors[ActorIndex];
					TArray<UStaticMeshComponent*> StaticMeshComponents;
					SelectedActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
					for (auto Component : StaticMeshComponents)
					{
						UStaticMesh* StaticMesh = Component->GetStaticMesh();
						if (!StaticMesh)
						{
							continue;
						}
						MeshActorMap.FindOrAdd(StaticMesh).AddUnique(SelectedActor);
					}
				}
				int32 MinmumNumConvertedInstances = UISetting.MinmumNumConvertedInstances;
				for (auto& Pair : MeshActorMap)
				{
					TArray<AActor*>& Actors = Pair.Value;
					if (Actors.Num() >= MinmumNumConvertedInstances)
					{
						for (AActor* Actor : Actors)
						{
							ConvertingActors.AddUnique(Actor);
						}
					}
				}
			}

			for (int32 ActorIndex = ConvertingActors.Num() - 1; ActorIndex >= 0; --ActorIndex)
			{
				AActor* ConvertingActor = ConvertingActors[ActorIndex];
				UWorld* World = ConvertingActor->GetLevel()->OwningWorld;
				
				TArray<UStaticMeshComponent*> StaticMeshComponents;
				ConvertingActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

				for (auto Component : StaticMeshComponents)
				{
					UStaticMesh* StaticMesh = Component->GetStaticMesh();
					if (!StaticMesh)
					{
						continue;
					}

					// Ensure Material has bUsedWithInstancedStaticMeshes Flag checked
					if (bCheckMaterialUsedWithInstancedStaticMeshesFlag)
					{
						for (int32 i = 0; i < Component->GetNumMaterials(); ++i)
						{
							if (UMaterialInterface* MaterialInterface = Component->GetMaterial(i))
							{
								UMaterial* MaterialToCheck = MaterialInterface->GetMaterial();
								Local::EnsureMaterialHasUsedWithInstancedStaticMeshesFlag(MaterialToCheck);
							}
						}
					}

					FUniqueStaticMesh UniqueMesh(Component);

					if (!ConvertMap.Contains(UniqueMesh))
					{
						AActor* TargetActor = nullptr;
						UInstancedStaticMeshComponent* TargetComponent = nullptr;

						if (ExistingTargetActor)
						{
							TargetActor = ExistingTargetActor;

							if (UISetting.bTryReuseComponentInActor)
							{
								TArray<UInstancedStaticMeshComponent*> ExistingComponents;
								ExistingTargetActor->GetComponents<UInstancedStaticMeshComponent>(ExistingComponents);
								UInstancedStaticMeshComponent* FirstEmptyISMC = nullptr;
								for (auto TargetISMC : ExistingComponents)
								{
									if (!FirstEmptyISMC && !TargetISMC->GetStaticMesh())
									{
										FirstEmptyISMC = TargetISMC;
									}

									if (TargetISMC->GetStaticMesh() == StaticMesh)
									{
										TargetComponent = TargetISMC;
										break;
									}
								}
								if (!TargetComponent && FirstEmptyISMC)
								{
									TargetComponent = FirstEmptyISMC;
								}
							}
						}
						else
						{
							FActorSpawnParameters SpawnInfo;
							SpawnInfo.OverrideLevel = ConvertingActor->GetLevel();
							SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass(), ConvertingActor->GetTransform(), SpawnInfo);

							if (NewActor)
							{
								FString ActorLabel = ActorCustomLabel;
								if (ActorLabelOption == EConvertFromActorLabelOption::FromStaticMesh)
								{
									ActorLabel = StaticMesh->GetName();
								}
								else if (ActorLabelOption == EConvertFromActorLabelOption::FromSelectedActor)
								{
									ActorLabel = ConvertingActor->GetActorLabel();
								}

								NewActor->SetActorLabel(ActorLabel + ActorLabelSubfix);
								NewActor->Layers = ConvertingActor->Layers;
								NewActor->SetFolderPath(ConvertingActor->GetFolderPath());
								NewActor->InvalidateLightingCache();
								NewActor->MarkPackageDirty();

								TargetActor = NewActor;
							}
							else
							{
								continue; // Todo InstanceTool: Error when creating new actor
							}
						}

						TargetActors.AddUnique(TargetActor);

						bool bReuseExistingComponent = TargetComponent != nullptr;

						if (!TargetComponent)
						{
							if (UISetting.PlaceInstancesInto == EConvertFromActorOption::HierarchicalInstancedStaticMeshActor)
							{
								FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(UHierarchicalInstancedStaticMeshComponent::StaticClass(), TargetActor);
								TargetComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(TargetActor, UHierarchicalInstancedStaticMeshComponent::StaticClass(), NewComponentName, RF_Transactional);
							}
							else
							{
								FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(UInstancedStaticMeshComponent::StaticClass(), TargetActor);
								TargetComponent = NewObject<UInstancedStaticMeshComponent>(TargetActor, UInstancedStaticMeshComponent::StaticClass(), NewComponentName, RF_Transactional);
							}

							USceneComponent* RootComponent = TargetActor->GetRootComponent();
							if (RootComponent)
							{
								TargetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
							}
							else
							{
								TargetActor->SetRootComponent(TargetComponent);
							}

							// Add to SerializedComponents array so it gets saved
							TargetActor->AddInstanceComponent(TargetComponent);

							TargetComponent->CreationMethod = EComponentCreationMethod::Instance;
							TargetComponent->OnComponentCreated();
							TargetComponent->RegisterComponent();

							TargetActor->RerunConstructionScripts();
						}

						// Pivot
						if (!ExistingTargetActor)
						{
							FVector Pivot(FVector::ZeroVector);
							EConvertFromActorPivotOption PivotOption = UISetting.ConvertFromActorPivotOption;
							if (PivotOption == EConvertFromActorPivotOption::LastSelectedActor)
							{
								Pivot = LastSelectedActor->GetActorLocation();
							}
							else if (PivotOption == EConvertFromActorPivotOption::SelectionCenter)
							{
								Pivot = GetSelectionCenter();
							}
							TargetActor->SetActorLocation(Pivot);
							TargetActor->PostEditMove(true);
						}

						if (bReuseExistingComponent)
						{
							if (!TargetComponent->GetStaticMesh())
							{
								TargetComponent->SetStaticMesh(Component->GetStaticMesh());

								//FProperty* StaticMeshProp = FindFieldChecked<FProperty>(UStaticMeshComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UInstancedStaticMeshComponent, StaticMesh));
								FProperty* StaticMeshProp = FindFieldChecked<FProperty>(UStaticMeshComponent::StaticClass(), "StaticMesh");
								FProperty* OverrideMaterialsProp = FindFieldChecked<FProperty>(UStaticMeshComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UInstancedStaticMeshComponent, OverrideMaterials));
								InstanceToolUtilities::CopySingleProperty(Component, TargetComponent, StaticMeshProp);
								InstanceToolUtilities::CopySingleProperty(Component, TargetComponent, OverrideMaterialsProp);
							}
						}
						else
						{
							TargetComponent->SetStaticMesh(Component->GetStaticMesh());

							InstanceToolUtilities::CopyComponentProperty<UStaticMeshComponent>(Component, TargetComponent);
							
							// From ISMC
							if (UInstancedStaticMeshComponent* FromISMC = Cast<UInstancedStaticMeshComponent>(Component))
							{
								TargetComponent->SetWorldTransform(FromISMC->GetComponentTransform());
							}
							// Or from SMC
							else
							{
								// Setup Default Property Value for ISMC/HISMC
								TargetComponent->bDisallowMeshPaintPerInstance_DEPRECATED = true;

								if (UISetting.PlaceInstancesInto == EConvertFromActorOption::HierarchicalInstancedStaticMeshActor)
								{
									//TargetComponent->bCanEverAffectNavigation = true;
									TargetComponent->bUseAsOccluder = false;
								}
							}

							if (bCreateNewActorByScale)
							{
								TargetComponent->SetWorldScale3D(UniqueMesh.GetScale());
							}

							//TargetComponent->RegisterComponent();

							// Hack: UpdateBounds to trigger KnowStaticMesh update in NotifyIfStaticMeshChanged
							TargetComponent->UpdateBounds();
						}

						TargetComponent->bHasPerInstanceHitProxies = true;

						//TargetActor->RerunConstructionScripts();

						ConvertMap.Add(UniqueMesh, TargetComponent);
					}
					
					// Adding instances
					{
						UInstancedStaticMeshComponent* ConvertedComponent = *ConvertMap.Find(UniqueMesh);

						bool bForceScaleAlign = UISetting.bAutoAlignScaleToGrid;
						float ForceScaleSnapSize = GEditor->GetScaleGridSize();

						EditMode->PreAddInstanceForUndo();

						UInstanceToolEditorObject* Proxy = NewObject<UInstanceToolEditorObject>();

						if (UInstancedStaticMeshComponent* FromISMC = Cast<UInstancedStaticMeshComponent>(Component))
						{
							int32 InstanceCount = FromISMC->GetInstanceCount();
							for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; ++InstanceIndex)
							{
								FTransform WorldTM;
								FromISMC->GetInstanceTransform(InstanceIndex, WorldTM, /*bWorldSpace=*/ true);

								int32 NewInstanceIndex = ConvertedComponent->AddInstance(WorldTM, /*bWorldSpace*/ true);

								if (bForceScaleAlign || bForcePositiveScale)
								{
									Proxy->Init(ConvertedComponent, NewInstanceIndex);

									if (bForceScaleAlign)
									{
										EditMode->Selection.ScaleSnapSelectedObject(Proxy, ForceScaleSnapSize, InstanceIndex == InstanceCount - 1);
									}
									else
									{
										EditMode->Selection.ForcePositiveScaleOnSelectedObject(Proxy, InstanceIndex == InstanceCount - 1);
									}
								}

								++ConvertedInstancesCount;
							}
						}
						else
						{
							FTransform SourceTransfrom = Component->GetComponentTransform();
							int32 NewInstanceIndex = ConvertedComponent->AddInstance(Component->GetComponentTransform(), /*bWorldSpace*/ true);

							if (bForceScaleAlign || bForcePositiveScale)
							{
								Proxy->Init(ConvertedComponent, NewInstanceIndex);

								if (bForceScaleAlign)
								{
									EditMode->Selection.ScaleSnapSelectedObject(Proxy, ForceScaleSnapSize, true);
								}
								else
								{
									EditMode->Selection.ForcePositiveScaleOnSelectedObject(Proxy, true);
								}
							}

							++ConvertedInstancesCount;
						}

						EditMode->PostAddInstanceForUndo();
					}

					ActualUsedSourceActors.Add(ConvertingActor);
				}
			}

			if (ExistingTargetActor && ExistingTargetActor->GetClass()->ClassGeneratedBy)
			{
				TArray<AActor*> ModifiedActors;
				ModifiedActors.Add(ExistingTargetActor);
				EditMode->NoteInstanceDataModified(ModifiedActors);
			}

			// Delete or Hide Old Actors
			if (UISetting.bDeleteSourceActors)
			{
				UActorGroupingUtils::Get()->UngroupSelected();
				//GEditor->edactDeleteSelected(LastSelectedActor->GetLevel()->OwningWorld);
				for (AActor* Actor : ConvertingActors)
				{
					if (Actor && !Actor->IsPendingKillPending())
					{
						GEditor->GetEditorSubsystem<ULayersSubsystem>()->DisassociateActorFromLayers(Actor);
						Actor->GetWorld()->EditorDestroyActor(Actor, true);
					}
				}
			}
			else
			{
				bool bSourceActorsInSameLevel = true;
				ULevel* ActorLevel = nullptr;
				for (auto Actor : ConvertingActors)
				{
					if (!ActorLevel)
					{
						ActorLevel = Actor->GetLevel();
					}
					else if (ActorLevel != Actor->GetLevel())
					{
						bSourceActorsInSameLevel = false;
						break;
					}
				}

				AGroupActor* SpawnedGroupActor = nullptr;
				if (bSourceActorsInSameLevel && UISetting.bGroupSourceActors)
				{
					FActorSpawnParameters SpawnInfo;
					SpawnInfo.OverrideLevel = LastSelectedActor->GetLevel();
					SpawnedGroupActor = LastSelectedActor->GetLevel()->OwningWorld->SpawnActor<AGroupActor>(SpawnInfo);
				}

				for (auto Actor : ConvertingActors)
				{
					if (UISetting.bSetSourceActorsNotRender)
					{
						auto RootComponent = Actor->GetRootComponent();
						if (RootComponent)
						{
							RootComponent->Modify();
							RootComponent->SetVisibility(false, true);
						}
					}

					if (UISetting.bPlaceSourceActorsInFolder)
					{
						const FName NewFolderName = FName(*(LastSelectedActor->GetFolderPath().ToString() / TEXT("Source Actors (Instance Tool)")));
						Actor->SetFolderPath(NewFolderName);
					}

					if (SpawnedGroupActor)
					{
						SpawnedGroupActor->Add(*Actor);
					}
				}

				if (SpawnedGroupActor)
				{
					SpawnedGroupActor->CenterGroupLocation();
				}
			}

			GEditor->SelectNone(true, true);

			const bool bSelectAllInstancesAfterConvert = UISetting.bSelectAllInstancesAfterConvert;
			if (bSelectAllInstancesAfterConvert)
			{
				EditMode->SelectAllInstancesOfActors(TargetActors);
			}
			else
			{
				GEditor->GetSelectedActors()->BeginBatchSelectOperation();
				for (auto Actor : TargetActors)
				{
					//Actor->Modify();
					Actor->MarkPackageDirty();
					GEditor->SelectActor(Actor, true, false);
				}
				GEditor->GetSelectedActors()->EndBatchSelectOperation();
				GEditor->NoteSelectionChange();
			}

			ULevel::LevelDirtiedEvent.Broadcast();
		}

		EditMode->ForceRebuildRenderData();

		EditMode->NotifyMessage(FText::Format(LOCTEXT("ConvertInstancesFinished", "{0} actors have been converted to {1} instances of {2} actors!"), ConvertingActors.Num(), ConvertedInstancesCount, TargetActors.Num()));
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE