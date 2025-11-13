cmake_minimum_required(VERSION 3.5)
project(kcl)

include(UseCXX11)

set(CMAKE_DEBUG_POSTFIX _d)

set(KCL_PATH "${CMAKE_CURRENT_LIST_DIR}/kcl")
set(KCL_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/kcl_libraries")

set(KCL_PATH_PUBLIC
	${KCL_PATH}
	PARENT_SCOPE
)
set(KCL_LIB_PATH_PUBLIC
	${KCL_LIB_PATH}
	PARENT_SCOPE
)

if(TARGET KCL)
	return()
endif()

set(SRC_KCL
	${KCL_PATH}/src/mesh_tool.cpp
	${KCL_PATH}/src/kcl_base.cpp
	${KCL_PATH}/src/kcl_actor.cpp
	${KCL_PATH}/src/kcl_animation4.cpp
	${KCL_PATH}/src/kcl_animationtrack.cpp
	${KCL_PATH}/src/kcl_camera2.cpp
	${KCL_PATH}/src/kcl_effect.cpp
	${KCL_PATH}/src/kcl_image.cpp
	${KCL_PATH}/src/kcl_light2.cpp
	${KCL_PATH}/src/kcl_lod.cpp
	${KCL_PATH}/src/kcl_material.cpp
	${KCL_PATH}/src/kcl_math3d.cpp
	${KCL_PATH}/src/kcl_mesh.cpp
	${KCL_PATH}/src/kcl_node.cpp
	${KCL_PATH}/src/kcl_object.cpp
	${KCL_PATH}/src/kcl_serializable.cpp
	${KCL_PATH}/src/kcl_room.cpp
	${KCL_PATH}/src/kcl_particlesystem.cpp
	${KCL_PATH}/src/kcl_particlesystem2.cpp
	${KCL_PATH}/src/kcl_planarmap.cpp
	${KCL_PATH}/src/kcl_scene_handler.cpp
	${KCL_PATH}/src/kcl_texture.cpp
	${KCL_PATH}/src/kcl_scene_xml.cpp
	${KCL_PATH}/src/kcl_io.cpp
	${KCL_PATH}/src/ogg_decoder.cpp
	${KCL_PATH}/src/kcl_buffer.cpp
	${KCL_PATH}/src/kcl_envprobe.cpp
	${KCL_PATH}/src/hdr.cpp
)

if ("${OPT_GRAPHICS}" STREQUAL "METAL")
	add_definitions(-DUSE_METAL)
elseif ("${OPT_GRAPHICS}" STREQUAL "DX")
	add_definitions(-DHAVE_DX)
endif()

add_definitions(-DGFXB_DEPRECATED)

if(WIN32)
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4800 /wd4244 /wd4251 /wd4131 /wd4996 /wd4244 /wd4018")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
	list(APPEND SRC_KCL
		${KCL_PATH}/platforms/windows/src/kcl_os.cpp
		${KCL_PATH}/platforms/windows/src/kcl_io_os.cpp
	)
endif()

if (ANDROID)
	find_package(platform_utils REQUIRED)
	list(APPEND SRC_KCL
		${KCL_PATH}/platforms/android/src/kcl_os.cpp
		${KCL_PATH}/platforms/android/src/kcl_io_os.cpp
	)
endif()

if(NACL)
	message(STATUS "KCL-PLATFORM: NACL")
	list(APPEND SRC_KCL
		${KCL_PATH}/platforms/nacl/src/kcl_os.cpp
		${KCL_PATH}/platforms/nacl/src/kcl_io_os.cpp
	)
endif()

if(QNX)
	list(APPEND SRC_KCL
		${KCL_PATH}/platforms/qnx/src/kcl_qnx_os.cpp
		${KCL_PATH}/platforms/qnx/src/kcl_io_qnx_os.cpp
	)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT NACL AND NOT QNX)
	list(APPEND SRC_KCL
		${KCL_PATH}/platforms/linux/src/kcl_os.cpp
		${KCL_PATH}/platforms/linux/src/kcl_io_os.cpp
	)
