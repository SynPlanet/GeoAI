// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;

public class JsonRpc : ModuleRules
{
	public JsonRpc(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);


		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"ICU",
			"ImageWrapper",
			"Json",
			"HTTP",
			"JsonUtilities",
			"RenderCore",
			"RHI",
		});

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"Core",
				"GeometryFramework",
				"Slate",
				"SlateCore",
				"ApplicationCore",
				"HeadMountedDisplay",
				"GeometryCore",
				"DeveloperSettings"
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}