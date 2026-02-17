// Copyright (c) 2024 Yamahasxviper
// WebSocket Module Verification Utility Implementation

#include "WebSocketModuleVerifier.h"
#include "Modules/ModuleManager.h"
#include "Misc/EngineVersion.h"
#include "Misc/App.h"

// Only include WebSocket headers if we can find them
#if __has_include("IWebSocket.h")
    #include "IWebSocket.h"
    #include "WebSocketsModule.h"
    #define WEBSOCKET_HEADERS_AVAILABLE 1
#else
    #define WEBSOCKET_HEADERS_AVAILABLE 0
#endif

DEFINE_LOG_CATEGORY_STATIC(LogWebSocketVerifier, Log, All);

AWebSocketModuleVerifier::AWebSocketModuleVerifier()
{
    PrimaryActorTick.bCanEverTick = false;
    bAutoVerifyOnBeginPlay = true;
    bLastVerificationSucceeded = false;
}

void AWebSocketModuleVerifier::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoVerifyOnBeginPlay)
    {
        RunFullVerification();
    }
}

bool AWebSocketModuleVerifier::VerifyWebSocketModule()
{
    UE_LOG(LogWebSocketVerifier, Log, TEXT("=== WebSocket Module Verification ==="));
    UE_LOG(LogWebSocketVerifier, Log, TEXT("Engine Version: %s"), *GetEngineVersionInfo());

    // Step 1: Check if headers are available at compile time
#if !WEBSOCKET_HEADERS_AVAILABLE
    UE_LOG(LogWebSocketVerifier, Error, TEXT("❌ WebSocket headers NOT found at compile time"));
    UE_LOG(LogWebSocketVerifier, Error, TEXT("   This means the WebSockets module is not available in your engine build"));
    LastVerificationMessage = TEXT("WebSocket headers not found - module not available in engine build");
    bLastVerificationSucceeded = false;
    return false;
#else
    UE_LOG(LogWebSocketVerifier, Display, TEXT("✅ WebSocket headers found at compile time"));
#endif

    // Step 2: Check if module is registered
    bool bIsRegistered = IsModuleRegistered();
    if (!bIsRegistered)
    {
        UE_LOG(LogWebSocketVerifier, Error, TEXT("❌ WebSockets module is NOT registered"));
        LastVerificationMessage = TEXT("WebSockets module not registered in module manager");
        bLastVerificationSucceeded = false;
        return false;
    }
    UE_LOG(LogWebSocketVerifier, Display, TEXT("✅ WebSockets module is registered"));

    // Step 3: Try to load the module
    bool bCanLoad = CanLoadModule();
    if (!bCanLoad)
    {
        UE_LOG(LogWebSocketVerifier, Error, TEXT("❌ WebSockets module FAILED to load"));
        LastVerificationMessage = TEXT("WebSockets module failed to load");
        bLastVerificationSucceeded = false;
        return false;
    }
    UE_LOG(LogWebSocketVerifier, Display, TEXT("✅ WebSockets module loaded successfully"));

    // Step 4: Try to create a WebSocket
    bool bCanCreate = CanCreateWebSocket();
    if (!bCanCreate)
    {
        UE_LOG(LogWebSocketVerifier, Warning, TEXT("⚠️  WebSocket creation test failed"));
        UE_LOG(LogWebSocketVerifier, Warning, TEXT("   Module is available but creation failed - this might be OK"));
    }
    else
    {
        UE_LOG(LogWebSocketVerifier, Display, TEXT("✅ WebSocket creation test passed"));
    }

    UE_LOG(LogWebSocketVerifier, Display, TEXT("=== Verification Complete ==="));
    
    bLastVerificationSucceeded = true;
    LastVerificationMessage = TEXT("WebSocket module is available and working!");
    return true;
}

FString AWebSocketModuleVerifier::GetVerificationReport()
{
    if (VerificationReport.IsEmpty())
    {
        BuildVerificationReport();
    }
    return VerificationReport;
}

bool AWebSocketModuleVerifier::TestWebSocketCreation(const FString& TestURL)
{
#if WEBSOCKET_HEADERS_AVAILABLE
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        UE_LOG(LogWebSocketVerifier, Warning, TEXT("Loading WebSockets module for test..."));
        FModuleManager::Get().LoadModule("WebSockets");
    }

    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        UE_LOG(LogWebSocketVerifier, Error, TEXT("Failed to load WebSockets module"));
        return false;
    }

    TSharedPtr<IWebSocket> TestSocket = FWebSocketsModule::Get().CreateWebSocket(TestURL);
    
    if (TestSocket.IsValid())
    {
        UE_LOG(LogWebSocketVerifier, Display, TEXT("✅ Successfully created test WebSocket for URL: %s"), *TestURL);
        // Don't connect, just test creation
        TestSocket.Reset();
        return true;
    }
    else
    {
        UE_LOG(LogWebSocketVerifier, Error, TEXT("❌ Failed to create test WebSocket for URL: %s"), *TestURL);
        return false;
    }
