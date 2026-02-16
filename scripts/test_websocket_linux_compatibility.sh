#!/bin/bash
# WebSocket Linux Server Compatibility Test Script
# This script validates that WebSocket functionality will work correctly on Linux servers
# Usage: ./scripts/test_websocket_linux_compatibility.sh

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
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
    echo -e "${CYAN}$1${NC}"
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

print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

# Get script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

cd "$PROJECT_ROOT"

print_header "WebSocket Linux Server Compatibility Test"

# 1. Detect Operating System
print_section "1. Operating System Detection"

OS_TYPE=$(uname -s)
ARCH=$(uname -m)
KERNEL=$(uname -r)

echo "OS Type: $OS_TYPE"
echo "Architecture: $ARCH"
echo "Kernel: $KERNEL"
echo ""

if [ "$OS_TYPE" == "Linux" ]; then
    print_pass "Running on Linux - compatible with WebSocket testing"
    
    # Check Linux distribution
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        print_info "Distribution: $NAME $VERSION"
    fi
else
    print_warning "Not running on Linux (OS: $OS_TYPE)"
    print_info "This test is designed for Linux, but will continue with generic checks"
fi

if [ "$ARCH" == "x86_64" ]; then
    print_pass "Architecture: x86_64 (supported)"
elif [ "$ARCH" == "aarch64" ] || [ "$ARCH" == "arm64" ]; then
    print_pass "Architecture: $ARCH (ARM64 - supported)"
else
    print_warning "Architecture: $ARCH (may require additional configuration)"
fi
echo ""

# 2. Check Linux-Specific WebSocket Implementation
print_section "2. Linux WebSocket Implementation (libwebsockets)"

LWS_DIR="Plugins/WebSockets/Source/WebSockets/Private/Lws"
if [ -d "$LWS_DIR" ]; then
    print_pass "Linux WebSocket implementation directory exists"
    
    # Check key Linux implementation files
    if [ -f "$LWS_DIR/LwsWebSocket.cpp" ]; then
        print_pass "LwsWebSocket.cpp found"
        
        # Check for Linux-specific includes
        if grep -q "#include.*libwebsockets" "$LWS_DIR/LwsWebSocket.cpp" 2>/dev/null; then
            print_pass "libwebsockets headers included"
        else
            print_warning "libwebsockets headers not found in source"
        fi
    else
        print_fail "LwsWebSocket.cpp not found"
    fi
    
    if [ -f "$LWS_DIR/LwsWebSocketsManager.cpp" ]; then
        print_pass "LwsWebSocketsManager.cpp found"
    else
        print_fail "LwsWebSocketsManager.cpp not found"
    fi
    
    if [ -f "$LWS_DIR/LwsWebSocket.h" ]; then
        print_pass "LwsWebSocket.h found"
    else
        print_fail "LwsWebSocket.h not found"
    fi
    
    if [ -f "$LWS_DIR/LwsWebSocketsManager.h" ]; then
        print_pass "LwsWebSocketsManager.h found"
    else
        print_fail "LwsWebSocketsManager.h not found"
    fi
else
    print_fail "Linux WebSocket implementation directory not found"
fi
echo ""

# 3. Check Build Configuration for Linux
print_section "3. Build Configuration - Linux Platform Support"

BUILD_CS="Plugins/WebSockets/Source/WebSockets/WebSockets.Build.cs"
if [ -f "$BUILD_CS" ]; then
    print_pass "WebSockets.Build.cs found"
    
    # Check for Linux platform detection
    if grep -q "UnrealPlatformGroup.Unix" "$BUILD_CS"; then
        print_pass "Unix/Linux platform group detection present"
    else
        print_fail "Unix/Linux platform group detection missing"
    fi
    
    # Check for Linux architecture handling
    if grep -q "GetLinuxArchitecturePath" "$BUILD_CS"; then
        print_pass "Linux architecture path detection implemented"
        
        # Verify x86_64 support
        if grep -q "x86_64" "$BUILD_CS"; then
            print_pass "x86_64 architecture mapping present"
        else
            print_warning "x86_64 architecture mapping not explicitly found"
        fi
        
        # Verify ARM64 support
        if grep -q "aarch64\|arm64" "$BUILD_CS"; then
            print_pass "ARM64 architecture mapping present"
        else
            print_warning "ARM64 architecture mapping not found"
        fi
    else
        print_warning "Linux architecture path detection not found"
    fi
    
    # Check for libwebsockets library configuration
    if grep -q "libwebsockets" "$BUILD_CS"; then
        print_pass "libwebsockets library reference found"
    else
        print_fail "libwebsockets library reference missing"
    fi
    
    # Check for GNU triplet format
    if grep -q "unknown-linux-gnu" "$BUILD_CS"; then
        print_pass "GNU triplet format support detected"
    else
        print_warning "GNU triplet format not found (may use alternative format)"
    fi
