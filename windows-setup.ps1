# setup vcpkg

# Check if Git is installed
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Error "Git is not installed or not in PATH. Install Git and try again"
    exit 1
}

# Define where to clone vcpkg
$VCPKG_DIR = "$PSScriptRoot\vcpkg"

# Clone vcpkg repo if it doesn't exist
if (-Not (Test-Path $VCPKG_DIR)) {
    Write-Host "Cloning vcpkg into $VCPKG_DIR..."
    git clone https://github.com/microsoft/vcpkg.git $VCPKG_DIR
}

# Bootstrap vcpkg (build vcpkg executable)
Write-Host "Bootstrapping vcpkg..."
Push-Location $VCPKG_DIR
try {
    .\bootstrap-vcpkg.bat
} catch {
    Write-Error "Failed to bootstrap vcpkg: $_"
    exit 1
}
Pop-Location

# Add vcpkg to PATH for current session
$env:PATH = "$VCPKG_DIR;$env:PATH"

Write-Host = "Added vcpkg to PATH for this session."

# Install dependecies
try {
    vcpkg install
} catch {
    Write-Error "Failed to install dependencies: $_"
    exit 1
}

# Generate build files
$TOOLCHAIN_FILE = "$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "CMake is not installed or not in PATH. Install CMake and try again"
    exit 1
}

Write-Host "Setting up cmake..."
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE"

Write-Host "Setup complete. Run 'cmake --build build' to compile."
exit 0