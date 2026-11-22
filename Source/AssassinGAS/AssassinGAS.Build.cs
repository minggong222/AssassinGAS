// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AssassinGAS : ModuleRules
{
	public AssassinGAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities" });

		PublicIncludePaths.AddRange(new string[] {
			"AssassinGAS",
			"AssassinGAS/Variant_Platforming",
			"AssassinGAS/Variant_Platforming/Animation",
			"AssassinGAS/Variant_Combat",
			"AssassinGAS/Variant_Combat/AI",
			"AssassinGAS/Variant_Combat/Animation",
			"AssassinGAS/Variant_Combat/Gameplay",
			"AssassinGAS/Variant_Combat/Interfaces",
			"AssassinGAS/Variant_Combat/UI",
			"AssassinGAS/Variant_SideScrolling",
			"AssassinGAS/Variant_SideScrolling/AI",
			"AssassinGAS/Variant_SideScrolling/Gameplay",
			"AssassinGAS/Variant_SideScrolling/Interfaces",
			"AssassinGAS/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
