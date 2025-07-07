// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassCrowdAnimationTypes.h"
#include "Animation/AnimMontage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MassCrowdAnimationTypes)

void FMassMontageFragment::Request(const UE::CrowdInteractionAnim::FRequest& InRequest)
{
	InteractionRequest = InRequest;
	SkippedTime = 0.0f;
	MontageInstance.Initialize(InRequest.QueryResult.Animation.Get(), InteractionRequest.QueryResult.AnimStartTime);
}

void FMassMontageFragment::Clear()
{
	*this = FMassMontageFragment();
}
