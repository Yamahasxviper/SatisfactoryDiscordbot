// Copyright (c) 2024 Yamahasxviper

#include "DiscordBotModule.h"
#include "DiscordBotErrorLogger.h"
#include "DiscordGatewayClient.h"
#include "Logging/LogMacros.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBot, Log, All);

void FDiscordBotModule::StartupModule()
{
    UE_LOG(LogDiscordBot, Log, TEXT("DiscordBot module starting up"));
    
    // Verify CustomWebSocket module is loaded
    if (!FModuleManager::Get().IsModuleLoaded(TEXT("CustomWebSocket")))
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("CustomWebSocket module not loaded, attempting to load..."));
        
        // First, check if the CustomWebSocket plugin is even discovered
        TSharedPtr<IPlugin> CustomWebSocketPlugin = IPluginManager::Get().FindPlugin(TEXT("CustomWebSocket"));
        
        if (!CustomWebSocketPlugin.IsValid())
        {
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
            UE_LOG(LogDiscordBot, Error, TEXT("CRITICAL ERROR: CustomWebSocket plugin not found!"));
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
            UE_LOG(LogDiscordBot, Error, TEXT("The DiscordBot mod requires the CustomWebSocket mod to be installed."));
            UE_LOG(LogDiscordBot, Error, TEXT(""));
            UE_LOG(LogDiscordBot, Error, TEXT("SOLUTION:"));
            UE_LOG(LogDiscordBot, Error, TEXT("  1. Install the CustomWebSocket mod from the Satisfactory Mod Manager (SMM)"));
            UE_LOG(LogDiscordBot, Error, TEXT("  2. Make sure both DiscordBot and CustomWebSocket are enabled"));
            UE_LOG(LogDiscordBot, Error, TEXT("  3. Restart your game/server"));
            UE_LOG(LogDiscordBot, Error, TEXT(""));
            UE_LOG(LogDiscordBot, Error, TEXT("NOTE: DiscordBot and CustomWebSocket are separate mods and must BOTH be installed."));
            UE_LOG(LogDiscordBot, Error, TEXT("They should be installed in the same mods directory (typically via mod manager)."));
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
        }
        else if (!CustomWebSocketPlugin->IsEnabled())
        {
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
            UE_LOG(LogDiscordBot, Error, TEXT("ERROR: CustomWebSocket plugin found but not enabled!"));
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
            UE_LOG(LogDiscordBot, Error, TEXT("Plugin location: %s"), *CustomWebSocketPlugin->GetBaseDir());
            UE_LOG(LogDiscordBot, Error, TEXT(""));
            UE_LOG(LogDiscordBot, Error, TEXT("SOLUTION:"));
            UE_LOG(LogDiscordBot, Error, TEXT("  1. Enable the CustomWebSocket mod in your mod manager"));
            UE_LOG(LogDiscordBot, Error, TEXT("  2. Restart your game/server"));
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
        }
        else if (!FModuleManager::Get().LoadModule(TEXT("CustomWebSocket")))
        {
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
            UE_LOG(LogDiscordBot, Error, TEXT("ERROR: Failed to load CustomWebSocket module!"));
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
            UE_LOG(LogDiscordBot, Error, TEXT("Plugin location: %s"), *CustomWebSocketPlugin->GetBaseDir());
            UE_LOG(LogDiscordBot, Error, TEXT("Plugin is enabled: %s"), CustomWebSocketPlugin->IsEnabled() ? TEXT("Yes") : TEXT("No"));
            UE_LOG(LogDiscordBot, Error, TEXT(""));
            UE_LOG(LogDiscordBot, Error, TEXT("This is likely a plugin loading order issue. Try:"));
            UE_LOG(LogDiscordBot, Error, TEXT("  1. Verify both mods are the latest versions"));
            UE_LOG(LogDiscordBot, Error, TEXT("  2. Try reinstalling both DiscordBot and CustomWebSocket mods"));
            UE_LOG(LogDiscordBot, Error, TEXT("  3. Make sure no other mods conflict with these mods"));
            UE_LOG(LogDiscordBot, Error, TEXT("  4. Check the game logs for additional errors"));
            UE_LOG(LogDiscordBot, Error, TEXT("========================================================================================"));
        }
        else
        {
            UE_LOG(LogDiscordBot, Log, TEXT("CustomWebSocket module loaded successfully from: %s"), *CustomWebSocketPlugin->GetBaseDir());
        }
    }
    else
    {
        TSharedPtr<IPlugin> CustomWebSocketPlugin = IPluginManager::Get().FindPlugin(TEXT("CustomWebSocket"));
        if (CustomWebSocketPlugin.IsValid())
        {
            UE_LOG(LogDiscordBot, Log, TEXT("CustomWebSocket module already loaded from: %s"), *CustomWebSocketPlugin->GetBaseDir());
        }
        else
        {
            UE_LOG(LogDiscordBot, Log, TEXT("CustomWebSocket module already loaded"));
        }
    }
    
    // Initialize error logger
    ErrorLogger = MakeUnique<FDiscordBotErrorLogger>();
    
    // Determine log directory path
    FString LogDirectory;
    
    // Resolve the plugin's DiscordBot.ini config file, falling back to Game.ini
    FString PluginConfigFilename;
    {
        TSharedPtr<IPlugin> DiscordBotPlugin = IPluginManager::Get().FindPlugin(TEXT("DiscordBot"));
        if (DiscordBotPlugin.IsValid())
        {
            FString PluginConfigPath = DiscordBotPlugin->GetBaseDir() / TEXT("Config") / TEXT("DiscordBot.ini");
            if (FPaths::FileExists(PluginConfigPath))
            {
                if (!GConfig->Find(PluginConfigPath, false))
                {
                    GConfig->LoadFile(PluginConfigPath);
                }
                PluginConfigFilename = PluginConfigPath;
            }
        }
        if (PluginConfigFilename.IsEmpty() && GConfig)
        {
            PluginConfigFilename = GConfig->GetConfigFilename(TEXT("Game"));
        }
    }

    // Try to load from config first
    if (GConfig && !PluginConfigFilename.IsEmpty())
    {
        GConfig->GetString(TEXT("DiscordBot"), TEXT("ErrorLogDirectory"), LogDirectory, PluginConfigFilename);
    }
    
    // If not configured, use default location in the mod's directory
    if (LogDirectory.IsEmpty())
    {
        // Use Saved/Logs/DiscordBot as default location
        LogDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Logs"), TEXT("DiscordBot"));
    }
    
    ErrorLogger->Initialize(LogDirectory);
    
    // Read LogLevel from config and apply to error logger
    // LogLevel: 0=Error, 1=Warning, 2=Log (default), 3=Verbose
    int32 LogLevelValue = 2;
    if (GConfig && !PluginConfigFilename.IsEmpty())
    {
        GConfig->GetInt(TEXT("DiscordBot"), TEXT("LogLevel"), LogLevelValue, PluginConfigFilename);
    }
    ELogVerbosity::Type ConfiguredVerbosity;
    switch (LogLevelValue)
    {
        case 0:  ConfiguredVerbosity = ELogVerbosity::Error;   break;
        case 1:  ConfiguredVerbosity = ELogVerbosity::Warning; break;
        case 3:  ConfiguredVerbosity = ELogVerbosity::Verbose; break;
        default: ConfiguredVerbosity = ELogVerbosity::Log;     break;
    }
    ErrorLogger->SetMinVerbosity(ConfiguredVerbosity);
    
    // Convert to absolute path for clearer logging
    FString AbsoluteLogDirectory = FPaths::ConvertRelativePathToFull(LogDirectory);
    UE_LOG(LogDiscordBot, Log, TEXT("Error logging initialized at: %s"), *AbsoluteLogDirectory);
}

void FDiscordBotModule::ShutdownModule()
{
    UE_LOG(LogDiscordBot, Log, TEXT("DiscordBot module shutting down"));
    
    // Shutdown error logger
    if (ErrorLogger)
    {
        ErrorLogger->Shutdown();
        ErrorLogger.Reset();
    }
}

IMPLEMENT_MODULE(FDiscordBotModule, DiscordBot)
