#!/bin/bash

# gfx color conversion to compute colors
for i in *.ui QMessageBox.css; do
	echo "$i"
	if [ "$i" = "QMessageBox.css" ]; then
		cp "$i" ../../compubench_cl/forms/"$i"
	else	
		# main color #<gfx>/#<compute>
		cat "$i" |
		sed -e 's/#54abe7/#a4e268/g' |
		# main color-pressed
		sed -e 's/#4fa0dd/#9ad461/g' |
		# main color-hover
		sed -e 's/#59b7ef/#b0ef73/g' |
		# tab-bar base
		sed -e 's/#342f27/#342f27/g' |
		# tab-bar darker line
		sed -e 's/#4389b9/#83b553/g' |
		# nav-bar
		sed -e 's/#2a5674/#527134/g' |
		# nav-bar-hover
		sed -e 's/#2d5c7c/#577837/g' |
		# nav-bar-pressed
		sed -e 's/#27516d/#4d6a31/g' |
		# group header text, start all button text, compare your device text
		sed -e 's/#28475c/#3d5624/g' |
		# list highlight, compare device bar
		sed -e 's/#d4eafc/#f5fcee/g' |
		# compare your device bar bg
		sed -e 's/#83c2ee/#bbe98e/g' |
		# tablet compare highlighted device bar bg
		sed -e 's/#d4eaf9/#e8f8d9/g' |
		# tablet compare highlighted device bar
		sed -e 's/#acd5f1/#d4f1b8/g' > ../../compubench_cl/forms/"$i"
	fi
done   