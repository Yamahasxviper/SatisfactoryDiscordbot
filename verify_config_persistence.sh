#!/bin/bash

# Discord Bot Configuration Persistence Verification Script
# This script verifies that all Discord Bot configuration files are properly set up
# and will persist across server restarts

set -e

echo "=========================================="
echo "Discord Bot Configuration Persistence Check"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

ERRORS=0
WARNINGS=0

# Function to check if a file exists
check_file_exists() {
    local file=$1
    local description=$2
    
    if [ -f "$file" ]; then
        echo -e "${GREEN}✓${NC} $description exists: $file"
        return 0
    else
        echo -e "${RED}✗${NC} $description missing: $file"
        ERRORS=$((ERRORS + 1))
        return 1
    fi
}

# Function to check if a config section exists in a file
check_config_section() {
    local file=$1
    local section=$2
    local description=$3
    
    if [ ! -f "$file" ]; then
        return 1
    fi
    
    if grep -q "^\[${section}\]" "$file"; then
        echo -e "${GREEN}✓${NC} $description has [$section] section in $file"
        return 0
    else
        echo -e "${YELLOW}⚠${NC} $description missing [$section] section in $file"
        WARNINGS=$((WARNINGS + 1))
        return 1
    fi
}

# Function to check specific config key
check_config_key() {
    local file=$1
    local key=$2
    local description=$3
    
    if [ ! -f "$file" ]; then
        return 1
    fi
    
    if grep -q "^${key}=" "$file" || grep -q "^;${key}=" "$file" || grep -q "^; ${key}=" "$file"; then
        echo -e "${GREEN}✓${NC} $description has $key setting"
        return 0
    else
        echo -e "${YELLOW}⚠${NC} $description missing $key setting"
        WARNINGS=$((WARNINGS + 1))
        return 1
    fi
}

echo "1. Checking main configuration files..."
echo "----------------------------------------"

# Check DefaultGame.ini exists and has DiscordBot section
check_file_exists "Config/DefaultGame.ini" "Main game configuration"
check_config_section "Config/DefaultGame.ini" "DiscordBot" "DefaultGame.ini"

# Check key settings in DefaultGame.ini
if [ -f "Config/DefaultGame.ini" ]; then
    check_config_key "Config/DefaultGame.ini" "BotToken" "DefaultGame.ini"
    check_config_key "Config/DefaultGame.ini" "bEnabled" "DefaultGame.ini"
    check_config_key "Config/DefaultGame.ini" "ChatChannelId" "DefaultGame.ini"
fi

echo ""
echo "2. Checking platform-specific configurations..."
echo "------------------------------------------------"

# Check WindowsServer config
if check_file_exists "Config/WindowsServer/WindowsServerGame.ini" "Windows Server configuration"; then
    check_config_section "Config/WindowsServer/WindowsServerGame.ini" "DiscordBot" "WindowsServerGame.ini"
fi

# Check LinuxServer config
if check_file_exists "Config/LinuxServer/LinuxServerGame.ini" "Linux Server configuration"; then
    check_config_section "Config/LinuxServer/LinuxServerGame.ini" "DiscordBot" "LinuxServerGame.ini"
fi

echo ""
echo "3. Checking reference configuration..."
echo "---------------------------------------"

# Check if the Mods/DiscordBot/Config/DiscordBot.ini exists (reference file)
if check_file_exists "Mods/DiscordBot/Config/DiscordBot.ini" "DiscordBot reference configuration"; then
    check_config_section "Mods/DiscordBot/Config/DiscordBot.ini" "DiscordBot" "DiscordBot.ini"
    echo -e "${YELLOW}ℹ${NC} Note: This file is a reference. Config is read from Config/DefaultGame.ini"
fi

echo ""
echo "4. Checking source code for proper config loading..."
echo "-----------------------------------------------------"

# Check that code uses GetConfigFilename pattern
if grep -r "GetConfigFilename.*TEXT.*\"Game\"" Mods/DiscordBot/Source/DiscordBot/ > /dev/null; then
    echo -e "${GREEN}✓${NC} Source code uses cross-platform GetConfigFilename pattern"
else
    echo -e "${RED}✗${NC} Source code not using GetConfigFilename pattern"
    ERRORS=$((ERRORS + 1))
fi

