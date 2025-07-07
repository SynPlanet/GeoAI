// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolEdMode.h"
#include "InstanceToolEdModeToolkit.h"
#include "InstanceToolEditorObject.h"
#include "InstanceToolEditorUISetting.h"
#include "InstanceToolEditorUndo.h"
#include "InstanceToolModule.h"

#include "Toolkits/ToolkitManager.h"
#include "SnappingUtils.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "InstancedFoliageActor.h"
#include "ScopedTransaction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "DrawDebugHelpers.h"
#include "Framework/Commands/GenericCommands.h"
#include "Engine/CollisionProfile.h"
#include "EngineUtils.h"
#include "LevelEditorViewport.h"
#include "Materials/Material.h"
#include "EditorModeManager.h"
#include "Engine/Selection.h"
#include "Components/SplineComponent.h"
#include "CanvasItem.h"
#include "Editor.h"
#include "UnrealWidget.h"

#include "Engine/SimpleConstructionScript.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/InheritableComponentHandler.h"
#include "K2Node_AddComponent.h"
#include "Algo/Find.h"

#define LOCTEXT_NAMESPACE "InstanceToolEdMode"

const FEditorModeID FInstanceToolEdMode::EM_InstanceToolEdModeId = TEXT("EM_InstanceToolEdMode");

namespace InstanceToolUtilities
{
	void RunGC()
	{
		::CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}

	// Copy from EditorUtilities
	void CopySinglePropertyRecursive(const void* const InSourcePtr, void* const InTargetPtr, UObject* const InTargetObject, FProperty* const InProperty)
	{
		// Properties that are *object* properties are tricky
		// Sometimes the object will be a reference to a PIE-world object, and copying that reference back to an actor CDO asset is not a good idea
		// If the property is referencing an actor or actor component in the PIE world, then we can try and fix that reference up to the equivalent
		// from the editor world; otherwise we have to skip it
		bool bNeedsGenericCopy = true;
		if (FObjectPropertyBase* const ObjectProperty = CastField<FObjectPropertyBase>(InProperty))
		{
			const int32 PropertyArrayDim = InProperty->ArrayDim;
			for (int32 ArrayIndex = 0; ArrayIndex < PropertyArrayDim; ArrayIndex++)
			{
				UObject* const SourceObjectPropertyValue = ObjectProperty->GetObjectPropertyValue_InContainer(InSourcePtr, ArrayIndex);
				if (SourceObjectPropertyValue && SourceObjectPropertyValue->GetOutermost()->HasAnyPackageFlags(PKG_PlayInEditor))
				{
					// Not all the code paths below actually copy the object, but even if they don't we need to claim that they
					// did, as copying a reference to an object in a PIE world leads to crashes
					bNeedsGenericCopy = false;
#if 0
					// REFERENCE an existing actor in the editor world from a REFERENCE in the PIE world
					if (SourceObjectPropertyValue->IsA(AActor::StaticClass()))
					{
						// We can try and fix-up an actor reference from the PIE world to instead be the version from the persistent world
						AActor* const EditorWorldActor = GetEditorWorldCounterpartActor(Cast<AActor>(SourceObjectPropertyValue));
						if (EditorWorldActor)
						{
							ObjectProperty->SetObjectPropertyValue_InContainer(InTargetPtr, EditorWorldActor, ArrayIndex);
						}
					}
					// REFERENCE an existing actor component in the editor world from a REFERENCE in the PIE world
					else if (SourceObjectPropertyValue->IsA(UActorComponent::StaticClass()) && InTargetObject->IsA(AActor::StaticClass()))
					{
						AActor* const TargetActor = Cast<AActor>(InTargetObject);
						TInlineComponentArray<UActorComponent*> TargetComponents;
						TargetActor->GetComponents(TargetComponents);

						// We can try and fix-up an actor component reference from the PIE world to instead be the version from the persistent world
						int32 TargetComponentIndex = 0;
						UActorComponent* const EditorWorldComponent = FindMatchingComponentInstance(Cast<UActorComponent>(SourceObjectPropertyValue), TargetActor, TargetComponents, TargetComponentIndex);
						if (EditorWorldComponent)
						{
							ObjectProperty->SetObjectPropertyValue_InContainer(InTargetPtr, EditorWorldComponent, ArrayIndex);
						}
					}
#endif
				}
			}
		}
		else if (FStructProperty* const StructProperty = CastField<FStructProperty>(InProperty))
		{
			// Ensure that the target struct is initialized before copying fields from the source.
			StructProperty->InitializeValue_InContainer(InTargetPtr);

			const int32 PropertyArrayDim = InProperty->ArrayDim;
			for (int32 ArrayIndex = 0; ArrayIndex < PropertyArrayDim; ArrayIndex++)
			{
				const void* const SourcePtr = StructProperty->ContainerPtrToValuePtr<void>(InSourcePtr, ArrayIndex);
				void* const TargetPtr = StructProperty->ContainerPtrToValuePtr<void>(InTargetPtr, ArrayIndex);

				for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
				{
					FProperty* const InnerProperty = *It;
					CopySinglePropertyRecursive(SourcePtr, TargetPtr, InTargetObject, InnerProperty);
				}
			}

			bNeedsGenericCopy = false;
		}
		else if (FArrayProperty* const ArrayProperty = CastField<FArrayProperty>(InProperty))
		{
			check(InProperty->ArrayDim == 1);
			FScriptArrayHelper SourceArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(InSourcePtr));
			FScriptArrayHelper TargetArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(InTargetPtr));

			FProperty* InnerProperty = ArrayProperty->Inner;
			int32 Num = SourceArrayHelper.Num();

			// here we emulate UArrayProperty::CopyValuesInternal()
			if (!(InnerProperty->PropertyFlags & CPF_IsPlainOldData))
			{
				TargetArrayHelper.EmptyAndAddValues(Num);
			}
			else
			{
				TargetArrayHelper.EmptyAndAddUninitializedValues(Num);
			}

			for (int32 Index = 0; Index < Num; Index++)
			{
				CopySinglePropertyRecursive(SourceArrayHelper.GetRawPtr(Index), TargetArrayHelper.GetRawPtr(Index), InTargetObject, InnerProperty);
			}

			bNeedsGenericCopy = false;
		}

		// Handle copying properties that either aren't an object, or aren't part of the PIE world
		if (bNeedsGenericCopy)
		{
			InProperty->CopyCompleteValue_InContainer(InTargetPtr, InSourcePtr);
		}
	}

	void CopySingleProperty(const UObject* const InSourceObject, UObject* const InTargetObject, FProperty* const InProperty)
	{
		CopySinglePropertyRecursive(InSourceObject, InTargetObject, InTargetObject, InProperty);
	}

	template<typename T>
	void CopyComponentProperty(T* SourceComponent, T* TargetComponent)
	{
		if (SourceComponent == nullptr || TargetComponent == nullptr)
		{
			return;
		}

		if (!SourceComponent->GetClass()->template IsChildOf<T>())
		{
			return;
		}

		UClass* ComponentClass = T::StaticClass();

		// Copy component properties
		for (FProperty* Property = ComponentClass->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			const bool bIsTransient = !!(Property->PropertyFlags & CPF_Transient);
			const bool bIsIdentical = Property->Identical_InContainer(SourceComponent, TargetComponent);
			const bool bIsComponent = !!(Property->PropertyFlags & (CPF_InstancedReference | CPF_ContainsInstancedReference));
			const bool bIsTransform =
				Property->GetFName() == USceneComponent::GetRelativeScale3DPropertyName() ||
				Property->GetFName() == USceneComponent::GetRelativeLocationPropertyName() ||
				Property->GetFName() == USceneComponent::GetRelativeRotationPropertyName();

			if (!bIsTransient && !bIsIdentical && !bIsComponent && !bIsTransform)
			{
				const bool bIsSafeToCopy = Property->HasAnyPropertyFlags(CPF_Edit | CPF_Interp);
				if (bIsSafeToCopy)
				{
					CopySingleProperty(SourceComponent, TargetComponent, Property);
				}
			}
		}
	}

	template void CopyComponentProperty<UStaticMeshComponent>(UStaticMeshComponent*, UStaticMeshComponent*);

	/** Options for CopyActorProperties */
	enum ECopyOptions
	{
		/** Default copy options */
		Default = 0,

		/** Set this option to preview the changes and not actually copy anything.  This will count the number of properties that would be copied. */
		PreviewOnly = 1 << 0,

		/** Call PostEditChangeProperty for each modified property */
		CallPostEditChangeProperty = 1 << 1,

		/** Call PostEditMove if we detect that a transform property was changed */
		CallPostEditMove = 1 << 2,

		/** Copy only Edit and Interp properties.  Otherwise we copy all properties by default */
		OnlyCopyEditOrInterpProperties = 1 << 3,

		/** Propagate property changes to archetype instances if the target actor is a CDO */
		PropagateChangesToArchetypeInstances = 1 << 4,

		/** Filters out Blueprint Read-only properties */
		FilterBlueprintReadOnly = 1 << 5,

		/** Filters out properties that are marked instance only. */
		SkipInstanceOnlyProperties = 1 << 6,
	};

	/** Copy options structure for CopyActorProperties */
	struct FCopyOptions
	{
		/** Implicit construction for an options enumeration */
		FCopyOptions(const ECopyOptions InFlags) : Flags(InFlags) {}

		/** Check whether we can copy the specified property */
		bool CanCopyProperty(FProperty& Property, UObject& Object) const
		{
			return !PropertyFilter || PropertyFilter(Property, Object);
		}

		/** User-specified flags for the copy */
		ECopyOptions Flags;

		/** User-specified custom property filter predicate */
		TFunction<bool(FProperty&, UObject&)> PropertyFilter;
	};

	template<class AllocatorType = FDefaultAllocator>
	UActorComponent* FindMatchingComponentInstance(UActorComponent* SourceComponent, AActor* TargetActor, const TArray<UActorComponent*, AllocatorType>& TargetComponents, int32& StartIndex)
	{
		UActorComponent* TargetComponent = StartIndex < TargetComponents.Num() ? TargetComponents[StartIndex] : nullptr;

		// If the source and target components do not match (e.g. context-specific), attempt to find a match in the target's array elsewhere
		const int32 NumTargetComponents = TargetComponents.Num();
		if ((SourceComponent != nullptr)
			&& ((TargetComponent == nullptr)
				|| (SourceComponent->GetFName() != TargetComponent->GetFName())))
		{
			const bool bSourceIsArchetype = SourceComponent->HasAnyFlags(RF_ArchetypeObject);
			// Reset the target component since it doesn't match the source
			TargetComponent = nullptr;

			if (NumTargetComponents > 0)
			{
				// Attempt to locate a match elsewhere in the target's component list
				const int32 StartingIndex = (bSourceIsArchetype ? StartIndex : StartIndex + 1);
				int32 FindTargetComponentIndex = (StartingIndex >= NumTargetComponents) ? 0 : StartingIndex;
				do
				{
					UActorComponent* FindTargetComponent = TargetComponents[FindTargetComponentIndex];

					if (FindTargetComponent->GetClass() == SourceComponent->GetClass())
					{
						// In the case that the SourceComponent is an Archetype there is a better than even chance the name won't match due to the way the SCS
						// is set up, so we're actually going to reverse search the archetype chain
						if (bSourceIsArchetype)
						{
							UActorComponent* CheckComponent = FindTargetComponent;
							while (CheckComponent)
							{
								if (SourceComponent == CheckComponent->GetArchetype())
								{
									TargetComponent = FindTargetComponent;
									StartIndex = FindTargetComponentIndex;
									break;
								}
								CheckComponent = Cast<UActorComponent>(CheckComponent->GetArchetype());
							}
							if (TargetComponent)
							{
								break;
							}
						}
						else
						{
							// If we found a match, update the target component and adjust the target index to the matching position
							if (FindTargetComponent != NULL && SourceComponent->GetFName() == FindTargetComponent->GetFName())
							{
								TargetComponent = FindTargetComponent;
								StartIndex = FindTargetComponentIndex;
								break;
							}
						}
					}

					// Increment the index counter, and loop back to 0 if necessary
					if (++FindTargetComponentIndex >= NumTargetComponents)
					{
						FindTargetComponentIndex = 0;
					}

				} while (FindTargetComponentIndex != StartIndex);
			}

			// If we still haven't found a match and we're targeting a class default object what we're really looking
			// for is an Archetype
			if (TargetComponent == nullptr && TargetActor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
			{
				if (bSourceIsArchetype)
				{
					UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(SourceComponent->GetOuter());

					// If the target actor's class is a child of our owner and we're both archetypes, then we're actually looking for an overridden version of ourselves
					if (BPGC && TargetActor->GetClass()->IsChildOf(BPGC))
					{
						TargetComponent = Cast<UActorComponent>(TargetActor->GetClass()->FindArchetype(SourceComponent->GetClass(), SourceComponent->GetFName()));

						// If it is us, then we're done, we don't need to find this
						if (TargetComponent == SourceComponent)
						{
							TargetComponent = nullptr;
						}
					}
				}
				else
				{
					TargetComponent = CastChecked<UActorComponent>(SourceComponent->GetArchetype(), ECastCheckedType::NullAllowed);

					// If the returned target component is not from the direct class of the actor we're targeting, we need to insert an inheritable component
					if (TargetComponent && (TargetComponent->GetOuter() != TargetActor->GetClass()))
					{
						// This component doesn't exist in the hierarchy anywhere and we're not going to modify the CDO, so we'll drop it
						if (TargetComponent->HasAnyFlags(RF_ClassDefaultObject))
						{
							TargetComponent = nullptr;
						}
						else
						{
							UBlueprintGeneratedClass* BPGC = CastChecked<UBlueprintGeneratedClass>(TargetActor->GetClass());
							UBlueprint* Blueprint = CastChecked<UBlueprint>(BPGC->ClassGeneratedBy);
							UInheritableComponentHandler* InheritableComponentHandler = Blueprint->GetInheritableComponentHandler(true);
							if (InheritableComponentHandler)
							{
								FComponentKey Key;
								FName const SourceComponentName = SourceComponent->GetFName();

								BPGC = Cast<UBlueprintGeneratedClass>(BPGC->GetSuperClass());
								while (!Key.IsValid() && BPGC)
								{
									USCS_Node* SCSNode = BPGC->SimpleConstructionScript->FindSCSNode(SourceComponentName);
									if (!SCSNode)
									{
										UBlueprint* SuperBlueprint = CastChecked<UBlueprint>(BPGC->ClassGeneratedBy);
										for (UActorComponent* ComponentTemplate : BPGC->ComponentTemplates)
										{
											if (ComponentTemplate->GetFName() == SourceComponentName)
											{
												if (UEdGraph* UCSGraph = FBlueprintEditorUtils::FindUserConstructionScript(SuperBlueprint))
												{
													TArray<UK2Node_AddComponent*> ComponentNodes;
													UCSGraph->GetNodesOfClass<UK2Node_AddComponent>(ComponentNodes);

													for (UK2Node_AddComponent* UCSNode : ComponentNodes)
													{
														if (ComponentTemplate == UCSNode->GetTemplateFromNode())
														{
															Key = FComponentKey(SuperBlueprint, FUCSComponentId(UCSNode));
															break;
														}
													}
												}
												break;
											}
										}
									}
									else
									{
										Key = FComponentKey(SCSNode);
										break;
									}
									BPGC = Cast<UBlueprintGeneratedClass>(BPGC->GetSuperClass());
								}

								if (ensure(Key.IsValid()))
								{
									check(InheritableComponentHandler->GetOverridenComponentTemplate(Key) == nullptr);
									TargetComponent = InheritableComponentHandler->CreateOverridenComponentTemplate(Key);
								}
								else
								{
									TargetComponent = nullptr;
								}
							}
						}
					}
				}
			}
		}

		return TargetComponent;
	}

	int32 CopyActorProperties(AActor* SourceActor, AActor* TargetActor, const FCopyOptions& Options, UActorComponent* CopyComponent = NULL,  FProperty* CopyProperty = NULL)
	{
		check(SourceActor != nullptr && TargetActor != nullptr);

		const bool bIsPreviewing = (Options.Flags & ECopyOptions::PreviewOnly) != 0;

		int32 CopiedPropertyCount = 0;

		// The actor's classes should be compatible, right?
		UClass* ActorClass = SourceActor->GetClass();
		check(TargetActor->GetClass()->IsChildOf(ActorClass));

		// Get archetype instances for propagation (if requested)
		TArray<AActor*> ArchetypeInstances;
		if (Options.Flags & ECopyOptions::PropagateChangesToArchetypeInstances)
		{
			TArray<UObject*> ObjectArchetypeInstances;
			TargetActor->GetArchetypeInstances(ObjectArchetypeInstances);

			for (UObject* ObjectArchetype : ObjectArchetypeInstances)
			{
				if (AActor* ActorArchetype = Cast<AActor>(ObjectArchetype))
				{
					ArchetypeInstances.Add(ActorArchetype);
				}
			}
		}

		bool bTransformChanged = false;

		// Copy non-component properties from the old actor to the new actor
		// @todo sequencer: Most of this block of code was borrowed (pasted) from UEditorEngine::ConvertActors().  If we end up being able to share these code bodies, that would be nice!
		TSet<UObject*> ModifiedObjects;
		for (FProperty* Property = ActorClass->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			if (CopyComponent != NULL)
			{
				continue;
			}
			if (CopyProperty != NULL && CopyProperty != Property)
			{
				continue;
			}
			const bool bIsTransient = !!(Property->PropertyFlags & CPF_Transient);
			const bool bIsComponentContainer = !!(Property->PropertyFlags & CPF_ContainsInstancedReference);
			const bool bIsComponentProp = !!(Property->PropertyFlags & (CPF_InstancedReference | CPF_ContainsInstancedReference));
			const bool bIsBlueprintReadonly = !!(Options.Flags & ECopyOptions::FilterBlueprintReadOnly) && !!(Property->PropertyFlags & CPF_BlueprintReadOnly);
			const bool bIsIdentical = Property->Identical_InContainer(SourceActor, TargetActor);

			if (!bIsTransient && !bIsIdentical && !bIsComponentContainer && !bIsComponentProp && !bIsBlueprintReadonly)
			{
				const bool bIsSafeToCopy = (!(Options.Flags & ECopyOptions::OnlyCopyEditOrInterpProperties) || (Property->HasAnyPropertyFlags(CPF_Edit | CPF_Interp)))
					&& (!(Options.Flags & ECopyOptions::SkipInstanceOnlyProperties) || (!Property->HasAllPropertyFlags(CPF_DisableEditOnTemplate)));

				if (bIsSafeToCopy)
				{
					if (!Options.CanCopyProperty(*Property, *SourceActor))
					{
						continue;
					}

					if (!bIsPreviewing)
					{
						if (!ModifiedObjects.Contains(TargetActor))
						{
							// Start modifying the target object
							TargetActor->Modify();
							ModifiedObjects.Add(TargetActor);
						}

						if (Options.Flags & ECopyOptions::CallPostEditChangeProperty)
						{
							TargetActor->PreEditChange(Property);
						}

						// Determine which archetype instances match the current property value of the target actor (before it gets changed). We only want to propagate the change to those instances.
						TArray<UObject*> ArchetypeInstancesToChange;
						if (Options.Flags & ECopyOptions::PropagateChangesToArchetypeInstances)
						{
							for (AActor* ArchetypeInstance : ArchetypeInstances)
							{
								if (ArchetypeInstance != nullptr && Property->Identical_InContainer(ArchetypeInstance, TargetActor))
								{
									ArchetypeInstancesToChange.Add(ArchetypeInstance);
								}
							}
						}

						CopySingleProperty(SourceActor, TargetActor, Property);

						if (Options.Flags & ECopyOptions::CallPostEditChangeProperty)
						{
							FPropertyChangedEvent PropertyChangedEvent(Property);
							TargetActor->PostEditChangeProperty(PropertyChangedEvent);
						}

						if (Options.Flags & ECopyOptions::PropagateChangesToArchetypeInstances)
						{
							for (int32 InstanceIndex = 0; InstanceIndex < ArchetypeInstancesToChange.Num(); ++InstanceIndex)
							{
								UObject* ArchetypeInstance = ArchetypeInstancesToChange[InstanceIndex];
								if (ArchetypeInstance != nullptr)
								{
									if (!ModifiedObjects.Contains(ArchetypeInstance))
									{
										ArchetypeInstance->Modify();
										ModifiedObjects.Add(ArchetypeInstance);
									}

									CopySingleProperty(TargetActor, ArchetypeInstance, Property);
								}
							}
						}
					}

					++CopiedPropertyCount;
				}
			}
		}

		// Copy component properties from source to target if they match. Note that the component lists may not be 1-1 due to context-specific components (e.g. editor-only sprites, etc.).
		TInlineComponentArray<UActorComponent*> SourceComponents;
		TInlineComponentArray<UActorComponent*> TargetComponents;

		SourceActor->GetComponents(SourceComponents);
		TargetActor->GetComponents(TargetComponents);

		int32 TargetComponentIndex = 0;
		for (UActorComponent* SourceComponent : SourceComponents)
		{
			if (CopyComponent != NULL && SourceComponent != CopyComponent)
			{
				continue;
			}
			if (SourceComponent->CreationMethod == EComponentCreationMethod::UserConstructionScript)
			{
				continue;
			}
			UActorComponent* TargetComponent = FindMatchingComponentInstance(SourceComponent, TargetActor, TargetComponents, TargetComponentIndex);

			if (TargetComponent != nullptr)
			{
				UClass* ComponentClass = SourceComponent->GetClass();
				check(ComponentClass == TargetComponent->GetClass());

				// Build a list of matching component archetype instances for propagation (if requested)
				TArray<UActorComponent*> ComponentArchetypeInstances;
				if (Options.Flags & ECopyOptions::PropagateChangesToArchetypeInstances)
				{
					for (AActor* ArchetypeInstance : ArchetypeInstances)
					{
						if (ArchetypeInstance != nullptr)
						{
							UActorComponent* ComponentArchetypeInstance = EditorUtilities::FindMatchingComponentInstance(TargetComponent, ArchetypeInstance);
							if (ComponentArchetypeInstance != nullptr)
							{
								ComponentArchetypeInstances.AddUnique(ComponentArchetypeInstance);
							}
						}
					}
				}

				TSet<const FProperty*> SourceUCSModifiedProperties;
				SourceComponent->GetUCSModifiedProperties(SourceUCSModifiedProperties);

				TArray<UActorComponent*> ComponentInstancesToReregister;

				// Copy component properties
				for (FProperty* Property = ComponentClass->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
				{
					if (CopyProperty != NULL && CopyProperty != Property)
					{
						continue;
					}
					const bool bIsTransient = !!(Property->PropertyFlags & CPF_Transient);
					const bool bIsIdentical = Property->Identical_InContainer(SourceComponent, TargetComponent);
					const bool bIsComponent = !!(Property->PropertyFlags & (CPF_InstancedReference | CPF_ContainsInstancedReference));
					const bool bIsTransform =
						Property->GetFName() == USceneComponent::GetRelativeScale3DPropertyName() ||
						Property->GetFName() == USceneComponent::GetRelativeLocationPropertyName() ||
						Property->GetFName() == USceneComponent::GetRelativeRotationPropertyName();;

					if (!bIsTransient && !bIsIdentical && !bIsComponent && !SourceUCSModifiedProperties.Contains(Property)
						&& (!bIsTransform || SourceComponent != SourceActor->GetRootComponent() || (!SourceActor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) && !TargetActor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))))
					{
						const bool bIsSafeToCopy = !(Options.Flags & ECopyOptions::OnlyCopyEditOrInterpProperties) || (Property->HasAnyPropertyFlags(CPF_Edit | CPF_Interp));
						if (bIsSafeToCopy)
						{
							if (!Options.CanCopyProperty(*Property, *SourceActor))
							{
								continue;
							}

							if (!bIsPreviewing)
							{
								if (!ModifiedObjects.Contains(TargetComponent))
								{
									TargetComponent->SetFlags(RF_Transactional);
									TargetComponent->Modify();
									ModifiedObjects.Add(TargetComponent);
								}

								if (Options.Flags & ECopyOptions::CallPostEditChangeProperty)
								{
									// @todo simulate: Should we be calling this on the component instead?
									TargetActor->PreEditChange(Property);
								}

								// Determine which component archetype instances match the current property value of the target component (before it gets changed). We only want to propagate the change to those instances.
								TArray<UActorComponent*> ComponentArchetypeInstancesToChange;
								if (Options.Flags & ECopyOptions::PropagateChangesToArchetypeInstances)
								{
									for (UActorComponent* ComponentArchetypeInstance : ComponentArchetypeInstances)
									{
										if (ComponentArchetypeInstance != nullptr && Property->Identical_InContainer(ComponentArchetypeInstance, TargetComponent))
										{
											bool bAdd = true;
											// We also need to double check that either the direct archetype of the target is also identical
											if (ComponentArchetypeInstance->GetArchetype() != TargetComponent)
											{
												UActorComponent* CheckComponent = CastChecked<UActorComponent>(ComponentArchetypeInstance->GetArchetype());
												while (CheckComponent != ComponentArchetypeInstance)
												{
													if (!Property->Identical_InContainer(CheckComponent, TargetComponent))
													{
														bAdd = false;
														break;
													}
													CheckComponent = CastChecked<UActorComponent>(CheckComponent->GetArchetype());
												}
											}

											if (bAdd)
											{
												ComponentArchetypeInstancesToChange.Add(ComponentArchetypeInstance);
											}
										}
									}
								}

								CopySingleProperty(SourceComponent, TargetComponent, Property);

								if (Options.Flags & ECopyOptions::CallPostEditChangeProperty)
								{
									FPropertyChangedEvent PropertyChangedEvent(Property);
									TargetActor->PostEditChangeProperty(PropertyChangedEvent);
								}

								if (Options.Flags & ECopyOptions::PropagateChangesToArchetypeInstances)
								{
									for (int32 InstanceIndex = 0; InstanceIndex < ComponentArchetypeInstancesToChange.Num(); ++InstanceIndex)
									{
										UActorComponent* ComponentArchetypeInstance = ComponentArchetypeInstancesToChange[InstanceIndex];
										if (ComponentArchetypeInstance != nullptr)
										{
											if (!ModifiedObjects.Contains(ComponentArchetypeInstance))
											{
												// Ensure that this instance will be included in any undo/redo operations, and record it into the transaction buffer.
												// Note: We don't do this for components that originate from script, because they will be re-instanced from the template after an undo, so there is no need to record them.
												if (!ComponentArchetypeInstance->IsCreatedByConstructionScript())
												{
													ComponentArchetypeInstance->SetFlags(RF_Transactional);
													ComponentArchetypeInstance->Modify();
													ModifiedObjects.Add(ComponentArchetypeInstance);
												}

												// We must also modify the owner, because we'll need script components to be reconstructed as part of an undo operation.
												AActor* Owner = ComponentArchetypeInstance->GetOwner();
												if (Owner != nullptr && !ModifiedObjects.Contains(Owner))
												{
													Owner->Modify();
													ModifiedObjects.Add(Owner);
												}
											}

											if (ComponentArchetypeInstance->IsRegistered())
											{
												ComponentArchetypeInstance->UnregisterComponent();
												ComponentInstancesToReregister.Add(ComponentArchetypeInstance);
											}

											CopySingleProperty(TargetComponent, ComponentArchetypeInstance, Property);
										}
									}
								}
							}

							++CopiedPropertyCount;

							if (bIsTransform)
							{
								bTransformChanged = true;
							}
						}
					}
				}

				for (UActorComponent* ModifiedComponentInstance : ComponentInstancesToReregister)
				{
					ModifiedComponentInstance->RegisterComponent();
				}
			}
		}

		if (!bIsPreviewing && CopiedPropertyCount > 0 && TargetActor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) && TargetActor->GetClass()->HasAllClassFlags(CLASS_CompiledFromBlueprint))
		{
			FBlueprintEditorUtils::PostEditChangeBlueprintActors(CastChecked<UBlueprint>(TargetActor->GetClass()->ClassGeneratedBy));
		}

		// If one of the changed properties was part of the actor's transformation, then we'll call PostEditMove too.
		if (!bIsPreviewing && bTransformChanged)
		{
			if (Options.Flags & ECopyOptions::CallPostEditMove)
			{
				const bool bFinishedMove = true;
				TargetActor->PostEditMove(bFinishedMove);
			}
		}

		return CopiedPropertyCount;
	}
}

