// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorCommands.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Notifications/SErrorText.h"
#include "PropertyEditorDelegates.h"

class IDetailsView;

class FInstanceToolEdModeToolkit;

class SInstanceToolEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInstanceToolEditor) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, TSharedRef<FInstanceToolEdModeToolkit> InParentToolkit);

	void NotifySelectionChanged();

	static class FInstanceToolEdMode* GetEditorMode();

protected:
	bool GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;

protected:
	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<IDetailsView> SelectionDetailView;
		
private:

/*
// 	bool IsSelectTool() const;
// 	bool IsDetailTool() const;
// 	bool IsArrayTool() const;
// 	bool IsOptionsTool() const;
// 	bool IsAboutTool() const;
*/

	static EVisibility GetVisibility(EToolMode InMode);
	static EVisibility GetVisibility(int32 InModes);
};
