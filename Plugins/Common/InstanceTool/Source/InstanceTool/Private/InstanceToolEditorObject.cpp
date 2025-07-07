// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorObject.h"
#include "InstanceToolEdMode.h"
#include "EditorModeManager.h"
#include "Components/InstancedStaticMeshComponent.h"

UInstanceToolEditorObject::UInstanceToolEditorObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UInstanceToolEditorObject::PostEditUndo()
{
/*	OnTransfromChanged(false);*/
}

void UInstanceToolEditorObject::SetRelativeLocation(FVector NewLocation, bool bCommitted)
{	
	if (NewLocation != RelativeLocation)
	{
		RelativeLocation = NewLocation;
		OnTransfromChanged(bCommitted);
	}
}

void UInstanceToolEditorObject::SetRelativeRotation(FRotator NewRotation, bool bCommitted)
{
	if (!NewRotation.Equals(RelativeRotation, INSTANCE_ROTATOR_TOLERANCE))
	{
		RelativeRotation = NewRotation;
		OnTransfromChanged(bCommitted);
	}
}

void UInstanceToolEditorObject::SetRelativeScale3D(FVector NewScale3D, bool bCommitted)
{
	if (NewScale3D != RelativeScale3D)
	{
		if (NewScale3D.ContainsNaN())
		{
			NewScale3D = FVector(1.f);
		}
		
		RelativeScale3D = NewScale3D;
		OnTransfromChanged(bCommitted);
	}
}

void UInstanceToolEditorObject::Init(UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex)
{
	Component = InComponent;
	InstanceIndex = InInstanceIndex;
	
	FTransform InstanceTM;
	Component->GetInstanceTransform(InstanceIndex, InstanceTM, /*bWorldSpace=*/ true);
	SetWorldTransform(InstanceTM);
}

void UInstanceToolEditorObject::UpdateInstanceTransform(FTransform& InTransform, bool bWorldSpace, bool bMarkRenderStateDirty)
{
	Component->UpdateInstanceTransform(InstanceIndex, InTransform, bWorldSpace, bMarkRenderStateDirty, /*bTeleport=*/ true);
	Component->GetOwner()->MarkPackageDirty();

	RefreshTransform();
}

void UInstanceToolEditorObject::SetRelativeTransform(FTransform& InRelativeTransfom)
{
	FTransform WorldTransform = InRelativeTransfom * Component->GetComponentTransform();
	InstanceToWorld = WorldTransform;

#if DEBUG_INSTANCE_MATRIX
	SetMatrixForDebug();
#endif

	RelativeLocation = Component->IsUsingAbsoluteLocation() ? InstanceToWorld.GetTranslation() : InRelativeTransfom.GetTranslation();
	RelativeRotation = Component->IsUsingAbsoluteRotation() ? InstanceToWorld.GetRotation().Rotator() : InRelativeTransfom.GetRotation().Rotator();
	RelativeScale3D = Component->IsUsingAbsoluteScale() ? InstanceToWorld.GetScale3D() : InRelativeTransfom.GetScale3D();
}

void UInstanceToolEditorObject::SetWorldTransform(FTransform& InWorldTransfom)
{
	InstanceToWorld = InWorldTransfom;

#if DEBUG_INSTANCE_MATRIX
	SetMatrixForDebug();
#endif

	FTransform RelativeTransform = InstanceToWorld.GetRelativeTransform(Component->GetComponentTransform());

	RelativeLocation = Component->IsUsingAbsoluteLocation() ? InstanceToWorld.GetTranslation() : RelativeTransform.GetTranslation();
	RelativeRotation = Component->IsUsingAbsoluteRotation() ? InstanceToWorld.GetRotation().Rotator() : RelativeTransform.GetRotation().Rotator();
	RelativeScale3D = Component->IsUsingAbsoluteScale() ? InstanceToWorld.GetScale3D() : RelativeTransform.GetScale3D();
}

void UInstanceToolEditorObject::OnTransfromChanged(bool bCommitted)
{
	if (Component && Component->PerInstanceSMData.IsValidIndex(InstanceIndex))
	{
		FVector NewWorldLoation = RelativeLocation;
		if (!Component->IsUsingAbsoluteLocation())
		{
			NewWorldLoation = Component->GetComponentTransform().TransformPosition(RelativeLocation);
		}

		FQuat NewWorldQuat = RelativeRotation.Quaternion();
		if (!Component->IsUsingAbsoluteRotation())
		{
			NewWorldQuat = RelativeRotation.Quaternion() * Component->GetComponentTransform().GetRotation();
		}

		FVector NewWorldScale3D = RelativeScale3D;
		if (!Component->IsUsingAbsoluteScale())
		{
			NewWorldScale3D = RelativeScale3D * Component->GetComponentTransform().GetScale3D();
		}

		InstanceToWorld = FTransform(NewWorldQuat, NewWorldLoation, NewWorldScale3D);

		if (!bCommitted)
		{
			Component->UpdateInstanceTransform(InstanceIndex, InstanceToWorld, /*bWorldSpace=*/ true, /*bMarkRenderStateDirty=*/ true, /*bTeleport=*/ true);
		}
		//Component->InvalidateLightingCache();
		else
		{
			FInstanceToolEdMode* EditMode = (FInstanceToolEdMode*)GLevelEditorModeTools().GetActiveMode(FInstanceToolEdMode::EM_InstanceToolEdModeId);
			if (EditMode)
			{
				EditMode->TransformInstance(this, bCommitted);
			}
		}
	}
}

void UInstanceToolEditorObject::RefreshTransform()
{
	FTransform RelativeTransform;
	Component->GetInstanceTransform(InstanceIndex, RelativeTransform, /*bWorldSpace=*/ false);
	SetRelativeTransform(RelativeTransform);
}

#if DEBUG_INSTANCE_MATRIX
void UInstanceToolEditorObject::SetMatrixForDebug()
{
	FMatrix& InstanceMatrix = Component->PerInstanceSMData[InstanceIndex].Transform;
	M1 = FVector4(InstanceMatrix.M[0][0], InstanceMatrix.M[0][1], InstanceMatrix.M[0][2], InstanceMatrix.M[0][3]);
	M2 = FVector4(InstanceMatrix.M[1][0], InstanceMatrix.M[1][1], InstanceMatrix.M[1][2], InstanceMatrix.M[1][3]);
	M3 = FVector4(InstanceMatrix.M[2][0], InstanceMatrix.M[2][1], InstanceMatrix.M[2][2], InstanceMatrix.M[2][3]);
	M4 = FVector4(InstanceMatrix.M[3][0], InstanceMatrix.M[3][1], InstanceMatrix.M[3][2], InstanceMatrix.M[3][3]);
}
#endif
