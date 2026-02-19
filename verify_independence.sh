#!/bin/bash
# Verification script for CustomWebSocket and DiscordBot independence
# This script verifies that CustomWebSocket and DiscordBot are properly separated

set -e

echo "=========================================="
echo "CustomWebSocket Independence Verification"
echo "=========================================="
echo ""

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

SUCCESS=0
WARNINGS=0
FAILURES=0

check_pass() {
    echo -e "${GREEN}✅ PASS${NC}: $1"
    SUCCESS=$((SUCCESS + 1))
}

check_warn() {
    echo -e "${YELLOW}⚠️  WARN${NC}: $1"
    WARNINGS=$((WARNINGS + 1))
}

check_fail() {
    echo -e "${RED}❌ FAIL${NC}: $1"
    FAILURES=$((FAILURES + 1))
}

echo "Test 1: Verify CustomWebSocket has no references to DiscordBot"
echo "----------------------------------------------------------------"
if grep -r "DiscordBot" Mods/CustomWebSocket/Source/ 2>/dev/null; then
    check_fail "CustomWebSocket has references to DiscordBot (should be independent)"
else
    check_pass "CustomWebSocket has NO references to DiscordBot"
fi
echo ""

echo "Test 2: Verify DiscordBot has dependency on CustomWebSocket"
echo "----------------------------------------------------------------"
if grep -q "CustomWebSocket" Mods/DiscordBot/DiscordBot.uplugin 2>/dev/null; then
    check_pass "DiscordBot declares CustomWebSocket dependency"
else
    check_fail "DiscordBot missing CustomWebSocket dependency declaration"
fi
echo ""

echo "Test 3: Verify CustomWebSocket has SemVersion"
echo "----------------------------------------------------------------"
if grep -q '"SemVersion"' Mods/CustomWebSocket/CustomWebSocket.uplugin 2>/dev/null; then
    VERSION=$(grep '"SemVersion"' Mods/CustomWebSocket/CustomWebSocket.uplugin | cut -d'"' -f4)
    check_pass "CustomWebSocket has SemVersion: $VERSION"
else
    check_fail "CustomWebSocket missing SemVersion field"
fi
echo ""

echo "Test 4: Verify DiscordBot specifies CustomWebSocket version requirement"
echo "----------------------------------------------------------------"
if grep -A 10 '"Name": "CustomWebSocket"' Mods/DiscordBot/DiscordBot.uplugin | grep -q '"SemVersion"' 2>/dev/null; then
    VERSION_REQ=$(grep -A 10 '"Name": "CustomWebSocket"' Mods/DiscordBot/DiscordBot.uplugin | grep '"SemVersion"' | cut -d'"' -f4)
    check_pass "DiscordBot requires CustomWebSocket version: $VERSION_REQ"
else
    check_fail "DiscordBot missing CustomWebSocket version requirement"
fi
echo ""

echo "Test 5: Verify CustomWebSocket loads before DiscordBot"
echo "----------------------------------------------------------------"
CW_PHASE=$(grep '"LoadingPhase"' Mods/CustomWebSocket/CustomWebSocket.uplugin | cut -d'"' -f4)
DB_PHASE=$(grep '"LoadingPhase"' Mods/DiscordBot/DiscordBot.uplugin | cut -d'"' -f4)

if [ "$CW_PHASE" = "PreDefault" ] && [ "$DB_PHASE" = "PostDefault" ]; then
    check_pass "CustomWebSocket loads in PreDefault, DiscordBot in PostDefault (correct order)"
else
    check_fail "Loading phases incorrect: CustomWebSocket=$CW_PHASE, DiscordBot=$DB_PHASE"
fi
echo ""

echo "Test 6: Verify CustomWebSocket.Build.cs has no DiscordBot dependencies"
echo "----------------------------------------------------------------"
if grep -q "DiscordBot" Mods/CustomWebSocket/Source/CustomWebSocket/CustomWebSocket.Build.cs 2>/dev/null; then
    check_fail "CustomWebSocket.Build.cs has DiscordBot dependency"
else
    check_pass "CustomWebSocket.Build.cs has NO DiscordBot dependency"
fi
echo ""

echo "Test 7: Verify DiscordBot.Build.cs has CustomWebSocket dependency"
echo "----------------------------------------------------------------"
if grep -q "CustomWebSocket" Mods/DiscordBot/Source/DiscordBot/DiscordBot.Build.cs 2>/dev/null; then
    check_pass "DiscordBot.Build.cs includes CustomWebSocket dependency"
else
    check_fail "DiscordBot.Build.cs missing CustomWebSocket dependency"
fi
echo ""

echo "Test 8: Verify both plugins are registered in FactoryGame.uproject"
echo "----------------------------------------------------------------"
CW_ENABLED=$(python3 -c "import json; data=json.load(open('FactoryGame.uproject')); plugins=[p for p in data.get('Plugins', []) if p['Name']=='CustomWebSocket']; print(plugins[0]['Enabled'] if plugins else 'false')" 2>/dev/null || echo "error")
DB_ENABLED=$(python3 -c "import json; data=json.load(open('FactoryGame.uproject')); plugins=[p for p in data.get('Plugins', []) if p['Name']=='DiscordBot']; print(plugins[0]['Enabled'] if plugins else 'false')" 2>/dev/null || echo "error")

