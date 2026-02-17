# WebSocket Module Troubleshooting Guide

## Issue: "I am not seeing the WebSocket in the custom Unreal Engine build (5.3.2-CSS)"

This guide helps you diagnose and resolve WebSocket module availability issues in Satisfactory's custom CSS Unreal Engine build.

## Quick Verification

### Step 1: Use the Verification Tool

We've created a tool to automatically check if WebSocket is available:

1. **In Editor (Recommended)**:
   - Open your level in Unreal Editor
   - Add `WebSocketModuleVerifier` actor to your level
   - Run the game/PIE (Play In Editor)
   - Check the Output Log for verification results

2. **In Blueprint**:
   ```
   Spawn Actor from Class -> WebSocketModuleVerifier
   Call "Run Full Verification"
   ```

3. **In C++**:
   ```cpp
   AWebSocketModuleVerifier* Verifier = GetWorld()->SpawnActor<AWebSocketModuleVerifier>();
   Verifier->RunFullVerification();
   ```

### Step 2: Check the Output

Look for these messages in your log:

✅ **Success** - You'll see:
```
✅ RESULT: WebSocket module is AVAILABLE and WORKING
```

❌ **Failure** - You'll see:
```
❌ RESULT: WebSocket module is NOT available
```

## Common Scenarios

### Scenario 1: Module Not Found in Editor

**Symptoms:**
- "WebSocket headers NOT found at compile time"
- Compilation errors when building

**Causes:**
- WebSockets module not included in engine build
- Module files missing from Engine/Plugins/Runtime/

**Solutions:**

1. **Check if WebSockets Plugin Exists**:
   ```
   Navigate to: YourEngine/Engine/Plugins/Runtime/WebSockets/
   ```
   
   If missing, the module was excluded from your engine build.

2. **Verify in .uproject**:
   Check `FactoryGame.uproject` doesn't explicitly disable WebSockets:
   ```json
   {
     "Plugins": [
       {
         "Name": "WebSockets",
         "Enabled": true  // Should be true or not listed
       }
     ]
   }
   ```

3. **Check Module Whitelist**:
   If using a custom build, check if WebSockets is in the build configuration.

### Scenario 2: Module Headers Missing at Compile Time

**Symptoms:**
- Build fails with "IWebSocket.h not found"
- "WebSocket headers NOT found at compile time"

**Cause:**
- WebSockets module source files not present
- Include paths not set correctly

**Solutions:**

1. **Verify Module Structure**:
   ```
   Engine/Plugins/Runtime/WebSockets/
   ├── WebSockets.uplugin
   └── Source/
       └── WebSockets/
           ├── Public/
           │   ├── IWebSocket.h
           │   └── WebSocketsModule.h
           └── Private/
   ```

2. **Rebuild the Engine**:
   If files are missing, you may need to rebuild your engine with WebSockets enabled:
   ```bash
   # In Engine source
   ./Setup.sh  # or Setup.bat on Windows
   ./GenerateProjectFiles.sh  # or .bat
   
   # Build with WebSockets explicitly
   # Edit Engine/Build/BuildConfiguration.xml to include WebSockets
   ```

3. **Check CSS Custom Build**:
   If using Satisfactory's CSS engine, contact Coffee Stain Studios or check their build documentation.

### Scenario 3: Module Registered but Won't Load

**Symptoms:**
- "WebSockets module is registered" ✅
- "WebSockets module FAILED to load" ❌

**Cause:**
- Module dependencies missing
- DLL/library files not found
- Platform-specific issues

**Solutions:**