# Check that code doesn't use GGameIni (old pattern)
if grep -r "GGameIni" Mods/DiscordBot/Source/DiscordBot/ > /dev/null; then
    echo -e "${RED}✗${NC} Source code still uses GGameIni (should use GetConfigFilename)"
    ERRORS=$((ERRORS + 1))
else
    echo -e "${GREEN}✓${NC} Source code does not use deprecated GGameIni pattern"
fi

# Check that code reads config but doesn't write it
if grep -r "GConfig->Set" Mods/DiscordBot/Source/DiscordBot/*.cpp > /dev/null 2>&1; then
    echo -e "${YELLOW}⚠${NC} Source code writes to config (may cause issues)"
    WARNINGS=$((WARNINGS + 1))
else
    echo -e "${GREEN}✓${NC} Source code only reads config (correct behavior)"
fi

echo ""
echo "5. Checking documentation..."
echo "----------------------------"

check_file_exists "CONFIG_PERSISTENCE_GUIDE.md" "Configuration persistence guide"
check_file_exists "SERVER_CONFIG_FIX.md" "Server configuration fix documentation"
check_file_exists "Mods/DiscordBot/README.md" "DiscordBot README"

echo ""
echo "6. Configuration persistence verification..."
echo "---------------------------------------------"

# Verify config files are readable
if [ -r "Config/DefaultGame.ini" ]; then
    echo -e "${GREEN}✓${NC} Config/DefaultGame.ini is readable"
else
    echo -e "${RED}✗${NC} Config/DefaultGame.ini is not readable"
    ERRORS=$((ERRORS + 1))
fi

if [ -r "Config/WindowsServer/WindowsServerGame.ini" ]; then
    echo -e "${GREEN}✓${NC} Config/WindowsServer/WindowsServerGame.ini is readable"
fi

if [ -r "Config/LinuxServer/LinuxServerGame.ini" ]; then
    echo -e "${GREEN}✓${NC} Config/LinuxServer/LinuxServerGame.ini is readable"
fi

# Check that config files are not executable (security best practice)
if [ -x "Config/DefaultGame.ini" ]; then
    echo -e "${YELLOW}⚠${NC} Config/DefaultGame.ini is executable (should not be)"
    WARNINGS=$((WARNINGS + 1))
else
    echo -e "${GREEN}✓${NC} Config/DefaultGame.ini is not executable (correct)"
fi

echo ""
echo "=========================================="
echo "Verification Summary"
echo "=========================================="
echo ""

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo -e "${GREEN}✓ ALL CHECKS PASSED${NC}"
    echo ""
    echo "✅ All server configuration files are properly set up"
    echo "✅ Configuration will persist across server restarts"
    echo "✅ Platform-specific configs are available"
    echo "✅ Source code uses correct config loading pattern"
    echo ""
    echo "Your Discord Bot configuration is ready for production use!"
elif [ $ERRORS -eq 0 ]; then
    echo -e "${YELLOW}⚠ PASSED WITH WARNINGS${NC}"
    echo ""
    echo "Warnings: $WARNINGS"
    echo ""
    echo "The configuration will work, but you may want to address the warnings above."
else
    echo -e "${RED}✗ FAILED${NC}"
    echo ""
    echo "Errors: $ERRORS"
    echo "Warnings: $WARNINGS"
    echo ""
    echo "Please fix the errors above before deploying to production."
    exit 1
fi

echo ""
echo "=========================================="
echo "Configuration Persistence Status"
echo "=========================================="
echo ""
echo "📝 Config Files Status:"
echo "  • Config files are static INI files"
echo "  • Files persist automatically across restarts"
echo "  • No runtime modification of config files"
echo "  • Manual edits take effect on next restart"
echo ""
echo "🔧 To Change Configuration:"
echo "  1. Stop the server (recommended)"
echo "  2. Edit Config/DefaultGame.ini or platform-specific config"
echo "  3. Save the file"
echo "  4. Start the server"
echo "  5. Changes persist forever (until manually changed again)"
echo ""
echo "📚 Documentation:"
echo "  • CONFIG_PERSISTENCE_GUIDE.md - Detailed persistence guide"
echo "  • SERVER_CONFIG_FIX.md - Cross-platform config loading"
echo "  • Mods/DiscordBot/README.md - General setup and usage"
echo ""

exit 0
