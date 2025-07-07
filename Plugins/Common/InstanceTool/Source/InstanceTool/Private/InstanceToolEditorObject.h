// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorObject.generated.h"

#define DEBUG_INSTANCE_MATRIX 0

#define INSTANCE_QUAT_TOLERANCE		(1.e-8f) // Comparison tolerance for checking if two FQuats are the same when moving instance.
#define INSTANCE_ROTATOR_TOLERANCE	(1.e-4f) // Comparison tolerance for checking if two FRotators are the same when moving instance.

UCLASS()
class UInstanceToolEditorObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:

#if WITH_EDITOR
	//~ Begin UObject Interface
	void PostEditUndo() override;
	//~ End UObject Interface
#endif // WITH_EDITOR

	UPROPERTY(Category = "Instance", VisibleAnywhere, Transient)
	int32 InstanceIndex;

	UPROPERTY(Category = "Instance", DisplayName = "Actor", VisibleAnywhere, Transient)
	FString ActorLabel;

	//UPROPERTY(Category = "Instance", EditAnywhere, Transient, meta = (DisplayName = "Instance Transform", ShowForTools = "Select"))
	FTransform InstanceToWorld;

	UPROPERTY(Category = "Debug", VisibleAnywhere, Transient)
	FVector4 M1;

	UPROPERTY(Category = "Debug", VisibleAnywhere, Transient)
	FVector4 M2;

	UPROPERTY(Category = "Debug", VisibleAnywhere, Transient)
	FVector4 M3;

	UPROPERTY(Category = "Debug", VisibleAnywhere, Transient)
	FVector4 M4;

	//UPROPERTY(Category = "Instance", VisibleAnywhere, Transient, meta = (DisplayName = "Component"))
	class UInstancedStaticMeshComponent* Component;

public:

	/** Location of the component relative to its parent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hidden")
	FVector RelativeLocation;

	/** Rotation of the component relative to its parent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hidden")
	FRotator RelativeRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Hidden")
	FVector RelativeScale3D;

// 	class FInstanceToolEdMode* ParentMode;
// 
// 	void SetParent(FInstanceToolEdMode* InstanceToolEdMode)
// 	{
// 		ParentMode = ParentMode;
// 	}


public:
	void SetRelativeLocation(FVector NewLocation, bool bCommitted);
	void SetRelativeRotation(FRotator NewRotation, bool bCommitted);
	void SetRelativeScale3D(FVector NewScale3D, bool bCommitted);

public:
	void Init(class UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex);
	void UpdateInstanceTransform(FTransform& InTransform, bool bWorldSpace = true, bool bMarkRenderStateDirty = true);
	void OnTransfromChanged(bool bNoteParentMode);

private:

	void RefreshTransform();

	void SetWorldTransform(FTransform& InWorldTransfom);
	void SetRelativeTransform(FTransform& InRelativeTransfom);

#if DEBUG_INSTANCE_MATRIX
	void SetMatrixForDebug();
#endif
};
