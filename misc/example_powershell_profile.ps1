# I have this saved at C:\Users\{username}\OneDrive\Documents\WindowsPowerShell

function rl {
    . $profile
    Write-Output "Reloaded PowerShell Profile"
}

function sdl {
    Set-Location W:\sdl_starter
}

function rb {
    # Try to stop any running instances of "main.exe" without showing errors if it's not found
    Stop-Process -Name "main" -Force -ErrorAction SilentlyContinue
    make
    run
}

function run {
    & "W:\sdl_starter\main.exe"
}

function b {
    make
}

function hm {
    Set-Location W:\handmade
}

function gs {
    git add .
    git commit -m "Save"
}