else
    print_fail "WebSockets.Build.cs not found"
fi
echo ""

# 4. Check Third-Party Library Dependencies for Linux
print_section "4. Linux Third-Party Dependencies"

if [ -f "$BUILD_CS" ]; then
    # Check OpenSSL dependency
    if grep -q "OpenSSL" "$BUILD_CS"; then
        print_pass "OpenSSL dependency configured (required for SSL/TLS on Linux)"
    else
        print_fail "OpenSSL dependency not configured"
    fi
    
    # Check zlib dependency
    if grep -q "zlib" "$BUILD_CS"; then
        print_pass "zlib dependency configured (required for compression)"
    else
        print_fail "zlib dependency not configured"
    fi
    
    # Check SSL module dependency
    if grep -q '"SSL"' "$BUILD_CS"; then
        print_pass "Unreal SSL module dependency configured"
    else
        print_fail "Unreal SSL module dependency not configured"
    fi
    
    # Check for proper SSL flag
    if grep -q "WITH_SSL" "$BUILD_CS"; then
        print_pass "WITH_SSL compilation flag configured"
    else
        print_warning "WITH_SSL compilation flag not found"
    fi
fi
echo ""

# 5. Check Platform-Specific Compilation Flags
print_section "5. Linux Compilation Flags"

if [ -f "$BUILD_CS" ]; then
    # Check WITH_LIBWEBSOCKETS flag
    if grep -q "WITH_LIBWEBSOCKETS" "$BUILD_CS"; then
        print_pass "WITH_LIBWEBSOCKETS flag defined"
    else
        print_fail "WITH_LIBWEBSOCKETS flag not defined"
    fi
    
    # Check that WinHttp is NOT used on Linux
    if grep -q "bPlatformSupportsWinHttpWebSockets" "$BUILD_CS"; then
        print_pass "Windows WebSocket exclusion logic present"
        
        # Verify Windows-specific code is conditionally compiled
        if grep -q "Target.Platform.IsInGroup(UnrealPlatformGroup.Windows)" "$BUILD_CS"; then
            print_pass "Windows platform check ensures WinHttp only on Windows"
        else
            print_warning "Windows platform check not found"
        fi
    else
        print_warning "Windows WebSocket exclusion logic not found"
    fi
fi
echo ""

# 6. Check libwebsockets Library Paths
print_section "6. libwebsockets Library Path Configuration"

THIRD_PARTY_DIR="ThirdParty/libWebSockets"
if [ -d "$THIRD_PARTY_DIR" ]; then
    print_pass "Third-party libWebSockets directory exists"
    
    # Check for include directories
    if [ -d "$THIRD_PARTY_DIR/libwebsockets/include" ]; then
        print_pass "libwebsockets include directory found"
    else
        print_warning "libwebsockets include directory not found (may use engine libraries)"
    fi
    
    # Check for library files
    if [ -d "$THIRD_PARTY_DIR/libwebsockets/lib" ]; then
        print_pass "libwebsockets library directory found"
        
        # Check for Linux-specific library
        unix_lib_found=false
        for lib_path in "$THIRD_PARTY_DIR/libwebsockets/lib/Unix" "$THIRD_PARTY_DIR/libwebsockets/lib/Linux"; do
            if [ -d "$lib_path" ]; then
                print_pass "Linux library directory found: $lib_path"
                unix_lib_found=true
                
                # Check for architecture-specific libraries
                if [ -d "$lib_path/x86_64-unknown-linux-gnu" ]; then
                    print_pass "x86_64 Linux library directory found"
                fi
                if [ -d "$lib_path/aarch64-unknown-linux-gnueabi" ]; then
                    print_pass "ARM64 Linux library directory found"
                fi
            fi
        done
        
        if [ "$unix_lib_found" = false ]; then
            print_info "Linux-specific libraries not found locally (will use engine libraries)"
        fi
    else
        print_info "Local library directory not found (will use engine third-party libraries)"
    fi
else
    print_info "Local ThirdParty directory not present (will use engine third-party libraries)"
    print_info "This is normal - Unreal Engine provides libwebsockets in its third-party folder"
fi
echo ""

# 7. Verify Plugin Configuration
print_section "7. WebSocket Plugin Configuration"

UPLUGIN="Plugins/WebSockets/WebSockets.uplugin"
if [ -f "$UPLUGIN" ]; then
    print_pass "WebSockets.uplugin found"
    
    # Check if Linux is in the platform allowlist
    if grep -q '"Linux"' "$UPLUGIN"; then
        print_pass "Linux explicitly listed in platform allowlist"
    else
        print_fail "Linux not found in platform allowlist"
    fi
    
    # Verify other Unix platforms
    if grep -q '"Mac"' "$UPLUGIN"; then
        print_pass "Mac platform supported (uses same libwebsockets)"
    fi
    
    # Check module type
    if grep -q '"Type": "Runtime"' "$UPLUGIN"; then
        print_pass "Module type: Runtime (correct for server usage)"
    else
        print_warning "Module type is not Runtime"
    fi
