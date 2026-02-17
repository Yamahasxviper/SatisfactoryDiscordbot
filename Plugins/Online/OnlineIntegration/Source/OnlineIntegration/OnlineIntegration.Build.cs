// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OnlineIntegration : ModuleRules
{
	public OnlineIntegration(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		// <FL> [WuttkeP] Add PLATFORM_PS5 and PLATFORM_XSX defines.
		bAllowConfidentialPlatformDefines = true;
		// </FL>
		bUseUnity = false; // Disable unity because mixing OnlineSubsystem and OnlineServices is no good (but OnlineGameActivityInterface.h only exists on OnlineSubsystem)
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
				"OnlineServicesInterface", 
				"Engine", 
				"UMG", 
				"ModelViewViewModel",
				"FieldNotification",
				"GameplayEvents",
				"SlateCore",
				"EOSShared" // <FL> [KonradA, BGR] required for advanced EOS logging
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ControlFlows",
				"CoreUObject",
				"Engine",
				"Slate",
				"DeveloperSettings",
				// ... add private dependencies that you statically link with here ...	
				"OnlineServicesCommonEngineUtils", 
				"OnlineSubsystemUtils",
				"GameplayTags", 
				"ApplicationCore", 
				"CoreOnline", 
				"InputCore", 
				"OnlineServicesCommon",
				"OnlineSubsystem"
			});
		
		// <FL> [ZimmermannA] Needed to display xbox system messages during joining
		// Commented out previously disabled (if false) platform-specific code to avoid CS0162 unreachable code warnings
		// if (Target.Platform == UnrealTargetPlatform.XSX)
		// {
		// 	PrivateDependencyModuleNames.AddRange(new string[] {
		// 		"OnlineSubsystemGDK",
		// 	});
		// }
		// </FL> 
		//<FL>[KonradA]
		// Commented out previously disabled (if false) platform-specific code to avoid CS0162 unreachable code warnings
		// if (Target.Platform == UnrealTargetPlatform.PS5)
		// {
		// 	PrivateDependencyModuleNames.AddRange(new string[] {
		// 			"OnlineSubsystemSony"
		// 		});
		// }
		//</FL>
		// Commented out previously disabled (if false) platform-specific code to avoid CS0162 unreachable code warnings
		// if (Target.Platform == UnrealTargetPlatform.PS5 || Target.Platform == UnrealTargetPlatform.XSX)
		// {
		// 	PrivateDependencyModuleNames.AddRange(new string[] {
		// 		"OnlineServicesOSSAdapter"
		// 	});
		// }
	}
}