endif()

if(APPLE AND NOT IOS)
	list(APPEND SRC_KCL
		${KCL_PATH}/platforms/mac/src/kcl_os.mm
		${KCL_PATH}/platforms/mac/src/kcl_io_os.cpp
	)
endif()

if(IOS)
	list(APPEND SRC_KCL
		${KCL_PATH}/platforms/ios/src/kcl_os.mm
		${KCL_PATH}/platforms/ios/src/kcl_io_os.cpp
	)
endif()

if(QNX)
	list(APPEND SRC_KCL
		${KCL_PATH}/platforms/qnx/src/kcl_qnx_os.cpp
		${KCL_PATH}/platforms/qnx/src/kcl_io_qnx_os.cpp
	)
endif()

set(INC_KCL
	${KCL_PATH}/src/mesh_tool.h
	${KCL_PATH}/src/progressbar.h
	${KCL_PATH}/src/kcl_factory_base.h
	${KCL_PATH}/src/kcl_base.h
	${KCL_PATH}/src/kcl_aabb.h
	${KCL_PATH}/src/kcl_actor.h
	${KCL_PATH}/src/kcl_animation4.h
	${KCL_PATH}/src/kcl_animationtrack.h
	${KCL_PATH}/src/kcl_camera2.h
	${KCL_PATH}/src/kcl_effect.h
	${KCL_PATH}/src/kcl_image.h
	${KCL_PATH}/src/kcl_keyframesequence.h
	${KCL_PATH}/src/kcl_light2.h
	${KCL_PATH}/src/kcl_material.h
	${KCL_PATH}/src/kcl_math3d.h
	${KCL_PATH}/src/kcl_mesh.h
	${KCL_PATH}/src/kcl_node.h
	${KCL_PATH}/src/kcl_object.h
	${KCL_PATH}/src/kcl_serializable.h
	${KCL_PATH}/src/kcl_particlesystem.h
	${KCL_PATH}/src/kcl_particlesystem2.h
	${KCL_PATH}/src/kcl_planarmap.h
	${KCL_PATH}/src/kcl_room.h
	${KCL_PATH}/src/kcl_scene_handler.h
	${KCL_PATH}/src/kcl_texture.h
	${KCL_PATH}/src/kcl_os.h
	${KCL_PATH}/src/kcl_io.h
	${KCL_PATH}/src/kcl_scene_version.h
	${KCL_PATH}/src/kcl_buffer.h
	${KCL_PATH}/src/kcl_cube_envmap_descriptor.h
	${KCL_PATH}/src/kcl_envprobe.h
	${KCL_PATH}/src/ogg_decoder.h
	${KCL_PATH}/src/jsonserializer.h
	${KCL_PATH}/platforms/windows/src/winapifamilynull.h
	${KCL_PATH}/src/hdr.h
)

set(SRC_LIBETC1
	${KCL_LIB_PATH}/libetc1/src/etc1.cpp
)

set(SRC_LIBETC1_headers
	${KCL_LIB_PATH}/libetc1/src/etc1.h
)

set(SRC_LIBTINYTHREAD
	${KCL_LIB_PATH}/tinythread/tinythread.cpp
)

set(SRC_LIBTINYTHREAD_headers
	${KCL_LIB_PATH}/tinythread/tinythread.h
	${KCL_LIB_PATH}/tinythread/fast_mutex.h
)

set(SRC_LIBTINYXML
	${KCL_LIB_PATH}/tinyxml/src/tinystr.cpp
	${KCL_LIB_PATH}/tinyxml/src/tinyxml.cpp
	${KCL_LIB_PATH}/tinyxml/src/tinyxmlerror.cpp
	${KCL_LIB_PATH}/tinyxml/src/tinyxmlparser.cpp
)

set(SRC_LIBTINYXML_headers
	${KCL_LIB_PATH}/tinyxml/src/tinystr.h
	${KCL_LIB_PATH}/tinyxml/src/tinyxml.h
)

