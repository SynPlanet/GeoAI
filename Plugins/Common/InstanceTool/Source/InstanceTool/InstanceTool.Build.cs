// Copyright 2016-2019 marynate. All Rights Reserved.

using UnrealBuildTool;

public class InstanceTool : ModuleRules
{
	public InstanceTool(ReadOnlyTargetRules Target) : base(Target)
    {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"AssetTools",
				"CoreUObject",
				"BlueprintGraph",
				"DetailCustomizations",
				"Engine",
                "EditorStyle",
				"EditorFramework",
				"Foliage",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"InputCore",
                "Projects",
                "PropertyEditor",
                "UnrealEd",
				"LevelEditor",	
			}
			);	
	}
}
