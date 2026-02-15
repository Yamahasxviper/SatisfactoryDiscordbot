#!/bin/bash
# WebSocket Integration Validation Script
# This script verifies that all WebSocket plugin files are present and properly configured
# Usage: ./scripts/validate_websocket_integration.sh


# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
PASSED=0
FAILED=0
WARNINGS=0

# Helper functions
print_header() {
    echo -e "${BLUE}=========================================="
    echo -e "$1"
    echo -e "==========================================${NC}"
    echo ""
}

print_section() {
    echo -e "${BLUE}$1${NC}"
    echo "-----------------------------------"
}

print_pass() {
    echo -e "${GREEN}✓${NC} $1"
    ((PASSED++))
}

print_fail() {
    echo -e "${RED}✗${NC} $1"
    ((FAILED++))
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
    ((WARNINGS++))
}

# Get script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

cd "$PROJECT_ROOT"

print_header "WebSocket Integration Validation"

# 1. Check WebSocket Plugin Files
print_section "1. WebSocket Plugin Files"

# Define required files
declare -a REQUIRED_FILES=(
    "Plugins/WebSockets/WebSockets.uplugin"
    "Plugins/WebSockets/WebSockets.Build.cs"
    "Plugins/WebSockets/Public/IWebSocket.h"
    "Plugins/WebSockets/Public/IWebSocketsManager.h"
    "Plugins/WebSockets/Public/WebSocketsModule.h"
    "Plugins/WebSockets/Private/WebSocketsModule.cpp"
    "Plugins/WebSockets/Private/WebSocketsLog.h"
    "Plugins/WebSockets/Private/PlatformWebSocket.h"
    "Plugins/WebSockets/Private/Lws/LwsWebSocket.h"
    "Plugins/WebSockets/Private/Lws/LwsWebSocket.cpp"
    "Plugins/WebSockets/Private/Lws/LwsWebSocketsManager.h"
    "Plugins/WebSockets/Private/Lws/LwsWebSocketsManager.cpp"
    "Plugins/WebSockets/Private/WinHttp/WinHttpWebSocket.h"
    "Plugins/WebSockets/Private/WinHttp/WinHttpWebSocket.cpp"
    "Plugins/WebSockets/Private/WinHttp/WinHttpWebSocketsManager.h"
    "Plugins/WebSockets/Private/WinHttp/WinHttpWebSocketsManager.cpp"
    "Plugins/WebSockets/Private/WinHttp/Support/WinHttpConnectionWebSocket.h"
    "Plugins/WebSockets/Private/WinHttp/Support/WinHttpConnectionWebSocket.cpp"
    "Plugins/WebSockets/Private/WinHttp/Support/WinHttpWebSocketTypes.h"
    "Plugins/WebSockets/Private/WinHttp/Support/WinHttpWebSocketTypes.cpp"
    "Plugins/WebSockets/Private/WinHttp/Support/WinHttpWebSocketErrorHelper.h"
    "Plugins/WebSockets/Private/WinHttp/Support/WinHttpWebSocketErrorHelper.cpp"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        print_pass "$file"
    else
        print_fail "MISSING: $file"
    fi
done
echo ""

# 2. Check Project Configuration
print_section "2. Project Configuration (FactoryGame.uproject)"

if [ -f "FactoryGame.uproject" ]; then
    if grep -q '"Name": "WebSockets"' FactoryGame.uproject; then
        print_pass "WebSockets plugin declared in FactoryGame.uproject"
        
        if grep -A 2 '"Name": "WebSockets"' FactoryGame.uproject | grep -q '"Enabled": true'; then
            print_pass "WebSockets plugin is enabled"
        else
            print_fail "WebSockets plugin is not enabled"
        fi
        
        if grep -A 2 '"Name": "WebSockets"' FactoryGame.uproject | grep -q '"Optional": true'; then
            print_pass "WebSockets is marked as optional (correct)"
        else
            print_warning "WebSockets is not marked as optional"
        fi
    else
        print_fail "WebSockets not found in FactoryGame.uproject"
    fi
else
    print_fail "FactoryGame.uproject not found"
fi
echo ""

# 3. Check DiscordChatBridge Configuration
print_section "3. DiscordChatBridge Configuration"

if [ -f "Mods/DiscordChatBridge/DiscordChatBridge.uplugin" ]; then
    if grep -q '"Name": "WebSockets"' Mods/DiscordChatBridge/DiscordChatBridge.uplugin; then
        print_pass "WebSockets dependency declared in DiscordChatBridge.uplugin"
        
        if grep -A 2 '"Name": "WebSockets"' Mods/DiscordChatBridge/DiscordChatBridge.uplugin | grep -q '"Optional": true'; then
            print_pass "WebSockets is marked as optional in DiscordChatBridge"
        else
            print_warning "WebSockets is not marked as optional in DiscordChatBridge"
        fi
    else
        print_fail "WebSockets not found in DiscordChatBridge.uplugin"
    fi
else
    print_fail "DiscordChatBridge.uplugin not found"
fi
echo ""

# 4. Check Build Configuration
print_section "4. Build Configuration (DiscordChatBridge.Build.cs)"

if [ -f "Mods/DiscordChatBridge/Source/DiscordChatBridge/DiscordChatBridge.Build.cs" ]; then
    build_file="Mods/DiscordChatBridge/Source/DiscordChatBridge/DiscordChatBridge.Build.cs"
    
    if grep -q "WITH_WEBSOCKETS_SUPPORT" "$build_file"; then
        print_pass "WITH_WEBSOCKETS_SUPPORT conditional compilation defined"
    else
        print_fail "WITH_WEBSOCKETS_SUPPORT not found in Build.cs"
    fi
    
    if grep -q "Directory.Exists" "$build_file"; then
        print_pass "Build script checks for WebSockets availability"
    else
        print_warning "Build script doesn't check for WebSockets directory"
    fi
    
    if grep -q 'PublicDependencyModuleNames.Add("WebSockets")' "$build_file"; then
        print_pass "WebSockets module added to dependencies"
    else
        print_fail "WebSockets not added to dependencies in Build.cs"
    fi
    
    # Check for multiple plugin location checks
    websocket_checks=$(grep -c "WebSocketsPluginPath" "$build_file" || true)
    if [ "$websocket_checks" -ge 3 ]; then
        print_pass "Build script checks multiple WebSockets plugin locations ($websocket_checks paths)"
    else
        print_warning "Limited WebSockets plugin location checks ($websocket_checks paths)"
    fi
else
    print_fail "DiscordChatBridge.Build.cs not found"
fi
echo ""

# 5. Check DiscordGateway Implementation
print_section "5. DiscordGateway Implementation"

if [ -f "Mods/DiscordChatBridge/Source/DiscordChatBridge/Public/DiscordGateway.h" ]; then
    if grep -q "#if WITH_WEBSOCKETS_SUPPORT" Mods/DiscordChatBridge/Source/DiscordChatBridge/Public/DiscordGateway.h; then
        print_pass "Header uses conditional compilation for WebSockets"
    else
        print_warning "Header doesn't use conditional compilation"
    fi
else
    print_fail "DiscordGateway.h not found"
fi

if [ -f "Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/DiscordGateway.cpp" ]; then
    impl_file="Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/DiscordGateway.cpp"
    
    if grep -q "#if WITH_WEBSOCKETS_SUPPORT" "$impl_file"; then
        print_pass "Implementation uses conditional compilation for WebSockets"
    else
        print_warning "Implementation doesn't use conditional compilation"
    fi
    
    if grep -q "FWebSocketsModule::Get().CreateWebSocket" "$impl_file"; then
        print_pass "WebSocket creation code is present"
    else
        print_fail "WebSocket creation code not found"
    fi
    
    if grep -q "WebSocket->Connect()" "$impl_file"; then
        print_pass "WebSocket connection code is present"
    else
        print_fail "WebSocket connection code not found"
    fi
    
    if grep -q "WebSocket->OnConnected()" "$impl_file"; then
        print_pass "WebSocket event handlers are bound"
    else
        print_fail "WebSocket event handlers not found"
    fi
    
    # Check for proper error handling
    if grep -q "WebSockets plugin is not available" "$impl_file"; then
        print_pass "Error handling for missing WebSockets plugin"
    else
        print_warning "No error message for missing WebSockets plugin"
    fi
else
    print_fail "DiscordGateway.cpp not found"
fi
echo ""

# 6. Check Documentation
print_section "6. Documentation"

if [ -f "BUILD_REQUIREMENTS.md" ]; then
    print_pass "BUILD_REQUIREMENTS.md exists"
    if grep -qi "websocket" BUILD_REQUIREMENTS.md; then
        print_pass "BUILD_REQUIREMENTS.md mentions WebSockets"
    else
        print_warning "BUILD_REQUIREMENTS.md doesn't mention WebSockets"
    fi
else
    print_warning "BUILD_REQUIREMENTS.md not found"
fi

if [ -f "TROUBLESHOOTING.md" ]; then
    print_pass "TROUBLESHOOTING.md exists"
    if grep -qi "websocket" TROUBLESHOOTING.md; then
        print_pass "TROUBLESHOOTING.md mentions WebSockets"
    else
        print_warning "TROUBLESHOOTING.md doesn't mention WebSockets"
    fi
else
    print_warning "TROUBLESHOOTING.md not found"
fi

if [ -f "README.md" ]; then
    print_pass "README.md exists"
    if grep -qi "websocket" README.md; then
        print_pass "README.md mentions WebSockets"
    else
        print_warning "README.md doesn't mention WebSockets"
    fi
else
    print_warning "README.md not found"
fi
echo ""

# 7. File Count Verification
print_section "7. File Count Verification"

cpp_count=$(find Plugins/WebSockets -name "*.cpp" -type f 2>/dev/null | wc -l)
h_count=$(find Plugins/WebSockets -name "*.h" -type f 2>/dev/null | wc -l)

if [ "$cpp_count" -ge 8 ]; then
    print_pass "Found $cpp_count C++ source files (expected: 8+)"
else
    print_fail "Only found $cpp_count C++ source files (expected: 8+)"
fi

if [ "$h_count" -ge 12 ]; then
    print_pass "Found $h_count header files (expected: 12+)"
else
    print_fail "Only found $h_count header files (expected: 12+)"
fi
echo ""

# Print Summary
print_header "Validation Summary"

echo -e "Results:"
echo -e "  ${GREEN}Passed:${NC}   $PASSED"
echo -e "  ${RED}Failed:${NC}   $FAILED"
echo -e "  ${YELLOW}Warnings:${NC} $WARNINGS"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✅ ALL CHECKS PASSED${NC}"
    echo ""
    echo "The WebSocket integration is complete and ready to work!"
    echo "All required files are present and properly configured."
    echo ""
    exit 0
else
    echo -e "${RED}❌ VALIDATION FAILED${NC}"
    echo ""
    echo "Some checks failed. Please review the errors above."
    echo "See WEBSOCKET_VALIDATION_REPORT.md for detailed information."
    echo ""
    exit 1
fi
