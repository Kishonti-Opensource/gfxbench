/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_5_TOOLS_H
#define GFXB_5_TOOLS_H

#include <ovr/Include/VrApi.h>
#include <kcl_math3d.h>

namespace GFXB
{
	class OvrTools
	{
	public:
		static inline void GetKCLMatrix(const ovrMatrix4f &ovr, KCL::Matrix4x4 &kcl)
		{
			kcl.v[0] = ovr.M[0][0];
			kcl.v[1] = ovr.M[1][0];
			kcl.v[2] = ovr.M[2][0];
			kcl.v[3] = ovr.M[3][0];

			kcl.v[4] = ovr.M[0][1];
			kcl.v[5] = ovr.M[1][1];
			kcl.v[6] = ovr.M[2][1];
			kcl.v[7] = ovr.M[3][1];

			kcl.v[8] = ovr.M[0][2];
			kcl.v[9] = ovr.M[1][2];
			kcl.v[10] = ovr.M[2][2];
			kcl.v[11] = ovr.M[3][2];

			kcl.v[12] = ovr.M[0][3];
			kcl.v[13] = ovr.M[1][3];
			kcl.v[14] = ovr.M[2][3];
			kcl.v[15] = ovr.M[3][3];

			//memcpy(kcl.v, ovr.M, 4 * 16);
		}

		static inline void GetOVRMatrix(const KCL::Matrix4x4 &kcl, ovrMatrix4f &ovr)
		{
			ovr.M[0][0] = kcl.v[0];
			ovr.M[1][0] = kcl.v[1];
			ovr.M[2][0] = kcl.v[2];
			ovr.M[3][0] = kcl.v[3];

			ovr.M[0][1] = kcl.v[4];
			ovr.M[1][1] = kcl.v[5];
			ovr.M[2][1] = kcl.v[6];
			ovr.M[3][1] = kcl.v[7];

			ovr.M[0][2] = kcl.v[8];
			ovr.M[1][2] = kcl.v[9];
			ovr.M[2][2] = kcl.v[10];
			ovr.M[3][2] = kcl.v[11];

			ovr.M[0][3] = kcl.v[12];
			ovr.M[1][3] = kcl.v[13];
			ovr.M[2][3] = kcl.v[14];
			ovr.M[3][3] = kcl.v[15];

			//memcpy(ovr.M, kcl.v, 4 * 16);
		}
	};
}

#endif