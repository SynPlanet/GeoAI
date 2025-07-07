// Copyright Epic Games, Inc. All Rights Reserved.

#include "PointCloudSliceAndDiceRulesFactoryNew.h"

#include "PointCloudSliceAndDiceRuleSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PointCloudSliceAndDiceRulesFactoryNew)

/* UPointCloudSliceAndDiceRuleSetFactoryNew structors
 *****************************************************************************/

UPointCloudSliceAndDiceRuleSetFactoryNew::UPointCloudSliceAndDiceRuleSetFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UPointCloudSliceAndDiceRuleSet::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UPointCloudSliceAndDiceRuleSetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UPointCloudSliceAndDiceRuleSet *NewRuleSet = NewObject<UPointCloudSliceAndDiceRuleSet>(InParent, InClass, InName, Flags);

	if (NewRuleSet)
	{
		NewRuleSet->MakeDefaultRules();
	}

	return NewRuleSet; 

}


bool UPointCloudSliceAndDiceRuleSetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
