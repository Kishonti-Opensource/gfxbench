/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  SystemInfoProperties.h
//  GFXBench
//
//  Created by Kishonti Kft on 14/11/2013.
//
//

#ifndef __GFXBench__SystemInfoProperties__
#define __GFXBench__SystemInfoProperties__

#include <string>

namespace sysinf {
    // The UI will use these keys as they primary source of data.
    // New keys may be added but only the server will use them.
    
    const std::string CORPORATE  = "is_corporate";

	const std::string WMI													= "wmi";
    
    const std::string API_3D                                                = "api/3d";
    const std::string API_3D_MAJOR                                          = "api/3d/major";
    const std::string API_3D_MINOR                                          = "api/3d/minor";
    
    
    const std::string API_GL                                                = "api/gl";
    const std::string API_GL_NAME                                           = "api/gl/name";
    const std::string API_GL_VERSION_STRING                                 = "api/gl/version";
    const std::string API_GL_ALPHA_BITS                                     = "api/gl/features/GL_ALPHA_BITS";
    const std::string API_GL_BLUE_BITS                                      = "api/gl/features/GL_BLUE_BITS";
    const std::string API_GL_DEPTH_BITS                                     = "api/gl/features/GL_DEPTH_BITS";
    const std::string API_GL_GREEN_BITS                                     = "api/gl/features/GL_GREEN_BITS";
    const std::string API_GL_RED_BITS                                       = "api/gl/features/GL_RED_BITS";
    const std::string API_GL_STENCIL_BITS                                   = "api/gl/features/GL_STENCIL_BITS";
    const std::string API_GL_ALIASED_LINE_WIDTH_MAX                         = "api/gl/features/GL_ALIASED_LINE_WIDTH_MAX";
    const std::string API_GL_ALIASED_LINE_WIDTH_MIN                         = "api/gl/features/GL_ALIASED_LINE_WIDTH_MIN";
    const std::string API_GL_ALIASED_POINT_SIZE_MAX                         = "api/gl/features/GL_ALIASED_POINT_SIZE_MAX";
    const std::string API_GL_ALIASED_POINT_SIZE_MIN                         = "api/gl/features/GL_ALIASED_POINT_SIZE_MIN";
    const std::string API_GL_EXTENSIONS                                     = "api/gl/features/GL_EXTENSIONS";
    const std::string API_GL_RENDERER                                       = "api/gl/features/GL_RENDERER";
    const std::string API_GL_VENDOR                                         = "api/gl/features/GL_VENDOR";
    const std::string API_GL_VERSION                                        = "api/gl/features/GL_VERSION";
    const std::string API_GL_MAX_3D_TEXTURE_SIZE                            = "api/gl/features/GL_MAX_3D_TEXTURE_SIZE";
    const std::string API_GL_MAX_ARRAY_TEXTURE_LAYERS                       = "api/gl/features/GL_MAX_ARRAY_TEXTURE_LAYERS";
    const std::string API_GL_MAX_COLOR_ATTACHMENTS                          = "api/gl/features/GL_MAX_COLOR_ATTACHMENTS";
    const std::string API_GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS       = "api/gl/features/GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS";
    const std::string API_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS               = "api/gl/features/GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS";
    const std::string API_GL_MAX_COMBINED_UNIFORM_BLOCKS                    = "api/gl/features/GL_MAX_COMBINED_UNIFORM_BLOCKS";
    const std::string API_GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS         = "api/gl/features/GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS";
    const std::string API_GL_MAX_CUBE_MAP_TEXTURE_SIZE                      = "api/gl/features/GL_MAX_CUBE_MAP_TEXTURE_SIZE";
    const std::string API_GL_MAX_DRAW_BUFFERS                               = "api/gl/features/GL_MAX_DRAW_BUFFERS";
    const std::string API_GL_MAX_ELEMENTS_VERTICES                          = "api/gl/features/GL_MAX_ELEMENTS_VERTICES";
    const std::string API_GL_MAX_ELEMENT_INDEX                              = "api/gl/features/GL_MAX_ELEMENT_INDEX";
    const std::string API_GL_MAX_ELEMENTS_INDICES                           = "api/gl/features/GL_MAX_ELEMENTS_INDICES";
    const std::string API_GL_MAX_FRAGMENT_INPUT_COMPONENTS                  = "api/gl/features/GL_MAX_FRAGMENT_INPUT_COMPONENTS";
    const std::string API_GL_MAX_FRAGMENT_UNIFORM_BLOCKS                    = "api/gl/features/GL_MAX_FRAGMENT_UNIFORM_BLOCKS";
    const std::string API_GL_MAX_FRAGMENT_UNIFORM_VECTORS                   = "api/gl/features/GL_MAX_FRAGMENT_UNIFORM_VECTORS";
    const std::string API_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS                = "api/gl/features/GL_MAX_FRAGMENT_UNIFORM_COMPONENTS";
    const std::string API_GL_MAX_PROGRAM_TEXEL_OFFSET                       = "api/gl/features/GL_MAX_PROGRAM_TEXEL_OFFSET";
    const std::string API_GL_MAX_RENDERBUFFER_SIZE                          = "api/gl/features/GL_MAX_RENDERBUFFER_SIZE";
    const std::string API_GL_MAX_SAMPLES                                    = "api/gl/features/GL_MAX_SAMPLES";
    const std::string API_GL_MAX_SERVER_WAIT_TIMEOUT                        = "api/gl/features/GL_MAX_SERVER_WAIT_TIMEOUT";
    const std::string API_GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS        = "api/gl/features/GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS";
    const std::string API_GL_MAX_UNIFORM_BLOCK_SIZE                         = "api/gl/features/GL_MAX_UNIFORM_BLOCK_SIZE";
    const std::string API_GL_MAX_UNIFORM_BUFFER_BINDINGS                    = "api/gl/features/GL_MAX_UNIFORM_BUFFER_BINDINGS";
    const std::string API_GL_MAX_VARYING_COMPONENTS                         = "api/gl/features/GL_MAX_VARYING_COMPONENTS";
    const std::string API_GL_MAX_VARYING_VECTORS                            = "api/gl/features/GL_MAX_VARYING_VECTORS";
    const std::string API_GL_MAX_VERTEX_ATTRIBS                             = "api/gl/features/GL_MAX_VERTEX_ATTRIBS";
    const std::string API_GL_MAX_VERTEX_OUTPUT_COMPONENTS                   = "api/gl/features/GL_MAX_VERTEX_OUTPUT_COMPONENTS";
    const std::string API_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                 = "api/gl/features/GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS";
    const std::string API_GL_MAX_VERTEX_UNIFORM_BLOCKS                      = "api/gl/features/GL_MAX_VERTEX_UNIFORM_BLOCKS";
    const std::string API_GL_MAX_VERTEX_UNIFORM_VECTORS                     = "api/gl/features/GL_MAX_VERTEX_UNIFORM_VECTORS";
    const std::string API_GL_MAX_VERTEX_UNIFORM_COMPONENTS                  = "api/gl/features/GL_MAX_VERTEX_UNIFORM_COMPONENTS";
    const std::string API_GL_MAX_TEXTURE_IMAGE_UNITS                        = "api/gl/features/GL_MAX_TEXTURE_IMAGE_UNITS";
    const std::string API_GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS     = "api/gl/features/GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS";
    const std::string API_GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS  = "api/gl/features/GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS";
    const std::string API_GL_MIN_PROGRAM_TEXEL_OFFSET                       = "api/gl/features/GL_MIN_PROGRAM_TEXEL_OFFSET";
    const std::string API_GL_NUM_COMPRESSED_TEXTURE_FORMATS                 = "api/gl/features/GL_NUM_COMPRESSED_TEXTURE_FORMATS";
    const std::string API_GL_MAX_TEXTURE_SIZE                               = "api/gl/features/GL_MAX_TEXTURE_SIZE";
    const std::string API_GL_MAX_VIEWPORT_HEIGHT                            = "api/gl/features/GL_MAX_VIEWPORT_HEIGHT";
    const std::string API_GL_MAX_VIEWPORT_WIDTH                             = "api/gl/features/GL_MAX_VIEWPORT_WIDTH";
    
