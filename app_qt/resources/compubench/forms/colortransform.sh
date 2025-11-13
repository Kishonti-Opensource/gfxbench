#!/bin/bash

# compute color conversion to gfx colors
for i in *.ui QMessageBox.css; do
	echo "$i"
	if [ "$i" = "QMessageBox.css" ]; then
		cp "$i" ../../gfx/forms/"$i"
	else
		# main color #<compute>/#<gfx>
		cat "$i" |
		sed -e 's/#a4e268/#54abe7/g' |
		# main color-pressed
		sed -e 's/#9ad461/#4fa0dd/g' |
		# main color-hover
		sed -e 's/#b0ef73/#59b7ef/g' |
		# tab-bar base
		sed -e 's/#342f27/#342f27/g' |
		# tab-bar darker line
		sed -e 's/#83b553/#4389b9/g' |
		# nav-bar
		sed -e 's/#527134/#2a5674/g' |
		# nav-bar-hover
		sed -e 's/#577837/#2d5c7c/g' |
		# nav-bar-pressed
		sed -e 's/#4d6a31/#27516d/g' |
		# group header text, start all button text, compare your device text
		sed -e 's/#3d5624/#28475c/g' |
		# list highlight, compare device bar
		sed -e 's/#f5fcee/#ecf5fc/g' |
		# compare your device bar bg
		sed -e 's/#bbe98e/#83c2ee/g' |
		# tablet compare highlighted device bar bg
		sed -e 's/#e8f8d9/#d4eaf9/g' |
		# tablet compare highlighted device bar
		sed -e 's/#d4f1b8/#acd5f1/g' > ../../gfx/forms/"$i"
	fi
done   