/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef UBO_CPP_DEFINES
#define UBO_CPP_DEFINES
template<typename T>
struct Vector4
{
   union
	{
		T v[4];
		struct
		{
			T x, y, z, w;
		};
	};
    Vector4 () : x(0), y(0), z(0), w(0) {}
	explicit Vector4 (const Vector4<T> &v) : x(v.x), y(v.x), z(v.z), w(v.w) {}
	Vector4(T nx, T ny, T nz, T nw) : x(nx), y(ny), z(nz), w(nw) {}
};
template<typename T>
struct Vector3
{
   union
	{
		T v[3];
		struct
		{
			T x, y, z;
		};
	};
    Vector3 () : x(0), y(0), z(0) {}
	explicit Vector3 (const Vector4<T> &v) : x(v.x), y(v.x), z(v.z) {}
	Vector3(T nx, T ny, T nz) : x(nx), y(ny), z(nz) {}
};
template<typename T>
struct Vector2
{
   union
	{
		T v[2];
		struct
		{
			T x, y;
		};
	};
    Vector2 () : x(0), y(0) {}
	explicit Vector2 (const Vector2<T> &v) : x(v.x), y(v.x) {}
	Vector2(T nx, T ny) : x(nx), y(ny) {}
};
 
#define layout(...) struct
#define uniform
#define lowp
#define mediump
#define highp
typedef KCL::uint32 uint;
typedef KCL::Vector2D vec2;
typedef KCL::Vector3D vec3;
typedef KCL::Vector4D vec4;
typedef Vector2<KCL::int32> ivec2;
typedef Vector3<KCL::int32> ivec3;
typedef Vector4<KCL::int32> ivec4;
typedef Vector2<KCL::uint32> uvec2;
typedef Vector3<KCL::uint32> uvec3;
typedef Vector4<KCL::uint32> uvec4;
typedef KCL::Matrix4x4 mat4;

#endif //UBO_CPP_DEFINES