    const std::string APPINFO_BENCHMARK_ID      = "appinfo/benchmark_id";
    const std::string APPINFO_INSTALLERNAME     = "appinfo/installername";
    const std::string APPINFO_LOCALE            = "appinfo/locale";
    const std::string APPINFO_PACKAGE_NAME      = "appinfo/packagename";
    const std::string APPINFO_PLATFORM          = "appinfo/platform";
    const std::string APPINFO_VERSION           = "appinfo/version";
    const std::string APPINFO_STORENAME         = "appinfo/storename";

    const std::string API_GL_CONFIGURATIONS                                 = "configurations";
    const std::string API_GL_CONFIGURATION_NAME                             = "name";
    const std::string API_GL_CONFIGURATION_TYPE                             = "type";
    const std::string API_GL_CONFIGURATION_COUNT                            = "configuration_count";

	const std::string MULTI_GPU												= "multi_gpu";
	const std::string MULTI_GPU_SLI											= "multi_gpu/sli";
	const std::string MULTI_GPU_SLI_GPU_COUNT								= "multi_gpu/sli/gpu_count";
	const std::string MULTI_GPU_SLI_ENABLED									= "multi_gpu/sli/enabled";
	const std::string MULTI_GPU_SLI_DRIVER									= "multi_gpu/sli/driver";
	const std::string MULTI_GPU_SLI_ERROR									= "multi_gpu/sli/error";
	const std::string MULTI_GPU_CROSSFIRE									= "multi_gpu/crossfire";
	const std::string MULTI_GPU_CROSSFIRE_DRIVER							= "multi_gpu/crossfire/driver";
	const std::string MULTI_GPU_CROSSFIRE_CATALYST							= "multi_gpu/crossfire/catalyst";
	const std::string MULTI_GPU_CROSSFIRE_GPU_COUNT							= "multi_gpu/crossfire/gpu_count";
	const std::string MULTI_GPU_CROSSFIRE_ENABLED							= "multi_gpu/crossfire/enabled";
	const std::string MULTI_GPU_CROSSFIRE_ERROR								= "multi_gpu/crossfire/error";
    