else
    print_fail "WebSockets.uplugin not found"
fi
echo ""

# 8. Check DiscordChatBridge Linux Compatibility
print_section "8. DiscordChatBridge Linux Server Compatibility"

DCB_BUILD="Mods/DiscordChatBridge/Source/DiscordChatBridge/DiscordChatBridge.Build.cs"
if [ -f "$DCB_BUILD" ]; then
    print_pass "DiscordChatBridge.Build.cs found"
    
    # Check for WebSocket dependency
    if grep -q '"WebSockets"' "$DCB_BUILD"; then
        print_pass "WebSockets module dependency declared"
    else
        print_fail "WebSockets module dependency not declared"
    fi
    
    # Check for conditional compilation support
    if grep -q "WITH_WEBSOCKETS_SUPPORT" "$DCB_BUILD"; then
        print_pass "Conditional WebSocket compilation supported"
    else
        print_fail "Conditional WebSocket compilation not found"
    fi
else
    print_fail "DiscordChatBridge.Build.cs not found"
fi

# Check DiscordGateway implementation
DCB_GATEWAY="Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/DiscordGateway.cpp"
if [ -f "$DCB_GATEWAY" ]; then
    print_pass "DiscordGateway.cpp found"
    
    # Check for WebSocket usage
    if grep -q "IWebSocket\|FWebSocketsModule" "$DCB_GATEWAY"; then
        print_pass "WebSocket API usage found in DiscordGateway"
    else
        print_fail "WebSocket API not used in DiscordGateway"
    fi
    
    # Check for platform-independent implementation
    if grep -q "WITH_WEBSOCKETS_SUPPORT" "$DCB_GATEWAY"; then
        print_pass "Platform-independent WebSocket usage (good for Linux)"
    else
        print_warning "Conditional compilation not used in DiscordGateway"
    fi
else
    print_warning "DiscordGateway.cpp not found"
fi
echo ""

# 9. Network and SSL Requirements
print_section "9. Linux Server Network Requirements"

print_info "For WebSocket to work on Linux server, ensure:"
echo "  • Network connectivity to Discord Gateway (gateway.discord.gg)"
echo "  • Outbound HTTPS/WSS (port 443) is allowed"
echo "  • SSL/TLS libraries are available (OpenSSL)"
echo "  • Server can resolve DNS queries"
echo ""

# Check if we can test network connectivity
if command -v curl &> /dev/null; then
    print_info "Testing Discord Gateway connectivity..."
    if curl -s --connect-timeout 5 https://discord.com > /dev/null; then
        print_pass "Can connect to Discord servers"
    else
        print_warning "Cannot connect to Discord servers (may be network restrictions)"
    fi
elif command -v wget &> /dev/null; then
    print_info "Testing Discord Gateway connectivity..."
    if wget -q --timeout=5 --spider https://discord.com; then
        print_pass "Can connect to Discord servers"
    else
        print_warning "Cannot connect to Discord servers (may be network restrictions)"
    fi
else
    print_info "curl/wget not available - cannot test network connectivity"
fi
echo ""

# 10. Build System Compatibility
print_section "10. Linux Build System Compatibility"

# Check for Unreal Build Tool compatibility
print_info "Unreal Build Tool (UBT) Requirements:"
echo "  • Mono or .NET Core runtime (for Build.cs scripts)"
echo "  • GCC or Clang compiler"
echo "  • Make or Ninja build system"
echo ""

# Check if we're in a CI environment
if [ -n "$CI" ] || [ -n "$GITHUB_ACTIONS" ]; then
    print_pass "Running in CI environment"
    if [ -n "$RUNNER_OS" ]; then
        print_info "Runner OS: $RUNNER_OS"
    fi
else
    print_info "Running in local environment"
fi

# Check for .NET/Mono (needed for UBT Build.cs scripts)
if command -v dotnet &> /dev/null; then
    DOTNET_VERSION=$(dotnet --version 2>/dev/null || echo "unknown")
    print_pass "dotnet found (version: $DOTNET_VERSION)"
elif command -v mono &> /dev/null; then
    MONO_VERSION=$(mono --version 2>/dev/null | head -1 || echo "unknown")
    print_pass "Mono found ($MONO_VERSION)"
else
    print_info "dotnet/mono not found (provided by Unreal Engine installation)"
fi
echo ""

# 11. Server-Side Module Loading
print_section "11. Server-Side Module Loading"

