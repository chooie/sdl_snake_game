# I have this saved at C:\Users\{username}\OneDrive\Documents\WindowsPowerShell

function rl {
    . $profile
    Write-Output "Reloaded PowerShell Profile"
}

function sdl {
    Set-Location W:\sdl_snake_game
}

function rb {
    # Try to stop any running instances of "main.exe" without showing errors if it's not found
    Stop-Process -Name "main" -Force -ErrorAction SilentlyContinue
    b
    if ($LASTEXITCODE -eq 0) {
        # Only run if build was successful
        run
    }
}

function run {
    Start-Process "W:\sdl_snake_game\build\main.exe"
}

function b {
    W:\sdl_snake_game\build-windows.bat
}

function hm {
    Set-Location W:\handmade
}

function gs {
    git add .
    git commit -m "Save"
}

function cim {
    param (
        [string]$message
    )

    if (-not $message) {
        Write-Output "Please provide a commit message."
        return
    }

    git add .
    git commit -m "$message"
}