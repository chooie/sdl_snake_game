# Turn off Mac OS verbose logging
export NSUnbufferedIO=NO

# Check if the build directory exists
if [ ! -d "build" ]; then
  echo "Creating build directory and copying .dylib files..."
  mkdir -p build

  # Copy .dylib files from vendor to build directory
  cp -P vendor/SDL2/macos/lib/*.dylib build/
fi

g++ -g -Wno-switch -I vendor/SDL2/macos/include -L vendor/SDL2/macos/lib -o build/sdl_snake_game src/main.cpp -lSDL2 -lSDL2_ttf -lSDL2_mixer

# SDL2
install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib @executable_path/libSDL2.dylib build/sdl_snake_game

# SDL2_ttf
install_name_tool -change /usr/local/opt/sdl2_ttf/lib/libSDL2_ttf-2.0.0.dylib @executable_path/libSDL2_ttf.dylib build/sdl_snake_game

# SDL2_mixer
install_name_tool -change /usr/local/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib @executable_path/libSDL2_mixer-2.0.0.dylib build/sdl_snake_game

# Update paths for SDL2_mixer dependencies
echo "Patching SDL2_mixer dependencies..."
# libgme
install_name_tool -change /usr/local/opt/game-music-emu/lib/libgme.0.dylib @executable_path/libgme.0.dylib build/libSDL2_mixer-2.0.0.dylib
# libxmp
install_name_tool -change /usr/local/opt/libxmp/lib/libxmp.4.dylib @executable_path/libxmp.4.dylib build/libSDL2_mixer-2.0.0.dylib
# libfluidsynth
install_name_tool -change /usr/local/opt/fluid-synth/lib/libfluidsynth.3.dylib @executable_path/libfluidsynth.3.dylib build/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /usr/local/opt/libsndfile/lib/libsndfile.1.dylib @executable_path/libsndfile.1.dylib build/libfluidsynth.3.dylib
install_name_tool -change /usr/local/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib build/libsndfile.1.dylib
install_name_tool -change /usr/local/opt/portaudio/lib/libportaudio.2.dylib @executable_path/libportaudio.2.dylib build/libfluidsynth.3.dylib
install_name_tool -change /usr/local/opt/opus/lib/libopus.0.dylib @executable_path/libopus.0.dylib build/libsndfile.1.dylib
install_name_tool -change /usr/local/opt/lame/lib/libmp3lame.0.dylib @executable_path/libmp3lame.0.dylib build/libsndfile.1.dylib

# libvorbisfile
install_name_tool -change /usr/local/opt/libvorbis/lib/libvorbis.0.dylib @executable_path/libvorbis.0.dylib build/libvorbisfile.3.dylib
install_name_tool -change /usr/local/opt/libvorbis/lib/libvorbisenc.2.dylib @executable_path/libvorbisenc.2.dylib build/libvorbisenc.2.dylib
install_name_tool -change /usr/local/opt/libvorbis/lib/libvorbisfile.3.dylib @executable_path/libvorbisfile.3.dylib build/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /usr/local/Cellar/libvorbis/1.3.7/lib/libvorbis.0.dylib @executable_path/libvorbis.0.dylib build/libvorbisfile.3.dylib
install_name_tool -change /usr/local/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib build/libvorbisfile.3.dylib
install_name_tool -change /usr/local/opt/libvorbis/lib/libvorbisenc.2.dylib @executable_path/libvorbisenc.2.dylib build/libsndfile.1.dylib
install_name_tool -change /usr/local/opt/libvorbis/lib/libvorbis.0.dylib @executable_path/libvorbis.0.dylib build/libsndfile.1.dylib
install_name_tool -change /usr/local/Cellar/libvorbis/1.3.7/lib/libvorbis.0.dylib @executable_path/libvorbis.0.dylib build/libvorbisenc.2.dylib
# libFLAC
install_name_tool -change /usr/local/opt/flac/lib/libFLAC.12.dylib @executable_path/libFLAC.12.dylib build/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /usr/local/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib build/libFLAC.12.dylib
install_name_tool -change /usr/local/opt/libogg/lib/libogg.0.dylib @executable_path/libogg.0.dylib build/libopusfile.0.dylib
# libmpg123
install_name_tool -change /usr/local/opt/mpg123/lib/libmpg123.0.dylib @executable_path/libmpg123.0.dylib build/libSDL2_mixer-2.0.0.dylib
# libopusfile
install_name_tool -change /usr/local/opt/opusfile/lib/libopusfile.0.dylib @executable_path/libopusfile.0.dylib build/libSDL2_mixer-2.0.0.dylib
install_name_tool -change /usr/local/opt/opus/lib/libopus.0.dylib @executable_path/libopus.0.dylib build/libopusfile.0.dylib
# libwavpack
install_name_tool -change /usr/local/opt/wavpack/lib/libwavpack.1.dylib @executable_path/libwavpack.1.dylib build/libSDL2_mixer-2.0.0.dylib