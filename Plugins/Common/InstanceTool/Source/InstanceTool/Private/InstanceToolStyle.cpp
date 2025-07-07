// Copyright 2016-2019 marynate. All Rights Reserved.

#include "InstanceToolStyle.h"

#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FInstanceToolStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...) FSlateFontInfo(StyleSet->RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define TTF_CORE_FONT(RelativePath, ...) FSlateFontInfo(StyleSet->RootToCoreContentDir(RelativePath, TEXT(".ttf") ), __VA_ARGS__)

FString FInstanceToolStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("InstanceTool"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

TSharedPtr<FSlateStyleSet> FInstanceToolStyle::StyleSet = nullptr;
TSharedPtr<class ISlateStyle> FInstanceToolStyle::Get() { return StyleSet; }

FName FInstanceToolStyle::GetStyleSetName()
{
	static FName InstanceToolStyleName(TEXT("InstanceToolStyle"));
	return InstanceToolStyleName;
}

void FInstanceToolStyle::Initialize()
{
	// Icon sizes
	const FVector2D Icon8x8(8.0f, 8.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin("InstanceTool")->GetBaseDir() / TEXT("Resources"));

	// Icon
	{
		StyleSet->Set("InstanceTool.InstanceToolMode", new IMAGE_BRUSH("Icons/icon_Mode_InstanceTool_40x", Icon40x40));
		StyleSet->Set("InstanceTool.InstanceToolMode.Small", new IMAGE_BRUSH("Icons/icon_Mode_InstanceTool_40x", Icon20x20));
		StyleSet->Set("InstanceTool.InstanceToolMode.Selected", new IMAGE_BRUSH("Icons/icon_Mode_InstanceTool_selected_40x", Icon40x40));
		StyleSet->Set("InstanceTool.InstanceToolMode.Selected.Small", new IMAGE_BRUSH("Icons/icon_Mode_InstanceTool_selected_40x", Icon20x20));
	}

	{
		StyleSet->Set("InstanceTool.SelectMode", new IMAGE_BRUSH("Icons/icon_SelectMode_40x", Icon40x40));
		StyleSet->Set("InstanceTool.SelectMode.Small", new IMAGE_BRUSH("Icons/icon_SelectMode_20x", Icon20x20));

		StyleSet->Set("InstanceTool.ConvertMode", new IMAGE_BRUSH("Icons/icon_ConvertMode_40x", Icon40x40));
		StyleSet->Set("InstanceTool.ConvertMode.Small", new IMAGE_BRUSH("Icons/icon_ConvertMode_20x", Icon20x20));
		
		StyleSet->Set("InstanceTool.SpawnMode", new IMAGE_BRUSH("Icons/icon_SpawnMode_40x", Icon40x40));
		StyleSet->Set("InstanceTool.SpawnMode.Small", new IMAGE_BRUSH("Icons/icon_SpawnMode_20x", Icon20x20));

		StyleSet->Set("InstanceTool.PlacementMode", new IMAGE_BRUSH("Icons/icon_PlacementMode_40x", Icon40x40));
		StyleSet->Set("InstanceTool.PlacementMode.Small", new IMAGE_BRUSH("Icons/icon_PlacementMode_20x", Icon20x20));
		
		StyleSet->Set("InstanceTool.OptionsMode", new IMAGE_BRUSH("Icons/icon_OptionsMode_40x", Icon40x40));
		StyleSet->Set("InstanceTool.OptionsMode.Small", new IMAGE_BRUSH("Icons/icon_OptionsMode_20x", Icon20x20));
		
		StyleSet->Set("InstanceTool.AboutMode", new IMAGE_BRUSH("Icons/icon_AboutMode_40x", Icon40x40));
		StyleSet->Set("InstanceTool.AboutMode.Small", new IMAGE_BRUSH("Icons/icon_AboutMode_20x", Icon20x20));
	}

	// Button
	{
		StyleSet->Set("InstanceTool.AxisButton.X", FButtonStyle());
		StyleSet->Set("InstanceTool.AxisButton.Y", FButtonStyle());
		StyleSet->Set("InstanceTool.AxisButton.Z", FButtonStyle());
	}

	// TextStle
	{
		StyleSet->Set("InstanceTool.ChangeLogText", FEditableTextBoxStyle()
			.SetForegroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
			.SetBackgroundColor(FLinearColor(.08f, .08f, 0.08f)));
	}
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void FInstanceToolStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}