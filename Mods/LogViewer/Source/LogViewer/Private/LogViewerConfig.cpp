// Copyright Coffee Stain Studios. All Rights Reserved.

#include "LogViewerConfig.h"

#include "Dom/JsonObject.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
	FString GetStringFieldOrDefault(const TSharedPtr<FJsonObject>& Json,
	                                const FString& Key,
	                                const FString& Default)
	{
		FString Value;
		if (Json->TryGetStringField(Key, Value))
		{
			return Value;
		}
		return Default;
	}
} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// FLogViewerConfig
// ─────────────────────────────────────────────────────────────────────────────

FString FLogViewerConfig::GetConfigFilePath()
{
	return FPaths::ProjectDir() + TEXT("Configs/LogViewer.cfg");
}

FLogViewerConfig FLogViewerConfig::LoadOrCreate()
{
	FLogViewerConfig Config;
	const FString FilePath = GetConfigFilePath();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	bool bNeedsSave = false;

	if (PlatformFile.FileExists(*FilePath))
	{
		FString FileContent;
		if (FFileHelper::LoadFileToString(FileContent, *FilePath))
		{
			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(FileContent);

			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				double PortDouble = static_cast<double>(Config.Port);
				if (JsonObject->TryGetNumberField(TEXT("Port"), PortDouble))
				{
					Config.Port = FMath::Clamp(static_cast<int32>(PortDouble), 1, 65535);
				}

				double LogLineCountDouble = static_cast<double>(Config.LogLineCount);
				if (JsonObject->TryGetNumberField(TEXT("LogLineCount"), LogLineCountDouble))
				{
					Config.LogLineCount = FMath::Clamp(static_cast<int32>(LogLineCountDouble), 0, 10000);
				}

				Config.AuthToken = GetStringFieldOrDefault(JsonObject, TEXT("AuthToken"), TEXT(""));

				UE_LOG(LogTemp, Log, TEXT("LogViewer: Loaded config from %s"), *FilePath);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("LogViewer: Failed to parse config JSON at '%s'. "
				            "Recreating the file with defaults."), *FilePath);
				bNeedsSave = true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("LogViewer: Failed to read config file at '%s'. "
			            "Using built-in defaults."), *FilePath);
			bNeedsSave = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Log,
		       TEXT("LogViewer: Config file not found at '%s'. "
		            "Creating it with defaults."), *FilePath);
		bNeedsSave = true;
	}

	if (bNeedsSave)
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		JsonObject->SetNumberField(TEXT("Port"),         Config.Port);
		JsonObject->SetNumberField(TEXT("LogLineCount"), Config.LogLineCount);
		JsonObject->SetStringField(TEXT("AuthToken"),    Config.AuthToken);

		FString JsonContent;
		const TSharedRef<TJsonWriter<>> Writer =
			TJsonWriterFactory<>::Create(&JsonContent);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

		PlatformFile.CreateDirectoryTree(*FPaths::GetPath(FilePath));

		if (FFileHelper::SaveStringToFile(JsonContent, *FilePath))
		{
			UE_LOG(LogTemp, Log,
			       TEXT("LogViewer: Wrote default config to '%s'. "
			            "Browse to http://<server-ip>:%d/logs to view the server log."),
			       *FilePath, Config.Port);
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("LogViewer: Could not write default config to '%s'."), *FilePath);
		}
	}

	return Config;
}
