# Build and Packaging Notes

## Including Documentation Files in Builds

### Issue
When building the DiscordChatBridge mod using Unreal Engine's packaging system (Alpakit), documentation files (README.md, QUICKSTART.md, etc.) and configuration directories were not being included in the packaged output.

### Solution
Created `Config/PluginSettings.ini` to specify which additional directories should be included in the packaged mod.

### What Gets Included
The following are now included when you build/package the mod:

1. **Documentation Files** (at mod root):
   - README.md
   - QUICKSTART.md
   - SETUP_GUIDE.md
   - EXAMPLES.md
   - COMPILATION_FIX.md
   - DEPENDENCY_EXPLANATION.md
   - IMPLEMENTATION_SUMMARY.md

2. **Configuration Directories**:
   - `Config/` - Contains DefaultDiscordChatBridge.ini and example configurations
   - `config/` - Contains the TXT format configuration system

### How It Works
The `PluginSettings.ini` file uses the `[StageSettings]` section to tell Unreal Engine's build system which additional directories to include:

```ini
[StageSettings]
+AdditionalNonUSFDirectories=Config
+AdditionalNonUSFDirectories=config
```

Markdown files at the mod root are automatically included by Alpakit's packaging system when `CanContainContent` is set to `true` in the `.uplugin` file (which it is).

### Verifying Files Are Included
After building/packaging your mod, check the output directory:
- **Development**: `Saved/ArchivedPlugins/DiscordChatBridge/`
- **Packaged**: The resulting `.zip` or installed mod directory

You should see:
```
DiscordChatBridge/
├── README.md
├── QUICKSTART.md
├── SETUP_GUIDE.md
├── [other .md files]
├── Config/
│   ├── DefaultDiscordChatBridge.ini
│   ├── PluginSettings.ini
│   └── ExampleConfigs/
└── config/
    ├── DiscordChatBridge.txt
    └── README.md
```

### For Mod Developers
If you're creating your own mod and want to include documentation or configuration files in the packaged output:

1. Create `Config/PluginSettings.ini` in your mod directory
2. Add the `[StageSettings]` section
3. List each directory you want to include with `+AdditionalNonUSFDirectories=DirectoryName`
4. Ensure your `.uplugin` has `"CanContainContent": true`

### References
- Unreal Engine Documentation: [Plugin Packaging](https://docs.unrealengine.com/en-US/ProductionPipelines/Plugins/)
- SML Documentation: [Creating Mods](https://docs.ficsit.app/)
- Alpakit Tool: The SML packaging system used for mod distribution
