// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorUISetting.generated.h"

UENUM()
enum class ESelectAllOption : uint8
{
	SameActorAsSelected,
	SameComponentAsSelected
};


UENUM()
enum class EMarqueeSelectOption : uint8
{
	Interset,
	Inside
};

UENUM()
enum class EAlignSnapCoord : uint8
{
	Auto,
	Local,
	World
};

UENUM()
enum class ESnapTraceOption : uint8
{
	BoundingBoxTrace,
	SingleLineTrace,
};

UENUM()
enum class EConvertFromActorLabelOption : uint8
{
	FromStaticMesh,
	FromSelectedActor,
	Custom
};

UENUM()
enum class EConvertFromActorOption : uint8
{
	InstancedStaticMeshActor,
	HierarchicalInstancedStaticMeshActor,
};

UENUM()
enum class EConvertFromActorPivotOption : uint8
{
	SelectionCenter,
	LastSelectedActor,
	WorldSpaceZero
};

// UENUM()
// enum class EForceScaleSnapOption : uint8
// {
// 	None,
// 	UseGridSnapValue,
// 	Custom
// };

UENUM()
enum class ERotationOrder : uint8
{
	XYZ,
	XZY,
	YXZ,
	YZX,
	ZXY,
	ZYX
};

UCLASS()
class UInstanceToolEditorUISetting : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Select

	UPROPERTY(Category = "Select", DisplayName = "Select All Option", EditAnywhere, NonTransactional)
	ESelectAllOption SelectAllOption;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByAxisTolerance;

	UPROPERTY(Category = "Select", DisplayName = "X", EditAnywhere, NonTransactional)
	bool bSelectByLocX;

	UPROPERTY(Category = "Select", DisplayName = "Y", EditAnywhere, NonTransactional)
	bool bSelectByLocY;

	UPROPERTY(Category = "Select", DisplayName = "X", EditAnywhere, NonTransactional)
	bool bSelectByLocZ;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByLocToleranceX;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByLocToleranceY;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByLocToleranceZ;

	UPROPERTY(Category = "Select", DisplayName = "X", EditAnywhere, NonTransactional)
	bool bSelectByRotX;

	UPROPERTY(Category = "Select", DisplayName = "Y", EditAnywhere, NonTransactional)
	bool bSelectByRotY;

	UPROPERTY(Category = "Select", DisplayName = "X", EditAnywhere, NonTransactional)
	bool bSelectByRotZ;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByRotToleranceX;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByRotToleranceY;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByRotToleranceZ;

	UPROPERTY(Category = "Select", DisplayName = "X", EditAnywhere, NonTransactional)
	bool bSelectByScaleX;

	UPROPERTY(Category = "Select", DisplayName = "Y", EditAnywhere, NonTransactional)
	bool bSelectByScaleY;

	UPROPERTY(Category = "Select", DisplayName = "X", EditAnywhere, NonTransactional)
	bool bSelectByScaleZ;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByScaleToleranceX;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByScaleToleranceY;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectByScaleToleranceZ;
	

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectInvalidToleranceLocation;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectInvalidToleranceRotation;

	UPROPERTY(Category = "Select", EditAnywhere, NonTransactional)
	float SelectInvalidToleranceScale;

	UPROPERTY(Category = "Select", DisplayName = "Apply to all in level", EditAnywhere, NonTransactional)
	bool bRebuildRenderDataApplyToAllActors;

	UPROPERTY(Category = "Select", DisplayName = "Marquee Select Option", EditAnywhere, NonTransactional)
	EMarqueeSelectOption MarqueeSelectOption;

	UPROPERTY(Category = "Select", DisplayName = "Marquee Select Same Actor Only?", EditAnywhere, NonTransactional)
	bool bMarqueeSelectSameActorOnly;

	UPROPERTY(Category = "Select", DisplayName = "Marquee Select Substract Mode [Tab]", EditAnywhere, NonTransactional)
	bool bMarqueeSelectSubtractMode;

	UPROPERTY(Category = "Select", DisplayName = "Hide Widget [Esc]", EditAnywhere, NonTransactional)
	bool bHideWidget;

	UPROPERTY(Category = "Select", DisplayName = "Lock Selection [Spacebar]", VisibleAnywhere, NonTransactional)
	bool bLockSelection;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Align

	UPROPERTY(Category = "Align", EditAnywhere, NonTransactional)
	EAlignSnapCoord AlignCoord;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Distribute

	UPROPERTY(Category = "Distribute", EditAnywhere, NonTransactional)
	EAlignSnapCoord DistributeCoord;
	
	UPROPERTY(Category = "Distribute", DisplayName = "Ignore Same Locatin?",  EditAnywhere, NonTransactional)
	bool bDistributeIgnoreSameLoc;

	UPROPERTY(Category = "Distribute", DisplayName = "Tolerance", EditAnywhere, NonTransactional)
	float DistributeIgnoreSameLocTolerance;

	UPROPERTY(Category = "Distribute", EditAnywhere, NonTransactional)
	bool bLineUpAlignRotation;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Snap

	UPROPERTY(Category = "Snap", EditAnywhere, NonTransactional)
	EAlignSnapCoord SnapCoord;

	UPROPERTY(Category = "Snap", EditAnywhere, NonTransactional)
	float SnapOffset;

	UPROPERTY(Category = "Snap", DisplayName = "Snap Follw Last Selected Instance", EditAnywhere, NonTransactional)
	bool bSnapFollowLastSelected;

	UPROPERTY(Category = "Snap",EditAnywhere, NonTransactional)
	bool bUseOwnSnapDir;

	UPROPERTY(Category = "Snap", EditAnywhere, NonTransactional)
	bool bAlwaysSnapToGrid;

	UPROPERTY(Category = "Snap", EditAnywhere, NonTransactional)
	bool bRotateToHitNormal;

	UPROPERTY(Category = "Snap", EditAnywhere, NonTransactional)
	ESnapTraceOption SnapTraceOption;

