// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorCommands.h"
#include "Editor.h"
#include "EdMode.h"
#include "EditorModeTools.h"
#include "Framework/Commands/UICommandList.h"
#include "CanvasTypes.h"

class UWorld;
class ULevel;
class UInstancedStaticMeshComponent;

namespace InstanceToolUtilities
{
	void RunGC();

	void CopySinglePropertyRecursive(const void* const InSourcePtr, void* const InTargetPtr, UObject* const InTargetObject, FProperty* const InProperty);
	
	void CopySingleProperty(const UObject* const InSourceObject, UObject* const InTargetObject, FProperty* const InProperty);

	template<typename T>
	void CopyComponentProperty(T* SourceComponent, T* TargetComponent);
}

DECLARE_DELEGATE(FOnSelectionChanged);

enum class EAlignOption
{
	Location,
	Rotation,
	Scale
};

struct FSelectItem //: public FGCObject
{
	FSelectItem(UInstancedStaticMeshComponent* InComponent, int32 InIndex);
	FSelectItem(const FSelectItem& that) = delete;
	FSelectItem& operator=(FSelectItem const&) = delete;

	~FSelectItem();

/*	void AddReferencedObjects(class FReferenceCollector& Collector) override;*/

	bool operator==(const FSelectItem& Other) const
	{
		return (Other.Component == Component && Other.InstanceIndex == InstanceIndex);
	}

	UInstancedStaticMeshComponent* Component;
	int32 InstanceIndex;

	class UInstanceToolEditorObject* Proxy;
};

struct FSelectionInfo
{
	FSelectionInfo(class FInstanceToolEdMode* InParentMode)
		: ParentMode(InParentMode)
	{}

	bool IsSelected() const { return Selection.Num() > 0; }
	const TArray<class UInstanceToolEditorObject*> GetSelectedProxyObjects() const;
	const TArray<AActor*> GetSelectedActors() const;
	TArray<TWeakObjectPtr<UInstancedStaticMeshComponent>> GetSelectedComponents() const;

	void ValidateSelection();

	class UInstanceToolEditorObject* GetLastSelectedObject() const;
	FTransform GetLastSelectedTransform(bool bWorldSpace = true) const;
	UInstancedStaticMeshComponent* GetLastSelectedComponent() const;
	int32 GetLastSelectedInstanceIndex() const;	

	FVector GetInstanceLocation(UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex, bool bWorldSpace = true) const;
	FVector GetSelectionLocation() const;
	FBox GetSelectionBoundingBox() const;
	void GetInstanceWorldBoundingBox(UInstancedStaticMeshComponent* InComponent, uint32 InIndex, FBox& OutBox) const;
	void GetInstanceLocalBoundingBox(UInstancedStaticMeshComponent* InComponent, uint32 InIndex, FBox& OutBox, FQuat& OutQuat) const;
	FVector GetSnapDir(FTransform& InInstanceTransform, EAxis::Type Axis, bool bNegative, ECoordSystem InCoordSystem) const;

	void SetSelectedNone(bool bNotify = false);
	void SelectInstance(bool InSelected, UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex, bool bMultiSelect, bool bBroadcastChange = false);
	
	void SelectAllInstances(AActor* InActor, bool bMultiSelect = false);
	void SelectAllInstances(UInstancedStaticMeshComponent* InComponent, bool bMultiSelect = false);

	int32 SelectAllInvalidInstances(AActor* InActor, const FVector& SelectInvalidTolerance, bool bMultiSelect = false, bool bNotify = true);
	int32 SelectAllInvalidInstances(UInstancedStaticMeshComponent* InComponent, const FVector& SelectInvalidTolerance, bool bMultiSelect = false, bool bNotify = true);

	void ClearInstanceSelection(AActor* InValidActor) const;
	bool HasInstanceSelected(AActor *InValidActor) const;
	bool HasInstancedComponent(AActor *InActor) const;
	
	void TransformSelectedObject(class UInstanceToolEditorObject* InProxy, FTransform& InNewTransform, bool bWorldSpace = true, bool bMarkRenderStateDirty = true) const;
	void TransformSelectedObjectToLocation(class UInstanceToolEditorObject* InProxy, FVector& NewLocation, bool bWorldSpace = true) const;
	void TransformSelectedObjectToRotation(class UInstanceToolEditorObject* InProxy, FQuat& NewWorldRotation) const;
	void TransformSelectedInstances(const FVector& InDrag, const FRotator& InRot, const FVector& InScale, bool bDuplicate = false);
	void RemoveSelectedInstances();

	void ScaleSnapSelectedObject(class UInstanceToolEditorObject* InProxy, float ForceScaleSnapSize, bool bMarkRenderStateDirty = true) const;
	void ForcePositiveScaleOnSelectedObject(class UInstanceToolEditorObject* InProxy, bool bMarkRenderStateDirty = true) const;

