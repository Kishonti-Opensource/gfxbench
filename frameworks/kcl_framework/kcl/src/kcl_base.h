/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_BASE_H
#define KCL_BASE_H


#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

namespace KCL
{
	// false: clip space Z range is -1..1, true: Z range is 0..1
	extern bool zRangeZeroToOneGlobalOpt;

	// zRangeZeroToOne false: clip space Z range is -1..1, true: Z range is 0..1
	void Initialize(bool zRangeZeroToOne = false);
	void Release();

	enum KCL_Status
	{
		KCL_TESTERROR_NOERROR = 0,
		KCL_TESTERROR_OUT_OF_VMEMORY,
		KCL_TESTERROR_SHADER_ERROR,
		KCL_TESTERROR_FILE_NOT_FOUND,
		KCL_TESTERROR_UNKNOWNERROR,
		KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED,
		//GLB_TESTERROR_SKIPPED = 9,
		KCL_TESTERROR_OUT_OF_MEMORY = 10,
		KCL_TESTERROR_Z24_NOT_SUPPORTED,
		KCL_TESTERROR_BATTERYTEST_PLUGGED_ON_CHARGER,
		KCL_TESTERROR_VBO_ERROR,
		KCL_TESTERROR_UNSUPPORTED_TC_TYPE,
		KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED_IN_MSAA,
		KCL_TESTERROR_INVALID_SCREEN_RESOLUTION,
		KCL_TESTERROR_BATTERYTEST_BRIGHTNESS_CHANGED,
		KCL_TESTERROR_MOTIONBLUR_WITH_MSAA_NOT_SUPPORTED,
		KCL_TESTERROR_ES3_NOT_SUPPORTED,
		KCL_TESTERROR_BUILT_WITH_INCOMPATIBLE_ES_VERSION,
		KCL_TESTERROR_GUI_BENCHMARK_NOT_SUPPORTED,
		KCL_TESTERROR_DX_FEATURE_LEVEL,
		KCL_TESTERROR_REQUIRED_FSAA_LEVEL_NOT_SUPPORTED,
		KCL_TESTERROR_CANCELLED,
		KCL_TESTERROR_INVALID_SCENE_VERSION,
		KCL_TESTERROR_INIT_RENDER_API,
		KCL_TESTERROR_NETWORK_ERROR,
		KCL_TESTERROR_MAX
	};
	inline const char* KCL_Status_To_Cstr(const KCL_Status value)
	{
		switch(value)
		{
		case KCL_TESTERROR_NOERROR:
			return "";
		case KCL_TESTERROR_OUT_OF_VMEMORY:
			return "OUT_OF_VMEMORY";
		case KCL_TESTERROR_SHADER_ERROR:
			return "SHADER_ERROR";
		case KCL_TESTERROR_FILE_NOT_FOUND:
			return "FILE_NOT_FOUND";
		case KCL_TESTERROR_UNKNOWNERROR:
			return "UNKNOWNERROR";
		case KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED:
			return "OFFSCREEN_NOT_SUPPORTED";
	//	case GLB_TESTERROR_SKIPPED:
	//		return "SKIPPED";
		case KCL_TESTERROR_OUT_OF_MEMORY:
			return "OUT_OF_MEMORY";
		case KCL_TESTERROR_BATTERYTEST_PLUGGED_ON_CHARGER:
			return "BATTERYTEST_PLUGGED_ON_CHARGER";
		case KCL_TESTERROR_VBO_ERROR:
			return "VBO_ERROR";
		case KCL_TESTERROR_UNSUPPORTED_TC_TYPE:
			return "UNSUPPORTED_TC_TYPE";
		case KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED_IN_MSAA:
			return "OFFSCREEN_NOT_SUPPORTED_IN_MSAA";
		case KCL_TESTERROR_INVALID_SCREEN_RESOLUTION:
			return "INVALID_SCREEN_RESOLUTION";
		case KCL_TESTERROR_BATTERYTEST_BRIGHTNESS_CHANGED:
			return "BATTERYTEST_BRIGHTNESS_CHANGED";
		case KCL_TESTERROR_MOTIONBLUR_WITH_MSAA_NOT_SUPPORTED:
			return "MOTIONBLUR_WITH_MSAA_NOT_SUPPORTED";
		case KCL_TESTERROR_ES3_NOT_SUPPORTED:
			return "ES3_NOT_SUPPORTED";
		case KCL_TESTERROR_BUILT_WITH_INCOMPATIBLE_ES_VERSION:
			return "BUILT_WITH_INCOMPATIBLE_ES_VERSION";
		case KCL_TESTERROR_GUI_BENCHMARK_NOT_SUPPORTED:
			return "GUI_BENCHMARK_NOT_SUPPORTED";
		case KCL_TESTERROR_DX_FEATURE_LEVEL:
			return "DX_FEATURE_LEVEL";
		case KCL_TESTERROR_REQUIRED_FSAA_LEVEL_NOT_SUPPORTED:
			return "REQUIRED_FSAA_LEVEL_NOT_SUPPORTED";
		case KCL_TESTERROR_CANCELLED:
			return "";
		case KCL_TESTERROR_INVALID_SCENE_VERSION:
			return "Invalid scene version";
		case KCL_TESTERROR_INIT_RENDER_API:
			return "Unable to initalize GPU context";
		default:
			return "UNDEFINED_ERROR";
		}
	}

	#define KCL_API

	typedef unsigned long long uint64;
	typedef signed long long int64;
	typedef signed int		int32;
	typedef unsigned int	uint32;
	typedef signed short	int16;
	typedef unsigned short	uint16;
	typedef signed char		int8;
	typedef unsigned char	uint8;
    typedef unsigned int	enum_t;

	// kcl_object
	class Object;

	// kcl_node
	class Node;

	// kcl_light
	class Light;

	// kcl_camera
	class Camera2;

	// kcl_math3d
	class Vector2D;
	class Vector3D;
	class Vector4D;
	class Matrix4x4;

	// kcl_mesh
	class Mesh;
	class Mesh3;

	// kcl_image
	class Image2D;
	class ImageCube;

	// kcl_material
	class Material;

	// kcl_scene_handler
	class SceneHandler;

	// kcl_room
	class XRoom;
	class XPortal;

	// kcl_planarmap
	class PlanarMap;

	// kcl_particlesystem
	class AnimatedEmitter;

	// kcl_aabb
	class AABB; 

	// kcl_actor
	class Actor;

	//kcl_animation4
	struct _key_node;

	class OS;
	
	class AssetFile;
    
    
    template <typename T>
    void FreeContainerData(T & t)
    {
        T tmp;
        t.clear() ;
        t.swap( tmp ) ;
    }
}

#endif
