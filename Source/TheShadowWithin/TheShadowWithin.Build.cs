// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheShadowWithin : ModuleRules
{
	public TheShadowWithin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TheShadowWithin",
			"TheShadowWithin/Variant_Platforming",
			"TheShadowWithin/Variant_Platforming/Animation",
			"TheShadowWithin/Variant_Combat",
			"TheShadowWithin/Variant_Combat/AI",
			"TheShadowWithin/Variant_Combat/Animation",
			"TheShadowWithin/Variant_Combat/Gameplay",
			"TheShadowWithin/Variant_Combat/Interfaces",
			"TheShadowWithin/Variant_Combat/UI",
			"TheShadowWithin/Variant_SideScrolling",
			"TheShadowWithin/Variant_SideScrolling/AI",
			"TheShadowWithin/Variant_SideScrolling/Gameplay",
			"TheShadowWithin/Variant_SideScrolling/Interfaces",
			"TheShadowWithin/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
