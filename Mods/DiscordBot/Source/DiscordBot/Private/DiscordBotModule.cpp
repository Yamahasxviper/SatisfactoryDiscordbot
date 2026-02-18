// Copyright (c) 2024 Yamahasxviper

#include "DiscordBotModule.h"
#include "DiscordBotErrorLogger.h"
#include "DiscordGatewayClient.h"
#include "Logging/LogMacros.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordBot, Log, All);

void FDiscordBotModule::StartupModule()
{
    UE_LOG(LogDiscordBot, Log, TEXT("DiscordBot module starting up"));
    
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
