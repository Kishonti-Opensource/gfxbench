/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GAUSS_BLUR_HELPER_H
#define GAUSS_BLUR_HELPER_H

#include <kcl_math3d.h>
#include <string>
#include <vector>

class GaussBlurHelper
{
public:
	static std::vector<float> GetGaussWeights(KCL::uint32 kernel_size, bool normalize);
	static std::vector<float> CalcOffsets(KCL::uint32 dimension_size, KCL::uint32 kernel_size);

	// Weights and offsets for linear samplers
	static std::vector<float> CalcPackedWeights(const std::vector<float> &gauss_weights);
	static std::vector<float> CalcPackedOffsets(KCL::uint32 dimension_size, KCL::uint32 kernel_size, const std::vector<float> &gauss_weights);

	static std::string GaussFloatListToString(const std::vector<float> &values, bool half_literals);
	static std::string GaussVector2DListToString(const std::vector<KCL::Vector2D> &values);
};

#endif