#!/bin/bash
# Server Configuration Loading Verification Tool
# Verifies that server config settings are properly loaded using GetConfigFilename

echo "==========================================="
echo "Discord Bot Server Config Loading Check"
echo "==========================================="
echo ""

ALL_CHECKS_PASSED=true
DISCORD_BOT_PATH="Mods/DiscordBot/Source/DiscordBot"

# Check 1: No GGameIni references remain
echo "✓ CHECK 1: No GGameIni direct references"
SUBSYSTEM_FILE="$DISCORD_BOT_PATH/Private/DiscordBotSubsystem.cpp"
MODULE_FILE="$DISCORD_BOT_PATH/Private/DiscordBotModule.cpp"

echo "  Checking DiscordBotSubsystem.cpp..."
if grep -q "GGameIni" "$SUBSYSTEM_FILE" 2>/dev/null; then
    echo "  ❌ GGameIni still referenced in DiscordBotSubsystem.cpp!"
    grep -n "GGameIni" "$SUBSYSTEM_FILE" | head -5
    ALL_CHECKS_PASSED=false
else
    echo "  ✓ No GGameIni references in DiscordBotSubsystem.cpp"
fi

echo "  Checking DiscordBotModule.cpp..."
if grep -q "GGameIni" "$MODULE_FILE" 2>/dev/null; then
    echo "  ❌ GGameIni still referenced in DiscordBotModule.cpp!"
    grep -n "GGameIni" "$MODULE_FILE" | head -5
    ALL_CHECKS_PASSED=false
else
    echo "  ✓ No GGameIni references in DiscordBotModule.cpp"
fi
echo ""

# Check 2: GetConfigFilename usage
echo "✓ CHECK 2: GetConfigFilename() is used for config loading"

echo "  Checking DiscordBotSubsystem.cpp..."
if grep -q 'GetConfigFilename.*"Game"' "$SUBSYSTEM_FILE"; then
    echo "  ✓ GetConfigFilename(\"Game\") found in DiscordBotSubsystem.cpp"
    COUNT=$(grep -c 'GetConfigFilename.*"Game"' "$SUBSYSTEM_FILE")
    echo "    Found $COUNT usage(s)"
else
    echo "  ❌ GetConfigFilename not found in DiscordBotSubsystem.cpp!"
    ALL_CHECKS_PASSED=false
fi

echo "  Checking DiscordBotModule.cpp..."
if grep -q 'GetConfigFilename.*"Game"' "$MODULE_FILE"; then
    echo "  ✓ GetConfigFilename(\"Game\") found in DiscordBotModule.cpp"
    COUNT=$(grep -c 'GetConfigFilename.*"Game"' "$MODULE_FILE")
    echo "    Found $COUNT usage(s)"
else
    echo "  ❌ GetConfigFilename not found in DiscordBotModule.cpp!"
    ALL_CHECKS_PASSED=false
fi
echo ""

# Check 3: All config loading uses ConfigFilename variable
echo "✓ CHECK 3: Config calls use ConfigFilename variable"

echo "  Checking for proper pattern in DiscordBotSubsystem.cpp..."
if grep -q 'FString ConfigFilename = GConfig->GetConfigFilename' "$SUBSYSTEM_FILE"; then
    echo "  ✓ ConfigFilename variable properly declared"
else
    echo "  ❌ ConfigFilename variable not declared!"
    ALL_CHECKS_PASSED=false
fi

echo "  Checking for GConfig calls with ConfigFilename..."
# Count GConfig calls that use ConfigFilename
CONFIG_CALLS=$(grep -c 'GConfig->.*ConfigFilename' "$SUBSYSTEM_FILE" 2>/dev/null || echo 0)
if [ "$CONFIG_CALLS" -gt 0 ]; then
    echo "  ✓ Found $CONFIG_CALLS GConfig calls using ConfigFilename"
else
    echo "  ❌ No GConfig calls using ConfigFilename found!"
    ALL_CHECKS_PASSED=false
fi
echo ""

# Check 4: Comments explain the change
echo "✓ CHECK 4: Code includes explanatory comments"

if grep -q "cross-platform compatibility.*dedicated servers" "$SUBSYSTEM_FILE"; then
    echo "  ✓ Explanatory comments found in DiscordBotSubsystem.cpp"
else
    echo "  ⚠ No explanatory comments found in DiscordBotSubsystem.cpp"
fi

if grep -q "cross-platform compatibility.*dedicated servers" "$MODULE_FILE"; then
    echo "  ✓ Explanatory comments found in DiscordBotModule.cpp"
else
    echo "  ⚠ No explanatory comments found in DiscordBotModule.cpp"
fi
echo ""

# Check 5: All config functions updated
echo "✓ CHECK 5: All config loading functions updated"

echo "  Checking LoadBotTokenFromConfig()..."
if grep -A 10 "LoadBotTokenFromConfig" "$SUBSYSTEM_FILE" | grep -q "GetConfigFilename"; then
    echo "  ✓ LoadBotTokenFromConfig() uses GetConfigFilename"
else
    echo "  ❌ LoadBotTokenFromConfig() not updated!"
    ALL_CHECKS_PASSED=false
fi

echo "  Checking LoadTwoWayChatConfig()..."
if grep -A 15 "LoadTwoWayChatConfig" "$SUBSYSTEM_FILE" | grep -q "GetConfigFilename"; then
    echo "  ✓ LoadTwoWayChatConfig() uses GetConfigFilename"
else
    echo "  ❌ LoadTwoWayChatConfig() not updated!"
    ALL_CHECKS_PASSED=false
fi

echo "  Checking LoadServerNotificationConfig()..."
if grep -A 15 "LoadServerNotificationConfig" "$SUBSYSTEM_FILE" | grep -q "GetConfigFilename"; then
    echo "  ✓ LoadServerNotificationConfig() uses GetConfigFilename"
else
    echo "  ❌ LoadServerNotificationConfig() not updated!"
    ALL_CHECKS_PASSED=false
fi

echo "  Checking Initialize() (startup config loading)..."
if grep -A 20 "void UDiscordBotSubsystem::Initialize" "$SUBSYSTEM_FILE" | grep -q "GetConfigFilename"; then
    echo "  ✓ Initialize() uses GetConfigFilename"
else
    echo "  ❌ Initialize() not updated!"
    ALL_CHECKS_PASSED=false
fi
echo ""

# Summary
echo "==========================================="
if [ "$ALL_CHECKS_PASSED" = true ]; then
    echo "✅ ALL CHECKS PASSED"
    echo "Server config loading is properly implemented for all platforms!"
    exit 0
else
    echo "❌ SOME CHECKS FAILED"
    echo "Please review the failed checks above."
    exit 1
fi
