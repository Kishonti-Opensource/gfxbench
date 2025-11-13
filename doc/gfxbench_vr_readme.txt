# GFXBench VR

## Table of Contents

1. Build steps for Android
2. About benchmarking mode

## 1. Build steps for Android

To be able to use VR with developer app must be built with your device unique signature. You can find the details here:  
https://developer.oculus.com/osig

After you get the signature file you have to place it under folder assets before you start any build script:  
`frameworks/testfw/android/ovr-testfw-app/assets/`

1. Build project root

		PLATFORM=android-armv7a CONFIG=Release scripts/build-3rdparty.sh
	
	Build params for benchmark mode (Oculus SDK not included):
	
		OVR_APP=true OVR_SDK=false OVR_BENCHMARK=true BUNDLE_DATA=false PLATFORM=android-armv7a CONFIG=Release scripts/build.sh
	
	Build params for Oculus Rift:
	
		OVR_APP=true OVR_SDK=true OVR_BENCHMARK=false BUNDLE_DATA=false PLATFORM=android-armv7a CONFIG=Release scripts/build.sh
	
	Build parameters:
	
		OVR_APP=true   Include the OVR files in the build
		OVR_SDK=true   Use the Samsung Gear VR goggles
		OVR_BENCHMARK=true   Use the benchmarking mode

2. Install apk

		adb install -r tfw-pkg/apk/testfw_app-release.apk

3. Push data to device

		adb push gfxbench/resources/gfxbench_vr/5.0/config   /sdcard/kishonti/tfw/config
		adb push gfxbench-data/data/gfx/scene5_ovr            /sdcard/kishonti/tfw/data/gfx/scene5_ovr
		adb push gfxbench/shaders/scene5/shaders_ovr      /sdcard/kishonti/tfw/data/gfx/shaders/scene5/shaders_ovr

4. Start apk on phone

5. Insert phone into gear vr (optional)


## 2. About benchmarking mode

The benchmarking mode goes through the following process:

	Before the measurement there's some warmup to load the shaders and the assets.
	<loop begin>
		Continuously measure and average frames.
		If this average is below a threshold, disable the least important (affect to the visual quality) and the most expensive effect.
		This will happen until the device reaches 58+ FPS or we run out of the effects.
	<loop end>
	The score is the number of the effects still on with 58+ FPS.
