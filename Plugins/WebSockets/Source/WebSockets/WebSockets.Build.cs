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

	// Cache for LibWebSocketsAvailable to avoid repeated directory checks and warnings
	private bool? _libWebSocketsAvailableCache = null;
	private string _libWebSocketsPath = null;
	private bool _usingLocalLibWebSockets = false;
	
	protected virtual bool LibWebSocketsAvailable
	{
		get
		{
			if (_libWebSocketsAvailableCache.HasValue)
			{
				return _libWebSocketsAvailableCache.Value;
			}
			
			// First check for local libWebSockets in plugin ThirdParty directory
			string LocalLibWebSocketsPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "libwebsockets");
			bool bLocalExists = Directory.Exists(LocalLibWebSocketsPath);
			
			if (bLocalExists)
			{
				// Check if include directory exists with headers
				string LocalIncludePath = Path.Combine(LocalLibWebSocketsPath, "include");
				bool bHasHeaders = Directory.Exists(LocalIncludePath) && 
				                   (File.Exists(Path.Combine(LocalIncludePath, "libwebsockets.h")) ||
				                    Directory.GetFiles(LocalIncludePath, "*.h").Length > 0);
				
				if (bHasHeaders)
				{
					Log.TraceInformation($"[WebSockets] Using local libwebsockets from plugin ThirdParty directory: {LocalLibWebSocketsPath}");
					_libWebSocketsPath = LocalLibWebSocketsPath;
					_usingLocalLibWebSockets = true;
					_libWebSocketsAvailableCache = true;
					return true;
				}
			}
			
			// Fall back to checking engine ThirdParty directory
			string EngineLibWebSocketsPath = Path.Combine(EngineDirectory, "Source", "ThirdParty", "libWebSockets");
			bool bEngineExists = Directory.Exists(EngineLibWebSocketsPath);
			
			if (bEngineExists)
			{
				Log.TraceInformation($"[WebSockets] Using engine libwebsockets from: {EngineLibWebSocketsPath}");
				_libWebSocketsPath = EngineLibWebSocketsPath;
				_usingLocalLibWebSockets = false;
				_libWebSocketsAvailableCache = true;
				return true;
			}
			
			// Not found in either location
			if (PlatformSupportsLibWebsockets)
			{
				Log.TraceWarning($"[WebSockets] libWebSockets not found in plugin ThirdParty: {LocalLibWebSocketsPath}");
				Log.TraceWarning($"[WebSockets] libWebSockets not found in engine ThirdParty: {EngineLibWebSocketsPath}");
				Log.TraceWarning($"[WebSockets] WebSocket functionality will be disabled. Dependent features (like Discord Gateway) will not work.");
			}
			
			_libWebSocketsAvailableCache = false;
			return false;
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
			if (PlatformSupportsLibWebsockets && LibWebSocketsAvailable)
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
			if (PlatformSupportsLibWebsockets && LibWebSocketsAvailable)
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
			if (PlatformSupportsLibWebsockets && LibWebSocketsAvailable)
			{
				bWithWebSockets = true;
				bWithLibWebSockets = true;

				if (_usingLocalLibWebSockets)
				{
					// Using local libwebsockets from plugin ThirdParty
					Log.TraceInformation("[WebSockets] Configuring build with local libwebsockets");
					
					// Add include path
					string IncludePath = Path.Combine(_libWebSocketsPath, "include");
					PublicSystemIncludePaths.Add(IncludePath);
					
					// Add library path based on platform
					if (Target.Platform == UnrealTargetPlatform.Linux)
					{
						string LibPath = Path.Combine(_libWebSocketsPath, "lib", "Linux", "x86_64-unknown-linux-gnu");
						PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libwebsockets.a"));
						
						// Still need OpenSSL and zlib from engine
						AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "zlib");
						PrivateDependencyModuleNames.Add("SSL");
					}
					else if (Target.Platform == UnrealTargetPlatform.Win64)
					{
						string LibPath = Path.Combine(_libWebSocketsPath, "lib", "Win64");
						
						// Try to find static lib first
						string StaticLib = Path.Combine(LibPath, "websockets_static.lib");
						if (File.Exists(StaticLib))
						{
							PublicAdditionalLibraries.Add(StaticLib);
						}
						else
						{
							// Try regular lib
							string ImportLib = Path.Combine(LibPath, "websockets.lib");
							if (File.Exists(ImportLib))
							{
								PublicAdditionalLibraries.Add(ImportLib);
							}
						}
						
						// Add OpenSSL and zlib from engine
						AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "zlib");
						PrivateDependencyModuleNames.Add("SSL");
					}
					else if (Target.Platform == UnrealTargetPlatform.Mac)
					{
						string LibPath = Path.Combine(_libWebSocketsPath, "lib", "Mac");
						PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libwebsockets.a"));
						
						// Add OpenSSL and zlib from engine
						AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "zlib");
						PrivateDependencyModuleNames.Add("SSL");
					}
				}
				else
				{
					// Using engine's libwebsockets
					if (UsePlatformSSL)
					{
						PrivateDefinitions.Add("WITH_SSL=0");
						AddEngineThirdPartyPrivateStaticDependencies(Target, "libWebSockets");
					}
					else
					{
						AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "libWebSockets", "zlib");
						PrivateDependencyModuleNames.Add("SSL");
					}
				}
			}
			else if (bPlatformSupportsWinHttpWebSockets)
			{
				// Enable WinHttp Support
				bWithWebSockets = true;
				bWithWinHttpWebSockets = true;

				AddEngineThirdPartyPrivateStaticDependencies(Target, "WinHttp");
			}
			// If neither implementation is available, WebSockets will be disabled (bWithWebSockets remains false)
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
