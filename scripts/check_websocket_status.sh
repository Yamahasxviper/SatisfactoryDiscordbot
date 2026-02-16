#!/bin/bash

################################################################################
# WebSocket Status Check Script
# 
# Quick script to verify WebSocket plugin status and configuration
# Run this if you're unsure whether WebSockets are available in your setup
################################################################################

set -e

echo "========================================"
echo "WebSocket Status Check"
echo "========================================"
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Track overall status
ALL_CHECKS_PASSED=true

# Helper function for checks
check_item() {
    local check_name="$1"
    local check_result="$2"
    
    if [ "$check_result" = "PASS" ]; then
        echo -e "${GREEN}✓${NC} $check_name"
    elif [ "$check_result" = "WARN" ]; then
        echo -e "${YELLOW}⚠${NC} $check_name"
    else
        echo -e "${RED}✗${NC} $check_name"
        ALL_CHECKS_PASSED=false
    fi
}

echo "1. Checking WebSocket Plugin Files"
echo "-----------------------------------"

# Check if WebSocket plugin directory exists
if [ -d "Plugins/WebSockets" ]; then
    check_item "WebSocket plugin directory exists" "PASS"
else
    check_item "WebSocket plugin directory exists" "FAIL"
fi

# Check for WebSockets.uplugin file
if [ -f "Plugins/WebSockets/WebSockets.uplugin" ]; then
    check_item "WebSockets.uplugin file present" "PASS"
else
    check_item "WebSockets.uplugin file present" "FAIL"
fi

# Check for Build.cs file
if [ -f "Plugins/WebSockets/Source/WebSockets/WebSockets.Build.cs" ]; then
    check_item "WebSockets.Build.cs file present" "PASS"
else
    check_item "WebSockets.Build.cs file present" "FAIL"
fi

# Check for source files
if [ -d "Plugins/WebSockets/Source/WebSockets/Private" ]; then
    source_files=$(find Plugins/WebSockets/Source/WebSockets/Private -name "*.cpp" 2>/dev/null | wc -l)
    if [ "$source_files" -gt 0 ]; then
        check_item "WebSocket source files present ($source_files files)" "PASS"
    else
        check_item "WebSocket source files present" "FAIL"
    fi
else
    check_item "WebSocket source directory" "FAIL"
fi

echo ""
echo "2. Checking Third-Party Libraries"
echo "-----------------------------------"

# Check for libWebSockets directory
if [ -d "ThirdParty/libWebSockets/libwebsockets" ]; then
    check_item "libWebSockets directory present" "PASS"
else
    check_item "libWebSockets directory present" "FAIL"
fi

# Check for include files
if [ -d "ThirdParty/libWebSockets/libwebsockets/include" ]; then
    check_item "libWebSockets include files present" "PASS"
else
    check_item "libWebSockets include files present" "FAIL"
fi

# Check for library files
if [ -d "ThirdParty/libWebSockets/libwebsockets/lib" ]; then
    check_item "libWebSockets library files present" "PASS"
else
    check_item "libWebSockets library files present" "FAIL"
fi

echo ""
echo "3. Checking Project Configuration"
echo "-----------------------------------"

# Check FactoryGame.uproject
if [ -f "FactoryGame.uproject" ]; then
    if grep -q '"Name": "WebSockets"' FactoryGame.uproject; then
        if grep -A 1 '"Name": "WebSockets"' FactoryGame.uproject | grep -q '"Enabled": true'; then
            check_item "WebSockets enabled in FactoryGame.uproject" "PASS"
        else
            check_item "WebSockets enabled in FactoryGame.uproject" "FAIL"
        fi
    else
        check_item "WebSockets declared in FactoryGame.uproject" "FAIL"
    fi
else
    check_item "FactoryGame.uproject exists" "FAIL"
fi

# Check DiscordChatBridge.uplugin
if [ -f "Mods/DiscordChatBridge/DiscordChatBridge.uplugin" ]; then
    if grep -q '"Name": "WebSockets"' Mods/DiscordChatBridge/DiscordChatBridge.uplugin; then
        check_item "WebSockets declared in DiscordChatBridge" "PASS"
    else
        check_item "WebSockets declared in DiscordChatBridge" "WARN"
        echo "   (Optional: DiscordChatBridge can work without Gateway)"
    fi
else
    check_item "DiscordChatBridge.uplugin exists" "WARN"
    echo "   (Not critical if you're not using Discord Chat Bridge)"
fi

# Check DiscordChatBridge Build.cs
if [ -f "Mods/DiscordChatBridge/Source/DiscordChatBridge/DiscordChatBridge.Build.cs" ]; then
    if grep -q "WebSockets" Mods/DiscordChatBridge/Source/DiscordChatBridge/DiscordChatBridge.Build.cs; then
        check_item "DiscordChatBridge.Build.cs checks for WebSockets" "PASS"
    else
        check_item "DiscordChatBridge.Build.cs checks for WebSockets" "WARN"
    fi
