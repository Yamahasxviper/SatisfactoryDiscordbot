#!/bin/bash
# Test script to verify that Linux server builds are configured for Monolithic linking
# This validates the fix for the "dlopen failed: libFactoryServer-WebSockets-Linux-Shipping.so" error
#
# Usage: ./scripts/test_linux_monolithic_config.sh
# Note: If you get a permission denied error, run: chmod +x scripts/test_linux_monolithic_config.sh

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=========================================="
echo "Linux Server Monolithic Build Configuration Test"
echo -e "==========================================${NC}\n"

# Get script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo -e "${BLUE}Testing FactoryShared.Target.cs configuration...${NC}\n"

# Check that the file exists
TARGET_FILE="$PROJECT_ROOT/Source/FactoryShared.Target.cs"
if [ ! -f "$TARGET_FILE" ]; then
    echo -e "${RED}✗ FactoryShared.Target.cs not found at: $TARGET_FILE${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Found FactoryShared.Target.cs${NC}\n"

# Test 1: Check that the file contains the Linux + Server check using Target.Name
echo -e "${BLUE}Test 1: Checking for Target.Name.Contains(\"Server\") condition...${NC}"
if grep -q 'Target.Name.Contains("Server")' "$TARGET_FILE"; then
    echo -e "${GREEN}✓ PASS: Found Target.Name.Contains(\"Server\") check${NC}"
    echo -e "   This correctly identifies server builds regardless of constructor order\n"
else
    echo -e "${RED}✗ FAIL: Target.Name.Contains(\"Server\") check not found${NC}"
    echo -e "${YELLOW}   The old Type == TargetType.Server check has a constructor order bug${NC}\n"
    exit 1
fi

# Test 2: Verify the condition includes Linux platform check
echo -e "${BLUE}Test 2: Checking for Linux platform condition...${NC}"
if grep -q 'Target.Platform == UnrealTargetPlatform.Linux && Target.Name.Contains("Server")' "$TARGET_FILE"; then
    echo -e "${GREEN}✓ PASS: Linux platform check found${NC}"
    echo -e "   Configuration specifically targets Linux server builds\n"
else
    echo -e "${RED}✗ FAIL: Linux platform check not found or incorrect${NC}\n"
    exit 1
fi

# Test 3: Verify Monolithic linking is set
echo -e "${BLUE}Test 3: Checking for Monolithic link type assignment...${NC}"
if grep -A 2 'Target.Name.Contains("Server")' "$TARGET_FILE" | grep -q 'TargetLinkType.Monolithic'; then
    echo -e "${GREEN}✓ PASS: Monolithic link type assignment found${NC}"
    echo -e "   Linux server builds will use static linking\n"
else
    echo -e "${RED}✗ FAIL: Monolithic link type not assigned${NC}\n"
    exit 1
fi

# Test 4: Check for explanatory comment about the fix
echo -e "${BLUE}Test 4: Checking for documentation comment...${NC}"
if grep -q 'Target.Name' "$TARGET_FILE" && grep -q 'derived class constructor' "$TARGET_FILE"; then
    echo -e "${GREEN}✓ PASS: Explanatory comment found${NC}"
    echo -e "   Comment explains why Target.Name is used instead of Type\n"
else
    echo -e "${YELLOW}⚠ WARNING: Explanatory comment not found or incomplete${NC}"
    echo -e "   Consider adding a comment explaining the constructor order issue\n"
fi

# Test 5: Verify that the old buggy pattern is not present
echo -e "${BLUE}Test 5: Checking that old buggy pattern is removed...${NC}"
if grep -q 'Type == TargetType.Server' "$TARGET_FILE"; then
    echo -e "${RED}✗ FAIL: Old buggy pattern 'Type == TargetType.Server' still present${NC}"
    echo -e "   This check fails due to constructor order - Type is set in derived class${NC}\n"
    exit 1
else
    echo -e "${GREEN}✓ PASS: Old buggy pattern not found${NC}"
    echo -e "   Constructor order issue has been fixed\n"
fi

# Summary
echo -e "${BLUE}=========================================="
echo "Test Summary"
echo -e "==========================================${NC}\n"
echo -e "${GREEN}All tests passed!${NC}"
echo ""
echo "The configuration correctly ensures that Linux server builds use"
echo "Monolithic linking, which prevents the runtime error:"
echo ""
echo -e "${YELLOW}  'dlopen failed: libFactoryServer-WebSockets-Linux-Shipping.so'"
echo -e "  'cannot open shared object file: No such file or directory'${NC}"
echo ""
echo "With Monolithic linking, all modules (including WebSockets) are"
echo "statically linked into the server executable, eliminating the need"
echo "for separate .so files at runtime."
echo ""
echo -e "${GREEN}✓ Fix validated successfully!${NC}"
exit 0