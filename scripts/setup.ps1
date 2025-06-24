if (-not (Test-Path "vcpkg"))
{
    git clone https://github.com/Microsoft/vcpkg.git
}

./vcpkg/bootstrap-vcpkg.bat

./vcpkg/vcpkg install sdl2:x64-mingw-static
./vcpkg/vcpkg install readline:x64-mingw-static

Write-Host "Setup complete. SDL2 and readline are installed for MinGW."
