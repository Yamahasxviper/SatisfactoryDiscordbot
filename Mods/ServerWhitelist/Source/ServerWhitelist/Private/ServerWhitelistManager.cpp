#include "ServerWhitelistManager.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogServerWhitelist, Log, All);

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
bool          FServerWhitelistManager::bEnabled = false;
TArray<FString> FServerWhitelistManager::Players;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

FString FServerWhitelistManager::GetFilePath()
{
	return FPaths::ProjectSavedDir() / TEXT("ServerWhitelist.json");
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void FServerWhitelistManager::Load()
{
	const FString FilePath = GetFilePath();

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		UE_LOG(LogServerWhitelist, Display,
			TEXT("Whitelist file not found — creating default at %s"), *FilePath);
		Save();
		return;
	}

	FString RawJson;
	if (!FFileHelper::LoadFileToString(RawJson, *FilePath))
	{
		UE_LOG(LogServerWhitelist, Error,
			TEXT("Failed to read whitelist from %s"), *FilePath);
		return;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogServerWhitelist, Warning,
			TEXT("Whitelist JSON is malformed — resetting to defaults"));
		bEnabled = false;
		Players.Empty();
		Save();
		return;
	}

	if (Root->HasTypedField<EJson::Boolean>(TEXT("enabled")))
	{
		bEnabled = Root->GetBoolField(TEXT("enabled"));
	}

	Players.Empty();
	if (Root->HasTypedField<EJson::Array>(TEXT("players")))
	{
		for (const TSharedPtr<FJsonValue>& Val : Root->GetArrayField(TEXT("players")))
		{
			if (Val->Type == EJson::String)
			{
				Players.AddUnique(Val->AsString().ToLower());
			}
		}
	}

	UE_LOG(LogServerWhitelist, Display,
		TEXT("Whitelist loaded: %s, %d player(s)"),
		bEnabled ? TEXT("ENABLED") : TEXT("disabled"),
		Players.Num());
}

void FServerWhitelistManager::Save()
{
	const FString FilePath = GetFilePath();
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FPaths::GetPath(FilePath));

	const TSharedRef<FJsonObject> Root = MakeShareable(new FJsonObject());
	Root->SetBoolField(TEXT("enabled"), bEnabled);

	TArray<TSharedPtr<FJsonValue>> PlayerArray;
	for (const FString& Name : Players)
	{
		PlayerArray.Add(MakeShareable(new FJsonValueString(Name)));
	}
	Root->SetArrayField(TEXT("players"), PlayerArray);

	FString OutJson;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	FJsonSerializer::Serialize(Root, Writer);

	if (!FFileHelper::SaveStringToFile(OutJson, *FilePath))
	{
		UE_LOG(LogServerWhitelist, Error,
			TEXT("Failed to save whitelist to %s"), *FilePath);
		return;
	}
	UE_LOG(LogServerWhitelist, Display,
		TEXT("Whitelist saved to %s"), *FilePath);
}

bool FServerWhitelistManager::IsEnabled()
{
	return bEnabled;
}

void FServerWhitelistManager::SetEnabled(bool bNewEnabled)
{
	bEnabled = bNewEnabled;
	Save();
}

bool FServerWhitelistManager::IsWhitelisted(const FString& PlayerName)
{
	return Players.Contains(PlayerName.ToLower());
}

bool FServerWhitelistManager::AddPlayer(const FString& PlayerName)
{
	const FString Lower = PlayerName.ToLower();
	if (Players.Contains(Lower))
	{
		return false;
	}
	Players.Add(Lower);
	Save();
	return true;
}

bool FServerWhitelistManager::RemovePlayer(const FString& PlayerName)
{
	const int32 Removed = Players.Remove(PlayerName.ToLower());
	if (Removed > 0)
	{
		Save();
		return true;
	}
	return false;
}

TArray<FString> FServerWhitelistManager::GetAll()
{
	return Players;
}
