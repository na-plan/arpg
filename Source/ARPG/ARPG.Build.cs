// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ARPG : ModuleRules
{
	public ARPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG", 
			"GameplayAbilities",
			"GameplayTags", 
			"GameplayTasks",
			"AIModule",
			"GeometryCollectionEngine"
		});

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange( new string[]
			{
				"UnrealEd"
			});
		}
	}
}
