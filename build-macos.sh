mkdir -p build
g++ -I vendor/SDL2/macos/include -L vendor/SDL2/macos/lib -o build/sdl_starter main.cpp -lSDL2
install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib @executable_path/../vendor/SDL2/macos/lib/libSDL2.dylib build/sdl_starter