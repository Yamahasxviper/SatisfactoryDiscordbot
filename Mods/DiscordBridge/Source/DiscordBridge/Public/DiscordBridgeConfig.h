// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "DiscordBridgeConfig.generated.h"

/**
 * Configuration for the Discord Bridge mod.
 *
 * Edit these settings via the Project Settings panel (category "Discord Bridge")
 * or directly in Saved/Config/{Platform}/DiscordBridge.ini.
 *
 * IMPORTANT: The BotToken is a sensitive secret.  Do not share your config file
 * or commit it to version control.
 *
 * Discord bot requirements
 * ────────────────────────
 *  • Privileged Gateway Intents that must be enabled in the Discord Developer
 *    Portal (Bot → Privileged Gateway Intents):
 *      – Presence Intent        (GUILD_PRESENCES)
 *      – Server Members Intent  (GUILD_MEMBERS)
 *      – Message Content Intent (MESSAGE_CONTENT)
 *  • The bot must be invited to your server with at minimum the
 *    "Send Messages" and "Read Message History" permissions in the target channel.
 */
UCLASS(Config=DiscordBridge, DefaultConfig, meta=(DisplayName="Discord Bridge"))
class DISCORDBRIDGE_API UDiscordBridgeConfig : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDiscordBridgeConfig();

	// UDeveloperSettings overrides
	virtual FName GetCategoryName() const override { return FName(TEXT("Mods")); }
	virtual FName GetSectionName()  const override { return FName(TEXT("Discord Bridge")); }

	// ── Connection ────────────────────────────────────────────────────────────

	/**
	 * Discord bot token.
	 * Found in the Discord Developer Portal → Your Application → Bot → Token.
	 * Treat this value as a password.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Discord Bridge|Connection")
	FString BotToken;

	/**
	 * Snowflake ID of the Discord text channel that will be bridged with the
	 * Satisfactory in-game chat.  Right-click the channel in Discord (with
	 * Developer Mode enabled) and choose "Copy Channel ID".
	 */
	UPROPERTY(Config, EditAnywhere, Category="Discord Bridge|Connection")
	FString ChannelId;

	// ── Display names ─────────────────────────────────────────────────────────

	/**
	 * Format used when a game-chat message is forwarded to Discord.
	 * Available placeholders:
	 *   {PlayerName} – the in-game player name
	 *   {Message}    – the chat message text
	 *
	 * Default: "**{PlayerName}**: {Message}"
	 */
	UPROPERTY(Config, EditAnywhere, Category="Discord Bridge|Display Names")
	FString GameToDiscordFormat;

	/**
	 * Format used when a Discord message is forwarded to the in-game chat.
	 * Available placeholders:
	 *   {Username} – the Discord user's display name / username
	 *   {Message}  – the Discord message text
	 *
	 * Default: "[Discord] {Username}: {Message}"
	 */
	UPROPERTY(Config, EditAnywhere, Category="Discord Bridge|Display Names")
	FString DiscordToGameFormat;

	// ── Behaviour ─────────────────────────────────────────────────────────────

	/**
	 * When true, messages sent by the bot itself are silently ignored so that
	 * messages forwarded from the game do not echo back into the game.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Discord Bridge|Behaviour")
	bool bIgnoreBotMessages{true};
};
