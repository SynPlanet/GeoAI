// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorUISetting.h"
#include "Misc/ConfigCacheIni.h"

///////////////////////////////////////////////////////////////////
UInstanceToolEditorUISetting::UInstanceToolEditorUISetting(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)

	// Select
	, SelectAllOption(ESelectAllOption::SameActorAsSelected)
	, SelectByAxisTolerance(KINDA_SMALL_NUMBER)

	, bSelectByLocX(true)
	, bSelectByLocY(true)
	, bSelectByLocZ(true)
	, SelectByLocToleranceX(KINDA_SMALL_NUMBER)
	, SelectByLocToleranceY(KINDA_SMALL_NUMBER)
	, SelectByLocToleranceZ(KINDA_SMALL_NUMBER)

	, bSelectByRotX(true)
	, bSelectByRotY(true)
	, bSelectByRotZ(true)
	, SelectByRotToleranceX(KINDA_SMALL_NUMBER)
	, SelectByRotToleranceY(KINDA_SMALL_NUMBER)
	, SelectByRotToleranceZ(KINDA_SMALL_NUMBER)

	, bSelectByScaleX(true)
	, bSelectByScaleY(true)
	, bSelectByScaleZ(true)
	, SelectByScaleToleranceX(KINDA_SMALL_NUMBER)
	, SelectByScaleToleranceY(KINDA_SMALL_NUMBER)
	, SelectByScaleToleranceZ(KINDA_SMALL_NUMBER)


	, SelectInvalidToleranceLocation(KINDA_SMALL_NUMBER)
	, SelectInvalidToleranceRotation(KINDA_SMALL_NUMBER)
	, SelectInvalidToleranceScale(KINDA_SMALL_NUMBER)
	, bRebuildRenderDataApplyToAllActors(false)
	, MarqueeSelectOption(EMarqueeSelectOption::Interset)
	, bMarqueeSelectSameActorOnly(false)
	, bMarqueeSelectSubtractMode(false)
	, bHideWidget(false)
	, bLockSelection(false)

	// Align
	, AlignCoord(EAlignSnapCoord::Auto)

	// Distribute
	, DistributeCoord(EAlignSnapCoord::Auto)
	, bDistributeIgnoreSameLoc(false)
	, DistributeIgnoreSameLocTolerance(KINDA_SMALL_NUMBER)
	, bLineUpAlignRotation(false)

	// Snap
	, SnapCoord(EAlignSnapCoord::Auto)
	, SnapOffset(0.f)
	, bSnapFollowLastSelected(false)
	, bUseOwnSnapDir(false)
	, bAlwaysSnapToGrid(false)
	, bRotateToHitNormal(false)
	, SnapTraceOption(ESnapTraceOption::SingleLineTrace)
	, bSnapIgnoreStartPenetrating(false)
	, SnapTraceDistance(2000.f)
	, bSnapDrawDebug(false)

	// Delta Transform
	, DeltaTransformRotationOrder(ERotationOrder::XYZ)
	, bDeltaTransformDuplicate(false)
	, DeltaTransformDuplicateCopies(1)

	// Convert
	, bPlaceConvertedActorsInFolder(true)
	, bGroupConvertedActors(false)
	, bSelectConvertedActors(false)
	, bDeleteInstancesAfterConvert(true)
	, bDeleteSourceActorsIfAllInstancesDeleted(true)

	, PlaceInstancesInto(EConvertFromActorOption::HierarchicalInstancedStaticMeshActor)
	, ConvertFromActorLabelOption(EConvertFromActorLabelOption::FromStaticMesh)
	, ConvertFromActorCustomLabel(TEXT(""))
	, bConvertFromActorLabelAddSurfix(true)
	, ConvertFromActorPivotOption(EConvertFromActorPivotOption::SelectionCenter)
	, bCreateNewActorByMaterial(false)
	, bCreateNewActorForNegativeScaledActors(true)
	, bForcePositiveInstanceScale(false)
	, bCheckMaterialUsedWithInstancedStaticMeshesFlag(true)
	, bTryReuseComponentInActor(true)
	, bSetSourceActorsNotRender(true)
	, bPlaceSourceActorsInFolder(true)
	, bGroupSourceActors(false)
	, bDeleteSourceActors(true)
	, bSelectAllInstancesAfterConvert(true)
	, MinmumNumConvertedInstances(1)

	// Spawn
	, SpawnAutoAssignInstanceIdVar(TEXT("InstanceID"))
	, SpawnOffsetTransform(FTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector))
	, bSpawnActorAlwaysUseIdentityScale(false)
	, bPlaceSpawnedActorsInFolder(false)
	, SpawnedActorsFolder(TEXT("Instance Tool Spawned Actors"))

	, bSpawnOnSplineIgnoreRotation(false)
	, bSelectSpawnedInstances(false)
	, SpawnOnSplineOffsetTransform(FTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector))

	// Options
	, bRebuildInstancesWhenMapOpned(true)
	, bAllowBlueprintEditing(false)
	, bAlwaysApplyInstanceChangesToBlueprint(true)
	, bCanSelectActorInWorldOutliner(true)
	, bCanSelectActorInViewport(false)
	, bDrawFrameWhenActorOrInstanceSelected(true)
	, FrameColorWhenActorSelected(230, 0, 0)
	, FrameColorWhenInstanceSelected(0, 230, 0)
	, bDisableUndo(false)
	, bAutoAlignScaleToGrid(false)
{
	ExistingActor.Reset();
}

