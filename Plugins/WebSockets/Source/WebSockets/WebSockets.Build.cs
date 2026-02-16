// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class WebSockets : ModuleRules
{
	protected virtual bool PlatformSupportsLibWebsockets
	{
		get
		{
			return
				Target.Platform == UnrealTargetPlatform.Win64 ||
				Target.Platform == UnrealTargetPlatform.Android ||
				Target.Platform == UnrealTargetPlatform.Mac ||
				Target.IsInPlatformGroup(UnrealPlatformGroup.Unix) ||
				Target.Platform == UnrealTargetPlatform.IOS;
		}
	}

	protected virtual bool bPlatformSupportsWinHttpWebSockets
	{
		get
		{
			// Availability requires Windows 8.1 or greater, as this is the min version of WinHttp that supports WebSockets
			return Target.Platform.IsInGroup(UnrealPlatformGroup.Windows) && Target.WindowsPlatform.TargetWindowsVersion >= 0x0603;
		}
	}

	protected virtual bool bPlatformSupportsWinRTWebsockets
	{
		get => false;
	}

	protected virtual bool UsePlatformSSL
	{
		get => false;
	}

	protected virtual bool ShouldUseModule
	{
		get
		{
			return PlatformSupportsLibWebsockets || bPlatformSupportsWinRTWebsockets || bPlatformSupportsWinHttpWebSockets;
		}
	}

	protected virtual string WebSocketsManagerPlatformInclude
	{
		get
		{
			if (PlatformSupportsLibWebsockets)
			{
				return "Lws/LwsWebSocketsManager.h";
			}
			else if (bPlatformSupportsWinHttpWebSockets)
			{
				return "WinHttp/WinHttpWebSocketsManager.h";
			}
			else
			{
				return "";
			}
		}
	}

	protected virtual string WebSocketsManagerPlatformClass
	{
		get
		{
			if (PlatformSupportsLibWebsockets)
			{
				return "FLwsWebSocketsManager";
			}
			else if (bPlatformSupportsWinHttpWebSockets)
			{
				return "FWinHttpWebSocketsManager";
			}
			else
			{
				return "";
			}
		}
	}

	protected virtual string GetLinuxArchitecturePath()
	{
		// Map Unreal Engine architecture names to the directory structure used by libwebsockets
		string archName = null;
		
		// Check if Target.Architecture is available before accessing its properties
		if (Target.Architecture != null)
		{
			try
			{
				archName = Target.Architecture.LinuxName;
			}
			catch (System.Exception)
			{
				// LinuxName property doesn't exist or invocation failed - will try ToString() below
				// This is expected for some UE versions that don't have LinuxName property
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
					// This is a last-resort fallback for unexpected architecture configurations
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

	protected virtual string GetLibWebSocketsIncludePath()
	{
		string ProjectRoot = Path.Combine(ModuleDirectory, "../../../..");
		string LibWebSocketsPath = Path.Combine(ProjectRoot, "ThirdParty/libWebSockets/libwebsockets");
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			return Path.Combine(LibWebSocketsPath, "include", Target.Platform.ToString(), "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName());
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Android uses multiple architecture paths - will be added separately
			return null;
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			return Path.Combine(LibWebSocketsPath, "include", "Unix", GetLinuxArchitecturePath());
		}
		else
		{
			return Path.Combine(LibWebSocketsPath, "include", Target.Platform.ToString());
		}
	}

	protected virtual string GetLibWebSocketsLibraryPath()
	{
		string ProjectRoot = Path.Combine(ModuleDirectory, "../../../..");
		string LibWebSocketsPath = Path.Combine(ProjectRoot, "ThirdParty/libWebSockets/libwebsockets");
		string ConfigName = (Target.Configuration == UnrealTargetConfiguration.Debug && Target.bDebugBuildsActuallyUseDebugCRT) ? "Debug" : "Release";
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			return Path.Combine(LibWebSocketsPath, "lib", Target.Platform.ToString(), "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName(), ConfigName, "websockets_static.lib");
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Android uses multiple architecture paths - will be added separately
			return null;
		}
		else if (Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
		{
			return Path.Combine(LibWebSocketsPath, "lib", "Unix", GetLinuxArchitecturePath(), ConfigName, "libwebsockets.a");
		}
		else
		{
			return Path.Combine(LibWebSocketsPath, "lib", Target.Platform.ToString(), ConfigName, "libwebsockets.a");
		}
	}

	public WebSockets(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"HTTP"
			}
		);

		bool bWithWebSockets = false;
		bool bWithLibWebSockets = false;
		bool bWithWinHttpWebSockets = false;

		if (ShouldUseModule)
		{
			bWithWebSockets = true;

			if (PlatformSupportsLibWebsockets)
			{
				bWithLibWebSockets = true;

				// Manually add libWebSockets include paths and libraries since the module is in the project's ThirdParty folder
				// and may not be automatically discovered by UBT in all build configurations
				string LibWebSocketsIncludePath = GetLibWebSocketsIncludePath();
				string LibWebSocketsLibraryPath = GetLibWebSocketsLibraryPath();
				
				// Add include path if available (Android handles this separately)
				if (!string.IsNullOrEmpty(LibWebSocketsIncludePath))
				{
					PrivateIncludePaths.Add(LibWebSocketsIncludePath);
				}
				
				// Add library path if available (Android handles this separately)
				if (!string.IsNullOrEmpty(LibWebSocketsLibraryPath))
				{
					PublicAdditionalLibraries.Add(LibWebSocketsLibraryPath);
				}

				if (UsePlatformSSL)
				{
					PrivateDefinitions.Add("WITH_SSL=0");
					// Module dependency removed: paths are manually configured above via PrivateIncludePaths
					// and PublicAdditionalLibraries to use project's local libWebSockets instead of engine's
				}
				else
				{
					AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "zlib");
					// Module dependency removed: paths are manually configured above via PrivateIncludePaths
					// and PublicAdditionalLibraries to use project's local libWebSockets instead of engine's
					PrivateDependencyModuleNames.Add("SSL");
				}
			}
			else if (bPlatformSupportsWinHttpWebSockets)
			{
				// Enable WinHttp Support
				bWithWinHttpWebSockets = true;

				AddEngineThirdPartyPrivateStaticDependencies(Target, "WinHttp");
			}
		}

		PublicDefinitions.Add("WEBSOCKETS_PACKAGE=1");
		PublicDefinitions.Add("WITH_WEBSOCKETS=" + (bWithWebSockets ? "1" : "0"));
		PublicDefinitions.Add("WITH_LIBWEBSOCKETS=" + (bWithLibWebSockets ? "1" : "0"));
		PublicDefinitions.Add("WITH_WINHTTPWEBSOCKETS=" + (bWithWinHttpWebSockets ? "1" : "0"));
		string PlatformInclude = WebSocketsManagerPlatformInclude;
		if (PlatformInclude.Length > 0)
		{
			PublicDefinitions.Add("WEBSOCKETS_MANAGER_PLATFORM_INCLUDE=\"" + WebSocketsManagerPlatformInclude + "\"");
			PublicDefinitions.Add("WEBSOCKETS_MANAGER_PLATFORM_CLASS=" + WebSocketsManagerPlatformClass);
		}
	}
}
