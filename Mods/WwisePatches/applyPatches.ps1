# Get the folders in the WwisePatches dir (same dir as this script)
$patchesFolder = Join-Path -Path (Split-Path -Parent $MyInvocation.MyCommand.Path) -ChildPath "Patches"
$thirdPartyDir = Join-Path -Path (Split-Path -Parent $MyInvocation.MyCommand.Path) -ChildPath "ThirdParty"
$ptchExecutable = Join-Path -Path $thirdPartyDir -ChildPath "ptch.exe"

# Ensure the patches folder exists
if (-Not (Test-Path -Path $patchesFolder)) {
    Write-Error "ERROR: The specified patches folder does not exist: $patchesFolder"
    exit 1
}

# Ensure the ptch.exe exists and can run
if (-Not (Test-Path -Path $ptchExecutable)) {
    Write-Error "ERROR: The ptch.exe executable was not found in: $thirdPartyDir"
    Write-Host "Please ensure the repository is properly cloned with all files."
    exit 1
}
Unblock-File $ptchExecutable

# Get the project dir passed by UE
if ($args.Count -lt 1) {
    Write-Error "ERROR: Missing project directory argument"
    Write-Host "Usage: applyPatches.ps1 <path>"
    exit 1
}
$projectDir = $args[0]

if (-Not (Test-Path -Path $projectDir)) {
    Write-Error "ERROR: The specified project directory does not exist: $projectDir"
    exit 1
}

# Check if Wwise plugin directory exists
$wwisePluginDir = Join-Path -Path $projectDir -ChildPath "Plugins\Wwise"
if (-Not (Test-Path -Path $wwisePluginDir)) {
    Write-Error "ERROR: Wwise plugin directory not found at: $wwisePluginDir"
    Write-Host ""
    Write-Host "The Wwise plugin is required for this project. Please follow these steps:"
    Write-Host "1. Download Wwise plugin using the project's CI process, or"
    Write-Host "2. Manually install the Wwise plugin to: $wwisePluginDir"
    Write-Host "3. For more information, see the project documentation."
    Write-Host ""
    Write-Host "Note: If you're running this in CI, ensure the Wwise download step completed successfully."
    exit 1
}

# Check patches that have already been applied
$appliedPatchesFile = Join-Path -Path $projectDir -ChildPath "Plugins\Wwise\applied_patches.txt"
$appliedPatches = @()
if (Test-Path -Path $appliedPatchesFile) {
    $appliedPatches = Get-Content -Path $appliedPatchesFile
}

# Enumerate all .patch files in the patches folder, sorted alphabetically
$files = Get-ChildItem -Path $patchesFolder -File | Where-Object { $_.Extension -eq ".patch" } | Sort-Object Name

# Check if there are any .patch files in the folder
if ($files.Count -eq 0) {
    Write-Warning "WARNING: No .patch files found in the folder: $patchesFolder"
    Write-Host "Wwise patches are not available. If you need to build with Wwise support,"
    Write-Host "please ensure the Wwise plugin is properly installed."
    exit 0
}

# Iterate through the patch files
foreach ($file in $files) {
    $patchFileName = $file.Name

    # Skip if the patch has already been applied
    if ($appliedPatches -contains $patchFileName) {
        Write-Host "Patch '$patchFileName' has already been applied. Skipping."
        continue
    }
    
    $patchContent = Get-Content -Path $file.FullName -Raw
    # patch expects the patch files to be CRLF
    if ($patchContent -notmatch "`r`n") {
        $patchContent = $patchContent -replace "`n", "`r`n"
    }
    
    # Piping to stdin didn't work, use a temp file instead
    $tempFile = [System.IO.Path]::GetTempFileName()
    $patchContent | Set-Content -Path $tempFile -Encoding UTF8
    
    $arguments = "-N -s -p1 -d `"$projectDir`" -i `"$tempFile`""
    
    # Verify the patch can be applied with --dry-run before applying it
    $dryRunArguments = "$arguments --dry-run"
    Write-Host "Checking if patch can be applied: $ptchExecutable $dryRunArguments"
    $dryRunProcess = Start-Process -FilePath $ptchExecutable -ArgumentList $dryRunArguments -NoNewWindow -Wait -PassThru
    if ($dryRunProcess.ExitCode -ne 0) {
        Write-Error "ERROR: Dry run failed for patch '$patchFileName' with exit code $($dryRunProcess.ExitCode)"
        Write-Host "This usually means:"
        Write-Host "  - The patch has already been applied manually"
        Write-Host "  - The Wwise plugin version doesn't match the expected version"
        Write-Host "  - The Wwise plugin files have been modified"
        Write-Host ""
        Write-Host "Try one of these solutions:"
        Write-Host "  1. Delete the Wwise plugin directory and re-download it"
        Write-Host "  2. Check if the patch was already applied to your Wwise installation"
        exit $dryRunProcess.ExitCode
    }

    # Apply the patch if dry run is successful
    Write-Host "Applying patch: $ptchExecutable $arguments"
    $process = Start-Process -FilePath $ptchExecutable -ArgumentList $arguments -NoNewWindow -Wait -PassThru
    if ($process.ExitCode -ne 0) {
        Write-Error "ERROR: Failed to apply patch '$patchFileName' with exit code $($process.ExitCode)"
        exit $process.ExitCode
    }
    
    # Cleanup temp file
    Remove-Item -Path $tempFile

    # After applying the patch successfully, log it
    Add-Content -Path $appliedPatchesFile -Value $patchFileName
    Write-Host "Patch '$patchFileName' applied successfully."
}
