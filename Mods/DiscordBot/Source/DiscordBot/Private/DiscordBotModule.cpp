// Copyright (c) 2024 Yamahasxviper

#include "DiscordBotModule.h"
#include "DiscordBotErrorLogger.h"
#include "DiscordGatewayClient.h"
#include "Logging/LogMacros.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBot, Log, All);

void FDiscordBotModule::StartupModule()
{
    UE_LOG(LogDiscordBot, Log, TEXT("DiscordBot module starting up"));
    
    // Verify CustomWebSocket module is loaded
    if (!FModuleManager::Get().IsModuleLoaded(TEXT("CustomWebSocket")))
    {
        UE_LOG(LogDiscordBot, Warning, TEXT("CustomWebSocket module not loaded, attempting to load..."));
        if (!FModuleManager::Get().LoadModule(TEXT("CustomWebSocket")))
        {
            UE_LOG(LogDiscordBot, Error, TEXT("Failed to load CustomWebSocket module! Please ensure the CustomWebSocket plugin is installed in the Mods folder."));
            UE_LOG(LogDiscordBot, Error, TEXT("The DiscordBot requires the CustomWebSocket plugin to function. Check that:"));
            UE_LOG(LogDiscordBot, Error, TEXT("  1. Mods/CustomWebSocket/ folder exists"));
            UE_LOG(LogDiscordBot, Error, TEXT("  2. CustomWebSocket.uplugin is present"));
            UE_LOG(LogDiscordBot, Error, TEXT("  3. The plugin is enabled in your project"));
        }
        else
        {
            UE_LOG(LogDiscordBot, Log, TEXT("CustomWebSocket module loaded successfully"));
        }
    }
    else
    {
        UE_LOG(LogDiscordBot, Log, TEXT("CustomWebSocket module already loaded"));
    }
    
    // Initialize error logger
    ErrorLogger = MakeUnique<FDiscordBotErrorLogger>();
    
    // Determine log directory path
    FString LogDirectory;
    
    // Try to load from config first
    if (GConfig)
    {
        // Use explicit config filename for cross-platform compatibility (especially dedicated servers)
        FString ConfigFilename = GConfig->GetConfigFilename(TEXT("Game"));
        GConfig->GetString(TEXT("DiscordBot"), TEXT("ErrorLogDirectory"), LogDirectory, ConfigFilename);
    }
    
    // If not configured, use default location in the mod's directory
    if (LogDirectory.IsEmpty())
    {
        // Use Saved/Logs/DiscordBot as default location
        LogDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Logs"), TEXT("DiscordBot"));
    }
    
    ErrorLogger->Initialize(LogDirectory);
    
    UE_LOG(LogDiscordBot, Log, TEXT("Error logging initialized at: %s"), *LogDirectory);
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
