// Copyright (c) 2024 Discord Chat Bridge Contributors

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

/**
 * Logger utility for Discord Chat Bridge
 * Provides file-based logging for errors and warnings
 */
class DISCORDCHATBRIDGE_API FDiscordChatLogger
{
public:
	/** Get the singleton instance */
	static FDiscordChatLogger& Get();

	/** Initialize the logger with log file path */
	void Initialize(const FString& LogDirectory);

	/** Log an error message to file */
	void LogError(const FString& Message);

	/** Log a warning message to file */
	void LogWarning(const FString& Message);

	/** Log an info message to file */
	void LogInfo(const FString& Message);

	/** Flush pending writes to disk */
	void Flush();

	/** Close the log file */
	void Shutdown();

	/** Check if logger is initialized */
	bool IsInitialized() const { return bIsInitialized; }

	/** Get the full path to the log file */
	FString GetLogFilePath() const { return LogFilePath; }

private:
	/** Private constructor for singleton */
	FDiscordChatLogger();

	/** Private destructor */
	~FDiscordChatLogger();

	/** Prevent copying */
	FDiscordChatLogger(const FDiscordChatLogger&) = delete;
	FDiscordChatLogger& operator=(const FDiscordChatLogger&) = delete;

	/** Write a log entry to file */
	void WriteToFile(const FString& Level, const FString& Message);

	/** Get current timestamp string */
	FString GetTimestamp() const;

	/** Ensure log directory exists */
	bool EnsureLogDirectoryExists(const FString& Directory);

	/** Path to the log file */
	FString LogFilePath;

	/** Whether the logger has been initialized */
	bool bIsInitialized;

	/** Critical section for thread safety */
	FCriticalSection LogFileLock;
};

/** Macro for logging errors to file */
#define DISCORD_LOG_ERROR(Format, ...) \
	do { \
		UE_LOG(LogTemp, Error, Format, ##__VA_ARGS__); \
		FDiscordChatLogger::Get().LogError(FString::Printf(Format, ##__VA_ARGS__)); \
	} while(0)

/** Macro for logging warnings to file */
#define DISCORD_LOG_WARNING(Format, ...) \
	do { \
		UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__); \
		FDiscordChatLogger::Get().LogWarning(FString::Printf(Format, ##__VA_ARGS__)); \
	} while(0)

/** Macro for logging info to file */
#define DISCORD_LOG_INFO(Format, ...) \
	do { \
		UE_LOG(LogTemp, Log, Format, ##__VA_ARGS__); \
		FDiscordChatLogger::Get().LogInfo(FString::Printf(Format, ##__VA_ARGS__)); \
	} while(0)
