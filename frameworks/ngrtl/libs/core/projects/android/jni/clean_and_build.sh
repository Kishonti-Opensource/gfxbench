../../../../../ngrtl/libs/core/projects/android/jni
rm -rf ../obj
rm -rf ../libs
ndk-build clean
ndk-build NDK_LOG=1 