set(SRC_LIBTINYXML2
	${KCL_LIB_PATH}/tinyxml2/src/tinyxml2.cpp
)

set(SRC_LIBTINYXML2_headers
	${KCL_LIB_PATH}/tinyxml2/src/tinyxml2.h
)

set(SRC_LIBOGG
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/bitpack.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/bitwise.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/decinfo.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/decode.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/dequant.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/fragment.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/framing.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/huffdec.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/idct.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/info.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/internal.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/quant.c
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/state.c
)

set(SRC_LIBOGG_headers
	${KCL_LIB_PATH}/libogg_theora_vorbis/ogg/config_types.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/ogg/ogg.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/ogg/os_types.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/bitpack.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/cpu.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/dct.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/decint.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/dequant.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/encint.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/enquant.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/huffdec.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/huffenc.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/huffman.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/internal.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/mathops.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/ocintrin.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/quant.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/src/state.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/theora/codec.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/theora/theora.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/theora/theoradec.h
	${KCL_LIB_PATH}/libogg_theora_vorbis/theora/theoraenc.h
)

SOURCE_GROUP("LibETC1" FILES ${SRC_LIBETC1} ${SRC_LIBETC1_headers})
SOURCE_GROUP("LibTinyXML" FILES ${SRC_LIBTINYXML} ${SRC_LIBTINYXML_headers})
SOURCE_GROUP("LibTinyXML2" FILES ${SRC_LIBTINYXML2} ${SRC_LIBTINYXML2_headers})
SOURCE_GROUP("LibOgg" FILES ${SRC_LIBOGG} ${SRC_LIBOGG_headers})

set(SRC_KCL_LIB
	${SRC_LIBETC1}
	${SRC_LIBTINYXML}
	${SRC_LIBTINYXML2}
	${SRC_LIBOGG}
)

set(INC_KCL_LIB
	${SRC_LIBETC1_headers}
	${SRC_LIBTINYXML_headers}
	${SRC_LIBTINYXML2_headers}
	${SRC_LIBOGG_headers}
)

if (ANDROID)
	include_directories(${PLATFORM_UTILS_INCLUDE_DIRS})
endif()

set(KCL_HEADERS
	${KCL_PATH}/src
	${KCL_LIB_PATH}/libetc1/src
	${KCL_LIB_PATH}/libogg_theora_vorbis
	${KCL_LIB_PATH}/tinythread
	${KCL_LIB_PATH}/tinyxml/src
	${KCL_LIB_PATH}/tinyxml2/src)

add_library(KCL STATIC ${INC_KCL} ${SRC_KCL})

add_library(KCL_libraries STATIC ${SRC_KCL_LIB} ${INC_KCL_LIB})
target_include_directories(KCL_libraries PUBLIC ${KCL_HEADERS})

if(APPLE AND NOT IOS)
    target_compile_options(KCL PRIVATE "-Wno-shift-negative-value")
    target_compile_options(KCL_libraries PRIVATE "-Wno-shift-negative-value")
endif()


target_include_directories(KCL PUBLIC ${KCL_HEADERS})
target_link_libraries(KCL KCL_libraries)

include(apply_find_vars)
find_package(PNG REQUIRED)
find_package(ngrtl REQUIRED core)
apply_find_vars(PNG)
apply_find_vars(NGRTL)

target_link_libraries(KCL ${ZLIB_LIBRARIES} ${PNG_LIBRARIES})

if (ANDROID)
	target_link_libraries(KCL ${PLATFORM_UTILS_LIBRARIES})
endif()


# KCL is building as "standalone" library, not with add_subdirectory. Installation is required.
if (${CMAKE_CURRENT_LIST_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    install(TARGETS KCL DESTINATION lib)
    install(TARGETS KCL_libraries DESTINATION lib)

    set(HEADERS ${INC_KCL} ${INC_KCL_LIB})
    install(FILES ${HEADERS} DESTINATION "include/kcl")
    install(FILES KCLConfig.cmake.in DESTINATION share/kcl/cmake RENAME KCLConfig.cmake)
endif()

