# Discord Config Files Consolidation

## Summary

Removed duplicate Discord configuration template files to eliminate redundancy while maintaining full functionality.

## What Changed

### Files Removed
- `Config/LinuxServer/DiscordChatBridge.ini` (135 lines)
- `Config/WindowsServer/DiscordChatBridge.ini` (135 lines)

These files were **100% identical** and served only as reference templates.

### Documentation Updated
- `Mods/DiscordChatBridge/README.md` - Updated to reference single template file
- `Mods/DiscordChatBridge/help/SETUP_GUIDE.md` - Simplified template references

## Why This Change?

### Redundancy
Both platform-specific config files contained identical content. The Discord Chat Bridge configuration is **platform-agnostic** - the same settings work identically on both Windows and Linux servers.

### Maintenance Burden
Having duplicate files meant:
- Any configuration changes had to be made twice
- Risk of files drifting out of sync
- Confusion about which file to use

## How Configuration Works

The mod uses a multi-tier configuration system:

1. **Primary (Recommended)**: `Mods/DiscordChatBridge/config/DiscordChatBridge.ini`
   - Loaded by ServerDefaultsConfigLoader
   - SML-integrated, persists across restarts
   - Platform-agnostic

2. **Secondary (Traditional)**: Runtime Unreal Engine Config
   - Windows: `%localappdata%/FactoryGame/Saved/Config/WindowsServer/DiscordChatBridge.ini`
   - Linux: `~/.config/Epic/FactoryGame/Saved/Config/LinuxServer/DiscordChatBridge.ini`
   - User creates these files based on templates
   - Unreal Engine automatically selects the right platform directory at runtime

3. **Template for Users**: `Mods/DiscordChatBridge/Config/DefaultDiscordChatBridge.ini`
   - Comprehensive template with all options
   - Documentation and examples
   - Users copy this to their runtime config location

## Impact

### ✅ No Breaking Changes
- The mod still works exactly as before
- All configuration options remain available
- Both configuration methods (config/ and Config/) still work
- Runtime platform-specific directories are still used correctly by Unreal Engine

### ✅ Benefits
- Reduced codebase size (-270 lines)
- Single source of truth for template configuration
- Clearer documentation
- Less maintenance overhead
- No risk of template files drifting out of sync

### ✅ Backwards Compatibility
- Existing server configurations are unaffected
- Users who already have configs in their runtime directories continue working
- No changes required by server administrators

## For Server Administrators

**No action required!** If your server is already configured and working, this change doesn't affect you at all. Your configuration files are in the runtime directories (outside this project), not in these template files.

## For Developers

If you're developing or setting up a new server:
- Use `Mods/DiscordChatBridge/Config/DefaultDiscordChatBridge.ini` as your template
- Copy it to your runtime config directory (platform-specific location)
- OR use `Mods/DiscordChatBridge/config/DiscordChatBridge.ini` for SML-based config

The configuration is platform-agnostic - you don't need separate configs for Linux and Windows.
