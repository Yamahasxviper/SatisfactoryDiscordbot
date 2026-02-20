// Copyright SatisfactoryDiscordbot. All Rights Reserved.

using UnrealBuildTool;

public class SatisfactoryDiscordBot : ModuleRules
{
	public SatisfactoryDiscordBot(ReadOnlyTargetRules Target) : base(Target)
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
			// Raw TCP socket access – used by the custom WebSocket implementation
			// because the CSS custom engine does not ship the WebSockets module.
			"Sockets",
			// Header stubs required by the Coffee Stain Studios custom UE build.
			"DummyHeaders",
			// Satisfactory game and mod-loader APIs.
			"FactoryGame",
			"SML",
		});

		// OpenSSL (bundled with Unreal Engine) – provides TLS/SSL for WSS support.
		// Kept private so that OpenSSL symbols do not leak into dependent modules.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OpenSSL",
		});
	}
}
