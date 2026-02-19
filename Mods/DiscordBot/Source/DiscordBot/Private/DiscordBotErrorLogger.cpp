// Copyright (c) 2024 Yamahasxviper

#include "DiscordBotErrorLogger.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/DateTime.h"

FDiscordBotErrorLogger::FDiscordBotErrorLogger()
    : bIsInitialized(false)
    , MinVerbosity(ELogVerbosity::Log)
{
}

FDiscordBotErrorLogger::~FDiscordBotErrorLogger()
{
    Shutdown();
}

void FDiscordBotErrorLogger::Initialize(const FString& InLogDirectory)
{
    if (bIsInitialized)
    {
        return;
    }

    LogDirectory = InLogDirectory;

    // Create the log directory if it doesn't exist
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*LogDirectory))
    {
        if (!PlatformFile.CreateDirectoryTree(*LogDirectory))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create Discord bot log directory: %s"), *LogDirectory);
            return;
        }
    }

    // Create log file with timestamp
    FString LogFileName = FString::Printf(TEXT("DiscordBot_%s.log"), 
        *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    LogFilePath = FPaths::Combine(LogDirectory, LogFileName);

    // Write initial header
    FString InitMessage = FString::Printf(
        TEXT("=== Discord Bot Error Log ===\n")
        TEXT("Started: %s\n")
        TEXT("=================================\n\n"),
        *FDateTime::Now().ToString());

    FFileHelper::SaveStringToFile(InitMessage, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);

    bIsInitialized = true;

    // Add ourselves to GLog's output device chain
    GLog->AddOutputDevice(this);

    UE_LOG(LogTemp, Log, TEXT("Discord Bot Error Logger initialized: %s"), *LogFilePath);
}

void FDiscordBotErrorLogger::Shutdown()
{
    if (!bIsInitialized)
    {
        return;
    }

    // Remove ourselves from the output device chain
    GLog->RemoveOutputDevice(this);

    // Write shutdown message
    FString ShutdownMessage = FString::Printf(
        TEXT("\n=================================\n")
        TEXT("Stopped: %s\n")
        TEXT("=================================\n"),
        *FDateTime::Now().ToString());

    FFileHelper::SaveStringToFile(ShutdownMessage, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);

    bIsInitialized = false;
}

void FDiscordBotErrorLogger::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
    if (!bIsInitialized)
    {
        return;
    }

    // Only log Discord bot and CustomWebSocket related messages
    FString CategoryString = Category.ToString();
    if (!CategoryString.StartsWith(TEXT("LogDiscord")) && !CategoryString.StartsWith(TEXT("LogCustomWebSocket")))
    {
        return;
    }

    // Only write messages at or more severe than the configured minimum verbosity
    if (Verbosity > MinVerbosity)
    {
        return;
    }

    // Format the log message
    FString VerbosityString;
    switch (Verbosity)
    {
        case ELogVerbosity::Error:
            VerbosityString = TEXT("ERROR");
            break;
        case ELogVerbosity::Warning:
            VerbosityString = TEXT("WARNING");
            break;
        case ELogVerbosity::Log:
            VerbosityString = TEXT("INFO");
            break;
        case ELogVerbosity::Verbose:
            VerbosityString = TEXT("VERBOSE");
            break;
        case ELogVerbosity::VeryVerbose:
            VerbosityString = TEXT("VERY VERBOSE");
            break;
        default:
            VerbosityString = TEXT("LOG");
            break;
    }

    FString FormattedMessage = FString::Printf(
        TEXT("[%s] [%s] %s: %s\n"),
        *GetTimestamp(),
        *VerbosityString,
        *CategoryString,
        V);

    WriteToFile(FormattedMessage);
}

void FDiscordBotErrorLogger::WriteToFile(const FString& Message)
{
    FScopeLock Lock(&LogMutex);

    if (!bIsInitialized || LogFilePath.IsEmpty())
    {
        return;
    }

    // Check if we need to rotate the log file
    RotateLogFileIfNeeded();

    // Append to the log file
    FFileHelper::SaveStringToFile(Message, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

FString FDiscordBotErrorLogger::GetTimestamp() const
{
    return FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}

void FDiscordBotErrorLogger::RotateLogFileIfNeeded()
{
    if (LogFilePath.IsEmpty())
    {
        return;
    }

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    // Check file size
    int64 FileSize = PlatformFile.FileSize(*LogFilePath);
    if (FileSize < MaxLogFileSize)
    {
        return;
    }

    // Rotate: rename current file with .old extension
    FString OldLogFile = LogFilePath + TEXT(".old");
    
    // Delete the previous .old file if it exists
    if (PlatformFile.FileExists(*OldLogFile))
    {
        PlatformFile.DeleteFile(*OldLogFile);
    }

    // Rename current log to .old
    PlatformFile.MoveFile(*OldLogFile, *LogFilePath);

    // Create new log file with header
    FString InitMessage = FString::Printf(
        TEXT("=== Discord Bot Error Log (Rotated) ===\n")
        TEXT("Started: %s\n")
        TEXT("=================================\n\n"),
        *FDateTime::Now().ToString());

    FFileHelper::SaveStringToFile(InitMessage, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}