// 	UPROPERTY(Category = "Snap", EditAnywhere, NonTransactional)
// 	bool bAlwaysUseLocalBoundForTracing;

	UPROPERTY(Category = "Snap", EditAnywhere, NonTransactional)
	bool bSnapIgnoreStartPenetrating;

	UPROPERTY(Category = "Snap", EditAnywhere, NonTransactional)
	float SnapTraceDistance;

	UPROPERTY(Category = "Snap", DisplayName = "Draw Trace Debug?", EditAnywhere, NonTransactional)
	bool bSnapDrawDebug;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Delta Transform

	UPROPERTY(Category = "DeltaTransform", DisplayName = "Location", EditAnywhere, NonTransactional, Transient)
	FVector DeltaTransformLoc;

	UPROPERTY(Category = "DeltaTransform", DisplayName = "Rotation", EditAnywhere, NonTransactional, Transient)
	FRotator DeltaTransformRotation;

	UPROPERTY(Category = "DeltaTransform", DisplayName = "Scale", EditAnywhere, NonTransactional, Transient)
	FVector DeltaTransformScale;

	UPROPERTY(Category = "DeltaTransform", DisplayName = "Rotation Order", EditAnywhere, NonTransactional)
	ERotationOrder DeltaTransformRotationOrder;

	UPROPERTY(Category = "DeltaTransform", DisplayName = "Duplicate Instance", EditAnywhere, NonTransactional, Transient)
	bool bDeltaTransformDuplicate;

	UPROPERTY(Category = "DeltaTransform", DisplayName = "Copies", EditAnywhere, NonTransactional, Transient)
	int32 DeltaTransformDuplicateCopies;


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Convert
	UPROPERTY(Category = "ConvertTo", EditAnywhere, NonTransactional)
	bool bPlaceConvertedActorsInFolder;

	UPROPERTY(Category = "ConvertTo", DisplayName = "Group Converted Actors (Slow Operation)", EditAnywhere, NonTransactional)
	bool bGroupConvertedActors;

	UPROPERTY(Category = "ConvertTo", DisplayName = "Select Converted Actors (Very Slow Operation)", EditAnywhere, NonTransactional)
	bool bSelectConvertedActors;

	UPROPERTY(Category = "ConvertTo", EditAnywhere, NonTransactional)
	bool bDeleteInstancesAfterConvert;

	UPROPERTY(Category = "ConvertTo", EditAnywhere, NonTransactional)
	bool bDeleteSourceActorsIfAllInstancesDeleted;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	EConvertFromActorOption PlaceInstancesInto;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	EConvertFromActorLabelOption ConvertFromActorLabelOption;

	UPROPERTY(Category = "ConvertFrom", DisplayName = "Custom Name", EditAnywhere, NonTransactional)
	FString ConvertFromActorCustomLabel;

	UPROPERTY(Category = "ConvertFrom", DisplayName = "Add HISMA/ISMA Surfix", EditAnywhere, NonTransactional)
	bool bConvertFromActorLabelAddSurfix;

	UPROPERTY(Category = "ConvertFrom", DisplayName = "Pivot of Converted Actor ", EditAnywhere, NonTransactional)
	EConvertFromActorPivotOption ConvertFromActorPivotOption;

	UPROPERTY(Category = "ConvertFrom", DisplayName = "Select Target Actor: ", EditAnywhere, NonTransactional)
	TLazyObjectPtr<AActor> ExistingActor;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bCreateNewActorByMaterial;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bCreateNewActorForNegativeScaledActors;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bForcePositiveInstanceScale;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bCheckMaterialUsedWithInstancedStaticMeshesFlag;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bTryReuseComponentInActor;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bSetSourceActorsNotRender;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bPlaceSourceActorsInFolder;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bGroupSourceActors;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bDeleteSourceActors;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	bool bSelectAllInstancesAfterConvert;

	UPROPERTY(Category = "ConvertFrom", EditAnywhere, NonTransactional)
	int32 MinmumNumConvertedInstances;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Spawn
	UPROPERTY(Category = "SpawnBlueprintByInstances", DisplayName = "Blueprint", EditAnywhere, NonTransactional)
	TLazyObjectPtr<UBlueprint> SpawnBlueprint;

	UPROPERTY(Category = "SpawnBlueprintByInstances", DisplayName = "Variable Name", EditAnywhere, NonTransactional)
	FString SpawnAutoAssignInstanceIdVar;

	UPROPERTY(Category = "SpawnBlueprintByInstances", DisplayName = "Offset Transform", EditAnywhere, NonTransactional)
	FTransform SpawnOffsetTransform;

	UPROPERTY(Category = "SpawnBlueprintByInstances", DisplayName = "Always Use Identity Scale", EditAnywhere, NonTransactional)
	bool bSpawnActorAlwaysUseIdentityScale;

	UPROPERTY(Category = "SpawnBlueprintByInstances", EditAnywhere, NonTransactional)
	bool bPlaceSpawnedActorsInFolder;

	UPROPERTY(Category = "SpawnBlueprintByInstances", DisplayName = "Folder", EditAnywhere, NonTransactional)
	FString SpawnedActorsFolder;


	UPROPERTY(Category = "SpawnInstancesOnSpline", DisplayName = "Spline Actor", EditAnywhere, NonTransactional)
	TLazyObjectPtr<AActor> SpawnSplineActor;

	UPROPERTY(Category = "SpawnInstancesOnSpline", DisplayName = "Ignore Rotation", EditAnywhere, NonTransactional)
	bool bSpawnOnSplineIgnoreRotation;

	UPROPERTY(Category = "SpawnInstancesOnSpline", DisplayName = "Select Spawned Instances", EditAnywhere, NonTransactional)
	bool bSelectSpawnedInstances;

	UPROPERTY(Category = "SpawnInstancesOnSpline", DisplayName = "Offset Transform", EditAnywhere, NonTransactional)
	FTransform SpawnOnSplineOffsetTransform;


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Options

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	bool bRebuildInstancesWhenMapOpned;

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	bool bAllowBlueprintEditing;

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	bool bAlwaysApplyInstanceChangesToBlueprint;

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	bool bCanSelectActorInWorldOutliner;

	UPROPERTY(Category = "Options", DisplayName = "Can Select Actor in Viewport and Outliner ", EditAnywhere, NonTransactional)
	bool bCanSelectActorInViewport;

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	bool bDrawFrameWhenActorOrInstanceSelected;

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	FColor FrameColorWhenActorSelected;

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	FColor FrameColorWhenInstanceSelected;

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	bool bDisableUndo;

// 	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
// 	EForceScaleSnapOption ForceScaleAlignOption;
// 
// 	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
// 	float ForceScaleAlignSize;

	UPROPERTY(Category = "Options", EditAnywhere, NonTransactional)
	bool bAutoAlignScaleToGrid;

public:

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void Load();
	void Save();
	void Reset();
};
