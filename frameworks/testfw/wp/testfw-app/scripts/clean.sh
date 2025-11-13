#!/bin/bash -e

PROJECT_DIR="${PWD}/$(dirname $0)"
PROJECT_DIR="$PROJECT_DIR/../"
echo "PROJECT_DIR $PROJECT_DIR"

echo "clean WindowsPhoneApp"
# GFXBenchAutoTest.Windows
cmake -E remove_directory "$PROJECT_DIR/WindowsPhoneApp/AppPackages"
cmake -E remove_directory "$PROJECT_DIR/WindowsPhoneApp/arm"
cmake -E remove_directory "$PROJECT_DIR/WindowsPhoneApp/bin"
cmake -E remove_directory "$PROJECT_DIR/WindowsPhoneApp/Generated Files"
cmake -E remove_directory "$PROJECT_DIR/WindowsPhoneApp/obj"