////////////////////////////////////////////////////////////////////////////////////
// FInstanceToolEdMode

FVector FInstanceToolEdMode::CachedWidgetLocation = FVector::ZeroVector;

FInstanceToolEdMode::FInstanceToolEdMode()
	: FEdMode()

	, Selection(this)

	, bRightClick(false)
	, bShiftDown(false)
	, bCanAltDrag(false)
	, bModifiedInTransaction(false)
	, bSelectionChangedInTransaction(false)
	, CurrentTransactionID(INDEX_NONE)

	, CurrentToolMode(EToolMode::Select)
{
	Tools.Add(new FModeTool_SelectTool);
	SetCurrentTool(MT_None);

	UMaterial* BrushMaterial = nullptr;
	UStaticMesh* StaticMesh = nullptr;
	if (!IsRunningCommandlet())
	{
		BrushMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EditorLandscapeResources/FoliageBrushSphereMaterial.FoliageBrushSphereMaterial"), nullptr, LOAD_None, nullptr);
		StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/EngineMeshes/Sphere.Sphere"), nullptr, LOAD_None, nullptr);
	}

	SphereBrushComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), TEXT("SphereBrushComponent"));
	SphereBrushComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SphereBrushComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereBrushComponent->SetStaticMesh(StaticMesh);
	SphereBrushComponent->CastShadow = false;
	SphereBrushComponent->SetAbsolute(true, true, true);
	SphereBrushComponent->OverrideMaterials.Add(BrushMaterial);

	bBrushTraceValid = false;
	BrushLocation = FVector::ZeroVector;

	UICommandList = MakeShareable(new FUICommandList);

	Undo = NewObject<UInstanceToolEditorUndo>(GetTransientPackage(), TEXT("InstanceToolEditorUndo"), RF_Transactional);
	Undo->SetParent(this);
}

FInstanceToolEdMode::~FInstanceToolEdMode()
{
}

void FInstanceToolEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdMode::AddReferencedObjects(Collector);

	Collector.AddReferencedObject(Undo);

	Collector.AddReferencedObject(SphereBrushComponent);
}

void FInstanceToolEdMode::Enter()
{
	FEdMode::Enter();

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();
	UISetting.Load();

	Undo->Reset();

	FCoreUObjectDelegates::OnObjectsReplaced.AddRaw(this, &FInstanceToolEdMode::OnObjectsReplaced);

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FInstanceToolEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	BindCommands();

	if (GEditor->GetSelectedActorCount() > 0 
		&& (!UISetting.bCanSelectActorInWorldOutliner && !UISetting.bCanSelectActorInViewport))
	{
		TArray<AActor*> Selected;
		GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(Selected);
		SelectAllInstancesOfActors(Selected);
	}

	if (HasAnyInstanceSelected())
	{
		GEditor->SelectNone(/*bNoteSelectionChange=*/ true, /*bDeselectBSPSurfs=*/ true);
	}

	bUseCachedWidgetLocation = false;
}

void FInstanceToolEdMode::Exit()
{
	FCoreUObjectDelegates::OnObjectsReplaced.RemoveAll(this);

	//SetSelectNone();
	Selection.SetSelectedNone();

	bUseCachedWidgetLocation = false;
	
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();
	UISetting.Save();
	UISetting.Reset();

	Undo->Reset();

	//InstanceToolUtilities::RunGC(); // Causing crash on 4.26+ if a ScriptableMode been garbage collected here before been pinned

	FEdMode::Exit();
}

void FInstanceToolEdMode::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);

	if (!GCurrentLevelEditingViewportClient || GCurrentLevelEditingViewportClient != ViewportClient)
	{
		return;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	bool bDrawFrame = UISetting.bDrawFrameWhenActorOrInstanceSelected 
		&& (GEditor->GetSelectedActorCount() > 0 || HasAnyInstanceSelected());

	if (bDrawFrame)
	{
		FLinearColor TineLineColor = GEditor->GetSelectedActorCount() > 0
			? UISetting.FrameColorWhenActorSelected
			: UISetting.FrameColorWhenInstanceSelected;

		const float TintLineSize = 4.f;
		FVector2D TintBox[4];
		FVector2D BoxSize(Viewport->GetSizeXY().X - 4.f, Viewport->GetSizeXY().Y - 4.f);
		TintBox[0] = FVector2D(2.f, 2.f);
		TintBox[1] = TintBox[0] + FVector2D(BoxSize.X, 0);
		TintBox[2] = TintBox[0] + FVector2D(BoxSize.X, BoxSize.Y);
		TintBox[3] = TintBox[0] + FVector2D(0, BoxSize.Y);
		for (int32 Index = 0; Index < 4; ++Index)
		{
			const int32 NextVertexIndex = (Index + 1) % 4;
			FCanvasLineItem BoxLine(TintBox[Index], TintBox[NextVertexIndex]);
			BoxLine.LineThickness = TintLineSize;
			BoxLine.SetColor(TineLineColor);
			Canvas->DrawItem(BoxLine);
		}
	}
}

bool FInstanceToolEdMode::Select(AActor* InActor, bool bInSelected)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bCanSelectActorInWorldOutliner || UISetting.bCanSelectActorInViewport)
	{
		if (bInSelected)
		{
			//Selection.SetSelectedNone();
			SetSelectNone(/*bEnableUndo=*/ true);
		}

		return false; // not handled
	}

	if (bInSelected && CanSelectActor(InActor)) 
	{
		//Selection.SelectAllInstances(InActor);
		TArray<AActor*> Actors;
		Actors.Add(InActor);
		SelectAllInstancesOfActors(Actors, true);
		return true;
	}

	if (bInSelected == false)
	{
		return false;
	}

	return true; // handled
}

bool FInstanceToolEdMode::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	return (UISetting.bCanSelectActorInWorldOutliner || UISetting.bCanSelectActorInViewport);
	//return FEdMode::IsSelectionAllowed(InActor, bInSelection);
}

void FInstanceToolEdMode::ActorSelectionChangeNotify()
{
}

bool FInstanceToolEdMode::UsesPropertyWidgets() const {
	return ShouldDrawWidget();
}

FVector FInstanceToolEdMode::GetWidgetLocation() const
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bCanSelectActorInWorldOutliner && GEditor->GetSelectedActorCount() > 0)
	{
		return FEdMode::GetWidgetLocation();
	}

	if (bUseCachedWidgetLocation)
	{
		return CachedWidgetLocation;
	}

	CachedWidgetLocation = Selection.GetSelectionLocation();
	return CachedWidgetLocation;
}

bool FInstanceToolEdMode::AllowWidgetMove()
{
	return ShouldDrawWidget();
}

bool FInstanceToolEdMode::ShouldDrawWidget() const
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bCanSelectActorInWorldOutliner && GEditor->GetSelectedActorCount() > 0)
	{
		return FEdMode::ShouldDrawWidget();
	}

	return !UISetting.bHideWidget && Selection.IsSelected();
}

bool FInstanceToolEdMode::UsesTransformWidget() const
{
	return ShouldDrawWidget();
}

EAxisList::Type FInstanceToolEdMode::GetWidgetAxisToDraw(UE::Widget::EWidgetMode InWidgetMode) const
{
	switch (InWidgetMode)
	{
	case UE::Widget::WM_Translate:
	case UE::Widget::WM_Rotate:
	case UE::Widget::WM_Scale:
		return EAxisList::All;
	default:
		return EAxisList::None;
	}
}

bool FInstanceToolEdMode::GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData)
{
	if (!Selection.IsSelected())
	{
		return 0;
	}

	InMatrix = FRotationMatrix::Make(Selection.GetLastSelectedTransform().GetRotation());
	return 1;
}

bool FInstanceToolEdMode::GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData)
{
	return GetCustomDrawingCoordinateSystem(InMatrix, InData);
}

bool FInstanceToolEdMode::CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	return false;
}

bool FInstanceToolEdMode::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	return false;
}

bool FInstanceToolEdMode::StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	//if (Selection.IsSelected())
	{
		if (InViewport->KeyState(EKeys::RightMouseButton))
		{
			bRightClick = true;
		}

		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		if (!UISetting.bDisableUndo && !bRightClick && CurrentTransactionID == INDEX_NONE)
		{
			CurrentTransactionID = GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_EditTransaction", "InstanceTool Editing"));
		}

		if (InViewport->KeyState(EKeys::LeftShift) || InViewport->KeyState(EKeys::RightShift))
		{
			bShiftDown = true;
		}
		else
		{
			bShiftDown = false;
		}

		bCanAltDrag = Selection.IsSelected();
		bModifiedInTransaction = false;
		bSelectionChangedInTransaction = false;

		return !bRightClick;
	}
	return FEdMode::StartTracking(InViewportClient, InViewport);
}

bool FInstanceToolEdMode::EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	//if (GetCurrentToolMode() == EToolMode::Select)
	{
		if (CurrentTransactionID != INDEX_NONE)
		{
			if (bSelectionChangedInTransaction || bModifiedInTransaction)
			{
				PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
				GEditor->EndTransaction();
				CurrentTransactionID = INDEX_NONE;
			}
			else
			{
				GEditor->CancelTransaction(CurrentTransactionID);
				CurrentTransactionID = INDEX_NONE;
			}

			if (bModifiedInTransaction)
			{
				NoteInstanceDataModified(Selection.GetSelectedActors());
			}
		}

		bModifiedInTransaction = false;
		bSelectionChangedInTransaction = false;
		
		bRightClick = false;

		return true;
	}

	//return FEdMode::EndTracking(InViewportClient, InViewport);
}

bool FInstanceToolEdMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!Selection.IsSelected() && UISetting.bCanSelectActorInViewport)
	{
		return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
	}

	// Alt + Middle Mouse
	if ( (InViewport->KeyState(EKeys::LeftAlt) || InViewport->KeyState(EKeys::RightAlt))
		&& (InViewport->KeyState(EKeys::MiddleMouseButton))
		&& InViewportClient->GetCurrentWidgetAxis() != EAxisList::None)
	{
		if (!InDrag.IsNearlyZero())
		{
			FEditorModeTools& EditorModeTools = GLevelEditorModeTools();
			CachedWidgetLocation = EditorModeTools.GetWidgetLocation() + InDrag;
			bUseCachedWidgetLocation = true;
			return true;
		}
	}

	if (Selection.IsSelected() && InViewportClient->GetCurrentWidgetAxis() != EAxisList::None) // && (UISettings.GetSelectToolSelected() || UISettings.GetLassoSelectToolSelected()))
	{
		const bool bDuplicateInstances = (bCanAltDrag && IsAltDown(InViewport) && (InViewportClient->GetCurrentWidgetAxis() & EAxisList::XYZ)
			// Hack, avoid duplicate while moving widget with Alt + Arrow Key
			&& !(InViewport->KeyState(EKeys::Left) || InViewport->KeyState(EKeys::Right) || InViewport->KeyState(EKeys::Up) || InViewport->KeyState(EKeys::Down)));

		FVector Drag(InDrag);

		// If vertex snapping
		// Only works for non-instance static mesh, disable for now
		//FSnappingUtils::SnapDragLocationToNearestVertex(SelectionInfo.GetSelectionLocation(), Drag, GCurrentLevelEditingViewportClient);
		
		if (!Drag.IsNearlyZero() || !InRot.IsNearlyZero() || !InScale.IsNearlyZero()) 
		{
			bModifiedInTransaction = true;

			if (bDuplicateInstances) {
				Undo->UpdateInstancesSnapshot();
			}

			if (bUseCachedWidgetLocation)
			{
				CachedWidgetLocation += InDrag;
			}

			Undo->Modify();

			Selection.TransformSelectedInstances(Drag, InRot, InScale, bDuplicateInstances);

			if (bDuplicateInstances) {
				Undo->UpdateSelected();
				Undo->UpdateInstancesSnapshot();
			}
			else
			{
				Undo->UpdateSelectedTransforms();
			}
			bCanAltDrag = false;
		}
	}
	return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
}

