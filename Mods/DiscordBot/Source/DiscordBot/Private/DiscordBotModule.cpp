#include "DiscordBotModule.h"

#include "Misc/ConfigCacheIni.h"

void FDiscordBotModule::StartupModule()
{
    // Explicitly load the DiscordBot.ini config hierarchy into GConfig.
    //
    // In Satisfactory's CSS custom Unreal Engine, the global config hierarchy is
    // built before SML mod plugins are mounted. This means a mod plugin's
    // DefaultXxx.ini files are absent from GConfig when the engine first
    // constructs config-bearing UObjects (CDOs and later subsystem instances).
    //
    // By calling LoadGlobalIniFile here — after the DiscordBot plugin is already
    // mounted — we ensure that DefaultDiscordBot.ini (from the plugin's Config/
    // directory) is merged into GConfig together with any platform-specific
    // overrides the server operator has placed in:
    //   Saved/Config/WindowsServer/DiscordBot.ini
    //   Saved/Config/LinuxServer/DiscordBot.ini
    //
    // Without this call, UDiscordBotSubsystem::Initialize() → LoadConfig()
    // reads from GConfig but finds no [/Script/DiscordBot.DiscordBotSubsystem]
    // section, so BotToken / GuildId / ChannelId / bAutoConnect remain empty.
    FString DiscordBotIniPath;
    FConfigCacheIni::LoadGlobalIniFile(DiscordBotIniPath, TEXT("DiscordBot"), nullptr, /*bForceReload=*/true);
}

void FDiscordBotModule::ShutdownModule()
{
}

IMPLEMENT_GAME_MODULE(FDiscordBotModule, DiscordBot);