	void BackupSelection();
	void RestoreSelection();

	// Sort by projected position on given direction, from max to min
	const TArray<const FSelectItem*> GetSelectionSortedByPosition(FVector& InNormalizedDir) const;

	TArray<FSelectItem> Selection;

private:
	struct BackupRecord
	{
		BackupRecord(UInstancedStaticMeshComponent* InComponent, int32 InIndex)
			: Component(InComponent)
			, InstanceIndex(InIndex)
		{}
		UInstancedStaticMeshComponent* Component;
		int32 InstanceIndex;
	};
	TArray<BackupRecord> SelectionBackup;
	
	class FInstanceToolEdMode* ParentMode;
};

struct FInstanceToolUtil
{
	static void InvalidateLightingCacheInLevel(UWorld* InWorld);

	static void ForceRebuildRenderDataInLevel(UWorld* InWorld, ULevel* InLevel = NULL);

	static void ForceRebuildRenderData(UInstancedStaticMeshComponent* InComponent);
	
	static void PostEditChangeChainProperty(class UInstancedStaticMeshComponent* InComponent, class FProperty* InProperty);

	static int32 GetActorInstanceCount(AActor* InActor);
};

class FInstanceToolEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_InstanceToolEdModeId;
public:
	FInstanceToolEdMode();
	virtual ~FInstanceToolEdMode();

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	//------------------------------------------------------------------------------
	// FEdMode interface

	virtual void Enter() override;
	virtual void Exit() override;
	//virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	//virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;

	virtual bool Select(AActor* InActor, bool bInSelected) override;
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const override;
	virtual void ActorSelectionChangeNotify() override;

	virtual bool UsesPropertyWidgets() const override;
	virtual FVector GetWidgetLocation() const override;
	virtual bool AllowWidgetMove() override;
	virtual bool ShouldDrawWidget() const override;
	virtual bool UsesTransformWidget() const override;
	virtual EAxisList::Type GetWidgetAxisToDraw(UE::Widget::EWidgetMode InWidgetMode) const override;
	virtual bool GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData) override;
	virtual bool GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData) override;

	virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;
	virtual bool StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport) override;
	virtual bool EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport) override;
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
	virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey, EInputEvent InEvent) override;
	bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click) override;

	virtual EEditAction::Type GetActionEditDelete() override;
	virtual bool ProcessEditDelete() override;

	virtual EEditAction::Type GetActionEditDuplicate() override;
	virtual bool ProcessEditDuplicate() override;

	virtual void ActorsDuplicatedNotify(TArray<AActor*>& PreDuplicateSelection, TArray<AActor*>& PostDuplicateSelection, bool bOffsetLocations) override;

	bool UsesToolkits() const override;

	// End of FEdMode interface
	//------------------------------------------------------------------------------

	const bool HasAnyInstanceSelected() const;
	const int32 GetSelectedInstanceCount() const;
	const TArray<UObject*> GetSelectedObjects() const;
	
	//~ Undoable
	void SelectAllInstances(bool bSelectSameComponentOnly = true, bool bEnableUndo = false);
	void SelectAllInstancesOfActors(TArray<AActor*>& Actors, bool bEnableUndo = false);
	void SetSelectNone(bool bEnableUndo = false);

	int32 SelectAllOverlappedInstances(bool bSelectSameComponentOnly = true);
	void SelectByAxis(EAxis::Type Axis, bool bNegative = false, bool bShowMessage = false);
	int32 SelectByLoc(bool bShowMessage = false);
	int32 SelectByRot(bool bShowMessage = false);
	int32 SelectByScale(bool bShowMessage = false);

	void AlignSelectionToGrid(UE::Widget::EWidgetMode InMode, bool bEnableUndo = true);
	void AlignSelectionLocation(ECoordSystem InCoordSystem, EAxis::Type Axis, bool bNegative = false, bool bEnableUndo = true, bool bNotify = true);
	void AlignSelectionRotation(EAxis::Type Axis, bool bEnableUndo = true, bool bNotify = true);

	void DistributeSelectionLocation(EAxis::Type Axis);
	void LineUpSelectionLocationFirstLast();
	void LineUpSelectionLocation(EAxis::Type Axis);

	void SnapSelection(EAxis::Type Axis, bool bNegative = false);

	void DeltaTransformSelection(FTransform& InDeltaTransform);
	FTransform GetDeltaedTransform(UInstanceToolEditorObject* InProxy, FTransform& InDeltaTransform);
	FTransform GetDeltaedTransform(const FTransform& ComponentWorldTransfrom, const FTransform& InstanceWorldTransfrom, FTransform& InDeltaTransform);

	void SpawnInstnceAtWidgetLocation();
	void SpawnInstncesOnSpline(class USplineComponent* InSplineComponent, FTransform& InOffsetTransform, bool bSpawnOnSplineIgnoreRotation, bool bSelectSpawnedInstances);

	void TransformInstance(UInstanceToolEditorObject* InProxy, bool bNoteModified = false);

	void SelectParentActors();
	void SelectParentComponents();
	void DeleteSelectedInstances();

	void DuplicateSelectedInstances();
	//~

	void PreAddInstanceForUndo();
	void PostAddInstanceForUndo();
	
	//void SelectInstance(UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex, bool bBroadcastChange = true);

	void SetOnSelectionChangedDelegate(FOnSelectionChanged InOnSelectionChangedDelegate);
	void BroadcastSelectionChanged();

	void FocusViewportOnSelection();

	void NoteInstanceDataModified(const TArray<AActor*>& InActors);
	void PropagateChangesToBlueprintInstances(const TArray<AActor*>& InActors, bool bModify = false);

	void ForceRebuildRenderData();
	void InvalidateLightingCache();

	void ApplyChangesToBlueprint();
	void ResetToBlueprintDefaults();

	void NotifyMessage(const FText& Message, float InDuration = 1.0f);

	////////////////////////////////////////////
	// Utils
	//static void DoPostEditChangeChainProperty(class UInstancedStaticMeshComponent* InComponent, class UProperty* InProperty);

	////////////////////////////////////////////
	// For FModeTool_SelectTool
	bool DoBoxSelect(FBox& InBox, bool InSelect = true);
	bool DoFrustumSelect(const FConvexVolume& InFrustum, FEditorViewportClient* InViewportClient, bool InSelect = true);


	////////////////////////////////////////////
	// Tool Modes
	void SetCurrentToolMode(EToolMode InMode);
	EToolMode GetCurrentToolMode() const;

	TObjectPtr<class UInstanceToolEditorUndo> Undo;

	FSelectionInfo Selection;

