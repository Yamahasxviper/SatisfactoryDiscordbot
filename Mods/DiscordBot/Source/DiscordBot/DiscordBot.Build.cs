// Copyright Coffee Stain Studios. All Rights Reserved.

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
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "WebSockets",
            "SML",
            "FactoryGame",
        });
    }
}
