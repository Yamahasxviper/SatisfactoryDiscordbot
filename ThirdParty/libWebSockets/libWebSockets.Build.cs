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
		string archName = null;
		
		// Check if Target.Architecture is available before accessing its properties
		if (Target.Architecture != null)
		{
			try
			{
				// In some versions of UE, the architecture name is accessed via LinuxName
				archName = Target.Architecture.LinuxName;
			}
			catch (System.Reflection.TargetException)
			{
				// LinuxName property invocation failed - will try ToString() below
			}
			catch (System.MissingMemberException)
			{
				// LinuxName property doesn't exist in this UE version - will try ToString() below
			}
			catch (Microsoft.CSharp.RuntimeBinder.RuntimeBinderException)
			{
				// Dynamic binding failed for LinuxName property - will try ToString() below
			}
			
			// If LinuxName failed, try ToString() as fallback
			if (string.IsNullOrEmpty(archName))
			{
				try
				{
					archName = Target.Architecture.ToString();
				}
				catch (System.Exception)
				{
					// ToString() failed - will use default x86_64 below
					// Exception suppressed as we have a reasonable default for this case
				}
			}
		}
		
		// Handle null or empty architecture name with sensible default
		if (string.IsNullOrEmpty(archName))
		{
			archName = "x86_64"; // Default to x86_64 for Linux (most common case for server builds)
		}
		
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
			string IncludePath = IncludeDirectory;
			string LibraryPath = Path.Combine(WebSocketsPackagePath, "lib", Target.Platform.ToString(), "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName(), ConfigName, "websockets_static.lib");
			
			if (Directory.Exists(IncludePath))
			{
				PublicSystemIncludePaths.Add(IncludePath);
			}
			
			if (File.Exists(LibraryPath))
			{
				PublicAdditionalLibraries.Add(LibraryPath);
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string IncludePathARM64 = Path.Combine(WebSocketsPackagePath, "include", Target.Platform.ToString(), "ARM64");
			string IncludePathx64 = Path.Combine(WebSocketsPackagePath, "include", Target.Platform.ToString(), "x64");
			string IncludePathBase = Path.Combine(WebSocketsPackagePath, "include", Target.Platform.ToString());
			string LibraryPathARM64 = Path.Combine(WebSocketsPackagePath, "lib", Target.Platform.ToString(), "ARM64", ConfigName, "libwebsockets.a");
			string LibraryPathx64 = Path.Combine(WebSocketsPackagePath, "lib", Target.Platform.ToString(), "x64", ConfigName, "libwebsockets.a");
			
			if (Directory.Exists(IncludePathARM64))
			{
				PublicSystemIncludePaths.Add(IncludePathARM64);
			}
			if (Directory.Exists(IncludePathx64))
			{
				PublicSystemIncludePaths.Add(IncludePathx64);
			}
			if (Directory.Exists(IncludePathBase))
			{
				PublicSystemIncludePaths.Add(IncludePathBase);
			}
			
			if (File.Exists(LibraryPathARM64))
			{
				PublicAdditionalLibraries.Add(LibraryPathARM64);
			}
			if (File.Exists(LibraryPathx64))
			{
				PublicAdditionalLibraries.Add(LibraryPathx64);
			}
		}
		else
		{
			string IncludePath = IncludeDirectory;
			string LibPath = LibraryDirectory;
			string FullLibraryPath = Path.Combine(LibPath, DefaultLibraryName);
			
			if (Directory.Exists(IncludePath))
			{
				PublicSystemIncludePaths.Add(IncludePath);
			}
			
			// Use PublicSystemLibraryPaths for better performance and dependency checking
			if (Directory.Exists(LibPath) && File.Exists(FullLibraryPath))
			{
				PublicSystemLibraryPaths.Add(LibPath);
				PublicAdditionalLibraries.Add("websockets");
			}
		}

		if (bRequireOpenSSL)
		{
			PublicDependencyModuleNames.Add("OpenSSL");
		}
	}
}
