// Copyright (c) 2024 Yamahasxviper

#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"

/**
 * Error logger that writes Discord bot errors and warnings to a log file
 * This allows server administrators to view logs without access to server console
 */
class DISCORDBOT_API FDiscordBotErrorLogger : public FOutputDevice
{
public:
    FDiscordBotErrorLogger();
    virtual ~FDiscordBotErrorLogger();

    /** Initialize the error logger with a log file path */
    void Initialize(const FString& LogDirectory);

    /** Shut down the error logger and close the file */
    void Shutdown();

    /** Write a log message to the file */
    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

    /** Check if the logger is initialized */
    bool IsInitialized() const { return bIsInitialized; }

    /** Set the minimum verbosity level for messages written to the log file */
    void SetMinVerbosity(ELogVerbosity::Type InMinVerbosity) { MinVerbosity = InMinVerbosity; }

private:
    /** Write a formatted message to the log file */
    void WriteToFile(const FString& Message);

    /** Get the current timestamp in readable format */
    FString GetTimestamp() const;

    /** Rotate log file if it exceeds max size */
    void RotateLogFileIfNeeded();

    /** Full path to the log file */
    FString LogFilePath;

    /** Directory where logs are stored */
    FString LogDirectory;

    /** Whether the logger is initialized */
    bool bIsInitialized;

    /** Maximum log file size in bytes (10MB) */
    static constexpr int64 MaxLogFileSize = 10 * 1024 * 1024;

    /** Minimum verbosity level – messages at this level or more severe are written to the log file */
    ELogVerbosity::Type MinVerbosity;

    /** Critical section for thread safety */
    FCriticalSection LogMutex;
};