bool FInstanceToolEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bCanSelectActorInWorldOutliner && GEditor->GetSelectedActorCount() > 0)
	{
		return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
	}

	bool bHandled = false;
	if (!bHandled)
	{
		// Todo: Move to configuration
		if (Event == IE_Pressed) // && (GetCurrentToolMode() == EToolMode::Select))
		{
			// Delete
// 			if (Key == EKeys::Platform_Delete)
// 			{
// 				DeleteSelectedInstances();
// 				bHandled = true;
// 			}

			// Focus
			if (Key == EKeys::F)
			{
				FocusViewportOnSelection();
				bHandled = true;
			}

			// Select All
			else if (IsCtrlDown(Viewport) && IsShiftDown(Viewport) && Key == EKeys::A)
			{
				SelectAllInstances(/*bSelectSameComponentOnly=*/ false, /*bEndableUndo=*/ true);
				bHandled = true;
			}
			else if (IsCtrlDown(Viewport) && Key == EKeys::A)
			{
				SelectAllInstances(/*bSelectSameComponentOnly=*/ true, /*bEndableUndo=*/ true);
				bHandled = true;
			}

			// Align Selection
			else if (IsShiftDown(Viewport) && IsCtrlDown(Viewport) && Key == EKeys::Right)
			{
				AlignSelectionLocation(GetAlignCoordSystem(), EAxis::X);
				bHandled = true;
			}
			else if (IsShiftDown(Viewport) && IsCtrlDown(Viewport) && Key == EKeys::Left)
			{
				AlignSelectionLocation(GetAlignCoordSystem(), EAxis::X, /*bNegative=*/ true);
				bHandled = true;
			}
			else if (IsShiftDown(Viewport) && IsCtrlDown(Viewport) && Key == EKeys::Down)
			{
				AlignSelectionLocation(GetAlignCoordSystem(), EAxis::Y);
				bHandled = true;
			}
			else if (IsShiftDown(Viewport) && IsCtrlDown(Viewport) && Key == EKeys::Up)
			{
				AlignSelectionLocation(GetAlignCoordSystem(), EAxis::Y, /*bNegative=*/ true);
				bHandled = true;
			}
			else if (IsShiftDown(Viewport) && IsCtrlDown(Viewport) && Key == EKeys::Home)
			{
				AlignSelectionLocation(GetAlignCoordSystem(), EAxis::Z);
				bHandled = true;
			}
			else if (IsShiftDown(Viewport) && IsCtrlDown(Viewport) && Key == EKeys::End)
			{
				AlignSelectionLocation(GetAlignCoordSystem(), EAxis::Z, /*bNegative=*/ true);
				bHandled = true;
			}

			// Snap to Grid
			else if (IsCtrlDown(Viewport) && Key == EKeys::End)
			{
				AlignSelectionToGrid(UE::Widget::WM_Translate);
				bHandled = true;
			}

			// SnapSelection
			else if (IsCtrlDown(Viewport) && Key == EKeys::Right)
			{
				SnapSelection(EAxis::X);
				bHandled = true;
			}
			else if (IsCtrlDown(Viewport) && Key == EKeys::Left)
			{
				SnapSelection(EAxis::X, /*bNegative=*/ true);
				bHandled = true;
			}
			else if (IsCtrlDown(Viewport) && Key == EKeys::Down)
			{
				SnapSelection(EAxis::Y);
				bHandled = true;
			}
			else if (IsCtrlDown(Viewport) && Key == EKeys::Up)
			{
				SnapSelection(EAxis::Y, /*bNegative=*/ true);
				bHandled = true;
			}
			else if (!IsCtrlDown(Viewport) && Key == EKeys::Home)
			{
				SnapSelection(EAxis::Z);
				bHandled = true;
			}
			else if (!IsCtrlDown(Viewport) && Key == EKeys::End)
			{
				SnapSelection(EAxis::Z, /*bNegative=*/ true);
				bHandled = true;
			}

			// Toggle widget
			else if (Key == EKeys::Escape)
			{
				UISetting.bHideWidget = !UISetting.bHideWidget;
				bHandled = true;
			}

			// Toggle Lock Selection
			else if (Key == EKeys::SpaceBar)
			{
				UISetting.bLockSelection = !Selection.IsSelected() ? false : !UISetting.bLockSelection;
				bHandled = true;
			}

			// Toggle Marquee Substract Mode
			else if (Key == EKeys::Tab)
			{
				UISetting.bMarqueeSelectSubtractMode = !UISetting.bMarqueeSelectSubtractMode;
				bHandled = true;
			}
		}
	}

	return bHandled;
}

bool FInstanceToolEdMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	//if (GetCurrentToolMode() == EToolMode::Select)	
	{
		if (UISetting.bLockSelection)
		{
			return true;
		}

		else if (HitProxy && HitProxy->IsA(HInstancedStaticMeshInstance::StaticGetType()) && !bRightClick)
		{
			HInstancedStaticMeshInstance* ISMIProxy = static_cast<HInstancedStaticMeshInstance*>(HitProxy);

			UInstancedStaticMeshComponent* ISMC = ISMIProxy->Component;
			int32 InstanceIndex = ISMIProxy->InstanceIndex;

			AActor*  CurrentSelectedActor = ISMC->GetOwner();

			if (!CanSelectActor(CurrentSelectedActor))
			{
				return true;
			}

			const bool bMultiSelect = Click.IsControlDown() || Click.IsShiftDown();

			if (!bMultiSelect)
			{
				Selection.ClearInstanceSelection(CurrentSelectedActor);
			}

			const bool bCurrentSelected = ISMC->IsInstanceSelected(InstanceIndex);
			bool bDeSelected = Click.IsControlDown() && bCurrentSelected;
			
			//Selection.SelectInstance(!bDeSelected, CurrentSelectedActor, ISMC, InstanceIndex, bMultiSelect, /*bBroadcastChange=*/ true);
			Selection.SelectInstance(!bDeSelected, ISMC, InstanceIndex, bMultiSelect, /*bBroadcastChange=*/ true);

			GEditor->SelectNone(/*bNoteSelectionChange=*/ true, /*bDeselectBSPSurfs=*/ true);

			Undo->Modify();
			Undo->UpdateSelected();
			bSelectionChangedInTransaction = true;
		}
		else
		{	
			bool bClickOnWidget = HitProxy && HitProxy->IsA(HWidgetAxis::StaticGetType());

			//if (!bClickOnWidget && !Click.IsAltDown() && !Click.IsControlDown() && !Click.IsShiftDown() && !bRightClick)
			if (!bClickOnWidget)
			{
				if (Selection.IsSelected() && !Click.IsAltDown() && !Click.IsControlDown() && !Click.IsShiftDown() && !bRightClick)
				{
					Selection.SetSelectedNone(/*bBroadcastChange=*/ true);

					Undo->Modify();
					Undo->UpdateSelected();
					bSelectionChangedInTransaction = true;
				}

				if (UISetting.bCanSelectActorInViewport)
				{
					return FEdMode::HandleClick(InViewportClient, HitProxy, Click);
				}

				if (UISetting.bCanSelectActorInWorldOutliner && HitProxy)
				{
					GEditor->SelectNone(/*bNoteSelectionChange=*/ true, /*bDeselectBSPSurfs=*/ true);
					bSelectionChangedInTransaction = true;
				}
			}
		}
	}

	return true;

// 	if (GetCurrentToolMode() == EToolMode::About)
// 	{
// 		return true;
// 	}
// 
// 	return FEdMode::HandleClick(InViewportClient, HitProxy, Click);
}

EEditAction::Type FInstanceToolEdMode::GetActionEditDelete()
{
	EEditAction::Type Result = EEditAction::Skip;

	if (HasAnyInstanceSelected())
	{
		return EEditAction::Process;
	}

	return Result;
}

bool FInstanceToolEdMode::ProcessEditDelete()
{
	if (HasAnyInstanceSelected())
	{
		DeleteSelectedInstances();
		return true;
	}
	return false;
}

EEditAction::Type FInstanceToolEdMode::GetActionEditDuplicate()
{
	EEditAction::Type Result = EEditAction::Skip;

	if (HasAnyInstanceSelected())
	{
		return EEditAction::Process;
	}

	return Result;
}

bool FInstanceToolEdMode::ProcessEditDuplicate()
{
	if (HasAnyInstanceSelected())
	{
		DuplicateSelectedInstances();
		return true;
	}
	return false;
}

void FInstanceToolEdMode::ActorsDuplicatedNotify(TArray<AActor*>& PreDuplicateSelection, TArray<AActor*>& PostDuplicateSelection, bool bOffsetLocations)
{
	ForceRebuildRenderData();
}

bool FInstanceToolEdMode::UsesToolkits() const
{
	return true;
}

const bool FInstanceToolEdMode::HasAnyInstanceSelected() const
{
	return Selection.IsSelected();
}

const int32 FInstanceToolEdMode::GetSelectedInstanceCount() const
{
	return Selection.Selection.Num();
}

const TArray<UObject*> FInstanceToolEdMode::GetSelectedObjects() const
{
	return static_cast<TArray<UObject*>>(Selection.GetSelectedProxyObjects());
}

void FInstanceToolEdMode::SetOnSelectionChangedDelegate(FOnSelectionChanged InOnSelectionChangedDelegate)
{
	OnSelectionChangedDelegate = InOnSelectionChangedDelegate;
}

void FInstanceToolEdMode::BroadcastSelectionChanged()
{
	if (OnSelectionChangedDelegate.IsBound())
	{
		OnSelectionChangedDelegate.Execute();
	}

	bUseCachedWidgetLocation = false;
}

void FInstanceToolEdMode::SetSelectNone(bool bEnableUndo)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		if (Selection.IsSelected())
		{
			UISetting.bLockSelection = false;
		}
		return;
	}

	if (Selection.IsSelected())
	{
		bool bTransaction = false;
		if (bEnableUndo && !UISetting.bDisableUndo && !GIsTransacting)
		{
			GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_DeSelectTransaction", "DeSelect All Instances"));
			bTransaction = true;
		}

		Undo->Modify();

		Selection.SetSelectedNone();

		Undo->UpdateSelected();

		if (bTransaction)
		{
			GEditor->EndTransaction();
		}

		BroadcastSelectionChanged();
	}
}

bool FInstanceToolEdMode::DoBoxSelect(FBox& InBox, bool InSelect)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		return false;
	}

	//const FScopedTransaction Transaction(LOCTEXT("InstanceToolBoxSelect", "Instance Tool Select"));

	bool bSelectSameActorOnly = UISetting.bMarqueeSelectSameActorOnly;
	const bool bStrictDragSelection = UISetting.MarqueeSelectOption == EMarqueeSelectOption::Inside;
	const bool bSubstractMode = UISetting.bMarqueeSelectSubtractMode;

	// In case Shift been hold while Frustum Select, which will leave other actors' instances keep selected
	auto SelectedActors = Selection.GetSelectedActors();
	if (!bShiftDown && !bSubstractMode)
	{	
		//SetSelectNone();
		Selection.SetSelectedNone();
	}

	bool bHasAnySelected = false;

	for (FActorIterator It(GCurrentLevelEditingViewportClient->GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (CanSelectActor(Actor) && !Actor->IsHiddenEd())
		{
			if (bSelectSameActorOnly && SelectedActors.Num() > 0 && !SelectedActors.Contains(Actor))
			{
				continue;
			}

			TArray<UInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UInstancedStaticMeshComponent>(Components);

			for (auto& Component : Components)
			{
				if (!Component->IsVisibleInEditor())
				{
					continue;
				}

				for (int32 Index = 0; Index < Component->PerInstanceSMData.Num(); ++Index)
				{
					FBox InstanceBound;
					Selection.GetInstanceWorldBoundingBox(Component, Index, InstanceBound);
					
					if ((!bStrictDragSelection && InBox.Intersect(InstanceBound))
						|| (bStrictDragSelection && InBox.IsInside(InstanceBound)))
					{
						bHasAnySelected = true;

						if (bSubstractMode)
						{
							if (Component->IsInstanceSelected(Index))
							{
								Selection.SelectInstance(/*InSelected=*/ false, Component, Index, /*bMultiSelect=*/true, /*bBroardcastChange*/false);
							}
						}
						else
						{
							Selection.SelectInstance(/*InSelected=*/ true, Component, Index, /*bMultiSelect=*/true, /*bBroardcastChange*/false);
						}
					}
				}
			}

			if (!bSubstractMode && bSelectSameActorOnly && bHasAnySelected)
			{
				break;
			}
		}
	}

	if (bHasAnySelected)
	{
		BroadcastSelectionChanged();
	}

	Undo->Modify();
	Undo->UpdateSelected();

	return bHasAnySelected;
}

bool FInstanceToolEdMode::DoFrustumSelect(const FConvexVolume& InFrustum, FEditorViewportClient* InViewportClient, bool InSelect)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		return false;
	}

	//const FScopedTransaction Transaction(LOCTEXT("InstanceToolFrustumSelect", "Instance Tool Select"));

	bool bSelectSameActorOnly = UISetting.bMarqueeSelectSameActorOnly;
	const bool bStrictDragSelection = UISetting.MarqueeSelectOption == EMarqueeSelectOption::Inside;
	const bool bSubstractMode = UISetting.bMarqueeSelectSubtractMode;

	// In case Shift been hold while Frustum Select, which will leave other actors' instances keep selected
	auto SelectedActors = Selection.GetSelectedActors();
	if (!bShiftDown && !bSubstractMode)
	{
		//SetSelectNone();
		Selection.SetSelectedNone();
	}

	bool bHasAnySelected = false;

	for (FActorIterator It(GCurrentLevelEditingViewportClient->GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (CanSelectActor(Actor) && !Actor->IsHiddenEd())
		{
			if (bSelectSameActorOnly && SelectedActors.Num() > 0 && !SelectedActors.Contains(Actor))
			{
				continue;
			}

			TArray<UInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UInstancedStaticMeshComponent>(Components);

			for (auto& Component : Components)
			{
				if (!Component->IsVisibleInEditor())
				{
					continue;
				}

				for (int32 Index = 0; Index < Component->PerInstanceSMData.Num(); ++Index)
				{
					FBox InstanceBound;
					Selection.GetInstanceWorldBoundingBox(Component, Index, InstanceBound);
					bool bIsFullyContained;
					if (InFrustum.IntersectBox(InstanceBound.GetCenter(), InstanceBound.GetExtent(), bIsFullyContained))
					{
						if (!bStrictDragSelection || bIsFullyContained) 
						{
							bHasAnySelected = true;

							if ( bSubstractMode)
							{
								if (Component->IsInstanceSelected(Index))
								{
									Selection.SelectInstance(/*InSelected=*/ false, Component, Index, /*bMultiSelect=*/true, /*bBroardcastChange*/false);
								}
							}
							else
							{
								Selection.SelectInstance(/*InSelected=*/ true, Component, Index, /*bMultiSelect=*/true, /*bBroardcastChange*/false);
							}
						}
					}
				}
			}

			if (!bSubstractMode && bSelectSameActorOnly && bHasAnySelected)
			{
				break;
			}
		}
	}

	Undo->Modify();
	Undo->UpdateSelected();


	if (bHasAnySelected)
	{
		BroadcastSelectionChanged();
	}

	return bHasAnySelected;
}

void FInstanceToolEdMode::SetCurrentToolMode(EToolMode InMode)
{
	CurrentToolMode = InMode;
}

EToolMode FInstanceToolEdMode::GetCurrentToolMode() const
{
	return CurrentToolMode;
}

void FInstanceToolEdMode::PropagateChangesToBlueprintInstances(const TArray<AActor*>& InActors, bool bModify)
{
	const bool bPropagateChangesToBlueprintInstances = false;
	if (!bPropagateChangesToBlueprintInstances)
	{
		return;
	}

	TArray<UBlueprint*> Blueprints;

	for (auto& Actor : InActors)
	{
		UBlueprint* Blueprint = (Actor != nullptr) ? Cast<UBlueprint>(Actor->GetClass()->ClassGeneratedBy) : nullptr;
		if (Blueprint == nullptr)
		{
			continue;
		}

		if (Blueprints.Contains(Blueprint))
		{
			continue;
		}

		Blueprints.Add(Blueprint);

		FProperty* PerInstanceSMDataProp = FindFieldChecked<FProperty>(UInstancedStaticMeshComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UInstancedStaticMeshComponent, PerInstanceSMData));

		AActor* BlueprintCDO = Actor->GetClass()->GetDefaultObject<AActor>();
		if (BlueprintCDO != NULL)
		{
			TArray<UInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UInstancedStaticMeshComponent>(Components);
			FProperty* TransformProp = FindFieldChecked<FProperty>(TBaseStructure<FInstancedStaticMeshInstanceData>::Get(), GET_MEMBER_NAME_CHECKED(FInstancedStaticMeshInstanceData, Transform));

			for (UActorComponent* SourceComponent : Components)
			{
				UActorComponent* InstancedStaticMeshComponentTemplate = EditorUtilities::FindMatchingComponentInstance(SourceComponent, BlueprintCDO);
				if (InstancedStaticMeshComponentTemplate)
				{
					TArray<UObject*> ArchetypeInstances;
					InstancedStaticMeshComponentTemplate->GetArchetypeInstances(ArchetypeInstances);
					for (UObject* ArchetypeInstance : ArchetypeInstances)
					{
						UInstancedStaticMeshComponent* InstancedStaticMeshComponent = CastChecked<UInstancedStaticMeshComponent>(ArchetypeInstance);
						if (InstancedStaticMeshComponent != SourceComponent)
						{
							if (bModify)
							{
								InstancedStaticMeshComponent->Modify();
							}
							const InstanceToolUtilities::ECopyOptions CopyOptions = (InstanceToolUtilities::ECopyOptions)(InstanceToolUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties);
							InstanceToolUtilities::CopyActorProperties(Actor, InstancedStaticMeshComponent->GetOwner(), CopyOptions, SourceComponent, PerInstanceSMDataProp);
							//Local::DoPostEditChangeChainProperty(InstancedStaticMeshComponent, TransformProp);
						}
					}
				}
			}
		}
	}
}

void FInstanceToolEdMode::ForceRebuildRenderData()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	const bool  bApplyToAll = UISetting.bRebuildRenderDataApplyToAllActors;

	if (bApplyToAll)
	{
		if (UWorld* World = GEditor->GetEditorWorldContext().World())
		{
			FInstanceToolUtil::ForceRebuildRenderDataInLevel(World);
		}
		return;
	}


	if (HasAnyInstanceSelected())
	{
		for (auto& Component : Selection.GetSelectedComponents())
		{
			FInstanceToolUtil::ForceRebuildRenderData(Component.Get());
		}
	}
	TArray<AActor*> Selected;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(Selected);
	for (auto& Actor : Selected)
	{
		if (IsValid(Actor))
		{
			if (Actor->IsA(AInstancedFoliageActor::StaticClass()))
			{
				continue;
			}

			TArray<UInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UInstancedStaticMeshComponent>(Components);
			for (auto& Component : Components)
			{
				FInstanceToolUtil::ForceRebuildRenderData(Component);
			}
		}
	}
}

void FInstanceToolEdMode::InvalidateLightingCache()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	const bool  bApplyToAll = UISetting.bRebuildRenderDataApplyToAllActors;
	if (bApplyToAll)
	{
		if (UWorld* World = GEditor->GetEditorWorldContext().World())
		{
			FInstanceToolUtil::InvalidateLightingCacheInLevel(World);
		}
		return;
	}

	TArray<AActor*> SelectedActors = Selection.GetSelectedActors();
	for (auto& Actor : SelectedActors)
	{
		if (IsValid(Actor))
		{
			if (Actor->IsA(AInstancedFoliageActor::StaticClass()))
			{
				continue;
			}

			TArray<UInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UInstancedStaticMeshComponent>(Components);
			for (auto& Component : Components)
			{
				if (Component)
				{
					Component->InvalidateLightingCache();
				}
			}
		}
	}
}

void FInstanceToolEdMode::ApplyChangesToBlueprint()
{
	TArray<AActor*> SelectedActors;
	if (GEditor->GetSelectedActorCount() > 0)
	{
		GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);
	}
	else
	{
		SelectedActors = Selection.GetSelectedActors();
	}

	TryApplyChangesToBlueprint(SelectedActors);
}

