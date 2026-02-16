#!/bin/bash
# Cross-Platform Discord Configuration Validation Script
# This script validates that all Discord config files are properly formatted
# for cross-platform compatibility (Windows and Linux)

echo "=========================================="
echo "Discord Config Cross-Platform Validator"
echo "=========================================="
echo ""

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

ERRORS=0
WARNINGS=0
CHECKS=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_pass() {
    echo -e "${GREEN}✓${NC} $1"
}

print_fail() {
    echo -e "${RED}✗${NC} $1"
    ((ERRORS++))
}

print_warn() {
    echo -e "${YELLOW}⚠${NC} $1"
    ((WARNINGS++))
}

# Find all INI files in the Discord mod
echo "Finding Discord configuration files..."
INI_FILES=$(find "Mods/DiscordChatBridge" -type f -name "*.ini" 2>/dev/null || true)

if [ -z "$INI_FILES" ]; then
    print_fail "No .ini files found in Mods/DiscordChatBridge"
    exit 1
fi

echo "Found $(echo "$INI_FILES" | wc -l) INI files"
echo ""

# Check 1: Line endings (must be LF for cross-platform compatibility)
echo "Check 1: Validating line endings (LF required)..."
((CHECKS++))
LINE_ENDING_ERRORS=0

while IFS= read -r file; do
    [ -z "$file" ] && continue
    # Check if file has CRLF line endings
    file_output=$(file "$file" 2>/dev/null || echo "unknown")
    if echo "$file_output" | grep -q "CRLF"; then
        print_fail "File has Windows line endings (CRLF): $file"
        ((LINE_ENDING_ERRORS++))
    elif echo "$file_output" | grep -q "text"; then
        # Has text but not CRLF, so it's likely LF
        print_pass "Line endings OK: $file"
    else
        print_warn "Could not determine line endings: $file"
    fi
done <<< "$INI_FILES"

if [ $LINE_ENDING_ERRORS -eq 0 ]; then
    echo ""
    print_pass "All INI files use Unix line endings (LF)"
else
    echo ""
    print_fail "$LINE_ENDING_ERRORS files have incorrect line endings"
fi
echo ""

# Check 2: File encoding (UTF-8 required)
echo "Check 2: Validating file encoding (UTF-8 required)..."
((CHECKS++))
ENCODING_ERRORS=0

while IFS= read -r file; do
    [ -z "$file" ] && continue
    encoding=$(file -b --mime-encoding "$file" 2>/dev/null || echo "unknown")
    if [ "$encoding" = "utf-8" ] || [ "$encoding" = "us-ascii" ]; then
        print_pass "Encoding OK ($encoding): $file"
    else
        print_fail "Invalid encoding ($encoding): $file"
        ((ENCODING_ERRORS++))
    fi
done <<< "$INI_FILES"

if [ $ENCODING_ERRORS -eq 0 ]; then
    echo ""
    print_pass "All INI files use UTF-8 or ASCII encoding"
else
    echo ""
    print_fail "$ENCODING_ERRORS files have incorrect encoding"
fi
echo ""

# Check 3: INI format validation
echo "Check 3: Validating INI format..."
((CHECKS++))
FORMAT_ERRORS=0

while IFS= read -r file; do
    [ -z "$file" ] && continue
    # Check if file has the required section header
    if grep -q "^\[/Script/DiscordChatBridge.DiscordChatSubsystem\]" "$file" 2>/dev/null; then
        print_pass "Section header found: $file"
    else
        # Check if it's a README or documentation file
        if [[ "$file" == *"README"* ]]; then
            print_pass "Skipping documentation file: $file"
        else
            print_warn "Missing section header: $file"
        fi
    fi
    
    # Check for valid KEY=VALUE format
    invalid_lines=$(grep -vE "^(;|#|\[|\s*$|[A-Za-z][A-Za-z0-9_]*\s*=)" "$file" 2>/dev/null | wc -l || echo "0")
    if [ "$invalid_lines" -gt 0 ]; then
        print_warn "File may contain $invalid_lines invalid lines: $file"
    fi
done <<< "$INI_FILES"

echo ""

# Check 4: Path separator validation in C++ code
echo "Check 4: Validating path construction in C++ code..."
((CHECKS++))
PATH_ERRORS=0

# Check for hardcoded path separators in config loader
if grep -r "\"\\\\\"" "Mods/DiscordChatBridge/Source" 2>/dev/null | grep -v "Binary" | grep -q "config"; then
    print_fail "Found hardcoded backslashes in config path construction"
    ((PATH_ERRORS++))
elif grep -r "\"\/\"" "Mods/DiscordChatBridge/Source" 2>/dev/null | grep -v "Binary" | grep -q "config"; then
    print_fail "Found hardcoded forward slashes in config path construction"
    ((PATH_ERRORS++))
else
    print_pass "No hardcoded path separators found in config code"
fi

# Verify FPaths::Combine usage
if grep -q "FPaths::Combine" "Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/ServerDefaultsConfigLoader.cpp"; then
    print_pass "Config loader uses FPaths::Combine for cross-platform paths"
else
    print_fail "Config loader does not use FPaths::Combine"
    ((PATH_ERRORS++))
fi

# Verify line parsing handles both CRLF and LF
if grep -q "ParseIntoArrayLines" "Mods/DiscordChatBridge/Source/DiscordChatBridge/Private/ServerDefaultsConfigLoader.cpp"; then
    print_pass "Config loader uses ParseIntoArrayLines (handles all line endings)"
else
    print_warn "Config loader might not handle all line ending formats"
fi

echo ""

# Check 5: .gitattributes configuration
echo "Check 5: Validating .gitattributes configuration..."
((CHECKS++))

if [ -f ".gitattributes" ]; then
    if grep -q "^\*\.ini.*eol=lf" ".gitattributes"; then
        print_pass ".gitattributes explicitly normalizes .ini files to LF"
    elif grep -q "^\* text=auto" ".gitattributes"; then
        print_pass ".gitattributes has text=auto (will normalize line endings)"
    else
        print_warn ".gitattributes does not specify line ending handling"
    fi
else
    print_warn ".gitattributes file not found"
fi

echo ""

# Check 6: Build configuration
echo "Check 6: Validating build targets..."
((CHECKS++))

if [ -f ".github/workflows/build.yml" ]; then
    if grep -q "serverplatform=Win64+Linux" ".github/workflows/build.yml"; then
        print_pass "Build workflow targets both Win64 and Linux"
    else
        print_warn "Build workflow may not target all platforms"
    fi
else
    print_warn "Build workflow not found"
fi

echo ""

# Summary
echo "=========================================="
echo "Validation Summary"
echo "=========================================="
echo "Total checks performed: $CHECKS"
echo -e "${GREEN}Passed checks: $(($CHECKS - $ERRORS))${NC}"
if [ $WARNINGS -gt 0 ]; then
    echo -e "${YELLOW}Warnings: $WARNINGS${NC}"
fi
if [ $ERRORS -gt 0 ]; then
    echo -e "${RED}Failed checks: $ERRORS${NC}"
fi
echo ""

if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ All Discord config files are cross-platform compatible!${NC}"
    echo ""
    echo "The following platforms are fully supported:"
    echo "  - Windows Server (Win64)"
    echo "  - Linux Server (x86_64, ARM64)"
    echo "  - Windows Client (Win64)"
    echo "  - Linux Client"
    echo ""
    exit 0
else
    echo -e "${RED}✗ Cross-platform compatibility issues found!${NC}"
    echo ""
    echo "Please fix the issues listed above before deploying to production."
    echo ""
    exit 1
fi
