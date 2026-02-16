#!/bin/bash
# Shell script to apply Wwise patches on Linux/Mac

# Get the directory containing this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PATCHES_FOLDER="$SCRIPT_DIR/Patches"
THIRDPARTY_DIR="$SCRIPT_DIR/ThirdParty"

# Ensure the patches folder exists
if [ ! -d "$PATCHES_FOLDER" ]; then
    echo "The specified patches folder does not exist: $PATCHES_FOLDER"
    exit 1
fi

# Get the project dir passed as argument
if [ $# -lt 1 ]; then
    echo "Usage: applyPatches.sh <path>"
    exit 1
fi
PROJECT_DIR="$1"

if [ ! -d "$PROJECT_DIR" ]; then
    echo "The specified project dir does not exist: $PROJECT_DIR"
    exit 1
fi

# Check if patch command is available
if ! command -v patch &> /dev/null; then
    echo "Error: 'patch' command not found. Please install it first."
    echo "  Ubuntu/Debian: sudo apt-get install patch"
    echo "  macOS: patch should be pre-installed"
    exit 1
fi

# Check patches that have already been applied
APPLIED_PATCHES_FILE="$PROJECT_DIR/Plugins/Wwise/applied_patches.txt"
declare -a APPLIED_PATCHES
if [ -f "$APPLIED_PATCHES_FILE" ]; then
    mapfile -t APPLIED_PATCHES < "$APPLIED_PATCHES_FILE"
fi

# Enumerate all .patch files in the patches folder, sorted alphabetically
mapfile -t PATCH_FILES < <(find "$PATCHES_FOLDER" -maxdepth 1 -name "*.patch" -type f | sort)

# Check if there are any .patch files in the folder
if [ ${#PATCH_FILES[@]} -eq 0 ]; then
    echo "No .patch files found in the folder: $PATCHES_FOLDER"
    exit 0
fi

# Iterate through the patch files
for PATCH_FILE in "${PATCH_FILES[@]}"; do
    PATCH_FILENAME="$(basename "$PATCH_FILE")"
    
    # Skip if the patch has already been applied
    ALREADY_APPLIED=false
    for APPLIED in "${APPLIED_PATCHES[@]}"; do
        if [ "$APPLIED" = "$PATCH_FILENAME" ]; then
            ALREADY_APPLIED=true
            break
        fi
    done
    
    if [ "$ALREADY_APPLIED" = true ]; then
        echo "Patch '$PATCH_FILENAME' has already been applied. Skipping."
        continue
    fi
    
    # Verify the patch can be applied with --dry-run before applying it
    echo "Checking if patch can be applied: patch -N -s -p1 -d \"$PROJECT_DIR\" --dry-run < \"$PATCH_FILE\""
    if ! patch -N -s -p1 -d "$PROJECT_DIR" --dry-run < "$PATCH_FILE" > /dev/null 2>&1; then
        echo "Error: Dry run failed for patch '$PATCH_FILENAME'. Exiting."
        exit 1
    fi
    
    # Apply the patch if dry run is successful
    echo "Applying patch: patch -N -s -p1 -d \"$PROJECT_DIR\" < \"$PATCH_FILE\""
    if ! patch -N -s -p1 -d "$PROJECT_DIR" < "$PATCH_FILE"; then
        echo "Error: Failed to apply patch '$PATCH_FILENAME'. Exiting."
        exit 1
    fi
    
    # After applying the patch successfully, log it
    echo "$PATCH_FILENAME" >> "$APPLIED_PATCHES_FILE"
    echo "Patch '$PATCH_FILENAME' applied successfully."
done

echo "All patches applied successfully."