    const std::string API_EGL                   = "api/egl";
    const std::string API_EGL_NAME              = "api/egl/name";
    const std::string API_EGL_VERSION_STRING    = "api/egl/version";
    const std::string API_EGL_CONFIGS           = "api/egl/features/EGL_CONFIGS";
    const std::string API_EGL_EXTENSIONS        = "api/egl/features/EGL_EXTENSIONS";
    const std::string API_EGL_VENDOR            = "api/egl/features/EGL_VENDOR";
    const std::string API_EGL_VERSION           = "api/egl/features/EGL_VERSION";
    
    
	const std::string API_CL                                       = "api/cl";
	const std::string API_CL_AVAILABLE                             = "available";
	const std::string API_CL_PLATFORMS                             = "platforms";
	const std::string API_CL_DEVICES                               = "devices";
    const std::string API_CL_PLATFORM_COUNT                        = "platform_count";
    const std::string API_CL_DEVICE_COUNT                          = "device_count";
	const std::string API_CL_OVERALL_DEVICE_COUNT                  = "overall_device_count";
    const std::string API_CL_CONFIGURATIONS                        = "configurations";
    const std::string API_CL_CONFIGURATION_NAME                    = "name";
    const std::string API_CL_CONFIGURATION_TYPE                    = "type";
    const std::string API_CL_CONFIGURATION_COUNT                   = "configuration_count";
	// CL platform info keys: "api/cl/platforms/0/..."
	const std::string API_CL_PLATFORM_NAME                         = "name";
	const std::string API_CL_PLATFORM_VENDOR                       = "vendor";
	const std::string API_CL_PLATFORM_PROFILE                      = "profile";
	const std::string API_CL_PLATFORM_VERSION                      = "version";
	const std::string API_CL_PLATFORM_EXTENSIONS                   = "extensions";
	// CL device info keys: "api/cl/platforms/0/devices/0/..."
	const std::string API_CL_DEVICE_ADDRESS_BITS                   = "address_bits";
	const std::string API_CL_DEVICE_COMPILER_AVAILABLE             = "compiler_available";
	const std::string API_CL_DEVICE_DOUBLE_FP_CONFIG               = "double_fp_config";
	const std::string API_CL_DEVICE_ENDIAN_LITTLE                  = "endian_little";
	const std::string API_CL_DEVICE_ERROR_CORRECTION_SUPPORT       = "error_correction_support";
	const std::string API_CL_DEVICE_EXECUTION_CAPABILITIES         = "execution_capabilities";
	const std::string API_CL_DEVICE_EXTENSIONS                     = "extensions";
	const std::string API_CL_DEVICE_GLOBAL_MEM_CACHE_SIZE          = "global_mem_cache_size";
	const std::string API_CL_DEVICE_GLOBAL_MEM_CACHE_TYPE          = "global_mem_cache_type";
	const std::string API_CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE      = "global_mem_cacheline_size";
	const std::string API_CL_DEVICE_GLOBAL_MEM_SIZE                = "global_mem_size";
	const std::string API_CL_DEVICE_HALF_FP_CONFIG                 = "half_fp_config";
	const std::string API_CL_DEVICE_HOST_UNIFIED_MEMORY            = "host_unified_memory";
	const std::string API_CL_DEVICE_IMAGE_SUPPORT                  = "image_support";
	const std::string API_CL_DEVICE_IMAGE2D_MAX_HEIGHT             = "image2d_max_height";
	const std::string API_CL_DEVICE_IMAGE2D_MAX_WIDTH              = "image2d_max_width";
	const std::string API_CL_DEVICE_IMAGE3D_MAX_DEPTH              = "image3d_max_depth";
	const std::string API_CL_DEVICE_IMAGE3D_MAX_HEIGHT             = "image3d_max_height";
	const std::string API_CL_DEVICE_IMAGE3D_MAX_WIDTH              = "image3d_max_width";
	const std::string API_CL_DEVICE_LOCAL_MEM_SIZE                 = "local_mem_size";
	const std::string API_CL_DEVICE_LOCAL_MEM_TYPE                 = "local_mem_type";
	const std::string API_CL_DEVICE_MAX_CLOCK_FREQUENCY            = "max_clock_frequency";
	const std::string API_CL_DEVICE_MAX_COMPUTE_UNITS              = "max_compute_units";
	const std::string API_CL_DEVICE_MAX_CONSTANT_ARGS              = "max_constant_args";
	const std::string API_CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE       = "max_constant_buffer_size";
	const std::string API_CL_DEVICE_MAX_MEM_ALLOC_SIZE             = "max_mem_alloc_size";
	const std::string API_CL_DEVICE_MAX_PARAMETER_SIZE             = "max_parameter_size";
	const std::string API_CL_DEVICE_MAX_READ_IMAGE_ARGS            = "max_read_image_args";
	const std::string API_CL_DEVICE_MAX_SAMPLERS                   = "max_samplers";
	const std::string API_CL_DEVICE_MAX_WORK_GROUP_SIZE            = "max_work_group_size";
	const std::string API_CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS       = "max_work_item_dimensions";
	const std::string API_CL_DEVICE_MAX_WORK_ITEM_SIZES            = "max_work_item_sizes";
	const std::string API_CL_DEVICE_MAX_WRITE_IMAGE_ARGS           = "max_write_image_args";
	const std::string API_CL_DEVICE_MEM_BASE_ADDR_ALIGN            = "mem_base_addr_align";
	const std::string API_CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE       = "min_data_type_align_size";
	const std::string API_CL_DEVICE_NAME                           = "name";
	const std::string API_CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR       = "native_vector_width_char";
	const std::string API_CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT      = "native_vector_width_short";
	const std::string API_CL_DEVICE_NATIVE_VECTOR_WIDTH_INT        = "native_vector_width_int";
	const std::string API_CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG       = "native_vector_width_long";
	const std::string API_CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT      = "native_vector_width_float";
	const std::string API_CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE     = "native_vector_width_double";
	const std::string API_CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF       = "native_vector_width_half";
	const std::string API_CL_DEVICE_OPENCL_C_VERSION               = "opencl_c_version";
	const std::string API_CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR    = "preferred_vector_width_char";
	const std::string API_CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT   = "preferred_vector_width_short";
	const std::string API_CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT     = "preferred_vector_width_int";
	const std::string API_CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG    = "preferred_vector_width_long";
	const std::string API_CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT   = "preferred_vector_width_float";
	const std::string API_CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE  = "preferred_vector_width_double";
	const std::string API_CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF    = "preferred_vector_width_half";
	const std::string API_CL_DEVICE_PROFILE                        = "profile";
	const std::string API_CL_DEVICE_PROFILING_TIMER_RESOLUTION     = "profiling_timer_resolution";
	const std::string API_CL_DEVICE_QUEUE_PROPERTIES               = "queue_properties";
	const std::string API_CL_DEVICE_SINGLE_FP_CONFIG               = "single_fp_config";
	const std::string API_CL_DEVICE_TYPE                           = "type";
	const std::string API_CL_DEVICE_VENDOR                         = "vendor";
	const std::string API_CL_DEVICE_VENDOR_ID                      = "vendor_id";
	const std::string API_CL_DEVICE_VERSION                        = "version";
	const std::string API_CL_DEVICE_DRIVER_VERSION                 = "driver_version";
	const std::string API_CL_DEVICE_TEST_BUILD_STATUS              = "test_build_status";
	const std::string API_CL_DEVICE_TEST_BUILD_LOG                 = "test_build_log";


