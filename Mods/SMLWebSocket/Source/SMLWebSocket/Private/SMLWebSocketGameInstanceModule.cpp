// Copyright Epic Games, Inc. All Rights Reserved.

#include "SMLWebSocketGameInstanceModule.h"

#include "DiscordGatewayWebSocket.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogSMLWebSocketModule, Log, All);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/** Return the absolute path to Config/PluginSettings.ini for this plugin. */
static FString GetPluginSettingsPath()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("SMLWebSocket"));
	if (!Plugin.IsValid())
	{
		return FString();
	}
	return FPaths::ConvertRelativePathToFull(
		FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config/PluginSettings.ini")));
}

// ---------------------------------------------------------------------------
// USMLWebSocketGameInstanceModule
// ---------------------------------------------------------------------------

USMLWebSocketGameInstanceModule::USMLWebSocketGameInstanceModule()
	: GatewayWebSocket(nullptr)
{
	// Mark this as the single root GameInstanceModule for the SMLWebSocket
	// plugin so that SML auto-discovers and instantiates it.
	bRootModule = true;
}

void USMLWebSocketGameInstanceModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
	Super::DispatchLifecycleEvent(Phase);

	if (Phase == ELifecyclePhase::INITIALIZATION)
	{
		AutoConnect();
	}
}

void USMLWebSocketGameInstanceModule::AutoConnect()
{
	// ---- Load plugin-specific ini file ----
	const FString ConfigPath = GetPluginSettingsPath();
	if (ConfigPath.IsEmpty())
	{
		UE_LOG(LogSMLWebSocketModule, Warning,
			TEXT("SMLWebSocket: could not locate plugin directory; skipping auto-connect."));
		return;
	}

	// Load (or re-use if already cached) the config file.
	FConfigFile PluginConfig;
	PluginConfig.Read(ConfigPath);

	static const TCHAR* Section = TEXT("/SMLWebSocket/Connection");

	// ---- bAutoConnect ----
	bool bAutoConnect = false;
	FString AutoConnectStr;
	if (PluginConfig.GetString(Section, TEXT("bAutoConnect"), AutoConnectStr))
	{
		bAutoConnect = AutoConnectStr.TrimStartAndEnd().Equals(TEXT("true"), ESearchCase::IgnoreCase);
	}

	if (!bAutoConnect)
	{
		UE_LOG(LogSMLWebSocketModule, Log,
			TEXT("SMLWebSocket: bAutoConnect is false – Discord Gateway will not connect automatically. "
			     "Set bAutoConnect=true in Config/PluginSettings.ini to enable auto-connect."));
		return;
	}

	// ---- BotToken ----
	FString BotToken;
	PluginConfig.GetString(Section, TEXT("BotToken"), BotToken);
	BotToken = BotToken.TrimStartAndEnd();

	if (BotToken.IsEmpty())
	{
		UE_LOG(LogSMLWebSocketModule, Warning,
			TEXT("SMLWebSocket: bAutoConnect is true but BotToken is empty. "
			     "Set BotToken in Config/PluginSettings.ini and restart."));
		return;
	}

	// ---- Intents ----
	int32 Intents = 33280; // GuildMessages | MessageContent (safe default)
	FString IntentsStr;
	if (PluginConfig.GetString(Section, TEXT("Intents"), IntentsStr))
	{
		const int32 Parsed = FCString::Atoi(*IntentsStr.TrimStartAndEnd());
		if (Parsed > 0)
		{
			Intents = Parsed;
		}
	}

	// ---- Connect ----
	UE_LOG(LogSMLWebSocketModule, Log,
		TEXT("SMLWebSocket: auto-connecting to Discord Gateway (intents=%d)…"), Intents);

	GatewayWebSocket = UDiscordGatewayWebSocket::CreateDiscordGateway(this);
	GatewayWebSocket->ConnectToDiscord(BotToken, Intents);
}