void FInstanceToolEdMode::TryApplyChangesToBlueprint(const TArray<AActor*>& InActors)
{
	/**
	This struct saves and deselects all selected instanced components (from given actor), then finds them (in recreated actor instance, after compilation) and selects them again.
	*/
	struct FRestoreSelectedInstanceComponent
	{
		TWeakObjectPtr<UClass> ActorClass;
		FName ActorName;
		TWeakObjectPtr<UObject> ActorOuter;

		struct FComponentKey
		{
			FName Name;
			TWeakObjectPtr<UClass> Class;

			FComponentKey(FName InName, UClass* InClass) : Name(InName), Class(InClass) {}
		};
		TArray<FComponentKey> ComponentKeys;

		FRestoreSelectedInstanceComponent()
			: ActorClass(nullptr)
			, ActorOuter(nullptr)
		{ }

		void Save(AActor* InActor)
		{
			check(InActor);
			ActorClass = InActor->GetClass();
			ActorName = InActor->GetFName();
			ActorOuter = InActor->GetOuter();

			check(GEditor);
			TArray<UActorComponent*> ComponentsToSaveAndDelesect;
			for (auto Iter = GEditor->GetSelectedComponentIterator(); Iter; ++Iter)
			{
				UActorComponent* Component = CastChecked<UActorComponent>(*Iter, ECastCheckedType::NullAllowed);
				if (Component && InActor->GetInstanceComponents().Contains(Component))
				{
					ComponentsToSaveAndDelesect.Add(Component);
				}
			}

			for (UActorComponent* Component : ComponentsToSaveAndDelesect)
			{
				USelection* SelectedComponents = GEditor->GetSelectedComponents();
				if (ensure(SelectedComponents))
				{
					ComponentKeys.Add(FComponentKey(Component->GetFName(), Component->GetClass()));
					SelectedComponents->Deselect(Component);
				}
			}
		}

		void Restore()
		{
			AActor* Actor = (ActorClass.IsValid() && ActorOuter.IsValid())
				? Cast<AActor>((UObject*)FindObjectWithOuter(ActorOuter.Get(), ActorClass.Get(), ActorName))
				: nullptr;
			if (Actor)
			{
				for (const FComponentKey& IterKey : ComponentKeys)
				{
					UActorComponent* const* ComponentPtr = Algo::FindByPredicate(Actor->GetComponents(), [&](UActorComponent* InComp)
					{
						return InComp && (InComp->GetFName() == IterKey.Name) && (InComp->GetClass() == IterKey.Class.Get());
					});
					if (ComponentPtr && *ComponentPtr)
					{
						check(GEditor);
						GEditor->SelectComponent(*ComponentPtr, true, false);
					}
				}
			}
		}
	};

	TArray<UBlueprint*> Blueprints;

	for (auto& Actor : InActors)
	{
		UBlueprint* Blueprint = (Actor != nullptr) ? Cast<UBlueprint>(Actor->GetClass()->ClassGeneratedBy) : nullptr;

		if (Blueprint == nullptr || Blueprints.Contains(Blueprint))
		{
			continue;
		}
		Blueprints.Add(Blueprint);

		int32 NumChangedProperties = 0;

		if (Actor != NULL && Blueprint != NULL && Actor->GetClass()->ClassGeneratedBy == Blueprint)
		{
			// Cache the actor label as by the time we need it, it may be invalid
			const FString ActorLabel = Actor->GetActorLabel();
			FRestoreSelectedInstanceComponent RestoreSelectedInstanceComponent;
			{
				const FScopedTransaction Transaction(LOCTEXT("PushToBlueprintDefaults_Transaction", "Apply Changes to Blueprint"));

				// The component selection state should be maintained
				GEditor->GetSelectedComponents()->Modify();

				Actor->Modify();

				// Mark components that are either native or from the SCS as modified so they will be restored
				for (UActorComponent* ActorComponent : Actor->GetComponents())
				{
					if (ActorComponent && (ActorComponent->CreationMethod == EComponentCreationMethod::SimpleConstructionScript || ActorComponent->CreationMethod == EComponentCreationMethod::Native))
					{
						ActorComponent->Modify();
					}
				}

				// Perform the actual copy
				{
					AActor* BlueprintCDO = Actor->GetClass()->GetDefaultObject<AActor>();
					if (BlueprintCDO != NULL)
					{
						const EditorUtilities::ECopyOptions::Type CopyOptions = (EditorUtilities::ECopyOptions::Type)(EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties | EditorUtilities::ECopyOptions::PropagateChangesToArchetypeInstances | EditorUtilities::ECopyOptions::SkipInstanceOnlyProperties);
						NumChangedProperties = EditorUtilities::CopyActorProperties(Actor, BlueprintCDO, CopyOptions);

						{
							// Copy component properties from source to target if they match. Note that the component lists may not be 1-1 due to context-specific components (e.g. editor-only sprites, etc.).
							TInlineComponentArray<UActorComponent*> SourceComponents;
							TInlineComponentArray<UActorComponent*> TargetComponents;

							Actor->GetComponents(SourceComponents);
							BlueprintCDO->GetComponents(TargetComponents);

							int32 TargetComponentIndex = 0;
							for (UActorComponent* SourceComponent : SourceComponents)
							{
								if (SourceComponent->CreationMethod == EComponentCreationMethod::UserConstructionScript)
								{
									continue;
								}
								UActorComponent* TargetComponent = InstanceToolUtilities::FindMatchingComponentInstance(SourceComponent, BlueprintCDO, TargetComponents, TargetComponentIndex);

								if (TargetComponent != nullptr)
								{
									UInstancedStaticMeshComponent* SourceInstancedComponent = Cast<UInstancedStaticMeshComponent>(SourceComponent);
									UInstancedStaticMeshComponent* TargetInstancedComponent = Cast<UInstancedStaticMeshComponent>(TargetComponent);
									if (SourceInstancedComponent && TargetInstancedComponent)
									{
										// Make sure hit proxies is created and selectable
										TargetInstancedComponent->bSelectable = true;
										TargetInstancedComponent->bHasPerInstanceHitProxies = true;

										INSTANCETOOL_LOG(Display, TEXT("      InstanceReorderTable? Source: %d, Target: %d")
											, SourceInstancedComponent->InstanceReorderTable.Num()
											, TargetInstancedComponent->InstanceReorderTable.Num());

										// Hack: Clear reorder table to make sure PerInstanceRenderData is properly initialized for instances
										TargetInstancedComponent->InstanceReorderTable.Empty();

										INSTANCETOOL_LOG(Display, TEXT("      bHasPerInstanceHitProxies? Source: %d, Target: %d")
											, SourceInstancedComponent->bHasPerInstanceHitProxies
											, TargetInstancedComponent->bHasPerInstanceHitProxies);
										INSTANCETOOL_LOG(Display, TEXT("      bSelectable? Source: %d, Target: %d")
											, SourceInstancedComponent->bSelectable
											, TargetInstancedComponent->bSelectable);
									}
								}
							}
						}

						if (Actor->GetInstanceComponents().Num() > 0)
						{
							RestoreSelectedInstanceComponent.Save(Actor);
							FKismetEditorUtilities::AddComponentsToBlueprint(Blueprint, Actor->GetInstanceComponents());
							NumChangedProperties += Actor->GetInstanceComponents().Num();
							Actor->ClearInstanceComponents(true);
						}
						if (NumChangedProperties > 0)
						{
							//Actor = nullptr; // It is unsafe to use Actor after this point as it may have been reinstanced, so set it to null to make this obvious
						}
					}
				}
			}

			// Compile the BP outside of the transaction
			if (NumChangedProperties > 0)
			{
				FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
				FKismetEditorUtilities::CompileBlueprint(Blueprint);
				//Blueprint->BroadcastChanged(); 
				RestoreSelectedInstanceComponent.Restore();
			}

			if (NumChangedProperties > 0)
			{
				//UE_LOG(LogTemp, Display, TEXT("Updated Blueprint %s (%d property changes applied from actor %s)."), *Blueprint->GetFName().ToString(), NumChangedProperties, *ActorLabel); 
			}
		}
	}
}

void FInstanceToolEdMode::ResetToBlueprintDefaults()
{
	TArray<AActor*> SelectedActors;
	if (GEditor->GetSelectedActorCount() > 0)
	{
		GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);
	}
	else
	{
		SelectedActors = Selection.GetSelectedActors();
	}

	if (SelectedActors.Num() > 0)
	{
		const FScopedTransaction Transaction(LOCTEXT("ResetToBlueprintDefaults_Transaction", "Reset to Class Defaults"));

		for (auto& Actor : SelectedActors)
		{
			int32 NumChangedProperties = 0;

			UBlueprint* Blueprint = (Actor != nullptr) ? Cast<UBlueprint>(Actor->GetClass()->ClassGeneratedBy) : nullptr;

			if ((Actor != NULL) && (Blueprint != NULL) && (Actor->GetClass()->ClassGeneratedBy == Blueprint))
			{
				if (Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
				{
					Actor->ClearFlags(RF_ClassDefaultObject | RF_ArchetypeObject);
				}

				{
					AActor* BlueprintCDO = Actor->GetClass()->GetDefaultObject<AActor>();
					const EditorUtilities::ECopyOptions::Type CopyOptions = (EditorUtilities::ECopyOptions::Type)(EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties | EditorUtilities::ECopyOptions::CallPostEditChangeProperty);

					if (BlueprintCDO != NULL)
					{
						NumChangedProperties = EditorUtilities::CopyActorProperties(BlueprintCDO, Actor, CopyOptions);
					}
					NumChangedProperties += Actor->GetInstanceComponents().Num();
					Actor->ClearInstanceComponents(true);

					{
						EditorUtilities::FCopyOptions Options(CopyOptions);

						TInlineComponentArray<UActorComponent*> TargetComponents;
						Actor->GetComponents(TargetComponents);

						TSet<UObject*> ModifiedObjects;
						for (UActorComponent* TargetComponent : TargetComponents)
						{
							if (TargetComponent->CreationMethod == EComponentCreationMethod::UserConstructionScript)
							{
								continue;
							}
							UActorComponent* BlueprintComponent = CastChecked<UActorComponent>(TargetComponent->GetArchetype(), ECastCheckedType::NullAllowed);

							// If the returned target component is not from the direct class of the actor we're targeting, we need to insert an inheritable component
							if (BlueprintComponent && (BlueprintComponent->GetOuter() != BlueprintCDO->GetClass()))
							{
								// This component doesn't exist in the hierarchy anywhere and we're not going to modify the CDO, so we'll drop it
								if (BlueprintComponent->HasAnyFlags(RF_ClassDefaultObject))
								{
									BlueprintComponent = nullptr;
								}
								else
								{
									UBlueprintGeneratedClass* BPGC = CastChecked<UBlueprintGeneratedClass>(BlueprintCDO->GetClass());
									UBlueprint* TargetBlueprint = CastChecked<UBlueprint>(BPGC->ClassGeneratedBy);
									UInheritableComponentHandler* InheritableComponentHandler = TargetBlueprint->GetInheritableComponentHandler(true);
									if (InheritableComponentHandler)
									{
										FComponentKey Key;
										FName const SourceComponentName = TargetComponent->GetFName();

										BPGC = Cast<UBlueprintGeneratedClass>(BPGC->GetSuperClass());
										while (!Key.IsValid() && BPGC)
										{
											USCS_Node* SCSNode = BPGC->SimpleConstructionScript->FindSCSNode(SourceComponentName);
											if (!SCSNode)
											{
												UBlueprint* SuperBlueprint = CastChecked<UBlueprint>(BPGC->ClassGeneratedBy);
												for (UActorComponent* ComponentTemplate : BPGC->ComponentTemplates)
												{
													if (ComponentTemplate->GetFName() == SourceComponentName)
													{
														if (UEdGraph* UCSGraph = FBlueprintEditorUtils::FindUserConstructionScript(SuperBlueprint))
														{
															TArray<UK2Node_AddComponent*> ComponentNodes;
															UCSGraph->GetNodesOfClass<UK2Node_AddComponent>(ComponentNodes);

															for (UK2Node_AddComponent* UCSNode : ComponentNodes)
															{
																if (ComponentTemplate == UCSNode->GetTemplateFromNode())
																{
																	Key = FComponentKey(SuperBlueprint, FUCSComponentId(UCSNode));
																	break;
																}
															}
														}
														break;
													}
												}
											}
											else
											{
												Key = FComponentKey(SCSNode);
												break;
											}
											BPGC = Cast<UBlueprintGeneratedClass>(BPGC->GetSuperClass());
										}

										if (ensure(Key.IsValid()))
										{
											check(InheritableComponentHandler->GetOverridenComponentTemplate(Key) == nullptr);
											BlueprintComponent = InheritableComponentHandler->CreateOverridenComponentTemplate(Key);
										}
										else
										{
											BlueprintComponent = nullptr;
										}
									}
								}
							}

							if (BlueprintComponent != nullptr)
							{
								UClass* ComponentClass = TargetComponent->GetClass();
								check(ComponentClass == TargetComponent->GetClass());

								TSet<const FProperty*> TargetUCSModifiedProperties;
								TargetComponent->GetUCSModifiedProperties(TargetUCSModifiedProperties);

								// Copy component properties
								for (FProperty* Property = ComponentClass->PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
								{
									const bool bIsTransient = !!(Property->PropertyFlags & CPF_Transient);
									const bool bIsIdentical = Property->Identical_InContainer(TargetComponent, BlueprintComponent);
									const bool bIsComponent = !!(Property->PropertyFlags & (CPF_InstancedReference | CPF_ContainsInstancedReference));
									const bool bIsTransform =
										Property->GetFName() == USceneComponent::GetRelativeScale3DPropertyName() ||
										Property->GetFName() == USceneComponent::GetRelativeLocationPropertyName() ||
										Property->GetFName() == USceneComponent::GetRelativeRotationPropertyName();

									if (!bIsTransient && !bIsIdentical && !bIsComponent && !TargetUCSModifiedProperties.Contains(Property)
										&& (!bIsTransform || TargetComponent != Actor->GetRootComponent() || (!Actor->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) && !BlueprintCDO->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))))
									{
										const bool bIsSafeToCopy = (!(Options.Flags & EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties) || (Property->HasAnyPropertyFlags(CPF_Edit | CPF_Interp)))
											&& (!(Options.Flags & EditorUtilities::ECopyOptions::SkipInstanceOnlyProperties) || (!Property->HasAllPropertyFlags(CPF_DisableEditOnTemplate)));
										if (bIsSafeToCopy)
										{
											if (!Options.CanCopyProperty(*Property, *Actor))
											{
												continue;
											}

											//if (!bIsPreviewing)
											{
												if (!ModifiedObjects.Contains(TargetComponent))
												{
													TargetComponent->SetFlags(RF_Transactional);
													TargetComponent->Modify();
													ModifiedObjects.Add(TargetComponent);
												}

												if (Options.Flags & EditorUtilities::ECopyOptions::CallPostEditChangeProperty)
												{
													// @todo simulate: Should we be calling this on the component instead?
													Actor->PreEditChange(Property);
												}

												EditorUtilities::CopySingleProperty(BlueprintComponent, TargetComponent, Property);

												if (Options.Flags & EditorUtilities::ECopyOptions::CallPostEditChangeProperty)
												{
													FPropertyChangedEvent PropertyChangedEvent(Property);
													Actor->PostEditChangeProperty(PropertyChangedEvent);
												}
											}

											// 										++CopiedPropertyCount;
											// 
											// 										if (bIsTransform)
											// 										{
											// 											bTransformChanged = true;
											// 										}
										}
									}
								}
							}
						}
					}
				}

				// Set up a notification record to indicate success/failure
				if (NumChangedProperties > 0)
				{
					if (NumChangedProperties > 1)
					{
						FFormatNamedArguments Args;
						Args.Add(TEXT("BlueprintName"), FText::FromName(Blueprint->GetFName()));
						Args.Add(TEXT("NumChangedProperties"), NumChangedProperties);
						Args.Add(TEXT("ActorName"), FText::FromString(Actor->GetActorLabel()));
						FText Message = FText::Format(LOCTEXT("ResetToBlueprintDefaults_ApplySuccess", "Reset {ActorName} ({NumChangedProperties} property changes applied from Blueprint {BlueprintName})."), Args);
						NotifyMessage(Message);
					}
					else
					{
						FFormatNamedArguments Args;
						Args.Add(TEXT("BlueprintName"), FText::FromName(Blueprint->GetFName()));
						Args.Add(TEXT("ActorName"), FText::FromString(Actor->GetActorLabel()));
						FText Message = FText::Format(LOCTEXT("ResetOneToBlueprintDefaults_ApplySuccess", "Reset {ActorName} (1 property change applied from Blueprint {BlueprintName})."), Args);
						NotifyMessage(Message);
					}
				}
				else
				{
					FText Message = LOCTEXT("ResetToBlueprintDefaults_Failed", "No properties were reset");
					NotifyMessage(Message);
				}
			}
		}

	}

	
}

void FInstanceToolEdMode::BindCommands()
{
	const TSharedRef<FUICommandList>& ToolkitCommands = Toolkit->GetToolkitCommands();

// 	ToolkitCommands->MapAction(FGenericCommands::Get().Delete,
// 		FExecuteAction::CreateSP(this, &FInstanceToolEdMode::DeleteSelectedInstances));

// 	ToolkitCommands->MapAction(FGenericCommands::Get().Duplicate,
// 		FExecuteAction::CreateSP(this, &FInstanceToolEdMode::DuplicateSelectedInstances));
}

void FInstanceToolEdMode::OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	Selection.ValidateSelection();

	for (auto& Item : Selection.Selection)
	{
		if (auto NewObject = ReplacementMap.Find(Item.Component))
		{
			const UInstancedStaticMeshComponent* NewConstComponent = Cast<UInstancedStaticMeshComponent>(*NewObject);
			if (NewConstComponent)
			{
				auto NewComponent = const_cast<UInstancedStaticMeshComponent*>(NewConstComponent);
				Item.Component = NewComponent;
				Item.Proxy->Component = NewComponent;
			}
		}
	}

	Selection.ValidateSelection();

	for (auto& Item : Selection.Selection)
	{
		Item.Component->SelectInstance(true, Item.InstanceIndex);
	}

	Undo->OnObjectsReplaced(ReplacementMap);
}

bool FInstanceToolEdMode::CanSelectActor(AActor* InActor) const
{
	if (!InActor)
	{
		return false;
	}

	if (InActor->IsA(AInstancedFoliageActor::StaticClass()))
	{
		return false;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (InActor->GetClass()->ClassGeneratedBy && !UISetting.bAllowBlueprintEditing)
	{
		return false;
	}

	if (!Selection.HasInstancedComponent(InActor))
	{
		return false;
	}
	
	return true;
}

ECoordSystem FInstanceToolEdMode::GetAlignCoordSystem()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	ECoordSystem CoordSystem = UISetting.AlignCoord == EAlignSnapCoord::Auto
		? GLevelEditorModeTools().GetCoordSystem()
		: (UISetting.AlignCoord == EAlignSnapCoord::Local) ? COORD_Local : COORD_World;

	return CoordSystem;
}

void FInstanceToolEdMode::SelectAllInstances(bool bSelectSameComponentOnly, bool bEnableUndo)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		return;
	}

	if (Selection.IsSelected())
	{
		if (bEnableUndo && !UISetting.bDisableUndo)
		{
			GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_SelectAllInstancesTransaction", "Select All Instances"));
			Undo->Modify();
		}

		if (bSelectSameComponentOnly) 
		{
			for (auto& Component : Selection.GetSelectedComponents())
			{
				Selection.SelectAllInstances(Component.Get(), /*bMultiSelect=*/ false);
			}
		}
		else
		{
			for (auto& SelectedAcotr : Selection.GetSelectedActors())
			{
				Selection.SelectAllInstances(SelectedAcotr, /*bMultiSelect=*/ true);
			}
		}

		Undo->UpdateSelected();

		if (bEnableUndo && !UISetting.bDisableUndo)
		{	
			GEditor->EndTransaction();
		}

		BroadcastSelectionChanged();
	}
}

void FInstanceToolEdMode::SelectAllInstancesOfActors(TArray<AActor*>& Actors, bool bEnableUndo)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (bEnableUndo && !UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_SelectAllInstancesTransaction", "Select All Instances"));
		Undo->Modify();
	}

	for (auto& Actor : Actors)
	{
		if (Actor && CanSelectActor(Actor))
		{
			Selection.SelectAllInstances(Actor, /*bMultiSelect=*/ true);
		}
	}

	Undo->UpdateSelected();

	if (bEnableUndo && !UISetting.bDisableUndo)
	{
		GEditor->EndTransaction();
	}

	BroadcastSelectionChanged();
}

int32 FInstanceToolEdMode::SelectAllOverlappedInstances(bool bSelectSameComponentOnly /*= true*/)
{
	int32 OverlappedCount = 0;

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		return OverlappedCount;
	}

	const bool bRestoreSelection = true;

	if (Selection.IsSelected())
	{
		if (!UISetting.bDisableUndo)
		{
			GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_SelectAllInvalidInstancesTransaction", "Select Invalid Instances"));
		}

		if (bRestoreSelection)
		{
			Selection.BackupSelection();
		}

		FVector SelectInvalidTolerance = FVector(UISetting.SelectInvalidToleranceLocation, UISetting.SelectInvalidToleranceRotation, UISetting.SelectInvalidToleranceScale);

		if (bSelectSameComponentOnly)
		{
			auto SelectedComponents = Selection.GetSelectedComponents();

			//SetSelectNone();
			Selection.SetSelectedNone();

			for (auto& Component : SelectedComponents)
			{
				OverlappedCount += Selection.SelectAllInvalidInstances(Component.Get(), SelectInvalidTolerance, true, false);
			}
		}
		else
		{
			auto SelectedActors = Selection.GetSelectedActors();

			//SetSelectNone();
			Selection.SetSelectedNone();

			for (auto& SelectedAcotr : SelectedActors)
			{
				OverlappedCount += Selection.SelectAllInvalidInstances(SelectedAcotr, SelectInvalidTolerance, true, false);
			}
		}

		if (bRestoreSelection && OverlappedCount <=0)
		{
			Selection.RestoreSelection();
		}

		Undo->Modify();
		Undo->UpdateSelected();

		if (!UISetting.bDisableUndo)
		{
			GEditor->EndTransaction();
		}

		BroadcastSelectionChanged();
	}

	return OverlappedCount;
}

void FInstanceToolEdMode::SelectParentActors()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (HasAnyInstanceSelected() && (UISetting.bCanSelectActorInViewport || UISetting.bCanSelectActorInWorldOutliner))
	{
		auto Selected = Selection.GetSelectedActors();

		const FScopedTransaction Transaction(LOCTEXT("InstanceToolSelectParentActors", "Select Parent Actor)"));

		SetSelectNone(/*bEnableUndo=*/ true);

		{
			USelection* SelectedActors = GEditor->GetSelectedActors();
			SelectedActors->Modify();
			SelectedActors->BeginBatchSelectOperation();
			SelectedActors->DeselectAll();

			for (auto Actor : Selected)
			{
				//GEditor->SelectActor(Actor, true, false);
				SelectedActors->Select(Actor);
			}
			SelectedActors->EndBatchSelectOperation();
			GEditor->NoteSelectionChange();
		}
	}
}

void FInstanceToolEdMode::SelectParentComponents()
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (HasAnyInstanceSelected() && (UISetting.bCanSelectActorInViewport || UISetting.bCanSelectActorInWorldOutliner))
	{
		AActor* LastSelectedActor = Selection.GetSelectedActors().Last();
		auto Selected = Selection.GetSelectedComponents();

		SetSelectNone(/*bEnableUndo=*/ true);

		{
			const FScopedTransaction Transaction(LOCTEXT("InstanceToolSelectParentComponents", "Select Parent Component)"));

			GEditor->SelectNone(false, true);

			USelection* SelectedComponents = GEditor->GetSelectedComponents();
			
			LastSelectedActor->Modify();
			GEditor->SelectActor(LastSelectedActor, true, true);
			
			SelectedComponents->Modify();
			
			SelectedComponents->BeginBatchSelectOperation();
			SelectedComponents->DeselectAll();
			for (auto Component : Selected)
			{
				if (Component->GetOwner() == LastSelectedActor)
				{
					SelectedComponents->Select(Component.Get());
				}
			}
			SelectedComponents->EndBatchSelectOperation();
			//GEditor->NoteSelectionChange();
		}
	}
}