void UInstanceToolEditorUISetting::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName(PropertyChangedEvent.Property->GetFName());

		if (PropertyName == GET_MEMBER_NAME_CHECKED(UInstanceToolEditorUISetting, MinmumNumConvertedInstances))
		{
			MinmumNumConvertedInstances = FMath::Clamp(MinmumNumConvertedInstances, 1, INT_MAX);
		}
		Save();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UInstanceToolEditorUISetting::Load()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Select

	int32 InSelectAllOption = (int32)ESelectAllOption::SameActorAsSelected;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("SelectAllOption"), InSelectAllOption, GEditorPerProjectIni);
	SelectAllOption = (ESelectAllOption)InSelectAllOption;

	GConfig->GetFloat(TEXT("InstanceTool"), TEXT("SelectByAxisTolerance"), SelectByAxisTolerance, GEditorPerProjectIni);

	FVector InSelectByLocTolerance;
	GConfig->GetVector(TEXT("InstanceTool"), TEXT("SelectByLocTolerance"), InSelectByLocTolerance, GEditorPerProjectIni);
	SelectByLocToleranceX = InSelectByLocTolerance.X;
	SelectByLocToleranceY = InSelectByLocTolerance.Y;
	SelectByLocToleranceZ = InSelectByLocTolerance.Z;

	FVector InSelectByRotTolerance;
	GConfig->GetVector(TEXT("InstanceTool"), TEXT("SelectByRotTolerance"), InSelectByRotTolerance, GEditorPerProjectIni);
	SelectByRotToleranceX = InSelectByRotTolerance.X;
	SelectByRotToleranceY = InSelectByRotTolerance.Y;
	SelectByRotToleranceZ = InSelectByRotTolerance.Z;

	FVector InSelectByScaleTolerance;
	GConfig->GetVector(TEXT("InstanceTool"), TEXT("SelectByScaleTolerance"), InSelectByScaleTolerance, GEditorPerProjectIni);
	SelectByScaleToleranceX = InSelectByScaleTolerance.X;
	SelectByScaleToleranceY = InSelectByScaleTolerance.Y;
	SelectByScaleToleranceZ = InSelectByScaleTolerance.Z;

	FVector InSelectInvalidTolerance;
	GConfig->GetVector(TEXT("InstanceTool"), TEXT("SelectInvalidTolerance"), InSelectInvalidTolerance, GEditorPerProjectIni);
	SelectInvalidToleranceLocation = InSelectInvalidTolerance.X;
	SelectInvalidToleranceRotation = InSelectInvalidTolerance.Y;
	SelectInvalidToleranceScale = InSelectInvalidTolerance.Z;

	int32 InMarqueeSelectOption = (int32)EMarqueeSelectOption::Interset;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("MarqueeSelectOption"), InMarqueeSelectOption, GEditorPerProjectIni);
	MarqueeSelectOption = (EMarqueeSelectOption)InMarqueeSelectOption;

	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bMarqueeSelectSameActorOnly"), bMarqueeSelectSameActorOnly, GEditorPerProjectIni);


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Align

	int32 InAlignCoord = (int32)EAlignSnapCoord::Auto;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("AlignCoord"), InAlignCoord, GEditorPerProjectIni);
	AlignCoord = EAlignSnapCoord(InAlignCoord);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Distribute

	int32 InDistributeCoord = (int32)EAlignSnapCoord::Auto;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("DistributeCoord"), InDistributeCoord, GEditorPerProjectIni);
	DistributeCoord = EAlignSnapCoord(InDistributeCoord);

	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bDistributeIgnoreSameLoc"), bDistributeIgnoreSameLoc, GEditorPerProjectIni);
	GConfig->GetFloat(TEXT("InstanceTool"), TEXT("DistributeIgnoreSameLocTolerance"), DistributeIgnoreSameLocTolerance, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bLineUpAlignRotation"), bLineUpAlignRotation, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Snap

	int32 InSnapCoord = (int32)EAlignSnapCoord::Auto;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("SnapCoord"), InSnapCoord, GEditorPerProjectIni);
	SnapCoord = (EAlignSnapCoord)InSnapCoord;

	GConfig->GetFloat(TEXT("InstanceTool"), TEXT("SnapOffset"), SnapOffset, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bSnapFollowLastSelected"), bSnapFollowLastSelected, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bUseOwnSnapDir"), bUseOwnSnapDir, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bAlwaysSnapToGrid"), bAlwaysSnapToGrid, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bRotateToHitNormal"), bRotateToHitNormal, GEditorPerProjectIni);

	int32 InSnapTraceOption = (int32)ESnapTraceOption::BoundingBoxTrace;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("SnapTraceOption"), InSnapTraceOption, GEditorPerProjectIni);
	SnapTraceOption = ESnapTraceOption(InSnapTraceOption);