protected:

	TSharedPtr<FUICommandList> UICommandList;

private:
	
	void OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap);

	bool CanSelectActor(AActor* InActor) const;

	ECoordSystem GetAlignCoordSystem();

	int32 SelectByTransform(UE::Widget::EWidgetMode InMode, bool bShowMessage = false);

	void FocusViewportOnBox(const FBox& BoundingBox) const;	
	bool InstanceBoxTrace(const UWorld* InWorld, const FVector& StartTrace, const FVector& EndTrace, FHitResult& OutHit, const FVector& TraceBoxExtent, const FQuat& InRot, int32 InInstanceIndex) const;
	bool InstanceLineTrace(const UWorld* InWorld, const FVector& StartTrace, const FVector& EndTrace, FHitResult& OutHit, int32 InInstanceIndex) const;
	FVector SnapSelectedObject(class UInstanceToolEditorObject* InProxy, FVector& InNormalizedDir, EAxis::Type Axis, bool bNegative, int32 NumTries=1);
	void AlignSelectedObjectLocation(class UInstanceToolEditorObject* InTargetProxy, class UInstanceToolEditorObject* InSourceProxy, EAxis::Type Axis, bool bWorldSpace);
	void AlignSelectedObjectRotation(class UInstanceToolEditorObject* InTargetProxy, class UInstanceToolEditorObject* InSourceProxy, EAxis::Type Axis, bool bWorldSpace);
	void DistributeSelectedObjectLocation(EAxis::Type Axis, bool bDistributeIgnoreSameLoc = false);

	void TryApplyChangesToBlueprint(const TArray<AActor*>& InActors);

	void BindCommands();
	
	FOnSelectionChanged OnSelectionChangedDelegate;

	bool bBrushTraceValid;
	FVector BrushLocation;
	FVector BrushTraceDirection;
	TObjectPtr<UStaticMeshComponent> SphereBrushComponent;
	
	bool bRightClick;
	bool bShiftDown;
	bool bCanAltDrag;
	bool bModifiedInTransaction;
	bool bSelectionChangedInTransaction;
	int32 CurrentTransactionID;

	bool bUseCachedWidgetLocation;
	static FVector CachedWidgetLocation;

	EToolMode CurrentToolMode;
};

class FModeTool_SelectTool : public FModeTool
{
public:
	FModeTool_SelectTool();

	//------------------------------------------------------------------------------
	// FEdMode interface
	/*     virtual void SelectNone() override;*/
	virtual bool BoxSelect(FBox& InBox, bool InSelect = true) override;
	virtual bool FrustumSelect(const FConvexVolume& InFrustum, FEditorViewportClient* InViewportClient, bool InSelect = true) override;
	// End of FEdMode interface
	//------------------------------------------------------------------------------
};
