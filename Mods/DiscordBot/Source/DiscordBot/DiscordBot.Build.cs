// Copyright Your Mod Team. All Rights Reserved.

using UnrealBuildTool;

public class DiscordBot : ModuleRules
{
    public DiscordBot(ReadOnlyTargetRules Target) : base(Target)
    {
        CppStandard = CppStandardVersion.Cpp20;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "JsonUtilities",
            // HTTP is a public dep because IHttpRequest.h is included in the
            // public header DiscordGatewayClient.h (FHttpRequestPtr / FHttpResponsePtr).
            // With bLegacyPublicIncludePaths=false, headers exposed through Public/
            // must come from public (not private) dependencies.
            "HTTP",
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            // SML is a private dep: it is only used in Private/ .cpp files.
            // FactoryGame is NOT listed here because no FactoryGame headers are
            // included anywhere in this module — it is available transitively
            // through SML's public dependency on FactoryGame if ever needed.
            "SML",
        });
    }
}