//	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bAlwaysUseLocalBoundForTracing"), bAlwaysUseLocalBoundForTracing, GEditorPerProjectIni); 
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bSnapIgnoreStartPenetrating"), bSnapIgnoreStartPenetrating, GEditorPerProjectIni);	
	GConfig->GetFloat(TEXT("InstanceTool"), TEXT("SnapTraceDistance"), SnapTraceDistance, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bSnapDrawDebug"), bSnapDrawDebug, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Delta Transform

	int32 InDeltaTransformRotationOrder = (int32)ERotationOrder::XYZ;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("DeltaTransformRotationOrder"), InDeltaTransformRotationOrder, GEditorPerProjectIni);
	DeltaTransformRotationOrder = (ERotationOrder)InDeltaTransformRotationOrder;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Convert

	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bPlaceConvertedActorsInFolder"), bPlaceConvertedActorsInFolder, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bGroupConvertedActors"), bGroupConvertedActors, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bSelectConvertedActors"), bSelectConvertedActors, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bDeleteInstancesAfterConvert"), bDeleteInstancesAfterConvert, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bDeleteSourceActorsIfAllInstancesDeleted"), bDeleteSourceActorsIfAllInstancesDeleted, GEditorPerProjectIni);
	
	int32 InPlaceInstancesInto = (int32)EConvertFromActorOption::HierarchicalInstancedStaticMeshActor;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("PlaceInstancesInto"), InPlaceInstancesInto, GEditorPerProjectIni);
	PlaceInstancesInto = (EConvertFromActorOption)InPlaceInstancesInto;

	int32 InConvertFromActorLabelOption = (int32)EConvertFromActorLabelOption::FromStaticMesh;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("ConvertFromActorLabelOption"), InConvertFromActorLabelOption, GEditorPerProjectIni);
	ConvertFromActorLabelOption = (EConvertFromActorLabelOption)InConvertFromActorLabelOption;

	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bConvertFromActorLabelAddSurfix"), bConvertFromActorLabelAddSurfix, GEditorPerProjectIni);

	int32 InConvertFromActorPivotOption = (int32)EConvertFromActorPivotOption::SelectionCenter;
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("ConvertFromActorPivotOption"), InConvertFromActorPivotOption, GEditorPerProjectIni);
	ConvertFromActorPivotOption = (EConvertFromActorPivotOption)InConvertFromActorPivotOption;

	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bCreateNewActorByMaterial"), bCreateNewActorByMaterial, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bCreateNewActorForNegativeScaledActors"), bCreateNewActorForNegativeScaledActors, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bForcePositiveInstanceScale"), bForcePositiveInstanceScale, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bTryReuseComponentInExistingActor"), bTryReuseComponentInActor, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bSetOldActorsNotRender"), bSetSourceActorsNotRender, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bPlaceOldActorsInFolder"), bPlaceSourceActorsInFolder, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bGroupSourceActors"), bGroupSourceActors, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bDeleteOldActors"), bDeleteSourceActors, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bSelectAllInstancesAfterConvert"), bSelectAllInstancesAfterConvert, GEditorPerProjectIni);
	GConfig->GetInt(TEXT("InstanceTool"), TEXT("MinmumNumConvertedInstances"), MinmumNumConvertedInstances, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Spawn
	GConfig->GetString(TEXT("InstanceTool"), TEXT("SpawnAutoAssignInstanceIdVar"), SpawnAutoAssignInstanceIdVar, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bSpawnActorAlwaysUseIdentityScale"), bSpawnActorAlwaysUseIdentityScale, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bPlaceSpawnedActorsInFolder"), bPlaceSpawnedActorsInFolder, GEditorPerProjectIni);
	GConfig->GetString(TEXT("InstanceTool"), TEXT("SpawnedActorsFolder"), SpawnedActorsFolder, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Options

	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bRebuildInstancesWhenMapOpned"), bRebuildInstancesWhenMapOpned, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bAllowBlueprintEditing"), bAllowBlueprintEditing, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bAlwaysApplyInstanceChangesToBlueprint"), bAlwaysApplyInstanceChangesToBlueprint, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bCanSelectActorInWorldOutliner"), bCanSelectActorInWorldOutliner, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bCanSelectActorInViewport"), bCanSelectActorInViewport, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bDrawFrameWhenActorOrInstanceSelected"), bDrawFrameWhenActorOrInstanceSelected, GEditorPerProjectIni);
	GConfig->GetColor(TEXT("InstanceTool"), TEXT("FrameColorWhenActorSelected"), FrameColorWhenActorSelected, GEditorPerProjectIni);
	GConfig->GetColor(TEXT("InstanceTool"), TEXT("FrameColorWhenInstanceSelected"), FrameColorWhenInstanceSelected, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bDisableUndo"), bDisableUndo, GEditorPerProjectIni);
	GConfig->GetBool(TEXT("InstanceTool"), TEXT("bAutoAlignScaleToGrid"), bAutoAlignScaleToGrid, GEditorPerProjectIni);

	Reset();
}

