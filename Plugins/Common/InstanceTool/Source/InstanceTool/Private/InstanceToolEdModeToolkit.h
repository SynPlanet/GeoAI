// Copyright 2016-2019 marynate. All Rights Reserved.

#pragma once

#include "InstanceToolEditorCommands.h"
#include "SInstanceToolEditor.h"
#include "Toolkits/BaseToolkit.h"

class FInstanceToolEdModeToolkit : public FModeToolkit
{
public:

	virtual void Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost) override;

	FInstanceToolEdModeToolkit();

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return InstanceToolEditorWidget; }

protected:
	void OnChangeMode(EToolMode InMode);
	bool IsModeEnabled(EToolMode InMode) const;
	bool IsModeActive(EToolMode InMode) const;

private:
	class FInstanceToolEdMode* GetInstanceToolEdMode() const;

	TSharedPtr<SInstanceToolEditor> InstanceToolEditorWidget;
};