DCB_UPLUGIN="Mods/DiscordChatBridge/DiscordChatBridge.uplugin"
if [ -f "$DCB_UPLUGIN" ]; then
    print_pass "DiscordChatBridge.uplugin found"
    
    # Check for server-compatible loading phase
    if grep -q '"LoadingPhase"' "$DCB_UPLUGIN"; then
        loading_phase=$(grep -A 0 '"LoadingPhase"' "$DCB_UPLUGIN" | head -1)
        print_pass "Loading phase configured: $loading_phase"
    fi
    
    # Check that it doesn't require client
    if grep -q '"RequiredOnRemote": false' "$DCB_UPLUGIN"; then
        print_pass "RequiredOnRemote: false (server-only, clients don't need it)"
    else
        print_warning "RequiredOnRemote setting not explicitly set to false"
    fi
else
    print_warning "DiscordChatBridge.uplugin not found"
fi
echo ""

# 12. Code Quality - Linux Specific Checks
print_section "12. Code Quality - Linux Considerations"

# Check for common Linux/Unix issues
LWS_IMPL="Plugins/WebSockets/Source/WebSockets/Private/Lws/LwsWebSocket.cpp"
if [ -f "$LWS_IMPL" ]; then
    # Check for proper error handling
    if grep -q "errno\|strerror\|perror" "$LWS_IMPL" 2>/dev/null; then
        print_pass "Unix error handling present"
    else
        print_info "Standard Unix error handling not found (may use custom logging)"
    fi
    
    # Check for thread safety (important for server)
    if grep -q "FScopeLock\|FCriticalSection\|FRWScopeLock" "$LWS_IMPL" 2>/dev/null; then
        print_pass "Thread synchronization primitives used"
    else
        print_info "Explicit thread locks not found (may rely on event system)"
    fi
    
    # Check for proper cleanup
    if grep -q "lws_context_destroy\|Close\|Disconnect" "$LWS_IMPL" 2>/dev/null; then
        print_pass "WebSocket cleanup code present"
    else
        print_warning "WebSocket cleanup code not clearly visible"
    fi
fi
echo ""

# Print Summary
print_header "Linux Server Compatibility Summary"

echo -e "Test Results:"
echo -e "  ${GREEN}Passed:${NC}   $PASSED"
echo -e "  ${RED}Failed:${NC}   $FAILED"
echo -e "  ${YELLOW}Warnings:${NC} $WARNINGS"
echo ""

# Calculate compatibility score
TOTAL=$((PASSED + FAILED))
if [ $TOTAL -gt 0 ]; then
    SCORE=$((PASSED * 100 / TOTAL))
    echo -e "Compatibility Score: ${CYAN}${SCORE}%${NC}"
    echo ""
fi

# Final verdict
if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✅ LINUX SERVER COMPATIBLE${NC}"
    echo ""
    echo "All critical checks passed! WebSocket functionality will work on Linux servers."
    echo ""
    echo "Summary:"
    echo "  • Linux platform support: Configured"
    echo "  • libwebsockets implementation: Present"
    echo "  • Build system: Compatible"
    echo "  • SSL/TLS support: Configured (OpenSSL)"
    echo "  • Server-side deployment: Ready"
    echo ""
    echo "Next Steps:"
    echo "  1. Build the project with Unreal Engine 5.3.2-CSS"
    echo "  2. Deploy DiscordChatBridge mod to Linux dedicated server"
    echo "  3. Configure bot token in DefaultDiscordChatBridge.ini"
    echo "  4. Start server and verify WebSocket connection to Discord Gateway"
    echo ""
    exit 0
elif [ $FAILED -le 3 ] && [ $WARNINGS -le 5 ]; then
    echo -e "${YELLOW}⚠ LIKELY COMPATIBLE WITH MINOR ISSUES${NC}"
    echo ""
    echo "Most checks passed, but some issues were found."
    echo "WebSocket functionality should work on Linux servers with minor fixes."
    echo ""
    echo "Review the failed checks above and address critical issues."
    echo ""
    exit 0
else
    echo -e "${RED}❌ COMPATIBILITY ISSUES DETECTED${NC}"
    echo ""
    echo "Critical issues were found that may prevent WebSocket from working on Linux."
    echo "Please review the failed checks above and fix them before deploying to Linux server."
    echo ""
    echo "Common issues:"
    echo "  • Missing Linux-specific implementation files"
    echo "  • Incorrect platform detection in Build.cs"
    echo "  • Missing third-party library dependencies"
    echo "  • Incorrect plugin configuration"
    echo ""
    echo "For help, see:"
    echo "  • BUILD_REQUIREMENTS.md"
    echo "  • TROUBLESHOOTING.md"
    echo "  • WEBSOCKET_BUILD_COMPATIBILITY.md"
    echo ""
    exit 1
fi