void UInstanceToolEditorUISetting::Save()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Select

	GConfig->SetInt(TEXT("InstanceTool"), TEXT("SelectAllOption"), (int32)SelectAllOption, GEditorPerProjectIni);
	GConfig->SetFloat(TEXT("InstanceTool"), TEXT("SelectByAxisTolerance"), SelectByAxisTolerance, GEditorPerProjectIni);
	
	GConfig->SetVector(TEXT("InstanceTool"), TEXT("SelectByLocTolerance"), FVector(SelectByLocToleranceX, SelectByLocToleranceY, SelectByLocToleranceZ), GEditorPerProjectIni);
	GConfig->SetVector(TEXT("InstanceTool"), TEXT("SelectByRotTolerance"), FVector(SelectByRotToleranceX, SelectByRotToleranceY, SelectByRotToleranceZ), GEditorPerProjectIni);
	GConfig->SetVector(TEXT("InstanceTool"), TEXT("SelectByScaleTolerance"), FVector(SelectByScaleToleranceX, SelectByScaleToleranceY, SelectByScaleToleranceZ), GEditorPerProjectIni);

	GConfig->SetVector(TEXT("InstanceTool"), TEXT("SelectInvalidTolerance"), FVector(SelectInvalidToleranceLocation, SelectInvalidToleranceRotation, SelectInvalidToleranceScale), GEditorPerProjectIni);
	GConfig->SetInt(TEXT("InstanceTool"), TEXT("MarqueeSelectOption"), (int32)MarqueeSelectOption, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bMarqueeSelectSameActorOnly"), bMarqueeSelectSameActorOnly, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Align

	GConfig->SetInt(TEXT("InstanceTool"), TEXT("AlignCoord"), (int32)AlignCoord, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Distribute

	GConfig->SetInt(TEXT("InstanceTool"), TEXT("DistributeCoord"), (int32)DistributeCoord, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bDistributeIgnoreSameLoc"), bDistributeIgnoreSameLoc, GEditorPerProjectIni);
	GConfig->SetFloat(TEXT("InstanceTool"), TEXT("DistributeIgnoreSameLocTolerance"), DistributeIgnoreSameLocTolerance, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bLineUpAlignRotation"), bLineUpAlignRotation, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Snap

	GConfig->SetInt(TEXT("InstanceTool"), TEXT("SnapCoord"), (int32)SnapCoord, GEditorPerProjectIni);
	GConfig->SetFloat(TEXT("InstanceTool"), TEXT("SnapOffset"), SnapOffset, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bSnapFollowLastSelected"), bSnapFollowLastSelected, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bUseOwnSnapDir"), bUseOwnSnapDir, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bAlwaysSnapToGrid"), bAlwaysSnapToGrid, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bRotateToHitNormal"), bRotateToHitNormal, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("InstanceTool"), TEXT("SnapTraceOption"), (int32)SnapTraceOption, GEditorPerProjectIni);
//	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bAlwaysUseLocalBoundForTracing"), bAlwaysUseLocalBoundForTracing, GEditorPerProjectIni); 
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bSnapIgnoreStartPenetrating"), bSnapIgnoreStartPenetrating, GEditorPerProjectIni);	
	GConfig->SetFloat(TEXT("InstanceTool"), TEXT("SnapTraceDistance"), SnapTraceDistance, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bSnapDrawDebug"), bSnapDrawDebug, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Delta Transform

	GConfig->SetInt(TEXT("InstanceTool"), TEXT("DeltaTransformRotationOrder"), (int32)DeltaTransformRotationOrder, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Convert

	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bPlaceConvertedActorsInFolder"), bPlaceConvertedActorsInFolder, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bGroupConvertedActors"), bGroupConvertedActors, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bSelectConvertedActors"), bSelectConvertedActors, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bDeleteInstancesAfterConvert"), bDeleteInstancesAfterConvert, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bDeleteSourceActorsIfAllInstancesDeleted"), bDeleteSourceActorsIfAllInstancesDeleted, GEditorPerProjectIni);

	GConfig->SetInt(TEXT("InstanceTool"), TEXT("PlaceInstancesInto"), (int32)PlaceInstancesInto, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("InstanceTool"), TEXT("ConvertFromActorLabelOption"), (int32)ConvertFromActorLabelOption, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bConvertFromActorLabelAddSurfix"), bConvertFromActorLabelAddSurfix, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("InstanceTool"), TEXT("ConvertFromActorPivotOption"), (int32)ConvertFromActorPivotOption, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bCreateNewActorByMaterial"), bCreateNewActorByMaterial, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bCreateNewActorForNegativeScaledActors"), bCreateNewActorForNegativeScaledActors, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bForcePositiveInstanceScale"), bForcePositiveInstanceScale, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bTryReuseComponentInExistingActor"), bTryReuseComponentInActor, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bSetOldActorsNotRender"), bSetSourceActorsNotRender, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bPlaceOldActorsInFolder"), bPlaceSourceActorsInFolder, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bGroupSourceActors"), bGroupSourceActors, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bDeleteOldActors"), bDeleteSourceActors, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bSelectAllInstancesAfterConvert"), bSelectAllInstancesAfterConvert, GEditorPerProjectIni);
	GConfig->SetInt(TEXT("InstanceTool"), TEXT("MinmumNumConvertedInstances"), MinmumNumConvertedInstances, GEditorPerProjectIni);

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Spawn
	GConfig->SetString(TEXT("InstanceTool"), TEXT("SpawnAutoAssignInstanceIdVar"), *SpawnAutoAssignInstanceIdVar, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bSpawnActorAlwaysUseIdentityScale"), bSpawnActorAlwaysUseIdentityScale, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bPlaceSpawnedActorsInFolder"), bPlaceSpawnedActorsInFolder, GEditorPerProjectIni);
	GConfig->SetString(TEXT("InstanceTool"), TEXT("SpawnedActorsFolder"), *SpawnedActorsFolder, GEditorPerProjectIni);


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Options

	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bRebuildInstancesWhenMapOpned"), bRebuildInstancesWhenMapOpned, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bAllowBlueprintEditing"), bAllowBlueprintEditing, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bAlwaysApplyInstanceChangesToBlueprint"), bAlwaysApplyInstanceChangesToBlueprint, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bCanSelectActorInWorldOutliner"), bCanSelectActorInWorldOutliner, GEditorPerProjectIni); 
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bCanSelectActorInViewport"), bCanSelectActorInViewport, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bDrawFrameWhenActorOrInstanceSelected"), bDrawFrameWhenActorOrInstanceSelected, GEditorPerProjectIni); 
	GConfig->SetColor(TEXT("InstanceTool"), TEXT("FrameColorWhenActorSelected"), FrameColorWhenActorSelected, GEditorPerProjectIni);
	GConfig->SetColor(TEXT("InstanceTool"), TEXT("FrameColorWhenInstanceSelected"), FrameColorWhenInstanceSelected, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bDisableUndo"), bDisableUndo, GEditorPerProjectIni);
	GConfig->SetBool(TEXT("InstanceTool"), TEXT("bAutoAlignScaleToGrid"), bAutoAlignScaleToGrid, GEditorPerProjectIni);
}

void UInstanceToolEditorUISetting::Reset()
{
	bHideWidget = false;
	bLockSelection = false;
	bMarqueeSelectSubtractMode = false;

	ExistingActor.Reset();
}