void FInstanceToolEdMode::SelectByAxis(EAxis::Type Axis, bool bNegative /*= false*/, bool bShowMessage /*= false*/)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		if (bShowMessage)
		{
			NotifyMessage(LOCTEXT("SelectionLocked", "Selection was locked."));
		}
		return;
	}

	if (Selection.IsSelected())
	{
		if (!UISetting.bDisableUndo)
		{
			GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_SelectByAxis", "Select by Axis"));
		}

		const bool bSelectSameComponentOnly = UISetting.bMarqueeSelectSameActorOnly;
		const bool bWorldSpace = GLevelEditorModeTools().GetCoordSystem() == COORD_World;

		UInstancedStaticMeshComponent* LastSelectedComponent = Selection.GetLastSelectedComponent();
		AActor* LastSelectedActor = LastSelectedComponent->GetOwner();
		FTransform LastSelectedTM = Selection.GetLastSelectedTransform(true);
		int32 LastSelectedIndex = Selection.GetLastSelectedInstanceIndex();
		
		//SetSelectNone();
		Selection.SetSelectedNone();

		TArray<UInstancedStaticMeshComponent*> Components;
		LastSelectedActor->GetComponents<UInstancedStaticMeshComponent>(Components);

		int32 Neg = bNegative ? -1 : 1;

		for (auto& Component : Components)
		{
			if (bSelectSameComponentOnly && Component != LastSelectedComponent)
			{
				continue;
			}

			for (int32 Index = Component->PerInstanceSMData.Num() - 1; Index >= 0; --Index)
			{
				if (Index == LastSelectedIndex)
				{
					continue;
				}

				FTransform InstanceTM;
				Component->GetInstanceTransform(Index, InstanceTM, true);
				FTransform RelativeTM = InstanceTM.GetRelativeTransform(LastSelectedTM);

				FVector RelativeLoc = bWorldSpace ? InstanceTM.GetLocation() - LastSelectedTM.GetLocation() : RelativeTM.GetLocation();

				bool bShouldSelect = false;

				float SelectByAxisTolerance = UISetting.SelectByAxisTolerance;
				switch (Axis)
				{
				case EAxis::X:
					bShouldSelect = bNegative ? RelativeLoc.X <= SelectByAxisTolerance : RelativeLoc.X >= -1 * SelectByAxisTolerance;
					break;
				case EAxis::Y:
					bShouldSelect = bNegative ? RelativeLoc.Y <= SelectByAxisTolerance : RelativeLoc.Y >= -1 * SelectByAxisTolerance;
					break;
				case EAxis::Z:
					bShouldSelect = bNegative ? RelativeLoc.Z <= SelectByAxisTolerance : RelativeLoc.Z >= -1 * SelectByAxisTolerance;
					break;
				}

				if (bShouldSelect)
				{
					Selection.SelectInstance(true, Component, Index, true, false);
				}
			}

			Selection.SelectInstance(true, LastSelectedComponent, LastSelectedIndex, true, false);
		}

		Undo->Modify();
		Undo->UpdateSelected();

		if (!UISetting.bDisableUndo)
		{
			GEditor->EndTransaction();
		}

		if (bShowMessage)
		{
			FText Message = FText::Format(LOCTEXT("SelectedInstances", "{0} instances selected."), GetSelectedInstanceCount());
			NotifyMessage(Message);
		}

		BroadcastSelectionChanged();
	}
}

int32 FInstanceToolEdMode::SelectByLoc(bool bShowMessage /*= false*/)
{
	int32 SelectedCount = SelectByTransform(UE::Widget::WM_Translate, bShowMessage);
	return SelectedCount;
}

int32 FInstanceToolEdMode::SelectByRot(bool bShowMessage /*= false*/)
{
	int32 SelectedCount = SelectByTransform(UE::Widget::WM_Rotate, bShowMessage);
	return SelectedCount;
}

int32 FInstanceToolEdMode::SelectByScale(bool bShowMessage /*= false*/)
{
	int32 SelectedCount = SelectByTransform(UE::Widget::WM_Scale, bShowMessage);
	return SelectedCount;
}

int32 FInstanceToolEdMode::SelectByTransform(UE::Widget::EWidgetMode InMode, bool bShowMessage /*= false*/)
{
	int32 SelectedCount = 0;

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bLockSelection)
	{
		if (bShowMessage)
		{
			NotifyMessage(LOCTEXT("SelectionLocked", "Selection was locked."));
			return SelectedCount;
		}
	}

	if (Selection.IsSelected())
	{
		if (!UISetting.bDisableUndo)
		{
			GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_SelectInstances", "Select Instances"));
		}

		const bool bSelectSameComponentOnly = UISetting.bMarqueeSelectSameActorOnly;
		const bool bWorldSpace = false; // GLevelEditorModeTools().GetCoordSystem() == COORD_World;
		
		bool bAxisX = false;
		bool bAxisY = false;
		bool bAxisZ = false;
		if (InMode == UE::Widget::WM_Translate)
		{
			bAxisX = UISetting.bSelectByLocX;
			bAxisY = UISetting.bSelectByLocY;
			bAxisZ = UISetting.bSelectByLocZ;
		}
		if (InMode == UE::Widget::WM_Rotate)
		{
			bAxisX = UISetting.bSelectByRotX;
			bAxisY = UISetting.bSelectByRotY;
			bAxisZ = UISetting.bSelectByRotZ;
		}
		if (InMode == UE::Widget::WM_Scale)
		{
			bAxisX = UISetting.bSelectByScaleX;
			bAxisY = UISetting.bSelectByScaleY;
			bAxisZ = UISetting.bSelectByScaleZ;
		}

		UInstancedStaticMeshComponent* LastSelectedComponent = Selection.GetLastSelectedComponent();
		AActor* LastSelectedActor = LastSelectedComponent->GetOwner();
		FTransform LastSelectedTM = Selection.GetLastSelectedTransform(/*bWorldSpace=*/ false);
		int32 LastSelectedIndex = Selection.GetLastSelectedInstanceIndex();

		Selection.SetSelectedNone();

		TArray<UInstancedStaticMeshComponent*> Components;
		LastSelectedActor->GetComponents<UInstancedStaticMeshComponent>(Components);

		for (auto& Component : Components)
		{
			if (bSelectSameComponentOnly && Component != LastSelectedComponent)
			{
				continue;
			}

			for (int32 Index = Component->PerInstanceSMData.Num() - 1; Index >= 0; --Index)
			{
				if (Index == LastSelectedIndex)
				{
					continue;
				}

				FTransform InstanceTM;
				Component->GetInstanceTransform(Index, InstanceTM, /*bWolrdSpace=*/ false);

				FVector CompareA, CompareB, Tolerance;
				if (InMode == UE::Widget::WM_Translate)
				{
					CompareA = InstanceTM.GetLocation();
					CompareB = LastSelectedTM.GetLocation();
					Tolerance = FVector(UISetting.SelectByLocToleranceX, UISetting.SelectByLocToleranceY, UISetting.SelectByLocToleranceZ).GetAbs();
				}
				if (InMode == UE::Widget::WM_Rotate)
				{
					CompareA = FVector(InstanceTM.Rotator().Roll, InstanceTM.Rotator().Pitch, InstanceTM.Rotator().Yaw);
					CompareB = FVector(LastSelectedTM.Rotator().Roll, LastSelectedTM.Rotator().Pitch, LastSelectedTM.Rotator().Yaw);
					Tolerance = FVector(UISetting.SelectByRotToleranceX, UISetting.SelectByRotToleranceY, UISetting.SelectByRotToleranceZ).GetAbs();
				}
				if (InMode == UE::Widget::WM_Scale)
				{
					CompareA = InstanceTM.GetScale3D();
					CompareB = LastSelectedTM.GetScale3D();
					Tolerance = FVector(UISetting.SelectByScaleToleranceX, UISetting.SelectByScaleToleranceY, UISetting.SelectByScaleToleranceZ).GetAbs();
				}

				bool bShouldSelect = bAxisX || bAxisY || bAxisZ;

				if (bShouldSelect && bAxisX)
				{
					bShouldSelect = FMath::Abs(CompareA.X - CompareB.X) <= Tolerance.X;
				}
				if (bShouldSelect && bAxisY)
				{
					bShouldSelect = FMath::Abs(CompareA.Y - CompareB.Y) <= Tolerance.Y;
				}
				if (bShouldSelect && bAxisZ)
				{
					bShouldSelect = FMath::Abs(CompareA.Z - CompareB.Z) <= Tolerance.Z;
				}
			
				if (bShouldSelect)
				{
					Selection.SelectInstance(true, Component, Index, true, false);
					++SelectedCount;
				}
			}

			Selection.SelectInstance(true, LastSelectedComponent, LastSelectedIndex, true, false);
		}

		Undo->Modify();
		Undo->UpdateSelected();

		if (!UISetting.bDisableUndo)
		{
			GEditor->EndTransaction();
		}

		if (bShowMessage)
		{
			FText Message = FText::Format(LOCTEXT("SelectedInstances", "{0} instances selected."), GetSelectedInstanceCount());
			NotifyMessage(Message);
		}

		BroadcastSelectionChanged();
	}

	return SelectedCount;
}

void FInstanceToolEdMode::DeleteSelectedInstances()
{
	if (Selection.IsSelected())
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		if (!UISetting.bDisableUndo)
		{
			GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceToolDeleteSelectedInstances", "Instance Tool Delete Selected Instances"));
		}
		//const FScopedTransaction Transaction(LOCTEXT("InstanceToolDeleteSelectedInstances", "Instance Tool Delete Selected Instances"));

		Undo->UpdateInstancesSnapshot();
		Undo->Modify();

		TArray<AActor*> SelectedActors = Selection.GetSelectedActors();

		Selection.RemoveSelectedInstances();

		Undo->UpdateSelected();
		Undo->UpdateInstancesSnapshot();

		BroadcastSelectionChanged();

		if (!UISetting.bDisableUndo)
		{
			PropagateChangesToBlueprintInstances(SelectedActors);
			GEditor->EndTransaction();
		}

		NoteInstanceDataModified(SelectedActors);
	}
}

void FInstanceToolEdMode::DuplicateSelectedInstances()
{
	if (Selection.IsSelected())
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		if (!UISetting.bDisableUndo)
		{
			GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceToolDuplicateSelectedInstances", "Instance Tool Duplicate Instances"));
			PreAddInstanceForUndo();
		}

		struct DuplicateRecord
		{
			UInstancedStaticMeshComponent* Component;
			int32 InstanceIndex;
			DuplicateRecord(UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex)
				: Component(InComponent)
				, InstanceIndex(InInstanceIndex)
			{}
		};

		TArray<DuplicateRecord> Duplicated;
		for (auto& Proxy : Selection.GetSelectedProxyObjects())
		{
			int32 DuplicatedInstanceIndex = Proxy->Component->AddInstance(Proxy->InstanceToWorld, /*bWorldSpace*/ true);
			Duplicated.Add(DuplicateRecord(Proxy->Component, DuplicatedInstanceIndex));
		}

		Selection.SetSelectedNone(/*bBroadcastChange=*/ false);

		for (auto& DuplicateItem : Duplicated)
		{
			Selection.SelectInstance(true, DuplicateItem.Component, DuplicateItem.InstanceIndex, /*bMultiSelect=*/ true, /*bBroadcastChange=*/ false);
		}

		BroadcastSelectionChanged();

		if (!UISetting.bDisableUndo)
		{
			PostAddInstanceForUndo();

//			Undo->Modify();
			Undo->UpdateSelected();

			PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
			GEditor->EndTransaction();
		}

		NoteInstanceDataModified(Selection.GetSelectedActors());

		FText Message = FText::Format(LOCTEXT("DuplicateInstances", "{0} instances duplicated."), Duplicated.Num());
		NotifyMessage(Message, .3f);
	}
}

void FInstanceToolEdMode::PreAddInstanceForUndo()
{
	Undo->UpdateInstancesSnapshot();
	Undo->Modify();
}

void FInstanceToolEdMode::PostAddInstanceForUndo()
{
	Undo->UpdateInstancesSnapshot();
}

void FInstanceToolEdMode::FocusViewportOnBox(const FBox& BoundingBox) const
{
	// Make sure we had at least one non-null actor in the array passed in.
	if (BoundingBox.GetSize() != FVector::ZeroVector)
	{	
		if (GCurrentLevelEditingViewportClient)
		{
			GCurrentLevelEditingViewportClient->FocusViewportOnBox(BoundingBox);

			if (GCurrentLevelEditingViewportClient->IsOrtho() && GetDefault<ULevelEditorViewportSettings>()->bUseLinkedOrthographicViewports)
			{
				for (auto ViewportIt = GEditor->GetLevelViewportClients().CreateConstIterator(); ViewportIt; ++ViewportIt)
				{
					FLevelEditorViewportClient* LinkedViewportClient = *ViewportIt;
					// Only update other orthographic viewports
					if (LinkedViewportClient && LinkedViewportClient != GCurrentLevelEditingViewportClient && LinkedViewportClient->IsOrtho())
					{
						LinkedViewportClient->FocusViewportOnBox(BoundingBox);
					}
				}
			}
		}
	}
}

void FInstanceToolEdMode::FocusViewportOnSelection()
{
	if (Selection.IsSelected())
	{
		FocusViewportOnBox(Selection.GetSelectionBoundingBox());
	}
}

void FInstanceToolEdMode::AlignSelectionToGrid(UE::Widget::EWidgetMode InMode, bool bEnableUndo)
{
	if (!Selection.IsSelected())
	{
		return;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo && bEnableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_AlignTransaction", "InstanceTool Aligning"));
	}

	Undo->Modify();

	float CurrentGridSize = GEditor->GetGridSize();
	float CurrentScaleGridSize = GEditor->GetScaleGridSize();
	FRotator CurrentRotGridSize = GEditor->GetRotGridSize();

	for (auto& Proxy : Selection.GetSelectedProxyObjects())
	{
		FTransform RelativeTM;
		Proxy->Component->GetInstanceTransform(Proxy->InstanceIndex, RelativeTM, false);

		FVector RelativeLocation = RelativeTM.GetLocation();
		if (InMode == UE::Widget::WM_Translate || InMode == UE::Widget::WM_None)
		{
			RelativeLocation = RelativeLocation.GridSnap(CurrentGridSize);
		}

		FRotator RelativeRotation = RelativeTM.Rotator();
		if (InMode == UE::Widget::WM_Rotate || InMode == UE::Widget::WM_None)
		{
			RelativeRotation = RelativeRotation.GridSnap(CurrentRotGridSize);
		}

		FVector RelativeScale = RelativeTM.GetScale3D();
		if (InMode == UE::Widget::WM_Scale || InMode == UE::Widget::WM_None)
		{
			RelativeScale = RelativeScale.GridSnap(CurrentScaleGridSize);
		}

		FTransform NewRelativeTM(RelativeRotation, RelativeLocation, RelativeScale);
		Selection.TransformSelectedObject(Proxy, NewRelativeTM, /*bWorldSpace=*/ false);

		if (InMode == UE::Widget::WM_Scale || InMode == UE::Widget::WM_None)
		{
			Selection.ScaleSnapSelectedObject(Proxy, GEditor->GetScaleGridSize(), true);
		}
	}

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo && bEnableUndo)
	{
		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	NoteInstanceDataModified(Selection.GetSelectedActors());
}

void FInstanceToolEdMode::SnapSelection(EAxis::Type Axis, bool bNegative)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	const bool bSnapFollowLastSelected = UISetting.bSnapFollowLastSelected;
	const bool bUseOwnSnapDir = UISetting.bUseOwnSnapDir;

	if (!Selection.IsSelected())
	{
		return;
	}

	FTransform LastSelectedTransform = Selection.GetLastSelectedTransform();
	ECoordSystem CoordSystem = GLevelEditorModeTools().GetCoordSystem();
	FVector SnapDir = Selection.GetSnapDir(LastSelectedTransform, Axis, bNegative, CoordSystem);

	if (!UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_SnapTransaction", "InstanceTool Snapping"));
		Undo->Modify();
	}
	
	if (bSnapFollowLastSelected)
	{
		auto& Proxies = Selection.GetSelectedProxyObjects();
		FVector DeltaLoc = SnapSelectedObject(Proxies[Proxies.Num() - 1], SnapDir, Axis, bNegative);

		if (!DeltaLoc.Equals(FVector::ZeroVector))
		{
			for (int32 Index = Proxies.Num() - 2; Index >= 0; --Index)
			{
				FVector NewLocation = Proxies[Index]->InstanceToWorld.GetLocation() + DeltaLoc;
				Selection.TransformSelectedObjectToLocation(Proxies[Index], NewLocation);
			}
		}
	} 
	else
	{
		auto& SortedSelection = Selection.GetSelectionSortedByPosition(SnapDir);
		for (int32 Index = 0; Index < SortedSelection.Num(); ++Index)
		{
			if (bUseOwnSnapDir)
			{
				FTransform InstanceTransform;
				SortedSelection[Index]->Component->GetInstanceTransform(SortedSelection[Index]->InstanceIndex, InstanceTransform, true);
				SnapDir = Selection.GetSnapDir(InstanceTransform, Axis, bNegative, CoordSystem);
			}

			SnapSelectedObject(SortedSelection[Index]->Proxy, SnapDir, Axis, bNegative);
		}
	}

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo)
	{
		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	NoteInstanceDataModified(Selection.GetSelectedActors());
}

void FInstanceToolEdMode::DeltaTransformSelection(FTransform& InDeltaTransform)
{
	if (!Selection.IsSelected())
	{
		return;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_DeltaTransformTransaction", "InstanceTool Delta Transform"));

		if (UISetting.bDeltaTransformDuplicate)
		{
			PreAddInstanceForUndo();
		}
	}

	Undo->Modify();

	for (auto& Proxy : Selection.GetSelectedProxyObjects())
	{
		if (!UISetting.bDeltaTransformDuplicate)
		{
			FTransform NewTransform = GetDeltaedTransform(Proxy, InDeltaTransform);
			Selection.TransformSelectedObject(Proxy, NewTransform);
		}
		else
		{
			UInstanceToolEditorObject *Source = Proxy;
			UInstanceToolEditorObject *Duplicated;
			for (int32 i = 0; i < UISetting.DeltaTransformDuplicateCopies; ++i)
			{
				
				FTransform NewTransform = GetDeltaedTransform(Source, InDeltaTransform);
				int32 DuplicatedInstanceIndex = Source->Component->AddInstance(NewTransform, /*bWorldSpace*/ true);

				Duplicated = NewObject<UInstanceToolEditorObject>();
				Duplicated->Init(Source->Component, DuplicatedInstanceIndex);

				Source = Duplicated;
			}

			// Technically there's no selection change, but it might affect cached widget location, so force broadcasting for now
			BroadcastSelectionChanged();
		}
	}

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo)
	{
		if (UISetting.bDeltaTransformDuplicate)
		{
			PostAddInstanceForUndo();
		}

		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	NoteInstanceDataModified(Selection.GetSelectedActors());
}

FTransform FInstanceToolEdMode::GetDeltaedTransform(UInstanceToolEditorObject* InProxy, FTransform& InDeltaTransform)
{
	return GetDeltaedTransform(InProxy->Component->GetComponentToWorld(), InProxy->InstanceToWorld, InDeltaTransform);
}

FTransform FInstanceToolEdMode::GetDeltaedTransform(const FTransform& ComponentWorldTransfrom, const FTransform& InstanceWorldTransfrom, FTransform& InDeltaTransform)
{
	bool bLocalSpace = GLevelEditorModeTools().GetCoordSystem() == COORD_Local;
	FVector DeltaLoc = InDeltaTransform.GetLocation();

	FVector NewLocation;
	if (bLocalSpace)
	{
		NewLocation = InstanceWorldTransfrom.GetLocation() + FRotationMatrix(InstanceWorldTransfrom.Rotator()).TransformPosition(DeltaLoc);
	}
	else
	{
		NewLocation = InstanceWorldTransfrom.GetLocation() + DeltaLoc;
	}

	FQuat NewRotation;
	if (bLocalSpace)
	{
		FQuat InstanceRotation = InstanceWorldTransfrom.GetRotation();

		FQuat RotX(InstanceRotation.GetAxisX(), InDeltaTransform.GetRotation().X);
		FQuat RotY(InstanceRotation.GetAxisY(), InDeltaTransform.GetRotation().Y);
		FQuat RotZ(InstanceRotation.GetAxisZ(), InDeltaTransform.GetRotation().Z);

		FTransform RelativeTransform = InstanceWorldTransfrom.GetRelativeTransform(ComponentWorldTransfrom);
		FQuat RelativeRotation = RelativeTransform.GetRotation();

		FQuat ComponentRotation = ComponentWorldTransfrom.GetRotation();

		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		switch (UISetting.DeltaTransformRotationOrder)
		{
		case ERotationOrder::XZY:
			NewRotation = RotX * RotZ * RotY * RelativeRotation * ComponentRotation;
			break;
		case ERotationOrder::YXZ:
			NewRotation = RotY * RotX * RotZ * RelativeRotation * ComponentRotation;
			break;
		case ERotationOrder::YZX:
			NewRotation = RotY * RotZ * RotX * RelativeRotation * ComponentRotation;
			break;
		case ERotationOrder::ZXY:
			NewRotation = RotZ * RotX * RotY * RelativeRotation * ComponentRotation;
			break;
		case ERotationOrder::ZYX:
			NewRotation = RotZ * RotY * RotX * RelativeRotation * ComponentRotation;
			break;
		case ERotationOrder::XYZ:
		default:
			NewRotation = RotX * RotY * RotZ * RelativeRotation * ComponentRotation;
			break;
		}
	}
	else
	{
		NewRotation = InDeltaTransform.GetRotation() * InstanceWorldTransfrom.GetRotation();
	}

	FVector NewScale = InDeltaTransform.GetScale3D() * ComponentWorldTransfrom.GetScale3D() + InstanceWorldTransfrom.GetScale3D();

	FTransform NewTransform(NewRotation, NewLocation, NewScale);

	return NewTransform;
}

