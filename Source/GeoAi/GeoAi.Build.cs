// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GeoAi : ModuleRules
{
	public GeoAi(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "Json",
            "EnhancedInput",
            
			// GeoAiModify
			"CitySample",
			"PointCloud",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			
			
			// GeoAiModify
			"CitySample",
			"PixelStreaming",
			"MetaPixelStreaming",
			"GameplayMessageRuntime", 
			"CartographicTransforms",
			"CesiumRuntime",
			"GeoJsonWrapper",
			"GeoReferencing",
			"PointCloud"
		});
        

		PrivateIncludePaths.AddRange(new string[]
		{
			"GeoAi"
		});
	}
}