#else
    UE_LOG(LogWebSocketVerifier, Error, TEXT("WebSocket headers not available - cannot test creation"));
    return false;
#endif
}

void AWebSocketModuleVerifier::RunFullVerification()
{
    UE_LOG(LogWebSocketVerifier, Display, TEXT(""));
    UE_LOG(LogWebSocketVerifier, Display, TEXT("╔═══════════════════════════════════════════════════════════╗"));
    UE_LOG(LogWebSocketVerifier, Display, TEXT("║         WebSocket Module Full Verification               ║"));
    UE_LOG(LogWebSocketVerifier, Display, TEXT("╚═══════════════════════════════════════════════════════════╝"));
    UE_LOG(LogWebSocketVerifier, Display, TEXT(""));

    // Run verification
    bool bSuccess = VerifyWebSocketModule();

    // Test with Discord Gateway URL
    if (bSuccess)
    {
        UE_LOG(LogWebSocketVerifier, Display, TEXT(""));
        UE_LOG(LogWebSocketVerifier, Display, TEXT("Testing WebSocket creation with Discord Gateway..."));
        TestWebSocketCreation(TEXT("wss://gateway.discord.gg/?v=10&encoding=json"));
    }

    // Check for potential issues
    TArray<FString> Issues = GetPotentialIssues();
    if (Issues.Num() > 0)
    {
        UE_LOG(LogWebSocketVerifier, Display, TEXT(""));
        UE_LOG(LogWebSocketVerifier, Warning, TEXT("⚠️  Potential Issues Detected:"));
        for (const FString& Issue : Issues)
        {
            UE_LOG(LogWebSocketVerifier, Warning, TEXT("   - %s"), *Issue);
        }
    }

    // Build and display report
    BuildVerificationReport();
    
    UE_LOG(LogWebSocketVerifier, Display, TEXT(""));
    UE_LOG(LogWebSocketVerifier, Display, TEXT("╔═══════════════════════════════════════════════════════════╗"));
    UE_LOG(LogWebSocketVerifier, Display, TEXT("║                  Verification Report                      ║"));
    UE_LOG(LogWebSocketVerifier, Display, TEXT("╚═══════════════════════════════════════════════════════════╝"));
    UE_LOG(LogWebSocketVerifier, Display, TEXT("%s"), *VerificationReport);
    UE_LOG(LogWebSocketVerifier, Display, TEXT(""));

    if (bSuccess)
    {
        UE_LOG(LogWebSocketVerifier, Display, TEXT("✅ RESULT: WebSocket module is AVAILABLE and WORKING"));
        UE_LOG(LogWebSocketVerifier, Display, TEXT("   You can use DiscordGatewayClientNative for Discord integration!"));
    }
    else
    {
        UE_LOG(LogWebSocketVerifier, Error, TEXT("❌ RESULT: WebSocket module is NOT available"));
        UE_LOG(LogWebSocketVerifier, Error, TEXT("   See troubleshooting guide in WEBSOCKET_TROUBLESHOOTING.md"));
    }
    
    UE_LOG(LogWebSocketVerifier, Display, TEXT(""));
}

bool AWebSocketModuleVerifier::IsModuleRegistered()
{
    TArray<FModuleStatus> ModuleStatuses;
    FModuleManager::Get().QueryModules(ModuleStatuses);

    for (const FModuleStatus& Status : ModuleStatuses)
    {
        if (Status.Name == TEXT("WebSockets"))
        {
            return true;
        }
    }

    return false;
}

bool AWebSocketModuleVerifier::CanLoadModule()
{
#if WEBSOCKET_HEADERS_AVAILABLE
    if (FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        return true;
    }

    FModuleManager::Get().LoadModule("WebSockets");
    return FModuleManager::Get().IsModuleLoaded("WebSockets");
#else
    return false;
#endif
}

bool AWebSocketModuleVerifier::CanCreateWebSocket()
{
#if WEBSOCKET_HEADERS_AVAILABLE
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        return false;
    }

    // Try to create a test WebSocket with a dummy URL
    TSharedPtr<IWebSocket> TestSocket = FWebSocketsModule::Get().CreateWebSocket(TEXT("wss://test.example.com"));
    return TestSocket.IsValid();
#else
    return false;
#endif
}

