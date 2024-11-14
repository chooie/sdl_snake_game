# Check if the build directory exists
if [ ! -d "build" ]; then
  echo "Creating build directory and copying .dylib files..."
  mkdir -p build

  # Copy .dylib files from vendor to build directory
  cp -P vendor/SDL2/macos/lib/*.dylib build/
fi

g++ -g -Wno-switch -I vendor/SDL2/macos/include -L vendor/SDL2/macos/lib -o build/sdl_starter main.cpp -lSDL2 -lSDL2_ttf

# SDL2
install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2.dylib build/sdl_starter

# SDL2_ttf
install_name_tool -change /usr/local/opt/sdl2_ttf/lib/libSDL2_ttf-2.0.0.dylib @executable_path/libSDL2_ttf.dylib build/sdl_starter