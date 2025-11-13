set(ARCHIVE_ROOT "$ENV{ARCHIVE_ROOT}")
if(NOT ARCHIVE_ROOT)
	message(FATAL_ERROR "No archive root specified!")

else()

	message("")
	message("+----------------------------------------------------------+")
	message("| iOS Distribution started                                 |")
	message("+----------------------------------+-----------------------+")
	message("| Used environment variables:      |                        ")
	message("|                                  |                        ")
	message("| ARCHIVE_ROOT                     | $ENV{ARCHIVE_ROOT}")
	message("| PRODUCT_ID                       | $ENV{PRODUCT_ID}")
	message("| DistribtutionType                | $ENV{DISTRIBUTION_TYPE}")
	message("| APPLE_ID                         | $ENV{APPLE_ID}")
	message("| submitterMail                    | $ENV{submitterMail}")
	message("| submitterPass                    | ***")
	message("| hockeyAppId                      | $ENV{hockeyAppId}")
	message("| hockeyApiKey                     | $ENV{hockeyApiKey}")
	message("| kotkodapass                      | ***")
	message("+----------------------------------+-----------------------+")

	set(ALLOWED_DISTRIBUTION_TYPES developer adhoc media store)
	list(FIND ALLOWED_DISTRIBUTION_TYPES $ENV{DISTRIBUTION_TYPE} FOUND)
	if(${FOUND} LESS 0)
		message(FATAL_ERROR "DISTRIBUTION_TYPE not valid (must be one of: developer, adhoc, media, store)")
	endif()

	# 
	# Check required variables
	#
	if( ("$ENV{PRODUCT_ID}" 				STREQUAL "") OR
		("$ENV{DISTRIBUTION_TYPE}" 			STREQUAL "") OR
		("$ENV{APPLE_ID}" 					STREQUAL "" AND "$ENV{DISTRIBUTION_TYPE}" STREQUAL "store") OR
		("$ENV{submitterMail}" 				STREQUAL "" AND "$ENV{DISTRIBUTION_TYPE}" STREQUAL "store") OR
		("$ENV{submitterPass}" 				STREQUAL "" AND "$ENV{DISTRIBUTION_TYPE}" STREQUAL "store") OR
		("$ENV{hockeyAppId}" 				STREQUAL "" AND "$ENV{DISTRIBUTION_TYPE}" STREQUAL "store") OR
		("$ENV{hockeyApiKey}" 				STREQUAL "" AND "$ENV{DISTRIBUTION_TYPE}" STREQUAL "store") OR
		("$ENV{kotkodapass}" 				STREQUAL "" AND ("$ENV{DISTRIBUTION_TYPE}" STREQUAL "adhoc" OR "$ENV{DISTRIBUTION_TYPE}" STREQUAL "media")))
		message(FATAL_ERROR "Required parameter(s) missing.....")
	endif()


	set(OPTIONS_METHOD "development")
	set(OPTIONS_UPLOAD_SYMBOLS "true")
	set(TARGET_SCHEME "app_ios")
	set(CODE_SIGNING_ID "iPhone Development: *")
	set(PROVISIONING_PROFILE $ENV{KISHONTI_DEVELOPER_WILDCARD})
	set(PROVISIONING_PROFILE_NAME $ENV{KISHONTI_DEVELOPER_WILDCARD_NAME})

	if("$ENV{DISTRIBUTION_TYPE}" STREQUAL "store")
		set(OPTIONS_METHOD "app-store")
		set(OPTIONS_UPLOAD_SYMBOLS "false")
		set(CODE_SIGNING_ID "iPhone Distribution: Kishonti (49X7A8FNR8)")
		if("$ENV{PRODUCT_ID}" STREQUAL "gfxbench_gl")
			set(PROVISIONING_PROFILE $ENV{GFXB27_APPSTORE})
			set(PROVISIONING_PROFILE_NAME $ENV{GFXB27_APPSTORE_NAME})
		else()
			set(PROVISIONING_PROFILE $ENV{KISHONTI_APP_STORE_SEPT23})
			set(PROVISIONING_PROFILE_NAME $ENV{KISHONTI_APP_STORE_SEPT23_NAME})
		endif()

	elseif("$ENV{DISTRIBUTION_TYPE}" STREQUAL "adhoc")
		set(OPTIONS_METHOD "ad-hoc")
		set(OPTIONS_UPLOAD_SYMBOLS "false")
		set(CODE_SIGNING_ID "iPhone Distribution: Kishonti (49X7A8FNR8)")
		set(PROVISIONING_PROFILE $ENV{KISHONTI_ADHOC_SEPT23})
		set(PROVISIONING_PROFILE_NAME $ENV{KISHONTI_ADHOC_SEPT23_NAME})

	elseif("$ENV{DISTRIBUTION_TYPE}" STREQUAL "media")
		set(OPTIONS_METHOD "ad-hoc")
		set(OPTIONS_UPLOAD_SYMBOLS "false")
		set(CODE_SIGNING_ID "iPhone Distribution: Kishonti (49X7A8FNR8)")
		set(PROVISIONING_PROFILE $ENV{KISHONTI_ADHOC_SEPT23})
		set(PROVISIONING_PROFILE_NAME $ENV{KISHONTI_ADHOC_SEPT23_NAME})
	endif()


	message("| Calculated variables:            |                        ")
	message("|                                  |                        ")
	message("| ProvisioningProfile              | ${PROVISIONING_PROFILE}")
	message("| ProvisioningProfileName          | ${PROVISIONING_PROFILE_NAME}")
	message("+----------------------------------+-----------------------+")
	message("")

	#
	# Create the archive_name variable
	#
	set(BENCHMARK_BUILD_NUMBER $ENV{BUILD_NUMBER})
	if(PRODUCT_ID STREQUAL "gfxbench_gl" AND STORE_VERSION AND "$ENV{DISTRIBUTION_TYPE}" STREQUAL "store")
		set(BUNDLE_ID "com.glbenchmark.glb27")

	elseif("${PRODUCT_ID}" STREQUAL "compubench_metal")
    	set(BUNDLE_ID "net.kishonti.compubench.metal.mobile")

	else()
		if("$ENV{COMMUNITY_BUILD}" STREQUAL "true")
			set(BUNDLE_ID "net.kishonti.$ENV{PRODUCT_ID}")
		else()
			set(BUNDLE_ID "net.kishonti.$ENV{PRODUCT_ID}.v${VERSION_CODE}.corporate")
		endif()
		string(REPLACE "_" "." BUNDLE_ID ${BUNDLE_ID})
	endif()

	if("$ENV{COMMUNITY_BUILD}" STREQUAL "true")
	    set(PRODUCT_VERSION_META_POSTFIX "+community")
	else()
	    set(PRODUCT_VERSION_META_POSTFIX "+corporate")
	endif()

	#
	# Get the benchmark name from the variables set up in buld.sh
	#
	set(ARCHIVE_NAME "${BUNDLE_ID}-$ENV{PRODUCT_VERSION}${PRODUCT_VERSION_META_POSTFIX}")

	#
	# Get the benchmark name from the variables set up in buld.sh
	#
	set(FINALNAME "${ARCHIVE_NAME}.$ENV{DISTRIBUTION_TYPE}")
	if("$ENV{STORE_VERSION}" STREQUAL "true")
		string(REPLACE "+" "-" FINALNAME ${FINALNAME})
	endif()
	
	#TODO HACK
	if("${PRODUCT_ID}" STREQUAL "compubench_metal")
		string(REPLACE "-RC5" "" FINALNAME ${FINALNAME})
	endif()

	message("")
	message("+----------------------------------------------------------+")
	message("| Generating IPA                                           |")
	message("+----------------------------------------------------------+")
	message("")

	#
	# The profile must be specified by name for this step
	#

	execute_process(COMMAND
			xcodebuild
	        -exportArchive
	        -exportFormat ipa
	        -archivePath $ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.xcarchive
	        -exportPath $ENV{ARCHIVE_ROOT}/${FINALNAME}.ipa
	        -exportProvisioningProfile ${PROVISIONING_PROFILE_NAME})


	if(NOT EXISTS "$ENV{ARCHIVE_ROOT}/${FINALNAME}.ipa")
		message(FATAL_ERROR "xcarchive is missing after generation")
	endif()

	message("")
	message("+----------------------------------------------------------+")
	message("| IPA at $ENV{ARCHIVE_ROOT}/${FINALNAME}.ipa")
	message("+----------------------------------------------------------+")


	#
	# Distribute ipa to the iTunes store if it's needed
	#
	if($ENV{DISTRIBUTION_TYPE} STREQUAL "store")
		message("")
		message("+----------------------------------------------------------+")
		message("| Distributing generated IPA to the iTunes store           |")
		message("+----------------------------------------------------------+")
		message("")

		execute_process(COMMAND ipa info $ENV{ARCHIVE_ROOT}/${FINALNAME}.ipa)
    	execute_process(COMMAND mkdir $ENV{ARCHIVE_ROOT}/${FINALNAME}.itmsp)

    	set(FILE_NAME "${FINALNAME}.ipa")
	    execute_process(COMMAND md5 -q $ENV{ARCHIVE_ROOT}/${FINALNAME}.ipa OUTPUT_VARIABLE MD5 OUTPUT_STRIP_TRAILING_WHITESPACE)
	    execute_process(COMMAND stat -f%z $ENV{ARCHIVE_ROOT}/${FINALNAME}.ipa OUTPUT_VARIABLE SIZE OUTPUT_STRIP_TRAILING_WHITESPACE)

	    file(WRITE $ENV{ARCHIVE_ROOT}/${FINALNAME}.itmsp/metadata.xml
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<package version=\"software4.7\" xmlns=\"http://apple.com/itunes/importer\">
    <software_assets apple_id=\"$ENV{APPLE_ID}\">
        <asset type=\"bundle\">
            <data_file>
                <file_name>${FILE_NAME}</file_name>
                <checksum type=\"md5\">${MD5}</checksum>
                <size>${SIZE}</size>
            </data_file>
        </asset>
    </software_assets>
</package>")

	    execute_process(COMMAND cp "$ENV{ARCHIVE_ROOT}/${FINALNAME}.ipa" "$ENV{ARCHIVE_ROOT}/${FINALNAME}.itmsp/${FINALNAME}.ipa")

		message("")
		message("+----------------------------------------------------------+")
		message("| Uploading to the iTunes store ....                       |")
		message("+----------------------------------------------------------+")
		message("")
	    execute_process(COMMAND /Applications/Xcode.app/Contents/Applications/Application\ Loader.app/Contents/itms/bin/iTMSTransporter
	    	-u $ENV{submitterMail}
	    	-p $ENV{submitterPass}
	    	-m upload
	    	-f /$ENV{ARCHIVE_ROOT}/${FINALNAME}.itmsp
	    	RESULT_VARIABLE STATUS)

		message("")
		message("+----------------------------------------------------------+")
if(${STATUS} EQUAL 0)
		message("| iTunes upload OK                                         |")
		message("+----------------------------------------------------------+")
else()
		message("| iTunes upload FAILED (see above)                         |")
		message("+----------------------------------------------------------+")
		message(FATAL_ERROR "iTunes upload FAILED (see above)")
endif()
# 		message("| Uploading dsyms to Crittercism                           |")
# 		message("+----------------------------------------------------------+")
# 		message("")

# 	    #
# 		# Upload dSYM to Crittercism
# 		#
# 		set(DSYM_UPLOAD_ENDPOINT "https://api.crittercism.com/api_beta/dsym/")
# 		set(URL "${DSYM_UPLOAD_ENDPOINT}$ENV{hockeyAppId}")
# 		message(STATUS "Uploading dSYM to Crittercism: ${URL}")
# 		execute_process(COMMAND curl ${URL} -F dsym=@$ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.dSYM.zip -F key=$ENV{hockeyApiKey} RESULT_VARIABLE STATUS)

# 		message(STATUS "Crittercism API server response: ${STATUS}")

# 		message("")
# 		message("+----------------------------------------------------------+")
# if(${STATUS} EQUAL 0)
# 		message("| Crittercism dSYM upload OK                               |")
# 		message("+----------------------------------------------------------+")
# else()
# 		message("| Crittercism dSYM upload FAILED (see above)               |")
# 		message("+----------------------------------------------------------+")
# endif()
		message("")

	#
	# If it's just an ad hoc distribution create ad hoc html, and plist
	#
	elseif($ENV{DISTRIBUTION_TYPE} STREQUAL "media" OR $ENV{DISTRIBUTION_TYPE} STREQUAL "adhoc")
		message("")
		message("+----------------------------------------------------------+")
		message("| Generating .html and .plist for AdHoc distribution       |")
		message("+----------------------------------------------------------+")
		message("")


		if(NOT IOS_DISTRIBUTION_OUTPUT_DIR)
			message(FATAL_ERROR "IOS_DISTRIBUTION_OUPTUT_DIR not set to full path where the ipa will be created")
		endif()

		execute_process(COMMAND /usr/libexec/PlistBuddy -c "Print" $ENV{ARCHIVE_ROOT}/${ARCHIVE_NAME}.xcarchive/Info.plist OUTPUT_VARIABLE BUNDLE_PLIST_CONTENT OUTPUT_STRIP_TRAILING_WHITESPACE)

		set(IPA_ARCHIVE_NAME "${FINALNAME}")
		set(DISPLAY_NAME "$ENV{BENCHMARK_ID}")
		set(BUNDLE_ID "${BUNDLE_ID}")
		set(BUNDLE_VERSION "$ENV{PRODUCT_VERSION}")
		set(DOWNLOAD_URL "https://$ENV{BENCHMARK_ID}.com/download")

		configure_file(${CMAKE_CURRENT_LIST_DIR}/generate_ipa.html.in "${IOS_DISTRIBUTION_OUTPUT_DIR}/${FINALNAME}.html")
		configure_file(${CMAKE_CURRENT_LIST_DIR}/generate_ipa.plist.in "${IOS_DISTRIBUTION_OUTPUT_DIR}/${FINALNAME}.plist")

		if($ENV{BENCHMARK_ID} STREQUAL "compubench")
			set(SITE_ID 4)
		else()
			set(SITE_ID 3)
		endif()

		message("")
		message("+----------------------------------------------------------+")
		message("| Uploading Ad Hoc plist, html, ipa to kotkoda             |")
		message("+----------------------------------------------------------+")
		message("")

		set(KOTKODA "https://kishonti.net/kotkoda/upload_public_file_action.jsp")

		execute_process(COMMAND curl
			-v
			-H public-file:true
			-H jenkins-agent-secret:$ENV{kotkodapass}
			-F site=${SITE_ID}
			-F overwrite=true
			-F files[]=@${ARCHIVE_ROOT}/${FINALNAME}.ipa
			-F files[]=@${IOS_DISTRIBUTION_OUTPUT_DIR}/${FINALNAME}.plist
			-F files[]=@${IOS_DISTRIBUTION_OUTPUT_DIR}/${FINALNAME}.html
			${KOTKODA})

		message("")
		message("+----------------------------------------------------------+")
		message("| Ad Hoc upload completed                                  |")
		if($ENV{BENCHMARK_ID} STREQUAL "compubench")
			message("| Link: http://compubench.com/download/${FINALNAME}.html")
		else()
			message("| Link: http://gfxbench.com/download/${FINALNAME}.html")
		endif()
		message("+----------------------------------------------------------+")

	endif()


	message("| iOS Distribution Ended                                   |")
	message("+----------------------------------------------------------+")
	message("")


endif()