    const std::string API_METAL                                     = "api/metal";
    const std::string API_METAL_NAME                                = "api/metal/name";
    const std::string API_METAL_MAX_COLOR_ATTACHMENT_PER_PASSDESC   = "api/metal/features/max_color_attachment_per_passdesc";
    const std::string API_METAL_MAX_COLOR_OUT_PER_SAMPLE_PER_PASS   = "api/metal/features/max_color_out_per_sample_per_pass";
    const std::string API_METAL_ASTC_SUPPORT                        = "api/metal/features/astc_support";
    const std::string API_METAL_MIN_ATTACHMENT_SIZE                 = "api/metal/features/min_attachment_size";
    const std::string API_METAL_THREADGROUP_MEMORY_ALLOC_SIZE_INC   = "api/metal/features/threadgroup_memory_alloc_size_inc";
    const std::string API_METAL_MAX_THREADGROUP_MEM_ALLOC           = "api/metal/features/max_threadgroup_mem_alloc";
    const std::string API_METAL_MAX_TEXTURE_DESC_HEIGHT_WIDTH       = "api/metal/features/max_texture_desc_height_width";
    const std::string API_METAL_MAX_TEXTURE_DESC_DEPTH              = "api/metal/features/max_texture_desc_depth";
    const std::string API_METAL_MAX_ENTRIES_BUFFER                  = "api/metal/features/max_entries_buffer";
    const std::string API_METAL_MAX_ENTRIES_TEXTURE                 = "api/metal/features/max_entries_texture";
    const std::string API_METAL_MAX_ENTRIES_SAMPLERSTATE            = "api/metal/features/max_entries_samplerstate";
    
    
    //TODO dx
    const std::string API_DX                    = "api/dx";
    const std::string API_DX_NAME               = "api/dx/name";
    const std::string API_DX_VERSION_STRING     = "api/dx/version";
    
    
    const std::string BATTERY                = "battery";
    const std::string BATTERY_IS_CHARGING    = "battery/is/charging";
    const std::string BATTERY_IS_CONNECTED   = "battery/is/connected";
    const std::string BATTERY_LEVEL          = "battery/level";
    const std::string BATTERY_MAH            = "battery/mah";			// Capacity in mAh
    const std::string BATTERY_MAJOR          = "battery/major";
    const std::string BATTERY_MINOR          = "battery/minor";
    const std::string BATTERY_TECHNOLOGY     = "battery/technology";	// NiMh, Li-ion
    
