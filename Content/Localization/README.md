# Localization Files

This directory contains minimal localization resource files to prevent "could not be opened" warnings during server startup.

## Structure

The following localization targets are provided:

- **Game/**: Default Unreal Engine game localization target
- **InputKeys/**: Input key localization for the game
- **Narrative/**: Narrative/story text localization
- **AllStringTables/**: Combined string tables localization
- **StringTables/**: CSV string tables (actual game content)

Each target directory contains `en` (English) and `en-US` (English - United States) subdirectories with minimal `.locres` (Localization Resource) files.

## About .locres Files

The `.locres` files are binary files in Unreal Engine's proprietary localization format. The minimal files provided here are based on the ExampleMod template and serve to satisfy the engine's localization system requirements.

In a production environment with the full Satisfactory game installation, these would contain actual localized game text. For the mod loader development environment, these minimal files prevent unnecessary warning messages during startup.

## Note on Engine Localization Files

Some localization warnings may still appear for Engine-level localization files (e.g., `Engine.locres`, `OnlineSubsystem.locres`). These are part of the Unreal Engine installation and are not included in this repository. These warnings are informational only and do not affect functionality.

## Regenerating Localization Files

If you need to regenerate these files with actual localized content, use Unreal Engine's localization dashboard:

1. Open the project in Unreal Editor
2. Go to Window > Localization Dashboard
3. Configure and compile the localization targets

For more information, see the [Unreal Engine Localization Documentation](https://docs.unrealengine.com/5.3/en-US/localizing-projects-in-unreal-engine/).
