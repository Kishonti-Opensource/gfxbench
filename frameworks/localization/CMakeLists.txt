cmake_minimum_required(VERSION 3.5)
project(localization)

if(NOT "${TFW_PACKAGE_DIR}" STREQUAL "")
    install(DIRECTORY "translated/" DESTINATION ${TFW_PACKAGE_DIR}/localization)
endif()
