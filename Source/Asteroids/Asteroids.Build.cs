// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Asteroids : ModuleRules
{
	public Asteroids(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"DeveloperSettings",
			"Engine", 
			"EnhancedInput",
			"GameplayTags",
			"InputCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateIncludePaths.AddRange(new string[]
			{
				"Asteroids",
				"Asteroids/Components",
				"Asteroids/Networking",
				"Asteroids/Pickups",
				"Asteroids/Utils"
			});
	}
}
