#!/bin/bash -x

# see docs: https://developer.apple.com/library/iOs/qa/qa1686/_index.html

IOS7_UNIVERSAL="120x120/60@2x~iphone 76x76/76~ipad 152x152/76@2x~ipad 40x40/Small-40 80x80/Small-40@2x 29x29/Small 58x58/Small@2x"
IOS7_LAUNCH_SIZES="
	320x480/~iphone/iPhone_iPod_touch
	640x960/@2x~iphone/iPhone_and_iPod_touch_Retina
	640x1136/-568h@2x~iphone/iPhone_5_and_iPod_touch_5th_generation
	1024x768/-Landscape~ipad/iPad
	2048x1536/-Landscape@2x~ipad/iPad_Retina"
BGCOLOR="lightblue"
FGCOLOR="black"

for rec in ${IOS7_UNIVERSAL}
do
  IFS=/ read DIM POSTFIX <<< "${rec}"
  convert -background ${BGCOLOR} -fill ${FGCOLOR} -font Arial -size ${DIM} -gravity center label:"${DIM}\n${POSTFIX}" Icon-${POSTFIX}.png
done

for rec in ${IOS7_LAUNCH_SIZES}
do
  echo "rec \"$rec\""
  IFS=/ read DIM MODIFIER LABEL <<< "${rec}"
  convert -background ${BGCOLOR} -fill ${FGCOLOR} -font Arial -size ${DIM} -gravity center label:"${DIM}\n${MODIFIER}\n${LABEL}" Default${MODIFIER}.png
done