    const std::string CHIPSET                = "chipset";
    const std::string CHIPSET_NAME           = "chipset/name";
    const std::string CHIPSET_MAJOR          = "chipset/major";
    const std::string CHIPSET_MINOR          = "chipset/minor";
    
	const std::string CPU            = "cpu";
	const std::string CPU_COUNT		 = "cpu/count";
    const std::string CPU_CORES      = "cores";
    const std::string CPU_THREADS    = "threads";
    const std::string CPU_FREQUENCY  = "frequency";
    const std::string CPU_FEATURES   = "features";
    const std::string CPU_NAME       = "name";
    const std::string CPU_MAJOR      = "major";
    const std::string CPU_MINOR      = "minor";
	/*
    const std::string CPU            = "cpu";
    const std::string CPU_CORES      = "cpu/cores";
    const std::string CPU_THREADS    = "cpu/threads";
    const std::string CPU_FREQUENCY  = "cpu/frequency";
    const std::string CPU_FEATURES   = "cpu/features";
    const std::string CPU_NAME       = "cpu/name";
    const std::string CPU_MAJOR      = "cpu/major";
    const std::string CPU_MINOR      = "cpu/minor";
    */
    const std::string DEVICE            = "device";
    const std::string DEVICE_NAME       = "device/name";
    const std::string DEVICE_MAJOR      = "device/major";
    const std::string DEVICE_MINOR      = "device/minor";
    
