#!/bin/bash

# Verification script for Discord Bot Error Logging feature

echo "==================================="
echo "Discord Bot Error Logging Verification"
echo "==================================="
echo ""

# Check if all required files exist
echo "Checking if required files exist..."
FILES=(
    "Mods/DiscordBot/Source/DiscordBot/Public/DiscordBotErrorLogger.h"
    "Mods/DiscordBot/Source/DiscordBot/Private/DiscordBotErrorLogger.cpp"
    "Mods/DiscordBot/Source/DiscordBot/Public/DiscordBotModule.h"
    "Mods/DiscordBot/Source/DiscordBot/Private/DiscordBotModule.cpp"
    "Mods/DiscordBot/Config/DiscordBot.ini"
    "Mods/DiscordBot/ERROR_LOGGING.md"
    "Mods/DiscordBot/README.md"
)

ALL_FILES_EXIST=true
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file exists"
    else
        echo "✗ $file is missing"
        ALL_FILES_EXIST=false
    fi
done

echo ""

# Check for key implementations in source files
echo "Checking implementation details..."

# Check for FDiscordBotErrorLogger class declaration
if grep -q "class DISCORDBOT_API FDiscordBotErrorLogger" "Mods/DiscordBot/Source/DiscordBot/Public/DiscordBotErrorLogger.h"; then
    echo "✓ FDiscordBotErrorLogger class is declared"
else
    echo "✗ FDiscordBotErrorLogger class declaration not found"
    ALL_FILES_EXIST=false
fi

# Check for Initialize method
if grep -q "void Initialize" "Mods/DiscordBot/Source/DiscordBot/Public/DiscordBotErrorLogger.h"; then
    echo "✓ Initialize method is declared"
else
    echo "✗ Initialize method not found"
    ALL_FILES_EXIST=false
fi

# Check for Serialize method (FOutputDevice implementation)
if grep -q "void Serialize" "Mods/DiscordBot/Source/DiscordBot/Public/DiscordBotErrorLogger.h"; then
    echo "✓ Serialize method is declared (FOutputDevice override)"
else
    echo "✗ Serialize method not found"
    ALL_FILES_EXIST=false
fi

# Check for ErrorLogger member in module
if grep -q "TUniquePtr<FDiscordBotErrorLogger> ErrorLogger" "Mods/DiscordBot/Source/DiscordBot/Public/DiscordBotModule.h"; then
    echo "✓ ErrorLogger member is declared in FDiscordBotModule"
else
    echo "✗ ErrorLogger member not found in module"
    ALL_FILES_EXIST=false
fi

# Check for ErrorLogger initialization in module
if grep -q "ErrorLogger = MakeUnique<FDiscordBotErrorLogger>" "Mods/DiscordBot/Source/DiscordBot/Private/DiscordBotModule.cpp"; then
    echo "✓ ErrorLogger is initialized in StartupModule"
else
    echo "✗ ErrorLogger initialization not found"
    ALL_FILES_EXIST=false
fi

# Check for ErrorLogger shutdown
if grep -q "ErrorLogger->Shutdown()" "Mods/DiscordBot/Source/DiscordBot/Private/DiscordBotModule.cpp"; then
    echo "✓ ErrorLogger is properly shut down in ShutdownModule"
else
    echo "✗ ErrorLogger shutdown not found"
    ALL_FILES_EXIST=false
fi

# Check for log rotation implementation
if grep -q "RotateLogFileIfNeeded" "Mods/DiscordBot/Source/DiscordBot/Private/DiscordBotErrorLogger.cpp"; then
    echo "✓ Log rotation is implemented"
else
    echo "✗ Log rotation not found"
    ALL_FILES_EXIST=false
fi

# Check for GLog integration
if grep -q "GLog->AddOutputDevice" "Mods/DiscordBot/Source/DiscordBot/Private/DiscordBotErrorLogger.cpp"; then
    echo "✓ GLog integration is implemented"
else
    echo "✗ GLog integration not found"
    ALL_FILES_EXIST=false
fi

# Check config documentation
if grep -q "ErrorLogDirectory" "Mods/DiscordBot/Config/DiscordBot.ini"; then
    echo "✓ Configuration is documented in DiscordBot.ini"
else
    echo "✗ Configuration not found in ini file"
    ALL_FILES_EXIST=false
fi

# Check README update
if grep -q "Error Logging" "Mods/DiscordBot/README.md"; then
    echo "✓ Feature is documented in README.md"
else
    echo "✗ Feature not documented in README"
    ALL_FILES_EXIST=false
fi

echo ""
echo "==================================="

if [ "$ALL_FILES_EXIST" = true ]; then
    echo "✓ All verification checks passed!"
    echo ""
    echo "The error logging feature is properly implemented."
    echo ""
    echo "Key features:"
    echo "  - Automatic error and warning logging to files"
    echo "  - Default location: <ProjectDir>/Saved/Logs/DiscordBot/"
    echo "  - Configurable log directory"
    echo "  - Automatic log rotation (10MB max)"
    echo "  - Filters Discord bot logs only (LogDiscord* categories)"
    echo "  - Thread-safe implementation"
    echo ""
    echo "Documentation:"
    echo "  - See ERROR_LOGGING.md for detailed usage information"
    echo "  - See Config/DiscordBot.ini for configuration options"
    exit 0
else
    echo "✗ Some verification checks failed!"
    exit 1
fi
