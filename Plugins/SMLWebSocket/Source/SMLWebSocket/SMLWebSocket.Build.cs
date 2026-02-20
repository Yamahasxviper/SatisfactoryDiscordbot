// Copyright Coffee Stain Studios. All Rights Reserved.

using UnrealBuildTool;

public class SMLWebSocket : ModuleRules
{
	public SMLWebSocket(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = false;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Sockets",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// SSL module provides Unreal's SSL abstraction and links openssl libs
			"SSL",
			// OpenSSL provides raw OpenSSL headers (ssl.h, sha.h, bio.h, etc.)
			"OpenSSL",
		});
	}
}