else
    check_item "DiscordChatBridge.Build.cs exists" "WARN"
fi

echo ""
echo "4. Platform Support Check"
echo "-----------------------------------"

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
    check_item "Platform detected: Linux" "PASS"
    
    # Check for Linux libraries
    if [ -d "ThirdParty/libWebSockets/libwebsockets/lib/Unix" ]; then
        check_item "Linux WebSocket libraries available" "PASS"
    else
        check_item "Linux WebSocket libraries available" "FAIL"
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
    check_item "Platform detected: macOS" "PASS"
    
    # Check for Mac libraries
    if [ -d "ThirdParty/libWebSockets/libwebsockets/lib/Mac" ]; then
        check_item "macOS WebSocket libraries available" "PASS"
    else
        check_item "macOS WebSocket libraries available" "WARN"
    fi
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
    PLATFORM="Windows"
    check_item "Platform detected: Windows" "PASS"
    
    # Check for Windows libraries
    if [ -d "ThirdParty/libWebSockets/libwebsockets/lib/Win64" ]; then
        check_item "Windows WebSocket libraries available" "PASS"
    else
        check_item "Windows WebSocket libraries available" "WARN"
    fi
else
    PLATFORM="Unknown"
    check_item "Platform detected: $OSTYPE" "WARN"
fi

echo ""
echo "========================================"
echo "Summary"
echo "========================================"
echo ""

if [ "$ALL_CHECKS_PASSED" = true ]; then
    echo -e "${GREEN}✅ ALL CHECKS PASSED${NC}"
    echo ""
    echo "WebSockets are properly configured and ready to use!"
    echo ""
    echo "What this means:"
    echo "  • WebSocket plugin will build automatically"
    echo "  • Discord Gateway features will be available"
    echo "  • Full presence/activity functionality enabled"
    echo ""
    echo "Next steps:"
    echo "  1. Build the project normally"
    echo "  2. Configure your Discord bot (see QUICKSTART.md)"
    echo "  3. Run your server"
    echo ""
    echo "For more information:"
    echo "  • How to use: WEBSOCKET_HOW_TO.md"
    echo "  • FAQ: WEBSOCKET_FAQ.md"
    echo "  • Build requirements: BUILD_REQUIREMENTS.md"
else
    echo -e "${RED}⚠️  SOME CHECKS FAILED${NC}"
    echo ""
    echo "There may be issues with your WebSocket setup."
    echo ""
    echo "Common solutions:"
    echo "  1. Verify you have a complete clone of the repository"
    echo "  2. Check that all files were downloaded properly"
    echo "  3. Try re-cloning the repository"
    echo ""
    echo "If you just cloned the repository:"
    echo "  • This is expected if files are still downloading"
    echo "  • Wait for the clone to complete, then run this script again"
    echo ""
    echo "For help:"
    echo "  • Read: TROUBLESHOOTING.md"
    echo "  • Read: WEBSOCKET_FAQ.md"
    echo "  • Run: ./scripts/verify_websocket_build_compatibility.sh"
    echo "  • Ask: https://discord.gg/QzcG9nX"
fi

echo ""
echo "========================================"
echo "Additional Information"
echo "========================================"
echo ""
echo "Platform: $PLATFORM"
echo "Repository root: $(pwd)"
echo ""

# Check for documentation files
echo "Available documentation:"
if [ -f "WEBSOCKET_HOW_TO.md" ]; then
    echo "  ✓ WEBSOCKET_HOW_TO.md - Complete guide"
else
    echo "  ✗ WEBSOCKET_HOW_TO.md - Missing"
fi

if [ -f "WEBSOCKET_FAQ.md" ]; then
    echo "  ✓ WEBSOCKET_FAQ.md - Common questions"
else
    echo "  ✗ WEBSOCKET_FAQ.md - Missing"
fi

if [ -f "BUILD_REQUIREMENTS.md" ]; then
    echo "  ✓ BUILD_REQUIREMENTS.md - Build information"
else
    echo "  ✗ BUILD_REQUIREMENTS.md - Missing"
fi

if [ -f "TROUBLESHOOTING.md" ]; then
    echo "  ✓ TROUBLESHOOTING.md - Problem solutions"
else
    echo "  ✗ TROUBLESHOOTING.md - Missing"
fi

echo ""

# Check for verification scripts
echo "Available verification scripts:"
if [ -f "scripts/verify_websocket_build_compatibility.sh" ]; then
    echo "  ✓ verify_websocket_build_compatibility.sh - Detailed build check"
else
    echo "  ✗ verify_websocket_build_compatibility.sh - Missing"
fi

if [ -f "scripts/validate_websocket_integration.sh" ]; then
    echo "  ✓ validate_websocket_integration.sh - Runtime validation"
else
    echo "  ✗ validate_websocket_integration.sh - Missing"
fi

echo ""
echo "========================================"
echo ""

if [ "$ALL_CHECKS_PASSED" = true ]; then
    exit 0
else
    exit 1
fi
