// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System.IO;

public class libWebSockets : ModuleRules
{
	protected virtual bool Supported
	{
		get
		{
			return
				Target.Platform == UnrealTargetPlatform.Android ||
				Target.Platform == UnrealTargetPlatform.IOS ||
				Target.Platform == UnrealTargetPlatform.Mac ||
				Target.IsInPlatformGroup(UnrealPlatformGroup.Unix) ||
				Target.Platform == UnrealTargetPlatform.Win64;
		}
	}

	protected virtual string LibRootDirectory { get { return ModuleDirectory; } }

	protected virtual string WebSocketsVersion { get { return "libwebsockets"; } }
	protected virtual string WebSocketsPackagePath { get { return Path.Combine(LibRootDirectory, WebSocketsVersion); } }

	protected virtual string ConfigName { get { return (Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT) ? "Debug" : "Release"; } }
	
	protected virtual bool bRequireOpenSSL { get { return true; } }

	protected virtual string DefaultLibraryName { get { return "libwebsockets.a"; } }

	protected virtual string GetLinuxArchitecturePath()
	{
		// Map Unreal Engine architecture names to the directory structure used by libwebsockets
		// In UE 5.3+, Target.Architecture.LinuxName may return simplified names,
		// but our prebuilt libraries use full GNU triplet naming convention
		string archName = Target.Architecture.LinuxName;
		
		// Map common architecture names to GNU triplets
		if (archName == "x64" || archName == "x86_64")
		{
			return "x86_64-unknown-linux-gnu";
		}
		else if (archName == "arm64" || archName == "aarch64")
		{
			return "aarch64-unknown-linux-gnueabi";
		}
		
		// Fallback: return the original name (handles cases where it's already in triplet format)
		return archName;
	}

	protected virtual string IncludeDirectory
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				return Path.Combine(WebSocketsPackagePath, "include", Target.Platform.ToString(), "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName());
			}
			else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
			{
				return Path.Combine(WebSocketsPackagePath, "include", "Unix", GetLinuxArchitecturePath());
			}
			else
			{
				return Path.Combine(WebSocketsPackagePath, "include", Target.Platform.ToString());
			}
		}
	}

	protected virtual string LibraryDirectory
	{
		get
		{
			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				return Path.Combine(WebSocketsPackagePath, "lib", Target.Platform.ToString(), "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName(), ConfigName);
			}
			else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
			{
				return Path.Combine(WebSocketsPackagePath, "lib", "Unix", GetLinuxArchitecturePath(), ConfigName);
			}
			else
			{
				return Path.Combine(WebSocketsPackagePath, "lib", Target.Platform.ToString(), ConfigName);
			}
		}
	}

	public libWebSockets(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (!Supported)
		{
			return;
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicSystemIncludePaths.Add(IncludeDirectory);
			PublicAdditionalLibraries.Add(Path.Combine(WebSocketsPackagePath, "lib", Target.Platform.ToString(), "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName(), ConfigName, "websockets_static.lib"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicSystemIncludePaths.Add(Path.Combine(WebSocketsPackagePath, "include", Target.Platform.ToString(), "ARM64"));
			PublicSystemIncludePaths.Add(Path.Combine(WebSocketsPackagePath, "include", Target.Platform.ToString(), "x64"));
			PublicSystemIncludePaths.Add(Path.Combine(WebSocketsPackagePath, "include", Target.Platform.ToString()));
			PublicAdditionalLibraries.Add(Path.Combine(WebSocketsPackagePath, "lib", Target.Platform.ToString(), "ARM64", ConfigName, "libwebsockets.a"));
			PublicAdditionalLibraries.Add(Path.Combine(WebSocketsPackagePath, "lib", Target.Platform.ToString(), "x64", ConfigName, "libwebsockets.a"));
		}
		else
		{
			PublicSystemIncludePaths.Add(IncludeDirectory);
			PublicAdditionalLibraries.Add(Path.Combine(LibraryDirectory, DefaultLibraryName));
		}

		if (bRequireOpenSSL)
		{
			PublicDependencyModuleNames.Add("OpenSSL");
		}
	}
}
