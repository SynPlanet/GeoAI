// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorUndo.generated.h"

USTRUCT()
struct FSelectUndoRecord //: public FGCObject
{
	GENERATED_USTRUCT_BODY()

	FSelectUndoRecord() 
		: Component(nullptr)
		, InstanceIndex(-1)
		, Transform(FTransform::Identity)
	{}
	FSelectUndoRecord(class UInstancedStaticMeshComponent* InComponent, int32 InIndex, FTransform& InTransform) 
		: Component(InComponent)
		, InstanceIndex(InIndex)
		, Transform(InTransform)
	{}
	FSelectUndoRecord(class UInstancedStaticMeshComponent* InComponent, int32 InIndex);

	UPROPERTY()
	class UInstancedStaticMeshComponent* Component;

	UPROPERTY()
	int32 InstanceIndex;

	UPROPERTY()
	FTransform Transform;
};

USTRUCT()
struct FInstancesSnapshot
{
	GENERATED_USTRUCT_BODY()

	FInstancesSnapshot()
		: Component(nullptr)
	{}
	FInstancesSnapshot(class UInstancedStaticMeshComponent* InComponent);

	void Update();

	UPROPERTY()
	TArray<FTransform> Transforms;

	UPROPERTY()
	class UInstancedStaticMeshComponent* Component;
};

UCLASS()
class UInstanceToolEditorUndo : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY()
		//UPROPERTY(Category = "UndoRedo", EditAnywhere)
		TArray<FSelectUndoRecord> Selected;

	UPROPERTY()
		TArray<FInstancesSnapshot> Instances;

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
	void PostEditUndo() override;
	//~ End UObject Interface
#endif // WITH_EDITOR

	void Reset();
	void SetParent(class FInstanceToolEdMode* EditMode);

	//~ UndoRedo
	void UpdateInstancesSnapshot();	
	void UpdateSelected();
	void UpdateSelectedTransform(UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex, FTransform& InTransform);
	void UpdateSelectedTransforms();
	void OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap);
	//~ UndoRedo

private:
	class FInstanceToolEdMode* ParentMode;
};

