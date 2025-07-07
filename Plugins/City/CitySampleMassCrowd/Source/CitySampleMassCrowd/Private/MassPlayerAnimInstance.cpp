// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/MassPlayerAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MassPlayerAnimInstance)

UMassPlayerAnimInstance::UMassPlayerAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UMassPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UMassPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	FMassCharacterProximity_UpdateContext UpdateContext;
	UpdateContext.DeltaSeconds = DeltaSeconds;
	UpdateContext.MeshTransform = GetOwningComponent()->GetComponentTransform();
	CrowdProximity.Update(UpdateContext);
}