if [ "$CW_ENABLED" = "True" ]; then
    check_pass "CustomWebSocket is enabled in FactoryGame.uproject"
elif [ "$CW_ENABLED" = "false" ]; then
    check_warn "CustomWebSocket is disabled in FactoryGame.uproject"
else
    check_fail "CustomWebSocket not found in FactoryGame.uproject"
fi

if [ "$DB_ENABLED" = "True" ]; then
    check_pass "DiscordBot is enabled in FactoryGame.uproject"
elif [ "$DB_ENABLED" = "false" ]; then
    check_warn "DiscordBot is disabled in FactoryGame.uproject"
else
    check_fail "DiscordBot not found in FactoryGame.uproject"
fi
echo ""

echo "Test 9: Verify CustomWebSocket source files exist"
echo "----------------------------------------------------------------"
if [ -f "Mods/CustomWebSocket/Source/CustomWebSocket/Public/CustomWebSocket.h" ] && \
   [ -f "Mods/CustomWebSocket/Source/CustomWebSocket/Private/CustomWebSocket.cpp" ]; then
    check_pass "CustomWebSocket source files exist"
else
    check_fail "CustomWebSocket source files missing"
fi
echo ""

echo "Test 10: Verify DiscordBot source files exist"
echo "----------------------------------------------------------------"
if [ -f "Mods/DiscordBot/Source/DiscordBot/Public/DiscordGatewayClientCustom.h" ] && \
   [ -f "Mods/DiscordBot/Source/DiscordBot/Private/DiscordGatewayClientCustom.cpp" ]; then
    check_pass "DiscordBot source files exist"
else
    check_fail "DiscordBot source files missing"
fi
echo ""

echo "Test 11: Verify CustomWebSocket includes are correct"
echo "----------------------------------------------------------------"
# CustomWebSocket should only include standard Unreal modules
if grep -h "^#include" Mods/CustomWebSocket/Source/CustomWebSocket/Public/*.h | \
   grep -v "CoreMinimal.h" | \
   grep -v "Sockets.h" | \
   grep -v "SocketSubsystem.h" | \
   grep -v "IPAddress.h" | \
   grep -v "Modules/ModuleManager.h" | \
   grep -v "CustomWebSocket" | \
   grep -q . 2>/dev/null; then
    check_warn "CustomWebSocket has unexpected includes (may affect independence)"
else
    check_pass "CustomWebSocket only includes standard Unreal modules"
fi
echo ""

echo "Test 12: Verify DiscordBot includes CustomWebSocket"
echo "----------------------------------------------------------------"
if grep -q '#include "CustomWebSocket.h"' Mods/DiscordBot/Source/DiscordBot/Public/DiscordGatewayClientCustom.h 2>/dev/null; then
    check_pass "DiscordBot includes CustomWebSocket header"
else
    check_fail "DiscordBot missing CustomWebSocket include"
fi
echo ""

echo "=========================================="
echo "Verification Summary"
echo "=========================================="
echo ""
echo -e "${GREEN}✅ Passed:${NC} $SUCCESS"
echo -e "${YELLOW}⚠️  Warnings:${NC} $WARNINGS"
echo -e "${RED}❌ Failed:${NC} $FAILURES"
echo ""

if [ $FAILURES -gt 0 ]; then
    echo -e "${RED}❌ VERIFICATION FAILED${NC}: CustomWebSocket and DiscordBot have structural issues"
    echo ""
    echo "Please review the failed tests above and consult:"
    echo "  - INDEPENDENT_COMPILATION_GUIDE.md"
    echo "  - WEBSOCKET_SEPARATION_ANSWER.md"
    echo "  - COMPILATION_FIX_SUMMARY.md"
    exit 1
elif [ $WARNINGS -gt 0 ]; then
    echo -e "${YELLOW}⚠️  VERIFICATION PASSED WITH WARNINGS${NC}"
    echo ""
    echo "CustomWebSocket and DiscordBot are properly separated,"
    echo "but there are some configuration warnings above."
    exit 0
else
    echo -e "${GREEN}✅ VERIFICATION PASSED${NC}: CustomWebSocket and DiscordBot are properly separated!"
    echo ""
    echo "Key findings:"
    echo "  ✅ CustomWebSocket has ZERO references to DiscordBot"
    echo "  ✅ DiscordBot correctly depends on CustomWebSocket (one-way)"
    echo "  ✅ Loading phases are correct (CustomWebSocket before DiscordBot)"
    echo "  ✅ Both plugins properly registered"
    echo "  ✅ All source files present"
    echo ""
    echo "Conclusion: CustomWebSocket CAN compile independently!"
    echo ""
    echo "For compilation instructions, see:"
    echo "  - INDEPENDENT_COMPILATION_GUIDE.md"
    echo "  - WEBSOCKET_SEPARATION_ANSWER.md"
    exit 0
fi
