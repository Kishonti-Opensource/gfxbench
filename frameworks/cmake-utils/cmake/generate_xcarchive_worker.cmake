set(ARCHIVE_ROOT "$ENV{ARCHIVE_ROOT}")
if(ARCHIVE_ROOT)

	message("")
	message("+----------------------------------------------------------+")
	message("| iOS Archiving started                                    |")
	message("+----------------------------------+-----------------------+")
	message("| Used environment variables:      |                        ")
	message("|                                  |                        ")
	message("| archive_name                     | ${ARCHIVE_NAME}")
	message("| product_id                       | $ENV{PRODUCT_ID}")
	message("| product_name                     | $ENV{PRODUCT_NAME}")
	message("| product_version                  | $ENV{PRODUCT_VERSION}")
	message("| distribution_type                | $ENV{DISTRIBUTION_TYPE}")
	message("+----------------------------------+-----------------------+")

	# 
	# Check required variables
	#
	if( ("${ARCHIVE_NAME}" STREQUAL "") OR
		("$ENV{PRODUCT_ID}" STREQUAL "") OR
		("$ENV{PRODUCT_NAME}" STREQUAL "") OR
		("$ENV{DISTRIBUTION_TYPE}" STREQUAL "") OR
		("$ENV{PRODUCT_VERSION}" STREQUAL ""))
		message(FATAL_ERROR "Required parameter(s) missing.....")
	endif()


	set(OPTIONS_METHOD "development")
	set(OPTIONS_UPLOAD_SYMBOLS "true")
	set(TARGET_SCHEME "app_ios")
	set(CODE_SIGNING_ID "iPhone Development: *")
	set(PROVISIONING_PROFILE $ENV{KISHONTI_DEVELOPER_WILDCARD})

	if("$ENV{DISTRIBUTION_TYPE}" STREQUAL "store")
		set(OPTIONS_METHOD "app-store")
		set(OPTIONS_UPLOAD_SYMBOLS "false")
		set(CODE_SIGNING_ID "iPhone Distribution: Kishonti (49X7A8FNR8)")
		if("$ENV{PRODUCT_ID}" STREQUAL "gfxbench_gl")
			set(PROVISIONING_PROFILE $ENV{GFXB27_APPSTORE})
		else()
			set(PROVISIONING_PROFILE $ENV{KISHONTI_APP_STORE_SEPT23})
		endif()

	elseif("$ENV{DISTRIBUTION_TYPE}" STREQUAL "adhoc")
		set(OPTIONS_METHOD "ad-hoc")
		set(OPTIONS_UPLOAD_SYMBOLS "false")
		set(CODE_SIGNING_ID "iPhone Distribution: Kishonti (49X7A8FNR8)")
		set(PROVISIONING_PROFILE $ENV{KISHONTI_ADHOC_SEPT23})

	elseif("$ENV{DISTRIBUTION_TYPE}" STREQUAL "media")
		set(OPTIONS_METHOD "ad-hoc")
		set(OPTIONS_UPLOAD_SYMBOLS "false")
		set(CODE_SIGNING_ID "iPhone Distribution: Kishonti (49X7A8FNR8)")
		set(PROVISIONING_PROFILE $ENV{KISHONTI_ADHOC_SEPT23})
	endif()


	message("| Calculated variables:            |                        ")
	message("|                                  |                        ")
	message("| targetScheme                     | ${TARGET_SCHEME}")
	message("| codeSignIdentity                 | ${CODE_SIGNING_ID}")
	message("| ProvisioningProfile              | ${PROVISIONING_PROFILE}")
	message("+----------------------------------+-----------------------+")
	message("")

	#
	# Clear the archive root before doing anything...
	#	
	execute_process(COMMAND rm -Rf "$ENV{ARCHIVE_ROOT}")

	#
	# Copy the schemes under the created xcodeproj because cmake do not make it
	#        
	# execute_process(COMMAND cp -Rf "$ENV{WORKSPACE}/app_ios/archive_data/xcshareddata" "${CMAKE_BINARY_DIR}/app_ios.xcodeproj/")
	set(SCHEME_PATH "${CMAKE_BINARY_DIR}/app_ios.xcodeproj")
	configure_file("$ENV{WORKSPACE}/app_ios/archive_data/xcshareddata/xcschemes/app_ios.xcscheme" 
		"${CMAKE_BINARY_DIR}/app_ios.xcodeproj/xcshareddata/xcschemes/app_ios.xcscheme")


	file(WRITE $ENV{ARCHIVE_ROOT}/product 
"PRODUCT_ID=\"$ENV{PRODUCT_ID}\"
PRODUCT_NAME=\"$ENV{PRODUCT_NAME}\"
VERSION=\"$ENV{PRODUCT_VERSION}\"")

	message("")
	message("+----------------------------------------------------------+")
	message("| Generating xcarchive                                     |")
	message("+----------------------------------------------------------+")
	message("")

	#
	# Build the xcarchive - this will only be done once, it can be then
	# distributed for Ad Hoc, App Store and Enterprise In House scenarios
	# (profile must be specified by UUID for this step)
	#
	execute_process(COMMAND 
		xcodebuild 
		-scheme "${TARGET_SCHEME}" 
		-archivePath "$ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.xcarchive"
		-sdk iphoneos 
		archive 
		PROVISIONING_PROFILE=${PROVISIONING_PROFILE})

	if(NOT EXISTS "$ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.xcarchive")
		message(FATAL_ERROR "xcarchive is missing after generation")
	endif()

	message("")
	message("+----------------------------------------------------------+")
	message("| xcarchive generated at $ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.xcarchive")
	message("+----------------------------------------------------------+")
	message("| Generating dsym.zip                                      |")
	message("+----------------------------------------------------------+")
	message("")

	#
	# Create a zip of the DSYMs for TestFlight
	#
	execute_process(COMMAND 7za a $ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.dSYM.zip $ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.xcarchive/dSYMs/${TARGET_SCHEME}.app.dSYM)

	if(NOT EXISTS "$ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.xcarchive/dSYMs/${TARGET_SCHEME}.app.dSYM")
		message(FATAL_ERROR "xcarchive is missing after generation")
	endif()

	message("")
	message("+----------------------------------------------------------+")
	message("| dsym.zip generated at $ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.dSYM.zip")
	message("+----------------------------------------------------------+")
	message("| iOS Archiving Ended                                      |")
	message("+----------------------------------------------------------+")
	message("")

endif()
