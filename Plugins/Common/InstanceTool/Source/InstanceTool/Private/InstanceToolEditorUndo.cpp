// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorUndo.h"
#include "InstanceToolEdMode.h"
#include "InstanceToolModule.h"
#include "InstanceToolEditorUISetting.h"
#include "InstanceToolEditorObject.h"

#include "Components/InstancedStaticMeshComponent.h"

///////////////////////////////////////////////////////////////////
FSelectUndoRecord::FSelectUndoRecord(UInstancedStaticMeshComponent* InComponent, int32 InIndex)
	: Component(InComponent)
	, InstanceIndex(InIndex)
{
	InComponent->GetInstanceTransform(InstanceIndex, Transform, true);
}

///////////////////////////////////////////////////////////////////
FInstancesSnapshot::FInstancesSnapshot(UInstancedStaticMeshComponent* InComponent)
	:Component(InComponent)
{
	Update();
}

void FInstancesSnapshot::Update()
{
	if (Component)
	{
		Transforms.Empty();
		int32 InstanceCount = Component->PerInstanceSMData.Num();
		for (int32 Index = 0; Index < InstanceCount; ++Index)
		{
			FTransform WorldTM;
			Component->GetInstanceTransform(Index, WorldTM, true);
			Transforms.Add(WorldTM);
		}
	}
}

bool UInstanceToolEditorUndo::Modify(bool bAlwaysMarkDirty /*= true*/)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bDisableUndo)
	{
		return false;
	}

	TArray<TWeakObjectPtr<UInstancedStaticMeshComponent>> SelectedComponents = ParentMode->Selection.GetSelectedComponents();
	for (auto& Component : SelectedComponents)
	{
		if (IsValid(Component.Get()))
		{
			if (!Component->HasAnyFlags(RF_Transactional))
			{
				Component->SetFlags(RF_Transactional);
			}
			Component->Modify();
		}
	}

	return Super::Modify(bAlwaysMarkDirty);
}

////////////////////////////////////////////////////////////////////////
void UInstanceToolEditorUndo::PostEditUndo()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bDisableUndo)
	{
		return;
	}

	TArray<AActor*> BlueprintActors;
	TArray<UInstancedStaticMeshComponent*> ProcessedComponents;

	if (Instances.Num() > 0)
	{
		for (FInstancesSnapshot& Item : Instances)
		{
			UInstancedStaticMeshComponent* Component = Item.Component;
			auto& Transforms = Item.Transforms;

			if (!IsValid(Component) || !Component->IsValidLowLevel())
			{
				continue;
			}

			if (ProcessedComponents.Contains(Component))
			{
				continue;
			}
			ProcessedComponents.Add(Component);

// 			int32 InstanceCount = Transforms.Num();
// 			if (Component->GetInstanceCount() == InstanceCount)
// 			{
// 				continue;
// 			}

 			FProperty* PerInstanceSMDataProp = FindFieldChecked<FProperty>(UInstancedStaticMeshComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UInstancedStaticMeshComponent, PerInstanceSMData));
			FInstanceToolUtil::PostEditChangeChainProperty(Component, PerInstanceSMDataProp);
			Component->ClearInstanceSelection();

			if (Component->GetOwner()->GetClass()->ClassGeneratedBy)
			{
				BlueprintActors.AddUnique(Component->GetOwner());
			}
		}
	}

	ParentMode->Selection.SetSelectedNone(/*bNotify=*/ true);	
	if (Selected.Num() > 0)
	{
		int32 SelectedCount = Selected.Num();
		for (int32 Index = 0; Index < SelectedCount; ++Index)
		{
			bool bNotify = Index == SelectedCount - 1;
			FSelectUndoRecord& SelectRecord = Selected[Index];
			if (SelectRecord.Component)
			{
				if (!ProcessedComponents.Contains(SelectRecord.Component))
				{
					FProperty* TransformProp = FindFieldChecked<FProperty>(TBaseStructure<FInstancedStaticMeshInstanceData>::Get(), GET_MEMBER_NAME_CHECKED(FInstancedStaticMeshInstanceData, Transform));					
					FInstanceToolUtil::PostEditChangeChainProperty(SelectRecord.Component, TransformProp);

					ProcessedComponents.Add(SelectRecord.Component);
				}

				AActor* Actor = SelectRecord.Component->GetOwner();
				if (IsValid(Actor))
				{
					if (Actor->GetClass()->ClassGeneratedBy)
					{
						BlueprintActors.AddUnique(Actor);
					}
					//SelectRecord.Component->UpdateInstanceTransform(SelectRecord.InstanceIndex, SelectRecord.Transform, true, bNotify, /*bTeleport=*/ true);
					ParentMode->Selection.SelectInstance(true, Selected[Index].Component, Selected[Index].InstanceIndex, true, /*bBroadcastChange=*/ bNotify);
				}
			}
		}

		if (UISetting.bAutoAlignScaleToGrid)
		{
			//ParentMode->AlignSelectionToGrid(FWidget::WM_Scale, false);
		}
	}

	if (BlueprintActors.Num() > 0)
	{
		//ParentMode->PropagateChangesToBlueprintInstances(BlueprintActors, /*bModify*/ false);
		//ParentMode->NoteInstanceDataModified(BlueprintActors);
	}

	ParentMode->ForceRebuildRenderData();

	GEditor->RedrawLevelEditingViewports(/*bInvalidateHitProxies=*/true);
}

