#!/bin/bash
set -e

SDK_URL="https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip"
SDK_DIR="$HOME/android-sdk"
TOOLS_DIR="$SDK_DIR/cmdline-tools/latest"

mkdir -p "$SDK_DIR"
cd "$SDK_DIR"
echo "Downloading Android SDK Command-line Tools..."
curl -o cmdline-tools.zip "$SDK_URL"

unzip -o cmdline-tools.zip -d cmdline-tools
mkdir -p "$TOOLS_DIR"
mv cmdline-tools/cmdline-tools/* "$TOOLS_DIR/"
rm -rf cmdline-tools.zip cmdline-tools/cmdline-tools

export ANDROID_HOME="$SDK_DIR"
export PATH="$TOOLS_DIR/bin:$PATH"

yes | sdkmanager --licenses
sdkmanager \
  "ndk;28.0.12674087" \
  "build-tools;35.0.0" \
  "platforms;android-35" \
  "platform-tools" \
  "cmdline-tools;latest"

echo "Android SDK setup complete."

#export ANDROID_NDK="$HOME/android-sdk/ndk/28.0.12674087"
#export ANDROID_HOME="$HOME/android-sdk"
#export NG_CMAKE_TOOLCHAIN_FILE="$HOME/android-sdk/ndk/28.0.12674087/build/cmake/android.toolchain.cmake"

