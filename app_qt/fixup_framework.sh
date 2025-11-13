#!/bin/sh

FRAMEWORK_ROOT=$(basename $1)
CURRENT_VERSION=$(ls "${FRAMEWORK_ROOT}/Versions")
LIBRARY=$(ls "${FRAMEWORK_ROOT}/Versions/${CURRENT_VERSION}")
mkdir -p "${FRAMEWORK_ROOT}/Versions/${CURRENT_VERSION}/Resources/"
ln -sfn "${CURRENT_VERSION}" "${FRAMEWORK_ROOT}/Versions/Current"
ln -sfn "Versions/Current/${LIBRARY}" "${FRAMEWORK_ROOT}/${LIBRARY}"
ln -sfn "Versions/Current/Resources" "${FRAMEWORK_ROOT}/Resources"
cp "${QT5_BIN_PATH}"/../lib/$lib_name.framework/Contents/Info.plist "${FULL_PATH}"/Contents/Frameworks/$lib_name.framework/Resources/
PLIST="${FRAMEWORK_ROOT}/Resources/Info.plist"
/usr/libexec/PlistBuddy -c "Set :CFBundleExecutable $lib_name" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :BuildMachineOSBuild string 13E28" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :CFBundleIdentifier string net.kishonti.osx.GFXBench" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :CFBundleName string $lib_name" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :CFBundleDevelopmentRegion string English" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :CFBundleInfoDictionaryVersion string 6.0" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :CFBundleVersion string 5.3" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :DTCompiler string com.apple.compilers.llvm.clang.1_0" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :DTPlatformBuild string 5B1008" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :DTPlatformVersion string GM" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :DTSDKBuild string 12F37" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :DTSDKName string macosx10.8" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :DTXcode string 0511" "$PLIST"
/usr/libexec/PlistBuddy -c "Add :DTXcodeBuild string 5B1008" "$PLIST"