void UInstanceToolEditorUndo::SetParent(FInstanceToolEdMode* EditMode)
{
	ParentMode = EditMode;
}

void UInstanceToolEditorUndo::Reset()
{
	Selected.Reset();
	Instances.Reset();
}

void UInstanceToolEditorUndo::UpdateSelected()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bDisableUndo)
	{
		return;
	}

	Selected.Empty();

	auto& Proxies = ParentMode->Selection.GetSelectedProxyObjects();
	for (auto Proxy : Proxies)
	{
		Selected.Emplace(Proxy->Component, Proxy->InstanceIndex);
	}
}

void UInstanceToolEditorUndo::UpdateInstancesSnapshot()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bDisableUndo)
	{
		return;
	}

	auto SelectedComponents = ParentMode->Selection.GetSelectedComponents();

	if (SelectedComponents.Num() > 0)
	{
		Instances.Empty();
		for (auto& Component : SelectedComponents)
		{
			Instances.Emplace(Component.Get());
		}
	}
	else
	{
		for (auto& Item : Instances)
		{
			Item.Update();
		}
	}
}

void UInstanceToolEditorUndo::UpdateSelectedTransform(UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex, FTransform& InTransform)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bDisableUndo)
	{
		return;
	}

	auto Predicate = [&](const FSelectUndoRecord& Other) {
		return Other.Component == InComponent && Other.InstanceIndex == InInstanceIndex;
	};

	FSelectUndoRecord* SelectRecord = Selected.FindByPredicate(Predicate);
	if (SelectRecord)
	{
		SelectRecord->Transform = InTransform;
	}
}

void UInstanceToolEditorUndo::UpdateSelectedTransforms()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bDisableUndo)
	{
		return;
	}

	auto& Proxies = ParentMode->Selection.GetSelectedProxyObjects();
	for (auto Proxy : Proxies)
	{
		UpdateSelectedTransform(Proxy->Component, Proxy->InstanceIndex, Proxy->InstanceToWorld);
	}
}

void UInstanceToolEditorUndo::OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bDisableUndo)
	{
		return;
	}

	int32 SelectedIndex = Selected.Num();
	while (SelectedIndex--)
	{
		auto& Item = Selected[SelectedIndex];
		if (auto NewObject = ReplacementMap.Find(Item.Component))
		{
			const UInstancedStaticMeshComponent* NewConstComponent = Cast<UInstancedStaticMeshComponent>(*NewObject);
			if (NewConstComponent && NewConstComponent->PerInstanceSMData.IsValidIndex(Item.InstanceIndex))
			{
				auto NewComponent = const_cast<UInstancedStaticMeshComponent*>(NewConstComponent);
				Item.Component = NewComponent;
			}
			else
			{
				Selected.RemoveAt(SelectedIndex);
			}
		}
	}

	int32 InstancesIndex = Instances.Num();
	while (InstancesIndex--)
	{
		auto& Item = Instances[InstancesIndex];
		if (auto NewObject = ReplacementMap.Find(Item.Component))
		{
			const UInstancedStaticMeshComponent* NewConstComponent = Cast<UInstancedStaticMeshComponent>(*NewObject);
			if (NewConstComponent)
			{
				auto NewComponent = const_cast<UInstancedStaticMeshComponent*>(NewConstComponent);
				Item.Component = NewComponent;
			}
			else
			{
				Instances.RemoveAt(InstancesIndex);
			}
		}
	}
}