void FInstanceToolEdMode::SpawnInstnceAtWidgetLocation()
{
	struct Local
	{
		static UInstancedStaticMeshComponent* GetISMCFromLastSelectedActors()
		{
			if (AActor* LastSelectedActor = GEditor->GetSelectedActors()->GetBottom<AActor>())
			{
				if (IsValid(LastSelectedActor) && !LastSelectedActor->IsA(AInstancedFoliageActor::StaticClass()))
				{
					TArray<UInstancedStaticMeshComponent*> Components;
					LastSelectedActor->GetComponents<UInstancedStaticMeshComponent>(Components);
					if (Components.Num() > 0)
					{
						return Components[0];
					}
				}
			}
			return NULL;
		}
	};

	UInstancedStaticMeshComponent* CurrentISMC = Selection.GetLastSelectedComponent();

	if (!CurrentISMC)
	{
		CurrentISMC = Local::GetISMCFromLastSelectedActors();
	}

	if (!CurrentISMC)
	{
		return;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_SpawnInstncesAtWidget", "InstanceTool Spawn Instance"));
		PreAddInstanceForUndo();
	}

	Undo->Modify();

	FVector CurrentWidgetLocation = GetWidgetLocation();

	const bool bSelectSpawnedInstances = true;
	if (bSelectSpawnedInstances)
	{
		Selection.SetSelectedNone();
	}

	FTransform NewTM(CurrentWidgetLocation);
	int32 NewInstanceIndex = CurrentISMC->AddInstance(NewTM, /*bWorldSpace*/ true);

	if (bSelectSpawnedInstances)
	{
		Selection.SelectInstance(true, CurrentISMC, NewInstanceIndex, true, false);
	}

	BroadcastSelectionChanged();

	if (!UISetting.bDisableUndo)
	{
		Undo->UpdateSelected();
		PostAddInstanceForUndo();

		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	NoteInstanceDataModified(Selection.GetSelectedActors());
}

void FInstanceToolEdMode::SpawnInstncesOnSpline(class USplineComponent* InSplineComponent, FTransform& InOffsetTransform, bool bSpawnOnSplineIgnoreRotation, bool bSelectSpawnedInstances)
{
	UInstancedStaticMeshComponent* CurrentISMC = Selection.GetLastSelectedComponent();

	if (!CurrentISMC)
	{
		return;
	}

	if (!IsValid(InSplineComponent) || InSplineComponent->GetNumberOfSplinePoints() == 0)
	{
		return;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_SpawnInstncesOnSpline", "InstanceTool Spawn Instances"));
		PreAddInstanceForUndo();
	}

	Undo->Modify();

	const int32 NumPoints = InSplineComponent->GetNumberOfSplinePoints();

	if (bSelectSpawnedInstances)
	{
		Selection.SetSelectedNone();
	}

	for (int32 PointIndex = 0; PointIndex < NumPoints; ++PointIndex)
	{
		FTransform PointWorldTM = bSpawnOnSplineIgnoreRotation
			? FTransform(InSplineComponent->GetWorldLocationAtSplinePoint(PointIndex))
			: InSplineComponent->GetTransformAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
		FTransform NewTM = GetDeltaedTransform(CurrentISMC->GetComponentToWorld(), PointWorldTM, InOffsetTransform);
		int32 DuplicatedInstanceIndex = CurrentISMC->AddInstance(NewTM, /*bWorldSpace*/ true);

		if (bSelectSpawnedInstances)
		{
			Selection.SelectInstance(true, CurrentISMC, DuplicatedInstanceIndex, true, false);
		}
// 		UInstanceToolEditorObject *Duplicated = NewObject<UInstanceToolEditorObject>();
// 		Duplicated->Init(Source->Component, DuplicatedInstanceIndex);
	}

	BroadcastSelectionChanged();

	if (!UISetting.bDisableUndo)
	{
		Undo->UpdateSelected(); 
		PostAddInstanceForUndo();

		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	NoteInstanceDataModified(Selection.GetSelectedActors());
}

FVector FInstanceToolEdMode::SnapSelectedObject(class UInstanceToolEditorObject* InProxy, FVector& InNormalizedDir, EAxis::Type Axis, bool bNegative, int32 NumTries)
{
	FVector DeltaLocation = FVector::ZeroVector; 
	
	if (!InProxy || !InProxy->Component || !InProxy->Component->GetStaticMesh())
	{
		return DeltaLocation;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	const bool bUseBoxTrace = UISetting.SnapTraceOption == ESnapTraceOption::BoundingBoxTrace;
	//const bool bAlwaysUseLocalBoundForTracing = UISetting.bAlwaysUseLocalBoundForTracing;
	const bool bLocalSpace = UISetting.SnapCoord == EAlignSnapCoord::Auto
		? GLevelEditorModeTools().GetCoordSystem() == COORD_Local
		: UISetting.SnapCoord == EAlignSnapCoord::Local;
	const bool bDrawDebug = UISetting.bSnapDrawDebug;
	const float SnapOffset = UISetting.SnapOffset;
	const float TraceDistance = UISetting.SnapTraceDistance;

	const float DrawDebugDuration = 5.f;
	const float DrawDebugThickness = 1.0f;
	FColor DrawDebugTraceLineColor = FColor::White;

	const UWorld* World = GetWorld();

	FTransform InstanceWorldTM;
	InProxy->Component->GetInstanceTransform(InProxy->InstanceIndex, InstanceWorldTM, true);
	FVector InstanceStartLocation = InstanceWorldTM.GetLocation();

	FBox InstanceBound;
	FQuat Rot = FQuat::Identity;
	if (!bLocalSpace && !bUseBoxTrace) // Line Trace in World Space use World Bound
	{
		Selection.GetInstanceWorldBoundingBox(InProxy->Component, InProxy->InstanceIndex, InstanceBound);
	}
	else // Line Trace use Local Bound in Local Space; Box Trace always use Local Bound
	{
		Selection.GetInstanceLocalBoundingBox(InProxy->Component, InProxy->InstanceIndex, InstanceBound, Rot);
	}

	float Extent = 0.f;
	switch (Axis)
	{
	case EAxis::X:
		Extent = InstanceBound.GetExtent().X;
		DrawDebugTraceLineColor = FColor::Red;
		break;
	case EAxis::Y:
		Extent = InstanceBound.GetExtent().Y;
		DrawDebugTraceLineColor = FColor::Green;
		break;
	case EAxis::Z:
		Extent = InstanceBound.GetExtent().Z;
		DrawDebugTraceLineColor = FColor::Blue;
		break;
	}

	FVector TraceStartLocation = bUseBoxTrace? InstanceBound.GetCenter() : InstanceBound.GetCenter() + Extent * InNormalizedDir;
	FVector PivotOffset = TraceStartLocation - InstanceWorldTM.GetLocation();
	FVector TraceEndLocation = TraceStartLocation + TraceDistance * InNormalizedDir;
	FHitResult Hit;

	if (bDrawDebug)
	{
		DrawDebugBox(World, InstanceBound.GetCenter(), InstanceBound.GetExtent(), Rot, FColor::White, false, DrawDebugDuration, SDPG_World, DrawDebugThickness);
		DrawDebugLine(World, TraceStartLocation, TraceEndLocation, DrawDebugTraceLineColor, false, DrawDebugDuration, SDPG_World, DrawDebugThickness);
	}

	if (bUseBoxTrace ? InstanceBoxTrace(World, TraceStartLocation, TraceEndLocation, Hit, InstanceBound.GetExtent(), Rot, InProxy->InstanceIndex)
		: InstanceLineTrace(World, TraceStartLocation, TraceEndLocation, Hit, InProxy->InstanceIndex))
	{
		FVector FinalLocation = bUseBoxTrace
			? InstanceWorldTM.GetLocation() + (Hit.ImpactPoint - TraceStartLocation).ProjectOnTo(InNormalizedDir) - Extent * InNormalizedDir
			// InstanceWorldTM.GetLocation() + Hit.Distance * InNormalizedDir)
			: Hit.ImpactPoint - PivotOffset;

		if (UISetting.bAlwaysSnapToGrid)
		{
			FinalLocation = FinalLocation.GridSnap(GEditor->GetGridSize());
		}

		FinalLocation += InNormalizedDir * SnapOffset;

		DeltaLocation = FinalLocation - InstanceStartLocation;
		InstanceWorldTM.SetLocation(FinalLocation);

		if (UISetting.bRotateToHitNormal)
		{
			//FVector SnapDir = SelectionInfo.GetSnapDir(InstanceTransform, EAxis::Z, bNegative, ECoordSystem::COORD_Local);
			FVector SnapDir = Selection.GetSnapDir(InstanceWorldTM, Axis, bNegative, ECoordSystem::COORD_Local);
			InstanceWorldTM.SetRotation(FQuat::FindBetweenNormals(SnapDir, Hit.ImpactNormal * -1) * InstanceWorldTM.GetRotation());
		}

		Selection.TransformSelectedObject(InProxy, InstanceWorldTM, /*bWorldSpace=*/ true);
		//SelectionInfo.TransformSelectedObjectToLocation(InProxy, FinalLocation);

		if (bDrawDebug)
		{
			//DrawDebugSphere(World, Hit.Location, 5.0f, 16, FColor::Red, false, DrawDebugDuration, SDPG_World, DrawDebugThickness);
			DrawDebugSphere(World, Hit.ImpactPoint, 5.0f, 16, FColor::Yellow, false, DrawDebugDuration, SDPG_World, DrawDebugThickness);
			DrawDebugLine(World, Hit.ImpactPoint, Hit.ImpactPoint + 250.f * Hit.ImpactNormal, FColor::Yellow, false, DrawDebugDuration, SDPG_World, DrawDebugThickness);
		}

		if (bDrawDebug)// && bUseBoxTrace)
		{	
			Selection.GetInstanceLocalBoundingBox(InProxy->Component, InProxy->InstanceIndex, InstanceBound, Rot);
			if (!bLocalSpace && !bUseBoxTrace)
			{
				Selection.GetInstanceWorldBoundingBox(InProxy->Component, InProxy->InstanceIndex, InstanceBound); 
				Rot = FQuat::Identity;
			}
			DrawDebugBox(World, InstanceBound.GetCenter(), InstanceBound.GetExtent(), Rot, FColor::Silver, false, DrawDebugDuration, SDPG_World, DrawDebugThickness);
		}

		if (UISetting.bRotateToHitNormal && bUseBoxTrace && !UISetting.bSnapIgnoreStartPenetrating && NumTries==1)
		{
			ECoordSystem CoordSystem = GLevelEditorModeTools().GetCoordSystem();
			FVector SnapDir = Selection.GetSnapDir(InstanceWorldTM, Axis, bNegative, CoordSystem);
			DeltaLocation += SnapSelectedObject(InProxy, SnapDir, Axis, bNegative, ++NumTries);
		}
	}

	return DeltaLocation;
}

void FInstanceToolEdMode::AlignSelectionLocation(ECoordSystem InCoordSystem, EAxis::Type Axis, bool bNegative, bool bEnableUndo, bool bNotify)
{	
	if (!Selection.IsSelected())
	{
		return;
	}

	bool bWorldCoord = InCoordSystem == COORD_World;

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo && bEnableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_AlignTransaction", "InstanceTool Aligning"));
		Undo->Modify();
	}

	for (int32 Index = 0; Index < Selection.Selection.Num() - 1; ++ Index)
	{
		AlignSelectedObjectLocation(Selection.Selection[Index].Proxy, Selection.GetLastSelectedObject(), Axis, bWorldCoord);
	}

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo && bEnableUndo)
	{
		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	if (bNotify)
	{
		NoteInstanceDataModified(Selection.GetSelectedActors());
	}
}

void FInstanceToolEdMode::AlignSelectionRotation(EAxis::Type Axis, bool bEnableUndo, bool bNotify)
{
	if (!Selection.IsSelected())
	{
		return;
	}

	const bool bWorldCoord = true; // todo: if local rot alignment make any sense?
// 	bool bWorldCoord = UISetting.AlignCoord == EAlignSnapCoord::Auto
// 		? GLevelEditorModeTools().GetCoordSystem() == COORD_World
// 		: UISetting.AlignCoord == EAlignSnapCoord::World;

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo && bEnableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_AlignTransaction", "InstanceTool Aligning"));
		Undo->Modify();
	}

	for (int32 Index = 0; Index < Selection.Selection.Num() - 1; ++Index)
	{
		AlignSelectedObjectRotation(Selection.Selection[Index].Proxy, Selection.GetLastSelectedObject(), Axis, bWorldCoord);
	}

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo && bEnableUndo)
	{
		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	if (bNotify)
	{
		NoteInstanceDataModified(Selection.GetSelectedActors());
	}
}

// void FInstanceToolEdMode::ForceScaleAlign(bool bEnableUndo, bool bNotify)
// {
// 	if (!Selection.IsSelected())
// 	{
// 		return;
// 	}
// 
// 	if (UISetting.ForceScaleAlignOption == EForceScaleSnapOption::None)
// 	{
// 		return;
// 	}
// 
// 	if (!UISetting.bDisableUndo && bEnableUndo)
// 	{
// 		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_AlignTransaction", "InstanceTool Aligning"));
// 		Undo->Modify();
// 	}
// 
// 	float ForceScaleSnapSize = (UISetting.ForceScaleAlignOption == EForceScaleSnapOption::UseGridSnapValue) ?
// 		ForceScaleSnapSize = GEditor->GetScaleGridSize() : UISetting.ForceScaleAlignSize;
// 
// 	int32 TotalSelected = Selection.Selection.Num();
// 	for (int32 Index = 0; Index < TotalSelected; ++Index)
// 	{
// 		Selection.ScaleSnapSelectedObject(Selection.Selection[Index].Proxy, ForceScaleSnapSize);
// 	}
// 
// 	Undo->UpdateSelectedTransforms();
// 
// 	if (!UISetting.bDisableUndo && bEnableUndo)
// 	{
// 		GEditor->EndTransaction();
// 	}
// 
// 	if (bNotify)
// 	{
// 		NoteInstanceDataModified(Selection.GetSelectedActors());
// 	}
// }

void FInstanceToolEdMode::DistributeSelectionLocation(EAxis::Type Axis)
{
	if (!Selection.IsSelected() || GetSelectedInstanceCount() < 3)
	{
		return;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_DistributeTransaction", "InstanceTool Distributing"));
		Undo->Modify();
	}
	
	bool bDistributeIgnoreSameLoc = UISetting.bDistributeIgnoreSameLoc;
	if (Axis != EAxis::None)
	{
		DistributeSelectedObjectLocation(Axis, bDistributeIgnoreSameLoc);
	}
	else
	{
		DistributeSelectedObjectLocation(EAxis::X, bDistributeIgnoreSameLoc);
		DistributeSelectedObjectLocation(EAxis::Y, bDistributeIgnoreSameLoc);
		DistributeSelectedObjectLocation(EAxis::Z, bDistributeIgnoreSameLoc);
	}

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo)
	{
		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	NoteInstanceDataModified(Selection.GetSelectedActors());
}

void FInstanceToolEdMode::LineUpSelectionLocationFirstLast()
{
	if (!Selection.IsSelected() || GetSelectedInstanceCount() < 3)
	{
		return;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_DistributeTransaction", "InstanceTool Distributing"));
		Undo->Modify();
	}

	auto& First = Selection.Selection[0];
	auto& Last = Selection.Selection.Last();
	FVector FirstLoc = Selection.GetInstanceLocation(First.Component, First.InstanceIndex, /*bWorldSpace=*/ true);
	FVector LastLoc = Selection.GetInstanceLocation(Last.Component, Last.InstanceIndex, /*bWorldSpace=*/ true);
	FVector DistributeDir = (LastLoc - FirstLoc).GetSafeNormal();
	if (DistributeDir == FVector::ZeroVector)
	{
		return;
	}

	int32 SelectedCount = Selection.Selection.Num();	
	float Length = (LastLoc - FirstLoc).Size();
	float Space = SelectedCount - 1;
	float Delta = Length / Space;

	// From first to last
	for (int32 Index = 1; Index < SelectedCount - 1; ++Index)
	{
		FVector FinalLocation = FirstLoc + Index * Delta * DistributeDir;
		Selection.TransformSelectedObjectToLocation(Selection.Selection[Index].Proxy, FinalLocation);
	}

	if (UISetting.bLineUpAlignRotation)
	{
		AlignSelectionRotation(EAxis::None, /*bEnableUndo=*/ false, /*bNotify=*/ false);
	}

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo)
	{
		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	NoteInstanceDataModified(Selection.GetSelectedActors());
}

void FInstanceToolEdMode::LineUpSelectionLocation(EAxis::Type Axis)
{
	int32 SelectedCount = GetSelectedInstanceCount();
	if (!Selection.IsSelected() || SelectedCount < 2)
	{
		return;
	}

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_LineUpTransaction", "InstanceTool Line Up Instances"));
		Undo->Modify();
	}

	ECoordSystem CoordSystem = UISetting.DistributeCoord == EAlignSnapCoord::Auto
		? GLevelEditorModeTools().GetCoordSystem()
		: (UISetting.DistributeCoord == EAlignSnapCoord::Local) ? COORD_Local : COORD_World;

	// Align
	if (Axis == EAxis::X)
	{
		AlignSelectionLocation(CoordSystem, EAxis::Y, /*bNegative=*/ false, /*bEnableUndo=*/ false, /*bNotify=*/ false);
		AlignSelectionLocation(CoordSystem, EAxis::Z, /*bNegative=*/ false, /*bEnableUndo=*/ false, /*bNotify=*/ false);
		if (SelectedCount >= 3)
		{
			DistributeSelectedObjectLocation(EAxis::X, /*bDistributeIgnoreSameLoc=*/ false);
		}
	}
	else if (Axis == EAxis::Y)
	{
		AlignSelectionLocation(CoordSystem, EAxis::Z, /*bNegative=*/ false, /*bEnableUndo=*/ false, /*bNotify=*/ false);
		AlignSelectionLocation(CoordSystem, EAxis::X, /*bNegative=*/ false, /*bEnableUndo=*/ false, /*bNotify=*/ false);
		if (SelectedCount >= 3)
		{
			DistributeSelectedObjectLocation(EAxis::Y, /*bDistributeIgnoreSameLoc=*/ false);
		}
	}
	else if (Axis == EAxis::Z)
	{
		AlignSelectionLocation(CoordSystem, EAxis::X, /*bNegative=*/ false, /*bEnableUndo=*/ false, /*bNotify=*/ false);
		AlignSelectionLocation(CoordSystem, EAxis::Y, /*bNegative=*/ false, /*bEnableUndo=*/ false, /*bNotify=*/ false);
		if (SelectedCount >= 3)
		{
			DistributeSelectedObjectLocation(EAxis::Z, /*bDistributeIgnoreSameLoc=*/ false);
		}
	}

	if (UISetting.bLineUpAlignRotation)
	{
		AlignSelectionRotation(EAxis::None, /*bEnableUndo=*/ false, /*bNotify=*/ false);
	}

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo)
	{
		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	NoteInstanceDataModified(Selection.GetSelectedActors());
}

void FInstanceToolEdMode::TransformInstance(UInstanceToolEditorObject* InProxy, bool bNoteModified)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (!UISetting.bDisableUndo)
	{
		GEditor->BeginTransaction(NSLOCTEXT("UnrealEd", "InstanceTool_InstanceToolTransform", "InstanceTool Transform"));
		Undo->Modify();
	}

	Selection.TransformSelectedObject(InProxy, InProxy->InstanceToWorld, true);

	Undo->UpdateSelectedTransforms();

	if (!UISetting.bDisableUndo)
	{
		PropagateChangesToBlueprintInstances(Selection.GetSelectedActors());
		GEditor->EndTransaction();
	}

	if (bNoteModified)
	{
		NoteInstanceDataModified(Selection.GetSelectedActors());
	}
}

