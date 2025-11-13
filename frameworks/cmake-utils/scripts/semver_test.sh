#!/usr/bin/env bash

. ./semver.sh

function doTest() {
    local TEST=$1
    local EXPECTED=$2
    local ACTUAL=$3

    echo -n "$TEST: "

    if [[ "$EXPECTED" == "$ACTUAL" ]]; then
        echo "passed"
    else
        echo "FAILED, expected '${EXPECTED}', actual: '${ACTUAL}'"
    fi
}

semverTest() {
local A=R1.3.2
local B=R2.3.2
local C=R1.4.2
local D=R1.3.3
local E=R1.3.2a
local F=R1.3.2b
local G=R1.2.3
local H=1.2.3-a
local I=1.2.3-alpha
local J=1.2.3-Gold3-vulkan_update
local K=1.2.3-alpha+build1
local L=1.2.3-alpha.2+build2
local M=1.2.3+build3
local N=1.2.3+build4.sha.5114f85
local O=1.2.3-Gold3-vulkan_update+build5.2014-08-29+corporate

local MAJOR=0
local MINOR=0
local PATCH=0
local SPECIAL=""

local VERSION=""

local PRERELEASE=""
local BUILD=""

semverParseInto $O MAJOR MINOR PATCH PRERELEASE BUILD
echo $MAJOR
echo $MINOR
echo $PATCH
echo $PRERELEASE
echo $BUILD

echo "Parsing"
semverParseInto $A MAJOR MINOR PATCH SPECIAL
doTest "semverParseInto $A -> M m p s" "M:1 m:3 p:2 s:" "M:$MAJOR m:$MINOR p:$PATCH s:$SPECIAL"

semverParseInto $B MAJOR MINOR PATCH SPECIAL
doTest "semverParseInto $B -> M m p s" "M:2 m:3 p:2 s:" "M:$MAJOR m:$MINOR p:$PATCH s:$SPECIAL"

semverParseInto $E MAJOR MINOR PATCH SPECIAL
doTest "semverParseInto $E -> M m p s" "M:1 m:3 p:2 s:a" "M:$MAJOR m:$MINOR p:$PATCH s:$SPECIAL"

semverParseInto $H MAJOR MINOR PATCH SPECIAL
doTest "semverParseInto $H -> M m p s" "M:1 m:2 p:3 s:a" "M:$MAJOR m:$MINOR p:$PATCH s:$SPECIAL"

semverParseInto $I MAJOR MINOR PATCH PRERELEASE BUILD
doTest "semverParseInto $I -> M m p pr b" "M:1 m:2 p:3 pr:alpha b:" "M:$MAJOR m:$MINOR p:$PATCH pr:$PRERELEASE b:$BUILD"

semverParseInto $J MAJOR MINOR PATCH PRERELEASE BUILD
doTest "semverParseInto $J -> M m p pr b" "M:1 m:2 p:3 pr:Gold3-vulkan_update b:" "M:$MAJOR m:$MINOR p:$PATCH pr:$PRERELEASE b:$BUILD"

semverParseInto $K MAJOR MINOR PATCH PRERELEASE BUILD
doTest "semverParseInto $K -> M m p pr b" "M:1 m:2 p:3 pr:alpha b:build1" "M:$MAJOR m:$MINOR p:$PATCH pr:$PRERELEASE b:$BUILD"

semverParseInto $L MAJOR MINOR PATCH PRERELEASE BUILD
doTest "semverParseInto $L -> M m p pr b" "M:1 m:2 p:3 pr:alpha.2 b:build2" "M:$MAJOR m:$MINOR p:$PATCH pr:$PRERELEASE b:$BUILD"

semverParseInto $M MAJOR MINOR PATCH PRERELEASE BUILD
doTest "semverParseInto $M -> M m p pr b" "M:1 m:2 p:3 pr: b:build3" "M:$MAJOR m:$MINOR p:$PATCH pr:$PRERELEASE b:$BUILD"

semverParseInto $N MAJOR MINOR PATCH PRERELEASE BUILD
doTest "semverParseInto $N -> M m p pr b" "M:1 m:2 p:3 pr: b:build4.sha.5114f85" "M:$MAJOR m:$MINOR p:$PATCH pr:$PRERELEASE b:$BUILD"

semverParseInto $O MAJOR MINOR PATCH PRERELEASE BUILD
doTest "semverParseInto $O -> M m p pr b" "M:1 m:2 p:3 pr:Gold3-vulkan_update b:build5.2014-08-29+corporate" "M:$MAJOR m:$MINOR p:$PATCH pr:$PRERELEASE b:$BUILD"

echo "Comparisons"
semverCmp $A $A
doTest "semverCmp $A $A" 0 $?

semverCmp $A $B
doTest "semverCmp $A $B" 255 $?

semverCmp $B $A
doTest "semverCmp $B $A" 1 $?


echo "Equality comparisons"
semverEQ $A $A
doTest "semverEQ $A $A" 0 $?

semverLT $A $A
doTest "semverLT $A $A" 1 $?

semverGT $A $A
doTest "semverGT $A $A" 1 $?


echo "Major number comparisons"
semverEQ $A $B
doTest "semverEQ $A $B" 1 $?

semverLT $A $B
doTest "semverLT $A $B" 0 $?

semverGT $A $B
doTest "semverGT $A $B" 1 $?

semverEQ $B $A
doTest "semverEQ $B $A" 1 $?

semverLT $B $A
doTest "semverLT $B $A" 1 $?

semverGT $B $A
doTest "semverGT $B $A" 0 $?


echo "Minor number comparisons"
semverEQ $A $C
doTest "semverEQ $A $C" 1 $?

semverLT $A $C
doTest "semverLT $A $C" 0 $?

semverGT $A $C
doTest "semverGT $A $C" 1 $?

semverEQ $C $A
doTest "semverEQ $C $A" 1 $?

semverLT $C $A
doTest "semverLT $C $A" 1 $?

semverGT $C $A
doTest "semverGT $C $A" 0 $?


echo "Patch number comparisons"
semverEQ $A $D
doTest "semverEQ $A $D" 1 $?

semverLT $A $D
doTest "semverLT $A $D" 0 $?

semverGT $A $D
doTest "semverGT $A $D" 1 $?

semverEQ $D $A
doTest "semverEQ $D $A" 1 $?

semverLT $D $A
doTest "semverLT $D $A" 1 $?

semverGT $D $A
doTest "semverGT $D $A" 0 $?


echo "Special section vs no special comparisons"
semverEQ $A $E
doTest "semverEQ $A $E" 1 $?

semverLT $A $E
doTest "semverLT $A $E" 1 $?

semverGT $A $E
doTest "semverGT $A $E" 0 $?

semverEQ $E $A
doTest "semverEQ $E $A" 1 $?

semverLT $E $A
doTest "semverLT $E $A" 0 $?

semverGT $E $A
doTest "semverGT $E $A" 1 $?


echo "Special section vs special comparisons"
semverEQ $E $F
doTest "semverEQ $E $F" 1 $?

semverLT $E $F
doTest "semverLT $E $F" 0 $?

semverGT $E $F
doTest "semverLT $E $F" 1 $?

semverEQ $F $E
doTest "semverEQ $F $E" 1 $?

semverLT $F $E
doTest "semverLT $F $E" 1 $?

semverGT $F $E
doTest "semverGT $F $E" 0 $?


echo "Minor and patch number comparisons"
semverEQ $A $G
doTest "semverEQ $A $G" 1 $?

semverLT $A $G
doTest "semverLT $A $G" 1 $?

semverGT $A $G
doTest "semverGT $A $G" 0 $?

semverEQ $G $A
doTest "semverEQ $G $A" 1 $?

semverLT $G $A
doTest "semverLT $G $A" 0 $?

semverGT $G $A
doTest "semverGT $G $A" 1 $?


echo "Bumping major"
semverBumpMajor $A VERSION
doTest "semverBumpMajor $A" "2.0.0" $VERSION

semverBumpMajor $E VERSION
doTest "semverBumpMajor $E" "2.0.0" $VERSION


echo "Bumping minor"
semverBumpMinor $A VERSION
doTest "semverBumpMinor $A" "1.4.0" $VERSION

semverBumpMinor $E VERSION
doTest "semverBumpMinor $E" "1.4.0" $VERSION


echo "Bumping patch"
semverBumpPatch $A VERSION
doTest "semverBumpPatch $A" "1.3.3" $VERSION

semverBumpPatch $E VERSION
doTest "semverBumpPatch $E" "1.3.3" $VERSION


echo "Strip special"
semverStripSpecial $A VERSION
doTest "semverStripSpecial $A" "1.3.2" $VERSION

semverStripSpecial $E VERSION
doTest "semverStripSpecial $E" "1.3.2" $VERSION
}

semverTest