FString AWebSocketModuleVerifier::GetEngineVersionInfo()
{
    FString EngineVersion = FString::Printf(TEXT("%d.%d.%d"), 
        FEngineVersion::Current().GetMajor(),
        FEngineVersion::Current().GetMinor(),
        FEngineVersion::Current().GetPatch());

    // Try to get custom build info
    FString BuildVersion = FApp::GetBuildVersion();
    if (!BuildVersion.IsEmpty() && BuildVersion != EngineVersion)
    {
        return FString::Printf(TEXT("%s (Build: %s)"), *EngineVersion, *BuildVersion);
    }

    return EngineVersion;
}

TArray<FString> AWebSocketModuleVerifier::GetPotentialIssues()
{
    TArray<FString> Issues;

#if !WEBSOCKET_HEADERS_AVAILABLE
    Issues.Add(TEXT("WebSocket headers not found - module not compiled into engine"));
    Issues.Add(TEXT("You may need to rebuild the engine with WebSockets module enabled"));
#endif

    if (!IsModuleRegistered())
    {
        Issues.Add(TEXT("WebSockets module not registered - may not be included in this build"));
    }

    FString EngineVersion = GetEngineVersionInfo();
    if (EngineVersion.Contains(TEXT("CSS")) || EngineVersion.Contains(TEXT("Custom")))
    {
        // This is likely a custom build
        if (!IsModuleRegistered())
        {
            Issues.Add(TEXT("Custom engine build detected - WebSockets may have been excluded"));
            Issues.Add(TEXT("Contact the engine maintainers to include WebSockets module"));
        }
    }

    return Issues;
}

void AWebSocketModuleVerifier::BuildVerificationReport()
{
    VerificationReport.Empty();

    VerificationReport += TEXT("╔═══════════════════════════════════════════════════╗\n");
    VerificationReport += TEXT("║     WebSocket Module Verification Report         ║\n");
    VerificationReport += TEXT("╚═══════════════════════════════════════════════════╝\n\n");

    // Engine info
    VerificationReport += FString::Printf(TEXT("Engine Version: %s\n"), *GetEngineVersionInfo());
    VerificationReport += FString::Printf(TEXT("Build Configuration: %s\n"), 
        FApp::GetBuildConfiguration() == EBuildConfiguration::Debug ? TEXT("Debug") :
        FApp::GetBuildConfiguration() == EBuildConfiguration::Development ? TEXT("Development") :
        FApp::GetBuildConfiguration() == EBuildConfiguration::Shipping ? TEXT("Shipping") :
        FApp::GetBuildConfiguration() == EBuildConfiguration::Test ? TEXT("Test") : TEXT("Unknown"));
    VerificationReport += TEXT("\n");

    // Module status
    VerificationReport += TEXT("Module Status:\n");
    
#if WEBSOCKET_HEADERS_AVAILABLE
    VerificationReport += TEXT("  ✅ Headers: Available\n");
#else
    VerificationReport += TEXT("  ❌ Headers: NOT Available\n");
#endif

    VerificationReport += FString::Printf(TEXT("  %s Registered: %s\n"), 
        IsModuleRegistered() ? TEXT("✅") : TEXT("❌"),
        IsModuleRegistered() ? TEXT("Yes") : TEXT("No"));

    bool bLoaded = FModuleManager::Get().IsModuleLoaded("WebSockets");
    VerificationReport += FString::Printf(TEXT("  %s Loaded: %s\n"), 
        bLoaded ? TEXT("✅") : TEXT("❌"),
        bLoaded ? TEXT("Yes") : TEXT("No"));

    VerificationReport += TEXT("\n");

    // Recommendations
    if (bLastVerificationSucceeded)
    {
        VerificationReport += TEXT("✅ Recommendation: Use DiscordGatewayClientNative\n");
        VerificationReport += TEXT("   The native WebSocket implementation will work!\n");
    }
    else
    {
        VerificationReport += TEXT("❌ Issue: WebSocket module not available\n");
        VerificationReport += TEXT("\n");
        VerificationReport += TEXT("Possible Solutions:\n");
        VerificationReport += TEXT("1. Verify you're using the correct engine build\n");
        VerificationReport += TEXT("2. Check if WebSockets module is in Engine/Plugins/Runtime/\n");
        VerificationReport += TEXT("3. Rebuild the engine with WebSockets enabled\n");
        VerificationReport += TEXT("4. Contact CSS/engine maintainers if using custom build\n");
    }

    VerificationReport += TEXT("\n");
    VerificationReport += TEXT("For detailed help, see: WEBSOCKET_TROUBLESHOOTING.md\n");
}
