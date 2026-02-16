// Copyright (c) 2024 Discord Chat Bridge Contributors

#include "DiscordChatLogger.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"

FDiscordChatLogger::FDiscordChatLogger()
	: bIsInitialized(false)
{
}

FDiscordChatLogger::~FDiscordChatLogger()
{
	Shutdown();
}

FDiscordChatLogger& FDiscordChatLogger::Get()
{
	static FDiscordChatLogger Instance;
	return Instance;
}

void FDiscordChatLogger::Initialize(const FString& LogDirectory)
{
	FScopeLock Lock(&LogFileLock);

	if (bIsInitialized)
	{
		return;
	}

	// Ensure the log directory exists
	if (!EnsureLogDirectoryExists(LogDirectory))
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordChatLogger: Failed to create log directory: %s"), *LogDirectory);
		return;
	}

	// Create log file with timestamp
	FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	FString LogFileName = FString::Printf(TEXT("DiscordChatBridge_%s.log"), *Timestamp);
	LogFilePath = FPaths::Combine(LogDirectory, LogFileName);

	// Write initial log entry
	FString InitMessage = FString::Printf(
		TEXT("=============================================================================\n")
		TEXT("Discord Chat Bridge - Log File\n")
		TEXT("Started: %s\n")
		TEXT("Log File: %s\n")
		TEXT("=============================================================================\n"),
		*FDateTime::Now().ToString(),
		*LogFilePath
	);

	if (FFileHelper::SaveStringToFile(InitMessage, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append))
	{
		bIsInitialized = true;
		UE_LOG(LogTemp, Log, TEXT("DiscordChatLogger: Initialized successfully. Log file: %s"), *LogFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DiscordChatLogger: Failed to create log file: %s"), *LogFilePath);
	}
}

void FDiscordChatLogger::LogError(const FString& Message)
{
	if (!bIsInitialized)
	{
		return;
	}
	WriteToFile(TEXT("ERROR"), Message);
}

void FDiscordChatLogger::LogWarning(const FString& Message)
{
	if (!bIsInitialized)
	{
		return;
	}
	WriteToFile(TEXT("WARNING"), Message);
}

void FDiscordChatLogger::LogInfo(const FString& Message)
{
	if (!bIsInitialized)
	{
		return;
	}
	WriteToFile(TEXT("INFO"), Message);
}

void FDiscordChatLogger::WriteToFile(const FString& Level, const FString& Message)
{
	FScopeLock Lock(&LogFileLock);

	if (!bIsInitialized)
	{
		return;
	}

	FString LogEntry = FString::Printf(
		TEXT("[%s] [%s] %s\n"),
		*GetTimestamp(),
		*Level,
		*Message
	);

	// Append to file
	FFileHelper::SaveStringToFile(LogEntry, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

FString FDiscordChatLogger::GetTimestamp() const
{
	return FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}

bool FDiscordChatLogger::EnsureLogDirectoryExists(const FString& Directory)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	// Check if directory exists
	if (PlatformFile.DirectoryExists(*Directory))
	{
		return true;
	}

	// Try to create directory
	if (PlatformFile.CreateDirectoryTree(*Directory))
	{
		UE_LOG(LogTemp, Log, TEXT("DiscordChatLogger: Created log directory: %s"), *Directory);
		return true;
	}

	return false;
}

void FDiscordChatLogger::Flush()
{
	// File writes are synchronous, so nothing to flush
}

void FDiscordChatLogger::Shutdown()
{
	FScopeLock Lock(&LogFileLock);
	
	if (bIsInitialized)
	{
		// Write shutdown message
		FString ShutdownMessage = FString::Printf(
			TEXT("=============================================================================\n")
			TEXT("Discord Chat Bridge - Shutting Down\n")
			TEXT("Stopped: %s\n")
			TEXT("=============================================================================\n"),
			*FDateTime::Now().ToString()
		);
		
		FFileHelper::SaveStringToFile(ShutdownMessage, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		
		bIsInitialized = false;
		UE_LOG(LogTemp, Log, TEXT("DiscordChatLogger: Shutdown complete"));
	}
}
