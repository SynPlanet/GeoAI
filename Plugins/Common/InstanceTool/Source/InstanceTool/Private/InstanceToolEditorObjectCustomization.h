// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class IPropertyHandle;
class FInstanceToolEdMode;

class FInstanceToolEditorObjectCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(FInstanceToolEdMode* InEditMode);

	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

public:
	static FText GetIndexNameText(TSharedRef<IPropertyHandle> PropertyHandle);
	static FText GetIndexValueText(TSharedRef<IPropertyHandle> PropertyHandle);
	static FText GetActorText(TSharedRef<IPropertyHandle> PropertyHandle);

private:
	FInstanceToolEditorObjectCustomization();

	static FInstanceToolEdMode* EditMode;
};