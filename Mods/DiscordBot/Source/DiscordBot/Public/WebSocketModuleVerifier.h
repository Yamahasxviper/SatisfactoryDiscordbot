// Copyright (c) 2024 Yamahasxviper
// WebSocket Module Verification Utility

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebSocketModuleVerifier.generated.h"

/**
 * Utility class to verify WebSocket module availability in CSS Unreal Engine
 * 
 * This tool helps diagnose whether the WebSocket module is available in your
 * Unreal Engine build, including custom CSS (Coffee Stain Studios) builds.
 * 
 * Usage:
 * 1. Place this actor in your level or spawn it
 * 2. Call VerifyWebSocketModule() or check automatically on BeginPlay
 * 3. Check the output log for verification results
 */
UCLASS(Blueprintable)
class DISCORDBOT_API AWebSocketModuleVerifier : public AActor
{
    GENERATED_BODY()

public:
    AWebSocketModuleVerifier();

    virtual void BeginPlay() override;

    /** Verify if WebSocket module is available and working */
    UFUNCTION(BlueprintCallable, Category = "Discord|Verification")
    bool VerifyWebSocketModule();

    /** Get detailed verification report */
    UFUNCTION(BlueprintCallable, Category = "Discord|Verification")
    FString GetVerificationReport();

    /** Test creating a WebSocket connection (doesn't actually connect) */
    UFUNCTION(BlueprintCallable, Category = "Discord|Verification")
    bool TestWebSocketCreation(const FString& TestURL);

    /** Run all verification checks */
    UFUNCTION(BlueprintCallable, Category = "Discord|Verification")
    void RunFullVerification();

    /** Automatically verify on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Verification")
    bool bAutoVerifyOnBeginPlay;

    /** Last verification result */
    UPROPERTY(BlueprintReadOnly, Category = "Discord|Verification")
    bool bLastVerificationSucceeded;

    /** Last verification message */
    UPROPERTY(BlueprintReadOnly, Category = "Discord|Verification")
    FString LastVerificationMessage;

private:
    /** Check if module is registered */
    bool IsModuleRegistered();

    /** Check if module can be loaded */
    bool CanLoadModule();

    /** Check if WebSocket can be created */
    bool CanCreateWebSocket();

    /** Get engine version info */
    FString GetEngineVersionInfo();

    /** Check for common issues */
    TArray<FString> GetPotentialIssues();

    /** Build detailed report */
    void BuildVerificationReport();

    FString VerificationReport;
};