    const std::string DISPLAY           = "display";
    const std::string DISPLAY_COUNT     = "display/count";
    const std::string DISPLAY_DIAGONAL  = "display/diagonal";
    const std::string DISPLAY_DPI_X     = "display/dpi/x";
    const std::string DISPLAY_DPI_Y     = "display/dpi/y";
    const std::string DISPLAY_RES_X     = "display/res/x";
    const std::string DISPLAY_RES_Y     = "display/res/y";
    const std::string DISPLAY_MAJOR     = "display/major";
    const std::string DISPLAY_MINOR     = "display/minor";
    const std::string DISPLAY_DIAGONAL_POSTFIX  = "diagonal";
    const std::string DISPLAY_DPI_X_POSTFIX     = "dpi/x";
    const std::string DISPLAY_DPI_Y_POSTFIX     = "dpi/y";
    const std::string DISPLAY_RES_X_POSTFIX     = "res/x";
    const std::string DISPLAY_RES_Y_POSTFIX	    = "res/y";
    const std::string DISPLAY_MAJOR_POSTFIX     = "major";
    const std::string DISPLAY_MINOR_POSTFIX     = "minor";
    
    const std::string FEATURES                   = "features";
    const std::string FEATURES_WIFI              = "features/wifi";
    const std::string FEATURES_GPS               = "features/gps";
    const std::string FEATURES_BLUETOOTH         = "features/bluetooth";
    const std::string FEATURES_NFC               = "features/nfc";
    const std::string FEATURES_BACK_CAMERA       = "features/camera (rear)";
    const std::string FEATURES_FRONT_CAMERA      = "features/camera (face)";
    const std::string FEATURES_SIMCARDS          = "features/simcards";
    const std::string FEATURES_ACCELEROMETER     = "features/accelerometer";
    const std::string FEATURES_PEDOMETER         = "features/pedometer";
    const std::string FEATURES_THEMOMETER        = "features/thermometer";
    const std::string FEATURES_ALTIMETER         = "features/altimeter";
    const std::string FEATURES_BAROMETER         = "features/barometer";
    const std::string FEATURES_GYROSCOPE         = "features/gyroscope";
    const std::string FEATURES_COMPASS           = "features/compass";
    const std::string FEATURES_PROXIMITY         = "features/proximity";
    const std::string FEATURES_LIGHTSENSOR       = "features/lightsensor";
    const std::string FEATURES_MAJOR             = "features/major";
    const std::string FEATURES_MINOR             = "features/minor";
    const std::string FEATURES_SERVER            = "features/server";
    
    const std::string MEMORY                     = "memory";
    const std::string MEMORY_SIZE                = "memory/size";
    const std::string MEMORY_MAJOR               = "memory/major";
    const std::string MEMORY_MINOR               = "memory/minor";
    
    const std::string OS                 = "os";
    const std::string OS_BUILD           = "os/build";
    const std::string OS_NAME            = "os/name";
    const std::string OS_FINGERPRINT     = "os/fingerprint";
    const std::string OS_MAJOR           = "os/major";
    const std::string OS_MINOR           = "os/minor";
    const std::string OS_ARCH            = "os/arch";
    const std::string OS_RELEASE         = "os/release";
    const std::string OS_SHORT           = "os/short";
    
    // Not supported test list provided by server
    const std::string TEST_NOT_SUPPORTED = "server/not_supported";
    const std::string TEST_RESTRICTION_REASON = "server/restriction_reason";
    
    // Group keys will be used like this:
    // camera/0/camera_type = CAMERA + "/0/" + CAMERA_TYPE
    // helper function for creating these : utils.h/CreateIndexedKey
    const std::string CAMERA                     = "camera";
    const std::string CAMERA_COUNT               = "camera/count";
    const std::string CAMERA_TYPE                = "type";
    const std::string CAMERA_PIC_X               = "pic/x";
    const std::string CAMERA_PIC_Y               = "pic/y";
    const std::string CAMERA_VID_X               = "vid/x";
    const std::string CAMERA_VID_Y               = "vid/y";
    const std::string CAMERA_PIC_MP              = "pic/mp";
    const std::string CAMERA_VID_MP              = "vid/mp";
    