1. **Check Module Dependencies**:
   WebSockets module requires:
   - Core
   - Sockets (basic socket module)
   - OpenSSL (for wss:// secure connections)

2. **Verify Binary Files**:
   ```
   Engine/Binaries/[Platform]/
   └── Look for WebSockets-related DLLs/SOs
   ```

3. **Check Platform Support**:
   WebSockets should support Win64, Linux, and Mac. Verify your platform is supported.

### Scenario 4: CSS Custom Build Without WebSockets

**Symptoms:**
- Using official Satisfactory CSS engine
- WebSocket module not found

**Cause:**
- CSS may have excluded WebSockets from their custom build
- Using an older or minimal engine configuration

**Solutions:**

1. **Verify Your Engine Version**:
   Check what CSS engine you're actually using:
   ```cpp
   // In code or output log
   FEngineVersion::Current()
   // Should show 5.3.2 or similar
   ```

2. **Check Satisfactory Modding Documentation**:
   - Visit https://docs.ficsit.app/
   - Check if there are known limitations
   - See if WebSockets is explicitly unsupported

3. **Alternative: Use HTTP Long-Polling** (Not recommended but possible):
   If WebSockets is truly unavailable, you could:
   - Use HTTP polling instead of WebSocket
   - Significantly less efficient for Discord
   - Not recommended for production

4. **Contact CSS/Community**:
   - Ask in Satisfactory Modding Discord
   - File an issue with CSS
   - Request WebSockets module inclusion

## Advanced Diagnostics

### Manual Module Check (C++)

Add this to any C++ code to check manually:

```cpp
#include "Modules/ModuleManager.h"

void CheckWebSocketModule()
{
    // Check if module exists
    TArray<FModuleStatus> Modules;
    FModuleManager::Get().QueryModules(Modules);
    
    bool bFound = false;
    for (const FModuleStatus& Module : Modules)
    {
        if (Module.Name == TEXT("WebSockets"))
        {
            UE_LOG(LogTemp, Log, TEXT("✅ WebSockets module found: %s"), 
                Module.bIsLoaded ? TEXT("Loaded") : TEXT("Not Loaded"));
            bFound = true;
            break;
        }
    }
    
    if (!bFound)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ WebSockets module NOT registered"));
    }
    
    // Try to load
    if (bFound)
    {
        FModuleManager::Get().LoadModule(TEXT("WebSockets"));
        bool bLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("WebSockets"));
        UE_LOG(LogTemp, Log, TEXT("Load attempt: %s"), 
            bLoaded ? TEXT("Success") : TEXT("Failed"));
    }
}
```

### Check at Build Time

Add this to your Build.cs to check during compilation:

```csharp
public DiscordBot(ReadOnlyTargetRules Target) : base(Target)
{
    // Try to add WebSockets
    PublicDependencyModuleNames.Add("WebSockets");
    
    // This will fail at build time if module doesn't exist
    // Good for early detection
}
```

### Engine Source Check

If you have engine source access:

1. Check `Engine/Source/Runtime/` for WebSockets
2. Check `Engine/Plugins/Runtime/` for WebSockets
3. Look at `Engine/Build/InstalledEngineBuild.xml` for exclusions

## Solutions Summary

### If WebSocket IS Available ✅

**Use:** `DiscordGatewayClientNative`

This is the correct implementation for Discord Gateway and will work perfectly.

### If WebSocket Is NOT Available ❌

**Options (in order of preference):**

1. **Rebuild Engine** (Best):
   - Include WebSockets module in build
   - Follow Unreal Engine build documentation
   - Or use standard UE 5.3.2 build

2. **Request Module from CSS** (Good):
   - Contact Coffee Stain Studios
   - File request in Satisfactory Modding Discord
   - Wait for updated engine build

3. **Use HTTP Long-Polling** (Poor):
   - Less efficient
   - Higher latency
   - More complex to implement
   - Not recommended for Discord

4. **Use External Process** (Workaround):
   - Run Discord bot as separate process
   - Communicate via IPC/files
   - More complex architecture
   - Not covered in this mod

## Verification Checklist

Run through this checklist:

- [ ] Ran `WebSocketModuleVerifier` tool
- [ ] Checked Output Log for results
- [ ] Verified engine version is 5.3.2-CSS
- [ ] Checked `Engine/Plugins/Runtime/WebSockets/` exists
- [ ] Verified `FactoryGame.uproject` doesn't disable WebSockets
- [ ] Tried rebuilding the project
- [ ] Checked Satisfactory modding documentation
- [ ] Asked in Satisfactory Modding Discord if needed

## Expected Results

### With Standard UE 5.3.2

WebSocket module **SHOULD** be available. If not, your engine installation may be corrupted.

### With CSS Custom Build

Results may vary:
- **If Available**: Use `DiscordGatewayClientNative` ✅
- **If Not Available**: Module may have been excluded for size/complexity reasons

## Getting Help

### Where to Ask:

1. **Satisfactory Modding Discord**: https://discord.ficsit.app
2. **Satisfactory Modding Documentation**: https://docs.ficsit.app
3. **This Repository's Issues**: https://github.com/Yamahasxviper/SatisfactoryDiscordbot/issues

### What to Include:

When asking for help, provide:
1. Output from `WebSocketModuleVerifier`
2. Your engine version (exact string from log)
3. How you obtained your engine (CSS official, custom build, etc.)
4. Build configuration (Development, Shipping, etc.)
5. Platform (Windows, Linux, Mac)

## Alternative Approach

If WebSockets truly isn't available and can't be added, you could:

1. **Use the SocketIOClient Plugin**:
   - Already in `Plugins/WebSockets/SocketIOClient/`
   - But remember: Won't work with Discord Gateway (protocol mismatch)
   - Only useful for Socket.IO servers, not Discord

2. **External Bot Process**:
   - Run a separate Discord bot (Python, Node.js, etc.)
   - Communicate with Satisfactory via files, HTTP API, or IPC
   - More complex but doesn't need WebSocket in engine

## Conclusion

The WebSocket module **should** be available in UE 5.3.2, including CSS builds. If it's not:

1. **First**: Use the verification tool to confirm
2. **Then**: Follow troubleshooting steps above
3. **Finally**: Reach out to CSS/community if truly unavailable

Most likely, it **IS** available and just needs to be properly detected/loaded. The verification tool will help you determine this!

## Technical Background

### Why WebSocket Module Might Seem "Missing":

1. **Not Auto-Loaded**: WebSocket module isn't loaded by default
   - It's loaded on-demand when first used
   - Our code explicitly loads it: `FModuleManager::Get().LoadModule("WebSockets")`

2. **Plugin vs Module**: WebSockets is a Runtime Plugin
   - Located in `Engine/Plugins/Runtime/`
   - Not in `Engine/Source/Runtime/`
   - This can make it "harder to see"

3. **Build Configuration**: May be excluded in Shipping builds
   - Development builds: Usually included
   - Shipping builds: Might be excluded if not used elsewhere
   - Our mod explicitly requires it in Build.cs

### How Our Code Handles It:

```cpp
// From DiscordGatewayClientNative.cpp
void ADiscordGatewayClientNative::ConnectWebSocket()
{
    // Explicitly load if not loaded
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::Get().LoadModule("WebSockets");
    }
    
    // Then use it
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(GatewayURL);
    
    if (!WebSocket.IsValid())
    {
        UE_LOG(LogDiscordGatewayNative, Error, 
            TEXT("Failed to create WebSocket"));
        return;
    }
    // ... rest of code
}
```

This approach ensures:
- Module is loaded when needed
- Clear error if module unavailable
- No unnecessary loading at startup

---

**Remember**: Run the `WebSocketModuleVerifier` first! It will tell you exactly what's happening with your WebSocket module.
