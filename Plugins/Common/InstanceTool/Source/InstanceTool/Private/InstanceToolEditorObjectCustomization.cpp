// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolEditorObjectCustomization.h"
#include "InstanceToolEditorObject.h"
#include "InstanceToolTransformDetails.h"
#include "InstanceToolEdMode.h"

#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "Widgets/Text/STextBlock.h"
#include "Components/InstancedStaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "InstanceTool.EditorObject"

FInstanceToolEdMode* FInstanceToolEditorObjectCustomization::EditMode = nullptr;

TSharedRef<IDetailCustomization> FInstanceToolEditorObjectCustomization::MakeInstance(class FInstanceToolEdMode* InEditMode)
{
	auto Instance = MakeShareable(new FInstanceToolEditorObjectCustomization());
	EditMode = InEditMode;
	return Instance;
}

void FInstanceToolEditorObjectCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& InstanceCategory = DetailBuilder.EditCategory("Instance", LOCTEXT("InstanceCategory", "Instance"), ECategoryPriority::Variable);
	
	auto ActorLabelPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorObject, ActorLabel));
	InstanceCategory.AddProperty(ActorLabelPropertyHandle)
		.CustomWidget()
		.NameContent()
		[
			ActorLabelPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(300.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text_Static(&GetActorText, ActorLabelPropertyHandle)
			]
		];

	auto IndexPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UInstanceToolEditorObject, InstanceIndex));
	InstanceCategory.AddProperty(IndexPropertyHandle)
		.CustomWidget()
		.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text_Static(&GetIndexNameText, IndexPropertyHandle)
			]
		]
		.ValueContent()
		.MinDesiredWidth(300.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(DetailBuilder.GetDetailFont())
				.Text_Static(&GetIndexValueText, IndexPropertyHandle)
			]
		];

	TSharedRef<FInstanceToolTransformDetails> TransformDetails = MakeShareable(new FInstanceToolTransformDetails(DetailBuilder.GetDetailsView()->GetSelectedObjects(), DetailBuilder));
	IDetailCategoryBuilder& TransformCategory = DetailBuilder.EditCategory("TransformCommon", LOCTEXT("TransformCommonCategory", "Transform"), ECategoryPriority::Transform);
	TransformCategory.AddCustomBuilder(TransformDetails);

	DetailBuilder.HideCategory("Hidden");

#if !DEBUG_INSTANCE_MATRIX
	DetailBuilder.HideCategory("Debug");
#endif

}

FInstanceToolEditorObjectCustomization::FInstanceToolEditorObjectCustomization()
{
}

FText FInstanceToolEditorObjectCustomization::GetIndexNameText(TSharedRef<IPropertyHandle> PropertyHandle)
{
	TArray<UObject*> CustomizedObjects;
	PropertyHandle->GetOuterObjects(CustomizedObjects);

	FString DisplayString = FString::Printf(TEXT("%i selected"), CustomizedObjects.Num());

	return FText::FromString(DisplayString);
}

FText FInstanceToolEditorObjectCustomization::GetIndexValueText(TSharedRef<IPropertyHandle> PropertyHandle)
{
	TArray<UObject*> CustomizedObjects;
	PropertyHandle->GetOuterObjects(CustomizedObjects);

	FString DisplayString = "";

	const int32 Max = 3;
	int32 Num = 0;
	int32 Total = CustomizedObjects.Num();

	for (auto& Object : CustomizedObjects)
	{
		FString Prefix = (Num > 0) ? TEXT(", ") : TEXT("");
		FString Postfix = (Num == Max - 1 && Total > Max + 1) ? TEXT(", ...") : TEXT("");

		if (UInstanceToolEditorObject* EditorObject = Cast<UInstanceToolEditorObject>(Object))
		{
			if (Num < Max || Num == Total - 1)
			{
				DisplayString += FString::Printf(TEXT("%s%i%s"), *Prefix, EditorObject->InstanceIndex, *Postfix);
			}
			++Num;
		}
	}

	return FText::FromString(DisplayString);
}

FText FInstanceToolEditorObjectCustomization::GetActorText(TSharedRef<IPropertyHandle> PropertyHandle)
{
	FString DisplayString = "";

	TArray<UObject*> CustomizedObjects;
	PropertyHandle->GetOuterObjects(CustomizedObjects);

	TArray<AActor*> Actors;
	for (auto& Object : CustomizedObjects)
	{
		if (UInstanceToolEditorObject* EditorObject = Cast<UInstanceToolEditorObject>(Object))
		{
			if (EditorObject->Component && EditorObject->Component->GetOwner() && !Actors.Contains(EditorObject->Component->GetOwner()))
			{
				Actors.Add(EditorObject->Component->GetOwner());
			}
		}
	}

	const int32 Max = 3;
	int32 Num = 0;
	int32 Total = Actors.Num();

	for (auto& Actor : Actors)
	{
		++Num;
		if (Num > Max) {
			break;
		}
		
		FString Prefix = (Num > 1) ? TEXT(", ") : TEXT("");
		FString Postfix = (Num == Max && Total > Max) ? TEXT(", ...") : TEXT("");
		
		FString ActorLabel = Actor->GetActorLabel();
		int32 InstaceCount = FInstanceToolUtil::GetActorInstanceCount(Actor);

		DisplayString += FString::Printf(TEXT("%s%s (%d)%s"), *Prefix, *ActorLabel, InstaceCount, *Postfix);
	}

	return FText::FromString(DisplayString);
}

#undef LOCTEXT_NAMESPACE