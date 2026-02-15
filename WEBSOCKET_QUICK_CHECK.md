# Quick WebSocket Validation Guide

## Purpose

This guide helps you quickly verify that the WebSocket plugin is properly integrated and all required files are present.

## Quick Check

Run the automated validation script:

```bash
./scripts/validate_websocket_integration.sh
```

## Expected Output

If everything is correct, you should see:

```
✅ ALL CHECKS PASSED

The WebSocket integration is complete and ready to work!
All required files are present and properly configured.
```

## What Gets Checked

The validation script performs **45 automated checks** covering:

1. ✅ **22 WebSocket Plugin Files** - All source files, headers, and build files
2. ✅ **Project Configuration** - FactoryGame.uproject settings
3. ✅ **Mod Configuration** - DiscordChatBridge.uplugin settings
4. ✅ **Build System** - Conditional compilation and module linking
5. ✅ **Implementation** - DiscordGateway WebSocket usage
6. ✅ **Documentation** - All docs mention WebSockets correctly
7. ✅ **File Counts** - Correct number of C++ and header files

## Interpreting Results

### ✅ Green Checkmarks (✓)
- Check passed successfully
- No action needed

### ❌ Red X Marks (✗)
- Critical issue detected
- Must be fixed before building
- See error message for details

### ⚠️ Yellow Warnings (⚠)
- Non-critical issue
- Project may still work
- Consider addressing for best practices

## Common Issues and Solutions

### Issue: "MISSING: Plugins/WebSockets/..."
**Solution:** WebSocket plugin files are missing. Ensure you have the complete repository:
```bash
git status
git pull origin main
```

### Issue: "WebSockets not found in FactoryGame.uproject"
**Solution:** The plugin declaration is missing. Check that you're on the correct branch:
```bash
git branch
git checkout main
```

### Issue: "WITH_WEBSOCKETS_SUPPORT not found"
**Solution:** Build configuration is missing conditional compilation support. Verify Build.cs file is present and up-to-date.

## Manual Verification

If you prefer to check manually:

### 1. Check Plugin Files Exist
```bash
ls -la Plugins/WebSockets/
```

Expected: You should see `Public/`, `Private/`, `WebSockets.uplugin`, and `WebSockets.Build.cs`

### 2. Check Project Configuration
```bash
grep -A 2 "WebSockets" FactoryGame.uproject
```

Expected output:
```json
{
    "Name": "WebSockets",
    "Enabled": true,
    "Optional": true
}
```

### 3. Check Mod Configuration
```bash
grep -A 2 "WebSockets" Mods/DiscordChatBridge/DiscordChatBridge.uplugin
```

Expected output:
```json
{
    "Name": "WebSockets",
    "Enabled": true,
    "Optional": true
}
```

### 4. Count Source Files
```bash
find Plugins/WebSockets -name "*.cpp" | wc -l  # Should be 8
find Plugins/WebSockets -name "*.h" | wc -l    # Should be 12
```

## Detailed Report

For a comprehensive analysis, see:
- [WEBSOCKET_VALIDATION_REPORT.md](WEBSOCKET_VALIDATION_REPORT.md)

This report includes:
- Complete file listing
- Platform-specific details
- Feature status matrix
- Code quality analysis
- Testing recommendations

## Integration with CI/CD

To add this validation to your CI pipeline:

### GitHub Actions

```yaml
- name: Validate WebSocket Integration
  run: |
    chmod +x scripts/validate_websocket_integration.sh
    ./scripts/validate_websocket_integration.sh
```

### Exit Codes

- **0**: All checks passed ✅
- **1**: One or more checks failed ❌

## After Validation Passes

Once validation passes, you can proceed with:

1. **Building the project** using the CI workflow
2. **Configuring DiscordChatBridge** with your bot token
3. **Enabling Gateway features** in the configuration
4. **Testing WebSocket connectivity** to Discord Gateway

## Need Help?

If validation fails and you can't resolve the issue:

1. Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
2. Review [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md)
3. Join the [Discord community](https://discord.gg/QzcG9nX)

## Version Compatibility

This validation script is designed for:
- **Unreal Engine**: 5.3.2-CSS
- **WebSockets Plugin**: v1.0
- **DiscordChatBridge**: v1.0.0
- **SML**: ^3.11.3

---

**Last Updated**: 2026-02-15  
**Script Location**: `scripts/validate_websocket_integration.sh`  
**Report Location**: `WEBSOCKET_VALIDATION_REPORT.md`