    const std::string CAMERA_HAS_FLASH           = "has/flash";
    const std::string CAMERA_HAS_AUTOFOCUS       = "has/autofocus";
    const std::string CAMERA_HAS_FACE_DETECTION  = "has/face_detection";
    const std::string CAMERA_HAS_TOUCH_FOCUS     = "has/touch_focus";
    const std::string CAMERA_HAS_GEO_TAGGING     = "has/geo_tagging";
    const std::string CAMERA_HAS_HDR             = "has/hdr";
    const std::string CAMERA_MAJOR               = "major";
    const std::string CAMERA_MINOR               = "minor";
    const std::string CAMERA_SERVER              = "server";
    
    
    const std::string GPU        = "gpu";
    const std::string GPU_COUNT  = "gpu/count";
    const std::string GPU_NAME   = "name";
    const std::string GPU_IDS    = "ids";
    const std::string GPU_MAJOR  = "major";
    const std::string GPU_MINOR  = "minor";
    const std::string GPU_VENDOR = "vendor";
    const std::string GPU_MEMORY = "memory";
    const std::string GPU_PCIE = "pcie";
    
    
    const std::string STORAGE                = "storage";
    const std::string STORAGE_MAJOR          = "storage/major";
    const std::string STORAGE_MINOR          = "storage/minor";
    const std::string STORAGE_MAJOR_POSTFIX  = "major";
    const std::string STORAGE_MINOR_POSTFIX  = "minor";
    const std::string STORAGE_COUNT          = "storage/count";
    const std::string STORAGE_SIZE           = "size";
    const std::string STORAGE_ISREMOVABLE    = "is/removable";
}

//----------------------------------------------------
// The client must fills these keys if he can:
//----------------------------------------------------
//"api/gl";
//"api/gl/name";
//"api/gl/version";
//"api/gl/features/GL_ALPHA_BITS";
//"api/gl/features/GL_BLUE_BITS";
//"api/gl/features/GL_DEPTH_BITS";
//"api/gl/features/GL_GREEN_BITS";
//"api/gl/features/GL_RED_BITS";
//"api/gl/features/GL_STENCIL_BITS";
//"api/gl/features/GL_ALIASED_LINE_WIDTH_MAX";
//"api/gl/features/GL_ALIASED_LINE_WIDTH_MIN";
//"api/gl/features/GL_ALIASED_POINT_SIZE_MAX";
//"api/gl/features/GL_ALIASED_POINT_SIZE_MIN";
//"api/gl/features/GL_EXTENSIONS";
//"api/gl/features/GL_RENDERER";
//"api/gl/features/GL_VENDOR";
//"api/gl/features/GL_VERSION";
//"api/gl/features/GL_MAX_3D_TEXTURE_SIZE";
//"api/gl/features/GL_MAX_ARRAY_TEXTURE_LAYERS";
//"api/gl/features/GL_MAX_COLOR_ATTACHMENTS";
//"api/gl/features/GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS";
//"api/gl/features/GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS";
//"api/gl/features/GL_MAX_COMBINED_UNIFORM_BLOCKS";
//"api/gl/features/GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS";
//"api/gl/features/GL_MAX_CUBE_MAP_TEXTURE_SIZE";
//"api/gl/features/GL_MAX_DRAW_BUFFERS";
//"api/gl/features/GL_MAX_ELEMENTS_VERTICES";
//"api/gl/features/GL_MAX_ELEMENT_INDEX";
//"api/gl/features/GL_MAX_ELEMENTS_INDICES";
//"api/gl/features/GL_MAX_FRAGMENT_INPUT_COMPONENTS";
//"api/gl/features/GL_MAX_FRAGMENT_UNIFORM_BLOCKS";
//"api/gl/features/GL_MAX_FRAGMENT_UNIFORM_VECTORS";
//"api/gl/features/GL_MAX_FRAGMENT_UNIFORM_COMPONENTS";
//"api/gl/features/GL_MAX_PROGRAM_TEXEL_OFFSET";
//"api/gl/features/GL_MAX_RENDERBUFFER_SIZE";
//"api/gl/features/GL_MAX_SAMPLES";
//"api/gl/features/GL_MAX_SERVER_WAIT_TIMEOUT";
//"api/gl/features/GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS";
//"api/gl/features/GL_MAX_UNIFORM_BLOCK_SIZE";
//"api/gl/features/GL_MAX_UNIFORM_BUFFER_BINDINGS";
//"api/gl/features/GL_MAX_VARYING_COMPONENTS";
//"api/gl/features/GL_MAX_VARYING_VECTORS";
//"api/gl/features/GL_MAX_VERTEX_ATTRIBS";
//"api/gl/features/GL_MAX_VERTEX_OUTPUT_COMPONENTS";
//"api/gl/features/GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS";
//"api/gl/features/GL_MAX_VERTEX_UNIFORM_BLOCKS";
//"api/gl/features/GL_MAX_VERTEX_UNIFORM_VECTORS";
//"api/gl/features/GL_MAX_VERTEX_UNIFORM_COMPONENTS";
//"api/gl/features/GL_MAX_TEXTURE_IMAGE_UNITS";
//"api/gl/features/GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS";
//"api/gl/features/GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS";
//"api/gl/features/GL_MIN_PROGRAM_TEXEL_OFFSET";
//"api/gl/features/GL_NUM_COMPRESSED_TEXTURE_FORMATS";
//"api/gl/features/GL_MAX_TEXTURE_SIZE";
//"api/gl/features/GL_MAX_VIEWPORT_HEIGHT";
//"api/gl/features/GL_MAX_VIEWPORT_WIDTH";
//
//
//"api/egl/name";
//"api/egl/version";
//"api/egl/EGL_CONFIGS";
//"api/egl/EGL_EXTENSIONS";
//"api/egl/EGL_VENDOR";
//"api/egl/EGL_VERSION";
//
//"api/cl/name";
//"api/cl/version";
//
//"api/dx/name";
//"api/dx/version";
//
//"battery/is/charging";    --bool
//"battery/is/connected";   --bool
//"battery/level";          --float (0-1)
//"battery/mah";            --int
//
//"chipset/name";
//
//"cpu/cores";              --int
//"cpu/frequency";          --int
//"cpu/name";
//
//"device/name";
//
//"display/diagonal";       --float
//"display/dpi/x";          --int
//"display/dpi/y";          --int
//"display/res/x";          --int
//"display/res/y";          --int
//
//"features/wifi";          --bool
//"features/gps";           --bool
//"features/bluetooth";     --bool
//"features/nfc";           --bool
//"features/camera/back";   --bool
//"features/camera/front";  --bool
//"features/simcards";      --bool
//"features/accelerometer"; --bool
//"features/pedometer";     --bool
//"features/thermometer";   --bool
//"features/altimeter";     --bool
//"features/barometer";     --bool
//"features/gyroscope";     --bool
//"features/compass";       --bool
//"features/proximity";     --bool
//"features/lightsensor";   --bool
//
//"memory/size";            --long
//
//"os/build";
//"os/name";
//"os/fingerprint";
//
//
//
// ? is a number
//
//"camera/count";
//"camera/?/type";          -- CAMERA_TYPE_BACK || CAMERA_TYPE_FRONT
//"camera/?/pic/x";         --int
//"camera/?/pic/y";         --int
//"camera/?/vid/x";         --int
//"camera/?/vid/y";         --int
//"camera/?/pic/mp";        --int
//"camera/?/vid/mp";        --int
//"camera/?/has/flash";     --bool
//"camera/?/has/autofocus"; --bool
//"camera/?/has/face_detection";    --bool
//"camera/?/has/touch_focus";       --bool
//"camera/?/has/geo_tagging";       --bool
//"camera/?/has/hdr";               --bool
//
//"gpu/count";              --int
//"gpu/?/name";
//"gpu/?/ids";
//
//"storage/count";          --int
//"storage/?/size";         --long
//"storage/?/is/removable"; --bool


