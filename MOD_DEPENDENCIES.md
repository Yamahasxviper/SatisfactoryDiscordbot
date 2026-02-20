# Mod Dependencies

This file explains the additional mods required to build and use the mods in this repository.

## DiscordBot Mod Dependencies

The **DiscordBot** mod (`Mods/DiscordBot/`) depends on the following mods:

| Mod Reference | Display Name | Version Required | Included in Repo |
|---|---|---|---|
| `SML` | Satisfactory Mod Loader | `^3.11.3` | ✅ Yes (`Mods/SML/`) |
| `CustomWebSocket` | Custom WebSocket | `^1.0.0` | ✅ Yes (`Mods/CustomWebSocket/`) |

Both dependencies are already included in this repository under the `Mods/` folder, so no additional downloads are required when building from source.

## How to Find a Mod's Dependencies

### Method 1: Check the `.uplugin` File

Every mod contains a `.uplugin` file that lists its dependencies in the `"Plugins"` section. For example, `Mods/DiscordBot/DiscordBot.uplugin` contains:

```json
"Plugins": [
    {
        "Name": "SML",
        "Enabled": true,
        "SemVersion": "^3.11.3"
    },
    {
        "Name": "CustomWebSocket",
        "Enabled": true,
        "SemVersion": "^1.0.0"
    }
]
```

The `"Name"` field is the **mod reference** (not the display name). Use this reference to look up the mod on the Satisfactory Mod Repository.

### Method 2: Check the Mod's SMR Page

Visit the mod's page on [ficsit.app](https://ficsit.app) and look for the **Mod Dependency** table, which lists all required mods by their mod reference.

## Finding Dependency Mods on ficsit.app

If a dependency mod is not included in this repository, you can find it on the Satisfactory Mod Repository using its **mod reference**:

```
https://ficsit.app/mod/<ModReference>
```

> ⚠️ The URL is **case sensitive** — use the mod reference exactly as it appears in the `.uplugin` file.

**Examples for this project's dependencies:**

- **SML**: [https://ficsit.app/mod/SML](https://ficsit.app/mod/SML)
- **CustomWebSocket**: [https://ficsit.app/mod/CustomWebSocket](https://ficsit.app/mod/CustomWebSocket)

> **Note:** A mod's display name does not always match its mod reference. If a dependency mod is hidden from the mod repository listing, you can still find its page by entering the mod reference directly into the URL as shown above.

## Setting Up External Dependency Mods

If a dependency mod is **not** included in this repository, follow these steps:

1. Find the dependency mod's source code repository (usually linked from its ficsit.app page).
2. Clone or download the dependency mod's source code.
3. Place the mod folder inside the `Mods/` directory of this Unreal project.
4. Verify the mod is registered in `FactoryGame.uproject` (add it to the `"Plugins"` section if needed).
5. Rebuild the project.

## Additional Setup Notes

- Check other text files in the root folder of this repository for additional build instructions:
  - [`BUILD_GUIDE.md`](BUILD_GUIDE.md) — Build instructions for all mods and plugins
  - [`PLUGIN_COMPILATION_FIX.md`](PLUGIN_COMPILATION_FIX.md) — Plugin registration fix details
  - [`INDEPENDENT_COMPILATION_GUIDE.md`](INDEPENDENT_COMPILATION_GUIDE.md) — Compiling plugins independently
- The `CustomWebSocket` mod is a dependency of `DiscordBot` and must be installed alongside it. See [`Mods/DiscordBot/Docs/Troubleshooting/MISSING_CUSTOMWEBSOCKET.md`](Mods/DiscordBot/Docs/Troubleshooting/MISSING_CUSTOMWEBSOCKET.md) if you encounter errors about a missing CustomWebSocket plugin.
