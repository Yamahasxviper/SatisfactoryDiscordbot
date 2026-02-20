// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "SMLWebSocket.h"
#include "DiscordChatBridgeSubsystem.generated.h"

/**
 * Mod subsystem that bridges the Satisfactory in-game chat with a Discord channel.
 *
 * This subsystem connects (via USMLWebSocket) to a local WebSocket relay service
 * which in turn connects to the Discord Gateway API using your bot token.
 *
 * Two-way message flow
 * --------------------
 *  Game → Discord
 *    When a player sends a chat message the subsystem forwards it to the relay
 *    as: {"type":"game_message","player":"<name>","text":"<message>"}
 *
 *  Discord → Game
 *    When the relay receives a Discord message it sends:
 *    {"type":"discord_message","author":"<user>","text":"<message>"}
 *    The subsystem injects this into the game chat so all players can see it.
 *
 * Setup
 * -----
 *  1. Set BotToken, ChannelId and RelayWebSocketUrl in
 *     Config/DefaultPLUGIN_NAME.ini (or via Editor Project Settings).
 *  2. Register ADiscordChatBridgeSubsystem in your GameWorldModule's
 *     ModSubsystems array.
 *  3. Start the companion relay service before launching the server.
 */
UCLASS()
class ADiscordChatBridgeSubsystem : public AModSubsystem
{
	GENERATED_BODY()

public:
	ADiscordChatBridgeSubsystem();

	/** Returns the singleton instance for the given world context, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "Discord Bridge",
		meta = (WorldContext = "WorldContext"))
	static ADiscordChatBridgeSubsystem* Get(UObject* WorldContext);

protected:
	virtual void Init() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// -----------------------------------------------------------------------
	// Connection management
	// -----------------------------------------------------------------------

	/** Open (or re-open) the WebSocket connection to the relay. */
	void Connect();

	/** Schedule a reconnect attempt after a back-off delay. */
	void ScheduleReconnect();

	// -----------------------------------------------------------------------
	// WebSocket event handlers
	// -----------------------------------------------------------------------

	UFUNCTION()
	void OnConnected();

	UFUNCTION()
	void OnConnectionError(const FString& Error);

	UFUNCTION()
	void OnClosed(int32 StatusCode, const FString& Reason);

	UFUNCTION()
	void OnMessageReceived(const FString& Message);

	// -----------------------------------------------------------------------
	// Chat integration
	// -----------------------------------------------------------------------

	/** Bound to AFGChatManager::OnChatMessageAdded. */
	UFUNCTION()
	void OnChatMessageAdded();

	/** Serialise a JSON object and send it over the WebSocket. */
	void SendJson(const TSharedRef<FJsonObject>& Json);

	/**
	 * Broadcast a chat message to all players in the game world.
	 * @param Sender  Display name shown as the message author.
	 * @param Text    Message body.
	 */
	void PostGameChatMessage(const FString& Sender, const FString& Text);

	// -----------------------------------------------------------------------
	// Members
	// -----------------------------------------------------------------------

	/** The WebSocket connection to the local relay service. */
	UPROPERTY()
	USMLWebSocket* WebSocket;

	/** True after the relay has acknowledged the identify frame. */
	bool bIdentified;

	/** Current back-off delay (seconds) before the next reconnect attempt. */
	float ReconnectDelaySec;

	/** Pending reconnect timer. */
	FTimerHandle ReconnectTimerHandle;

	/**
	 * Set to true while we are injecting a Discord message into the game chat
	 * so that OnChatMessageAdded does not echo it back to Discord.
	 */
	bool bInjectingMessage;
};