//----------------------------------------------------
// The server should fill these values
//----------------------------------------------------
//"api/3d";
//"api/3d/major";
//"api/3d/minor";
//
//"battery/minor";
//
//"chipset/major";
//"chipset/minor";
//
//"cpu/major";
//"cpu/minor";
//
//"device/major";
//"device/minor";
//
//"display/major";
//"display/minor";
//
//"features/wifi";
//"features/gps";
//"features/bluetooth";
//"features/nfc";
//"features/camera/back";
//"features/camera/front";
//"features/simcards";
//"features/accelerometer";
//"features/pedometer";
//"features/thermometer";
//"features/altimeter";
//"features/barometer";
//"features/gyroscope";
//"features/compass";
//"features/proximity";
//"features/lightsensor";
//
//"memory/major";
//"memory/minor";
//
//"os/major";
//"os/minor";
//
//
//
// ? is a number
//
//"camera/count"
//"camera/?/has/flash";
//"camera/?/has/autofocus";
//"camera/?/has/face_detection";
//"camera/?/has/touch_focus";
//"camera/?/has/geo_tagging";
//"camera/?/has/hdr";
//"camera/?/major";
//"camera/?/minor";
//
//"gpu/count";
//"gpu/?/major";
//"gpu/?/minor";
//
//"storage/count";
//"storage/?/major";
//"storage/?/minor";

#endif /* defined(__GFXBench__SystemInfoProperties__) */
