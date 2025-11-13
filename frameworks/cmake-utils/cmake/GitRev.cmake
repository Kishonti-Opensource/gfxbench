# Get GIT revision SHA1 commit id and refspec
# implemented according to the rpavlik method (using his GetGitRevisionDescription module)
# see (http://stackoverflow.com/questions/1435953/how-can-i-pass-git-sha1-to-compiler-as-definition-using-cmake)
#
# Usage:
#
# In your CMakeLists.txt include this module
#
#     include(GitRev)
#
# Then add GITREV_SOURCES to each of your targets to link with gitrev.cpp/h
#
#     add_executable/library(myTarget ... ${GITREV_SOURCES} ...)
#
# That's all. In you cpp files where you need the git revision data include the gitrev.h:
#     #include "gitrev.h"
# See the private/gitrev/gitrev.h file for more info.
#
# The include(GitRev) also sets the following variables which can also be used in the CMakeLists file, too:
#
#     GITREV_REFSPEC : something like refs/origin/mybranchname
#     GITREV_SHA1 : commit id
#     GITREV_SOURCES : paths of the gitrev.cpp and .h files
#

option(OPT_DISABLE_GITREV "Disable GitRev to prevent frequent CMake re-generations during development" OFF)

if (NOT OPT_DISABLE_GITREV)
	include(${CMAKE_CURRENT_LIST_DIR}/private/gitrev/GetGitRevisionDescription.cmake)
	get_git_head_revision(GITREV_REFSPEC GITREV_SHA1)
else()
	set(GITREV_SHA1 "GITREV_DISABLED-NOTFOUND")
	set(GITREV_REFSPEC "GITREV_DISABLED-NOTFOUND")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gitrev)
configure_file(${CMAKE_CURRENT_LIST_DIR}/private/gitrev/gitrev.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/gitrev/gitrev.cpp @ONLY)
configure_file(${CMAKE_CURRENT_LIST_DIR}/private/gitrev/gitrev.h ${CMAKE_CURRENT_BINARY_DIR}/gitrev/gitrev.h COPYONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/gitrev)
set(GITREV_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/gitrev/gitrev.cpp ${CMAKE_CURRENT_BINARY_DIR}/gitrev/gitrev.h)