void FInstanceToolEdMode::AlignSelectedObjectLocation(class UInstanceToolEditorObject* InTargetProxy, class UInstanceToolEditorObject* InSourceProxy, EAxis::Type Axis, bool bWorldSpace)
{
	if (!InTargetProxy || !InTargetProxy->Component || !InTargetProxy->Component->GetStaticMesh())
	{
		return;
	}

	if (!InSourceProxy || !InSourceProxy->Component || !InSourceProxy->Component->GetStaticMesh())
	{
		return;
	}

	FTransform SourceWorldTM; 
	InSourceProxy->Component->GetInstanceTransform(InSourceProxy->InstanceIndex, SourceWorldTM, true);
	FTransform SourceRelativeTM;
	InSourceProxy->Component->GetInstanceTransform(InSourceProxy->InstanceIndex, SourceRelativeTM, false);
	FTransform SourceTM = bWorldSpace ? SourceWorldTM : SourceRelativeTM;
	FVector SourceLocation = SourceTM.GetLocation();

	FTransform TargetWroldTM;
	InTargetProxy->Component->GetInstanceTransform(InTargetProxy->InstanceIndex, TargetWroldTM, true);
	FTransform TargetRelativeTM = TargetWroldTM.GetRelativeTransform(SourceWorldTM);
	FTransform TargetTM = bWorldSpace ? TargetWroldTM : TargetRelativeTM;
	FVector TargetLocation = TargetTM.GetLocation();
	
	switch (Axis)
	{
	case EAxis::X:
		TargetLocation.X = bWorldSpace ? SourceLocation.X : 0;
		break;
	case EAxis::Y:
		TargetLocation.Y = bWorldSpace ? SourceLocation.Y : 0;
		break;
	case EAxis::Z:
		TargetLocation.Z = bWorldSpace ? SourceLocation.Z : 0;
		break;
	case EAxis::None: // treated as all axies
		TargetLocation = bWorldSpace ? SourceLocation : FVector::ZeroVector;
		break;
	}

	FVector InstanceWorldLocation = bWorldSpace ? TargetLocation : SourceWorldTM.TransformPosition(TargetLocation);
	Selection.TransformSelectedObjectToLocation(InTargetProxy, InstanceWorldLocation);
}

void FInstanceToolEdMode::AlignSelectedObjectRotation(class UInstanceToolEditorObject* InTargetProxy, class UInstanceToolEditorObject* InSourceProxy, EAxis::Type Axis, bool bWorldSpace)
{
	if (!InTargetProxy || !InTargetProxy->Component || !InTargetProxy->Component->GetStaticMesh())
	{
		return;
	}

	if (!InSourceProxy || !InSourceProxy->Component || !InSourceProxy->Component->GetStaticMesh())
	{
		return;
	}

	FTransform SourceWorldTM;
	InSourceProxy->Component->GetInstanceTransform(InSourceProxy->InstanceIndex, SourceWorldTM, true);
	FTransform SourceRelativeTM;
	InSourceProxy->Component->GetInstanceTransform(InSourceProxy->InstanceIndex, SourceRelativeTM, false);
	FTransform SourceTM = bWorldSpace ? SourceWorldTM : SourceRelativeTM;
	FRotator SourceRotator = SourceTM.Rotator();

	FTransform TargetWroldTM;
	InTargetProxy->Component->GetInstanceTransform(InTargetProxy->InstanceIndex, TargetWroldTM, true);
	FTransform TargetRelativeTM = TargetWroldTM.GetRelativeTransform(SourceWorldTM);
	FTransform TargetTM = bWorldSpace ? TargetWroldTM : TargetRelativeTM;
	FRotator TargetRotator = TargetTM.Rotator();

	switch (Axis)
	{
	case EAxis::X:
		TargetRotator.Roll = bWorldSpace ? SourceRotator.Roll : 0;
		break;
	case EAxis::Y:
		TargetRotator.Pitch = bWorldSpace ? SourceRotator.Pitch : 0;
		break;
	case EAxis::Z:
		TargetRotator.Yaw = bWorldSpace ? SourceRotator.Yaw : 0;
		break;
	case EAxis::None: // treated as all axies
		TargetRotator = bWorldSpace ? SourceRotator : FRotator::ZeroRotator;
		break;
	}

	FQuat InstanceWorldRotation = bWorldSpace ? TargetRotator.Quaternion() : SourceWorldTM.GetRotation().Inverse() * TargetRotator.Quaternion();

	Selection.TransformSelectedObjectToRotation(InTargetProxy, InstanceWorldRotation);
}

void FInstanceToolEdMode::DistributeSelectedObjectLocation(EAxis::Type Axis, bool bDistributeIgnoreSameLoc)
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	float DistributeIgnoreSameLocTolerance = UISetting.DistributeIgnoreSameLocTolerance;

	ECoordSystem CoordSystem = UISetting.DistributeCoord == EAlignSnapCoord::Auto
		? GLevelEditorModeTools().GetCoordSystem()
		: (UISetting.DistributeCoord == EAlignSnapCoord::Local) ? ECoordSystem::COORD_Local : ECoordSystem::COORD_World;

	FTransform LastSelectedTransform = Selection.GetLastSelectedTransform();
	FVector SnapDir = Selection.GetSnapDir(LastSelectedTransform, Axis, /*bNegative=*/ false, CoordSystem);
	auto& SortedSelection = Selection.GetSelectionSortedByPosition(SnapDir);

	int32 SelectedCount = SortedSelection.Num();

	int32 UniqueLocItems = 0;
	FVector UniqueLoc = FVector::ZeroVector;
	if (bDistributeIgnoreSameLoc)
	{
		for (int32 Index = 0; Index <= SelectedCount - 1; ++Index)
		{
			auto Item = SortedSelection[Index];
			FVector ItemLocation = Selection.GetInstanceLocation(Item->Component, Item->InstanceIndex, true);
			if (Index == 0
				|| (ItemLocation - UniqueLoc).ProjectOnTo(SnapDir).Size() > DistributeIgnoreSameLocTolerance)
			{
				UniqueLoc = ItemLocation;
				++UniqueLocItems;
				continue;
			}
		}

		if (UniqueLocItems <= 2)
		{
			return;
		}
	}

	FTransform 	MaxTM, MinTM;
	SortedSelection[0]->Component->GetInstanceTransform(SortedSelection[0]->InstanceIndex, MaxTM, true);
	SortedSelection.Last()->Component->GetInstanceTransform(SortedSelection.Last()->InstanceIndex, MinTM, true);

	FVector MaxLoc = MaxTM.GetLocation();
	FVector MinLoc = MinTM.GetLocation();

	float Length = (MaxLoc - MinLoc).ProjectOnTo(SnapDir).Size();
	float Space = bDistributeIgnoreSameLoc ? float(UniqueLocItems - 1) : float(SelectedCount - 1);
	float Delta = Length / Space;

	// From max to min
	UniqueLoc = MaxLoc;
	UniqueLocItems = 0;
	float LastDiff = 0.f;
	for (int32 Index = 1; Index <= SelectedCount - 1; ++Index)
	{
		FVector SelectedLoc = Selection.GetInstanceLocation(SortedSelection[Index]->Component, SortedSelection[Index]->InstanceIndex, true);
		float Diff = 0.f;

		if (bDistributeIgnoreSameLoc)
		{
			if ((SelectedLoc - UniqueLoc).ProjectOnTo(SnapDir).Size() <= DistributeIgnoreSameLocTolerance)
			{
				Diff = LastDiff;
			}
			else {
				++UniqueLocItems;
				UniqueLoc = SelectedLoc;

				Diff = (MaxLoc - SelectedLoc).ProjectOnTo(SnapDir).Size() - UniqueLocItems * Delta;
			}
		}
		else
		{
			Diff = (MaxLoc - SelectedLoc).ProjectOnTo(SnapDir).Size() - Index * Delta;
		}
		FVector FinalLocation = SelectedLoc + Diff * SnapDir;
		LastDiff = Diff;

		Selection.TransformSelectedObjectToLocation(SortedSelection[Index]->Proxy, FinalLocation);
	}
}

void FInstanceToolEdMode::NoteInstanceDataModified(const TArray<AActor*>& InActors)
{
	InvalidateLightingCache();

	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	if (UISetting.bAllowBlueprintEditing)
	{
		if (UISetting.bAlwaysApplyInstanceChangesToBlueprint)
		{
			TryApplyChangesToBlueprint(InActors);
		}
	}
}

void FInstanceToolEdMode::NotifyMessage(const FText& Message, float InDuration)
{
	if (!Message.IsEmpty())
	{
		FNotificationInfo NotificationInfo(Message);
		NotificationInfo.ExpireDuration = InDuration;
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}
}

bool FInstanceToolEdMode::InstanceBoxTrace(const UWorld* InWorld, const FVector& InStartTrace, const FVector& InEndTrace, FHitResult& OutHit, const FVector& TraceBoxExtent, const FQuat& InRot, int32 InInstanceIndex) const
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	const bool bIgnoreStartPenetrating = UISetting.bSnapIgnoreStartPenetrating;

	static FName NAME_InstanceTrance = FName("InstanceToolTrace");
	FCollisionQueryParams QueryParams(NAME_InstanceTrance, /*bInTraceComplex=*/true);
	QueryParams.bFindInitialOverlaps = true;

	TArray<FHitResult> Hits;

	FVector Dir = (InEndTrace - InStartTrace).GetSafeNormal();

	FCollisionShape CollisionShape;

	CollisionShape.SetBox((FVector3f)TraceBoxExtent);
	InWorld->SweepMultiByObjectType(Hits, InStartTrace, InEndTrace, InRot, FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic)), CollisionShape, QueryParams);

#if 0
	int32 Count = 0;
	for (const FHitResult& Hit : Hits)
	{
		if (Hit.bStartPenetrating) {
			++Count;
			float Angle = FVector::DotProduct(Dir, Hit.ImpactNormal);
			UE_LOG(LogInstanceTool, Display, TEXT("Item [%d], Angle [%g]"), Hit.Item, Angle)
			DrawDebugLine(InWorld, Hit.ImpactPoint, Hit.ImpactPoint + 550.f * Hit.ImpactNormal,
				Count == 1 ? FColor::Red : Count == 2 ? FColor::Green : Count == 3 ? FColor::Blue : Count == 4 ? FColor::Yellow : FColor::White
				, false, 20.f, SDPG_World, 10.f);
		}
	}
#endif

	for (const FHitResult& Hit : Hits)
	{
		if (Hit.bStartPenetrating) 
		{
			// Always ignore self penetrating
			// todo: edge case: check for different component, same instanceindex
			if (Hit.Item == InInstanceIndex)
			{
				continue;
			}

 			float Angle = FVector::DotProduct(Dir, Hit.ImpactNormal);
			if (Angle < 0 && !bIgnoreStartPenetrating)
			{
				return false;
			}
			else
			{
				continue;
			}
		}

		const UPrimitiveComponent* HitComponent = Hit.Component.Get();
		if (HitComponent && HitComponent->GetComponentLevel())
		{
			OutHit = Hit;
			return true;
		}
	}

	return false;
}

