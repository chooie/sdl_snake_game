# Check if the build directory exists
if [ ! -d "build" ]; then
  echo "Creating build directory and copying .dylib files..."
  mkdir -p build

  # Copy .dylib files from vendor to build directory
  cp vendor/SDL2/macos/lib/*.dylib build/
fi

g++ -I vendor/SDL2/macos/include -L vendor/SDL2/macos/lib -o build/sdl_starter main.cpp -lSDL2 -lSDL2_ttf -Wl,-rpath,@executable_path/../vendor/SDL2/macos/lib