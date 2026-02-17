// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class FactoryServerTarget : FactorySharedTarget
{
    public FactoryServerTarget(TargetInfo Target) : base(Target)
    {
	    Type = TargetType.Server;

	    // Override to Monolithic for Linux Server builds to statically link all modules (including WebSockets)
	    // This prevents "dlopen failed: libFactoryServer-WebSockets-Linux-Shipping.so" errors
	    if (Target.Platform == UnrealTargetPlatform.Linux && LinkTypeOverride == TargetLinkType.Default)
	    {
		    LinkType = TargetLinkType.Monolithic;
	    }

	    // Build additional console application so that the server can be launched from command line on windows
	    bBuildAdditionalConsoleApp = true;
	    // Use custom config override to provide some platform overrides for the dedicated server
	    CustomConfig = "FactoryServer";

	    ExtraModuleNames.AddRange(new[] {
		    "FactoryDedicatedServer"
	    });
    }
}