bool FInstanceToolEdMode::InstanceLineTrace(const UWorld* InWorld, const FVector& InStartTrace, const FVector& InEndTrace, FHitResult& OutHit, int32 InInstanceIndex) const
{
	UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

	const bool bIgnoreStartPenetrating = UISetting.bSnapIgnoreStartPenetrating;

	static FName NAME_InstanceTrance = FName("InstanceToolTrace");
	FCollisionQueryParams QueryParams(NAME_InstanceTrance, /*bInTraceComplex=*/true);
	QueryParams.bFindInitialOverlaps = true;

	FVector StartTrace = InStartTrace - 0.01f * (InEndTrace - InStartTrace).GetSafeNormal();

	TArray<FHitResult> Hits;
	InWorld->LineTraceMultiByObjectType(Hits, StartTrace, InEndTrace, FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic)), QueryParams);

	for (const FHitResult& Hit : Hits)
	{

		if (Hit.bStartPenetrating && Hit.Item == InInstanceIndex)
		{
			continue;
		}

		if (Hit.bStartPenetrating && Hit.Item != InInstanceIndex && !bIgnoreStartPenetrating)
		{
			return false;
		}

		const UPrimitiveComponent* HitComponent = Hit.Component.Get();

		if (HitComponent && HitComponent->GetComponentLevel())
		{
			OutHit = Hit;
			return true;
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////
// FSelectedInfo

const TArray<class UInstanceToolEditorObject*> FSelectionInfo::GetSelectedProxyObjects() const
{
	TArray<class UInstanceToolEditorObject*> Objects;
	for (auto& Item : Selection)
	{
		Objects.Add(Item.Proxy);
	}
	return Objects;
}

const TArray<AActor*> FSelectionInfo::GetSelectedActors() const
{
	TArray<AActor*> Actors;

	if (IsSelected())
	{
		for (auto& Item : Selection)
		{
			if (Item.Component && Item.Component->GetOwner())
			{
				Actors.AddUnique(Item.Component->GetOwner());
			}
		}
	}

	return Actors;
}

TArray<TWeakObjectPtr<UInstancedStaticMeshComponent>> FSelectionInfo::GetSelectedComponents() const
{
	TArray<TWeakObjectPtr<UInstancedStaticMeshComponent>> InstanceComponents;
	
	if (IsSelected())
	{
		for (auto& Item : Selection)
		{
			auto Precicate = [&](const TWeakObjectPtr<UInstancedStaticMeshComponent>& Other) {
				return Other.Get() == Item.Component;
			};

			if (Item.Component && !InstanceComponents.ContainsByPredicate(Precicate))
			{
				InstanceComponents.Add(Item.Component);
			}
		}
	}

	return InstanceComponents;
}

void FSelectionInfo::ValidateSelection()
{
	if (IsSelected())
	{
		for (int32 Index = Selection.Num() - 1; Index >= 0; --Index)
		{
			FSelectItem& Item = Selection[Index];
			if (!Item.Component 
//				|| !Item.Component->SelectedInstances.IsValidIndex(Item.InstanceIndex) 
				|| !Item.Component->PerInstanceSMData.IsValidIndex(Item.InstanceIndex))
			{
				Selection.RemoveAt(Index);
			}
		}
	}
}

class UInstanceToolEditorObject* FSelectionInfo::GetLastSelectedObject() const
{
	if (IsSelected() && Selection.Num() > 0)
	{
		return Selection.Last().Proxy;
	}
	return nullptr;
}

FTransform FSelectionInfo::GetLastSelectedTransform(bool bWorldSpace) const
{
	FTransform Transform;
	if (IsSelected() && GetLastSelectedComponent())
	{
		GetLastSelectedComponent()->GetInstanceTransform(GetLastSelectedInstanceIndex(), Transform, bWorldSpace);
	}
	return Transform;
}

UInstancedStaticMeshComponent* FSelectionInfo::GetLastSelectedComponent() const
{
	if (IsSelected() && Selection.Num() > 0)
	{
		return Selection.Last().Component;
	}
	return nullptr;
}

int32 FSelectionInfo::GetLastSelectedInstanceIndex() const
{
	if (IsSelected() && Selection.Num() > 0)
	{
		return	Selection.Last().InstanceIndex;
	}
	return -1;
}

FVector FSelectionInfo::GetInstanceLocation(UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex, bool bWorldSpace /*= true*/) const
{
	FTransform InstanceTM;
	InComponent->GetInstanceTransform(InInstanceIndex, InstanceTM, bWorldSpace);
	return InstanceTM.GetLocation();
}

FVector FSelectionInfo::GetSelectionLocation() const
{
	return GetLastSelectedTransform().GetLocation();
}

void FSelectionInfo::GetInstanceWorldBoundingBox(UInstancedStaticMeshComponent* InComponent, uint32 InIndex, FBox& OutBox) const
{
	if (InComponent && InComponent->GetStaticMesh())
	{
		FTransform InstanceWorldTM;
		InComponent->GetInstanceTransform(InIndex, InstanceWorldTM, /*bWorldSpace=*/ true);
		FBox RenderBounds = InComponent->GetStaticMesh()->GetBoundingBox();
		OutBox = RenderBounds.TransformBy(InstanceWorldTM);
	}
}

void FSelectionInfo::GetInstanceLocalBoundingBox(UInstancedStaticMeshComponent* InComponent, uint32 InIndex, FBox& OutBox, FQuat& OutQuat) const
{
	if (InComponent && InComponent->GetStaticMesh())
	{
		FTransform InstanceWorldTM;
		InComponent->GetInstanceTransform(InIndex, InstanceWorldTM, true);
		FBox RenderBounds = InComponent->GetStaticMesh()->GetBoundingBox();
		FBox WourldBound = RenderBounds.TransformBy(InstanceWorldTM);

		FTransform InstanceTransformNoRot = InstanceWorldTM;
		InstanceTransformNoRot.SetRotation(FQuat::Identity);

		FBox InstanceBoundNoRot = RenderBounds.TransformBy(InstanceTransformNoRot);
		InstanceBoundNoRot = InstanceBoundNoRot.MoveTo(WourldBound.GetCenter());

		OutBox = InstanceBoundNoRot;
		OutQuat = InstanceWorldTM.GetRotation();		
	}
}

FBox FSelectionInfo::GetSelectionBoundingBox() const
{
	FBox SelectionBound(ForceInit);

	if (IsSelected())
	{

		for (auto& Actor : GetSelectedActors()) 
		{
			TArray<UInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UInstancedStaticMeshComponent>(Components);

			if (Components.Num() > 0)
			{
				bool bHasAnyComponentBound = false;

				for (auto& ComponentIter : Components)
				{
					if (!ComponentIter->GetStaticMesh()) 
					{
						continue;
					}

					bool bHasAnyInstanceBound = false;
					FBox ComponentBound(ForceInit);

					FTransform BoundTransform = ComponentIter->GetComponentTransform();
					FMatrix BoundTransformMatrix = BoundTransform.ToMatrixWithScale();
					FBox RenderBounds = ComponentIter->GetStaticMesh()->GetBoundingBox();

					for (int32 InstanceIndex = 0; InstanceIndex < ComponentIter->SelectedInstances.Num(); ++InstanceIndex)
					{
						const FBox InstanceBound = RenderBounds.TransformBy(ComponentIter->PerInstanceSMData[InstanceIndex].Transform * BoundTransformMatrix);
						if (ComponentIter->SelectedInstances[InstanceIndex])
						{
							if (bHasAnyInstanceBound)
							{
								ComponentBound = ComponentBound + InstanceBound;
							}
							else
							{
								ComponentBound = InstanceBound;
								bHasAnyInstanceBound = true;
							}
						}
					}

					if (bHasAnyComponentBound)
					{
						SelectionBound = SelectionBound + ComponentBound;
					}
					else
					{
						SelectionBound = ComponentBound;
						bHasAnyComponentBound = true;
					}

				}
			}
		}
		
	}

	return SelectionBound;
}

FVector FSelectionInfo::GetSnapDir(FTransform& InInstanceTransform, EAxis::Type Axis, bool bNegative, ECoordSystem InCoordSystem) const
{
	bool bWorldCoord = InCoordSystem == COORD_World;

	FVector ForwardVector = bWorldCoord ? FVector::ForwardVector : InInstanceTransform.GetRotation().GetForwardVector();
	FVector RightVector = bWorldCoord ? FVector::RightVector : InInstanceTransform.GetRotation().GetRightVector();
	FVector UpVector = bWorldCoord ? FVector::UpVector : InInstanceTransform.GetRotation().GetUpVector();

	FVector Dir(ForceInitToZero);
	int Neg = bNegative ? -1 : 1;
	switch (Axis)
	{
	case EAxis::X:
		Dir = ForwardVector * Neg;
		break;
	case EAxis::Y:
		Dir = RightVector * Neg;
		break;
	case EAxis::Z:
		Dir = UpVector * Neg;
		break;
	}

	return Dir;
}

void FSelectionInfo::SetSelectedNone(bool bBroadcastChange)
{
	if (IsSelected())
	{
		for (auto& Actor : GetSelectedActors())
		{
			ClearInstanceSelection(Actor);
		}
	}

	Selection.Empty();

	if (bBroadcastChange)
	{
		ParentMode->BroadcastSelectionChanged();
	}

	//InstanceToolUtilities::RunGC();
}

void FSelectionInfo::SelectAllInstances(AActor* InActor, bool bMultiSelect)
{
	check(InActor)

	if (!bMultiSelect)
	{
		SetSelectedNone();
	}

	TArray<UInstancedStaticMeshComponent*> Components;
	InActor->GetComponents<UInstancedStaticMeshComponent>(Components);

	for (auto& Component : Components)
	{	
		Component->SelectInstance(true, 0, Component->GetInstanceCount());
		Component->MarkRenderStateDirty();

		for (int32 Index = 0; Index < Component->PerInstanceSMData.Num(); ++Index)
		{
			auto Precicate = [&](const FSelectItem& Other) {
				return Other.Component == Component && Other.InstanceIndex == Index;
			};

			if (!Selection.ContainsByPredicate(Precicate))
			{
				Selection.Emplace(Component, Index);
			}
		}
	}
}

void FSelectionInfo::SelectAllInstances(UInstancedStaticMeshComponent* InComponent, bool bMultiSelect)
{
	check(InComponent)

	if (!bMultiSelect)
	{
		SetSelectedNone();
	}

	InComponent->SelectInstance(true, 0, InComponent->GetInstanceCount());
	InComponent->MarkRenderStateDirty();

	for (int32 Index = 0; Index < InComponent->PerInstanceSMData.Num(); ++Index)
	{
		auto Precicate = [&](const FSelectItem& Other) {
			return Other.Component == InComponent && Other.InstanceIndex == Index;
		};

		if (!Selection.ContainsByPredicate(Precicate))
		{
			Selection.Emplace(InComponent, Index);
		}
	}
}

void FSelectionInfo::SelectInstance(bool InSelected, UInstancedStaticMeshComponent* InComponent, int32 InInstanceIndex, bool bMultiSelect, bool bBroadcastChange)
{
	AActor* Actor = InComponent->GetOwner();
	//check(Actor && InComponent && InInstanceIndex >= 0);

	if (!InComponent->PerInstanceSMData.IsValidIndex(InInstanceIndex))
	{
		return;
	}

	if (InSelected == InComponent->IsInstanceSelected(InInstanceIndex))
	{
		return;
	}
	
	if (InSelected)
	{
		GEditor->SelectNone(/*bNoteSelectionChange=*/ true, /*bDeselectBSPSurfs=*/ true);
	}

	InComponent->SelectInstance(InSelected, InInstanceIndex, 1);
	InComponent->MarkRenderStateDirty();

	// Last Instance been De-selected
	if (!InSelected && !bMultiSelect && !HasInstanceSelected(Actor))
	{
		SetSelectedNone();
		return;
	}

	// Selected actor changed
#if 0 // Allow select across actors
	if (InSelected && IsSelected() && SelectedActor != InActor)
	{
		ClearInstanceSelection(SelectedActor);
		Selection.Empty();
	}
#endif

	if (!bMultiSelect)
	{
		for (auto& SelectedActor : GetSelectedActors())
		{
			if (SelectedActor != Actor) {
				ClearInstanceSelection(SelectedActor);
			}
		}
		Selection.Empty();
	}

	int32 Index = Selection.IndexOfByPredicate([&](const FSelectItem& Other){
		return Other.Component == InComponent && Other.InstanceIndex == InInstanceIndex;
	});
	
	if (Index >= 0)
	{
		Selection.RemoveAt(Index);
	}

	if (InSelected)
	{
		Selection.Emplace(InComponent, InInstanceIndex);
	}

	if (bBroadcastChange)
	{
		ParentMode->BroadcastSelectionChanged(); 
	}
}

void FSelectionInfo::TransformSelectedObject(UInstanceToolEditorObject* InProxy, FTransform& InNewTransform, bool bWorldSpace /*= true*/, bool bMarkRenderStateDirty /*=true*/) const
{
	if (IsSelected() && InProxy)
	{
		UInstanceToolEditorUISetting& UISetting = FInstanceToolModule::GetInstanceToolSetting();

		bool bForceScaleSnap = UISetting.bAutoAlignScaleToGrid;

		InProxy->UpdateInstanceTransform(InNewTransform, bWorldSpace, bForceScaleSnap ? false : bMarkRenderStateDirty);

		if (bForceScaleSnap)
		{
 			float ForceScaleSnapSize = GEditor->GetScaleGridSize();
 			ScaleSnapSelectedObject(InProxy, ForceScaleSnapSize);
		}
	}
}

void FSelectionInfo::TransformSelectedObjectToLocation(UInstanceToolEditorObject* InProxy, FVector& NewWorldLocation, bool bWorldSpace) const
{
	if (IsSelected() && InProxy)
	{
		FTransform InstanceTM;
		InProxy->Component->GetInstanceTransform(InProxy->InstanceIndex, InstanceTM, bWorldSpace);

		InstanceTM.SetLocation(NewWorldLocation);

		TransformSelectedObject(InProxy, InstanceTM, /*bWorldSpace=*/ bWorldSpace);
	}
}

void FSelectionInfo::TransformSelectedObjectToRotation(class UInstanceToolEditorObject* InProxy, FQuat& NewWorldRotation) const
{
	if (IsSelected() && InProxy)
	{
		FTransform WorldTM;
		InProxy->Component->GetInstanceTransform(InProxy->InstanceIndex, WorldTM, true);

		WorldTM.SetRotation(NewWorldRotation);

		TransformSelectedObject(InProxy, WorldTM, /*bWorldSpace=*/ true);
	}
}

void FSelectionInfo::TransformSelectedInstances(const FVector& InDrag, const FRotator& InRot, const FVector& InScale, bool bDuplicate)
{
	if (IsSelected())
	{
		TArray<FSelectItem> DuplicatedIitems;
		if (bDuplicate)
		{
			for (auto& Item : Selection)
			{
				auto& Proxy = Item.Proxy;

				FTransform WorldTM;
				Proxy->Component->GetInstanceTransform(Proxy->InstanceIndex, WorldTM, true);
				int32 DuplicatedInstanceIndex = Proxy->Component->AddInstance(WorldTM, /*bWorldSpace*/ true);
				DuplicatedIitems.Emplace(Proxy->Component, DuplicatedInstanceIndex);
			}
			SetSelectedNone(); 

			int32 DuplicatedCount = DuplicatedIitems.Num();
			for (int32 Index = 0; Index < DuplicatedCount; ++Index)
			{
				auto& Proxy = DuplicatedIitems[Index].Proxy;
				SelectInstance(true, Proxy->Component, Proxy->InstanceIndex, true, Index == DuplicatedCount-1);
			}
		}

		int32 TotalSelected = Selection.Num();
		for (int32 i = 0; i < TotalSelected; ++i)
		//for (auto& Item : Selection)
		{
			//auto Proxy = Item.Proxy;
			auto Proxy = Selection[i].Proxy;

			FTransform WorldTM;
			Proxy->Component->GetInstanceTransform(Proxy->InstanceIndex, WorldTM, true);

			if (!InRot.IsNearlyZero())
			{
				WorldTM.SetRotation(InRot.Quaternion() * WorldTM.GetRotation());
				FVector PivotLocation = ParentMode->GetWidgetLocation();// GetLastSelectedTransform().GetLocation();
				FVector NewLocation = WorldTM.GetLocation();
				NewLocation -= PivotLocation;
				NewLocation = FRotationMatrix(InRot).TransformPosition(NewLocation);
				NewLocation += PivotLocation;
				WorldTM.SetLocation(NewLocation);
			}

			if (!InDrag.IsNearlyZero())
			{
				WorldTM.SetLocation(WorldTM.GetLocation() + InDrag);
			}

			if (!InScale.IsNearlyZero())
			{
				WorldTM.SetScale3D(WorldTM.GetScale3D() + InScale);
			}

			TransformSelectedObject(Proxy, WorldTM, /*bWorldSpace=*/ true, /*bMarkRenderStateDirty=*/ true);
		}

// 		for (auto& Actor : GetSelectedActors())
// 		{
// 			Actor->MarkPackageDirty();
// 		}
	}
}

void FSelectionInfo::RemoveSelectedInstances()
{
	if (!IsSelected())
	{
		return;
	}

	for (auto& Component : GetSelectedComponents())
	{	
		TArray<int32> InstancesToRemove;
		for (int32 InstanceIndex = 0; InstanceIndex < Component->SelectedInstances.Num(); ++InstanceIndex)
		{
			if (Component->IsInstanceSelected(InstanceIndex))
			{
				InstancesToRemove.Add(InstanceIndex);
			}
		}
			
		Component->ClearInstanceSelection();

		if (InstancesToRemove.Num() > 0)
		{
			TArray<int32> SortedInstancesToRemove(InstancesToRemove);
				
			SortedInstancesToRemove.Sort(TGreater<int32>());
				
			for (int32 Index : SortedInstancesToRemove)
			{
				Component->RemoveInstance(Index);
			}
		}

		Component->MarkRenderStateDirty();
	}

	SetSelectedNone();
}

void FSelectionInfo::ScaleSnapSelectedObject(class UInstanceToolEditorObject* InProxy, float ForceScaleSnapSize, bool bMarkRenderStateDirty) const
{
	FTransform RelativeTransform;
	InProxy->Component->GetInstanceTransform(InProxy->InstanceIndex, RelativeTransform, false);

	FVector RelativeScale3D = RelativeTransform.GetScale3D() + ForceScaleSnapSize / 3.0f;
	FVector RelativeScale3DSnapped = RelativeScale3D.GridSnap(ForceScaleSnapSize);

	FVector RelativeLocation = RelativeTransform.GetTranslation();
	FRotator RelativeRotation = RelativeTransform.Rotator();

	FTransform NewRelativeTransform(RelativeRotation, RelativeLocation, RelativeScale3DSnapped);
	InProxy->UpdateInstanceTransform(NewRelativeTransform, /*bWorldSpace=*/ false, /*bMarkRenderStateDirty=*/ bMarkRenderStateDirty);
}

void FSelectionInfo::ForcePositiveScaleOnSelectedObject(class UInstanceToolEditorObject* InProxy, bool bMarkRenderStateDirty /*= true*/) const
{
	FTransform RelativeTransform;
	InProxy->Component->GetInstanceTransform(InProxy->InstanceIndex, RelativeTransform, false);

	FVector RelativeScale3D = RelativeTransform.GetScale3D();
	FVector RelativeScalePositive = RelativeScale3D.GetAbs();

	FVector RelativeLocation = RelativeTransform.GetTranslation();
	FRotator RelativeRotation = RelativeTransform.Rotator();

	FTransform NewRelativeTransform(RelativeRotation, RelativeLocation, RelativeScalePositive);
	InProxy->UpdateInstanceTransform(NewRelativeTransform, /*bWorldSpace=*/ false, /*bMarkRenderStateDirty=*/ bMarkRenderStateDirty);
}

const TArray<const FSelectItem*> FSelectionInfo::GetSelectionSortedByPosition(FVector& InNormalizedDir) const
{
	TArray<const FSelectItem*> SortedSelectObjects;	
	for (auto& Item : Selection)
	{
		SortedSelectObjects.Add(&Item);
	}

	FVector Dir = InNormalizedDir;

	SortedSelectObjects.Sort([&](const FSelectItem& A, const FSelectItem& B)
	{
		FTransform TransformA;
		A.Proxy->Component->GetInstanceTransform(A.Proxy->InstanceIndex, TransformA, true);
		FVector ProjectedLocationA = TransformA.GetLocation().ProjectOnTo(Dir);
		float DotA = ProjectedLocationA | Dir;
		float SizeA = DotA * ProjectedLocationA.SizeSquared();

		FTransform TransformB;
		B.Proxy->Component->GetInstanceTransform(B.Proxy->InstanceIndex, TransformB, true);
		FVector ProjectedLocationB = TransformB.GetLocation().ProjectOnTo(Dir);
		float DotB = ProjectedLocationB | Dir;
		float SizeB = DotB * ProjectedLocationB.SizeSquared();

		return SizeA > SizeB;
	});
	return SortedSelectObjects;
}

void FSelectionInfo::BackupSelection()
{
	SelectionBackup.Empty();
	for (auto& Item : Selection)
	{
		SelectionBackup.Emplace(Item.Component, Item.InstanceIndex);
	}
}

void FSelectionInfo::RestoreSelection()
{
	for (auto& Item : SelectionBackup)
	{
		if (Item.Component)
		{
			Item.Component->SelectInstance(true, Item.InstanceIndex); 
			Selection.Emplace(Item.Component, Item.InstanceIndex);
		}
	}
	SelectionBackup.Empty();
}

int32 FSelectionInfo::SelectAllInvalidInstances(AActor* InActor, const FVector& InSelectInvalidTolerance, bool bMultiSelect, bool bNotify)
{
	int32 InvalidCount = 0;

	if (!InActor)
	{
		return 0;
	}
	if (!bMultiSelect)
	{
		SetSelectedNone();
	}

	ClearInstanceSelection(InActor);

	TArray<UInstancedStaticMeshComponent*> Components;
	InActor->GetComponents<UInstancedStaticMeshComponent>(Components);

	for (auto& Component : Components)
	{
		InvalidCount += SelectAllInvalidInstances(Component, InSelectInvalidTolerance, /*bMultiSelect=*/ true, /*bNotify=*/ false);
	}

	if (bNotify)
	{
		ParentMode->BroadcastSelectionChanged();
	}

	return InvalidCount;
}

int32 FSelectionInfo::SelectAllInvalidInstances(UInstancedStaticMeshComponent* InComponent, const FVector& InSelectInvalidTolerance, bool bMultiSelect, bool bNotify)
{
	int32 InvalidCount = 0;

	if (!InComponent)
	{
		return 0;
	}

	if (!bMultiSelect)
	{
		SetSelectedNone();
	}

	InComponent->SelectInstance(false, 0, InComponent->PerInstanceSMData.Num());

	TArray<FTransform> UniqueTransforms;	

	for (int32 Index = InComponent->PerInstanceSMData.Num() - 1; Index >= 0; --Index)
	{
		FTransform InstanceTM;
		InComponent->GetInstanceTransform(Index, InstanceTM);

		auto TransformPrecicate = [&](const FTransform& Other) {
			return Other.GetLocation().Equals(InstanceTM.GetLocation(), InSelectInvalidTolerance.X)
				&& Other.Rotator().Equals(InstanceTM.Rotator(), InSelectInvalidTolerance.Y)
				&& Other.GetScale3D().Equals(InstanceTM.GetScale3D(), InSelectInvalidTolerance.Z);
		};

		if (!UniqueTransforms.ContainsByPredicate(TransformPrecicate))
		{
			UniqueTransforms.Add(InstanceTM);
		}
		else
		{
			InComponent->SelectInstance(true, Index);
			Selection.Emplace(InComponent, Index);
			++InvalidCount;
		}
	}

	InComponent->MarkRenderStateDirty();

	if (bNotify)
	{
		ParentMode->BroadcastSelectionChanged();
	}

	return InvalidCount;
}

void FSelectionInfo::ClearInstanceSelection(AActor* InActor) const
{
	if (InActor)
	{
		TArray<UInstancedStaticMeshComponent*> InstanceComponents;
		InActor->GetComponents<UInstancedStaticMeshComponent>(InstanceComponents);

		if (InstanceComponents.Num() > 0)
		{
			for (auto& ComponentItr : InstanceComponents)
			{
				ComponentItr->ClearInstanceSelection();
				ComponentItr->MarkRenderStateDirty();
			}
		}
	}
}

bool FSelectionInfo::HasInstanceSelected(AActor *InActor) const
{
	bool bSelected = false;

	if (InActor)
	{
		TArray<UInstancedStaticMeshComponent*> InstanceComponents;
		InActor->GetComponents<UInstancedStaticMeshComponent>(InstanceComponents);

		if (InstanceComponents.Num() > 0)
		{
			for (auto& ComponentItr : InstanceComponents)
			{
				for (int32 Index = 0; Index < ComponentItr->SelectedInstances.Num(); ++Index)
				{
					if (ComponentItr->SelectedInstances[Index])
					{
						bSelected = true;
						break;
					}
				}
			}
		}
	}

	return bSelected;
}

bool FSelectionInfo::HasInstancedComponent(AActor *InActor) const
{
	if (InActor)
	{
		TArray<UInstancedStaticMeshComponent*> InstanceComponents;
		InActor->GetComponents<UInstancedStaticMeshComponent>(InstanceComponents);

		return (InstanceComponents.Num() > 0);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// FSelectItem

FSelectItem::FSelectItem(UInstancedStaticMeshComponent* InComponent, int32 InIndex)
	: Component(InComponent)
	, InstanceIndex(InIndex)
{
	Proxy = NewObject<UInstanceToolEditorObject>();
	//Proxy->SetFlags(RF_Transactional);
	Proxy->ClearFlags(RF_Transactional);
	Proxy->Init(InComponent, InIndex);
	Proxy->AddToRoot();
}

FSelectItem::~FSelectItem()
{
	if (IsValid(Proxy) && Proxy->IsValidLowLevelFast(true))
	{
		Proxy->RemoveFromRoot();
	}
}

// void FSelectItem::AddReferencedObjects(class FReferenceCollector& Collector)
// {
// 	Collector.AddReferencedObject(Proxy);
// }

///////////////////////////////////////////////////////////////////////////////
// FModeTool_BoxSelect

FModeTool_SelectTool::FModeTool_SelectTool()
{
	ID = EModeTools::MT_None;
}

bool FModeTool_SelectTool::BoxSelect(FBox& InBox, bool InSelect)
{
	bool bResult = false;
	if (GLevelEditorModeTools().IsModeActive(FInstanceToolEdMode::EM_InstanceToolEdModeId))
	{
		if (FInstanceToolEdMode* EditMode = (FInstanceToolEdMode*)GLevelEditorModeTools().GetActiveMode(FInstanceToolEdMode::EM_InstanceToolEdModeId))
		{
			bResult = EditMode->DoBoxSelect(InBox, InSelect);
		}
	}
	return bResult;
}

bool FModeTool_SelectTool::FrustumSelect(const FConvexVolume& InFrustum, FEditorViewportClient* InViewportClient, bool InSelect)
{
	bool bResult = false;
	if (GLevelEditorModeTools().IsModeActive(FInstanceToolEdMode::EM_InstanceToolEdModeId))
	{
		if (FInstanceToolEdMode* EditMode = (FInstanceToolEdMode*)GLevelEditorModeTools().GetActiveMode(FInstanceToolEdMode::EM_InstanceToolEdModeId))
		{
			bResult = EditMode->DoFrustumSelect(InFrustum, InViewportClient, InSelect);
		}
	}
	return bResult;
}

////////////////////////////////////////////

void FInstanceToolUtil::InvalidateLightingCacheInLevel(UWorld* InWorld)
{
	for (FActorIterator It(InWorld); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && !Actor->IsPendingKillPending() && !Actor->IsA(AInstancedFoliageActor::StaticClass()))
		{
			TArray<UInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UInstancedStaticMeshComponent>(Components);
			for (auto& Component : Components)
			{
				if (Component)
				{
					Component->InvalidateLightingCache();
				}
			}
		}
	}
}



void FInstanceToolUtil::ForceRebuildRenderDataInLevel(UWorld* InWorld, ULevel* InLevel/* = NULL*/)
{
	for (FActorIterator It(InWorld); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && !Actor->IsPendingKillPending() && !Actor->IsA(AInstancedFoliageActor::StaticClass()))
		{
			if (InLevel && Actor->GetLevel() != InLevel)
			{
				continue;
			}

			TArray<UInstancedStaticMeshComponent*> Components;
			Actor->GetComponents<UInstancedStaticMeshComponent>(Components);
			for (auto& Component : Components)
			{
				ForceRebuildRenderData(Component);
			}
		}
	}
}

void FInstanceToolUtil::ForceRebuildRenderData(UInstancedStaticMeshComponent* InComponent)
{
	if (InComponent)
	{
		INSTANCETOOL_LOG(Display, TEXT("[ForceRebuildRenderData]: %s[%s] : Selectable: %d, HasPerInstanceHitProxies? %d"), *InComponent->GetOwner()->GetActorLabel(), *InComponent->GetName(), InComponent->bSelectable, InComponent->bHasPerInstanceHitProxies);

		if (!InComponent->bHasPerInstanceHitProxies)
		{
			InComponent->bHasPerInstanceHitProxies = true;
		}

		FProperty* TransformProp = FindFieldChecked<FProperty>(TBaseStructure<FInstancedStaticMeshInstanceData>::Get(), GET_MEMBER_NAME_CHECKED(FInstancedStaticMeshInstanceData, Transform));
		FInstanceToolUtil::PostEditChangeChainProperty(InComponent, TransformProp);

		if (UHierarchicalInstancedStaticMeshComponent* HISMC = Cast<UHierarchicalInstancedStaticMeshComponent>(InComponent))
		{
			HISMC->BuildTreeIfOutdated(false, true);
			FProperty* PerInstanceSMDataProp = FindFieldChecked<FProperty>(UInstancedStaticMeshComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UInstancedStaticMeshComponent, PerInstanceSMData));
			PostEditChangeChainProperty(InComponent, PerInstanceSMDataProp);
		}
	}
}


void FInstanceToolUtil::PostEditChangeChainProperty(class UInstancedStaticMeshComponent* InComponent, class FProperty* InProperty)
{
	FPropertyChangedEvent PropertyChangedEvent(InProperty);
	FEditPropertyChain PropertyChain;
	PropertyChain.AddHead(InProperty);
	FPropertyChangedChainEvent PropertyChangedChainEvent(PropertyChain, PropertyChangedEvent);
	InComponent->PostEditChangeChainProperty(PropertyChangedChainEvent);
}

int32 FInstanceToolUtil::GetActorInstanceCount(AActor* InActor)
{
	int32 NumInstances = 0;

	TArray<UInstancedStaticMeshComponent*> Components;
	InActor->GetComponents<UInstancedStaticMeshComponent>(Components);

	for (auto& Component : Components)
	{
		NumInstances += Component->PerInstanceSMData.Num();
	}

	return NumInstances;
}

#undef LOCTEXT_NAMESPACE


