#!/bin/bash

# Get the script directory (WwisePatches folder)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PATCHES_FOLDER="$SCRIPT_DIR/Patches"

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
    exit 1
fi

# Check patches that have already been applied
APPLIED_PATCHES_FILE="$PROJECT_DIR/Plugins/Wwise/applied_patches.txt"
APPLIED_PATCHES=()
if [ -f "$APPLIED_PATCHES_FILE" ]; then
    while IFS= read -r line; do
        APPLIED_PATCHES+=("$line")
    done < "$APPLIED_PATCHES_FILE"
fi

# Create directory for applied patches file if it doesn't exist
mkdir -p "$(dirname "$APPLIED_PATCHES_FILE")"

# Enumerate all .patch files in the patches folder, sorted alphabetically
shopt -s nullglob
PATCH_FILES=("$PATCHES_FOLDER"/*.patch)
shopt -u nullglob

# Check if there are any .patch files in the folder
if [ ${#PATCH_FILES[@]} -eq 0 ]; then
    echo "No .patch files found in the folder: $PATCHES_FOLDER"
    exit 0
fi

# Sort patch files
IFS=$'\n' PATCH_FILES=($(sort <<<"${PATCH_FILES[*]}"))
unset IFS

# Iterate through the patch files
for PATCH_FILE in "${PATCH_FILES[@]}"; do
    PATCH_FILENAME=$(basename "$PATCH_FILE")
    
    # Skip if the patch has already been applied
    if printf '%s\n' "${APPLIED_PATCHES[@]}" | grep -qx "$PATCH_FILENAME"; then
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

exit 0
