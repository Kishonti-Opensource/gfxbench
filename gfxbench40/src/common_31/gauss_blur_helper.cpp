/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gauss_blur_helper.h"
#include <sstream>
#include <iomanip> //std::setprecision

std::vector<float> COMMON31::GaussBlurHelper::GetGaussWeights(KCL::uint32 kernel_size, bool normalize)
{
	int gauss_weights_count = 2*kernel_size+1;

	std::vector<float> gauss_weights;
    double sigma = (1.0*kernel_size)/3.0;   
	for (KCL::int32 i = -KCL::int32(kernel_size); i <= KCL::int32(kernel_size); i++)
    {
		float w = exp(-0.5*i*i/(sigma*sigma));
		gauss_weights.push_back(w);	
	}

    if (normalize)
    {
        double sum = 0.0;
        for (KCL::uint32 i = 0; i < gauss_weights.size(); i++)
        {
            sum += gauss_weights[i];
        }
        for (KCL::uint32 i = 0; i < gauss_weights.size(); i++)
        {
            gauss_weights[i] /= sum;
        }
    }

	return gauss_weights;
}

std::vector<float> COMMON31::GaussBlurHelper::CalcOffsets(KCL::uint32 dimension_size, KCL::uint32 kernel_size)
{
    std::vector<float> results;
    results.resize(2*kernel_size+1);

    float pixel_size = 1.0f / dimension_size;
    for (KCL::int32 i = 0; i <= 2*kernel_size; i++)
    {
        results[i] = (i - KCL::int32(kernel_size)) * pixel_size;
    }

    return results;
}

std::vector<float> COMMON31::GaussBlurHelper::CalcPackedOffsets(KCL::uint32 dimension_size, KCL::uint32 kernel_size, const std::vector<float> &gauss_weights)
{
    std::vector<float> offsets = CalcOffsets(dimension_size,kernel_size);

    std::vector<float> results;
    for (KCL::uint32 i = 0; i < gauss_weights.size() / 2; i++)
    {
        KCL::uint32 i1 = 2*i+0;
        KCL::uint32 i2 = 2*i+1;

        float w1 = gauss_weights[i1];
        float w2 = gauss_weights[i2];

        float o1 = offsets[i1];
        float o2 = offsets[i2];

        float o12 = (o1*w1+o2*w2)/(w1+w2);

        results.push_back(o12);
    }
    results.push_back(offsets[offsets.size()-1]);

    return results;
}

std::vector<float> COMMON31::GaussBlurHelper::CalcPackedWeights(const std::vector<float> &gauss_weights)
{
    std::vector<float> packed_weights;

    for (KCL::uint32 i = 0; i < gauss_weights.size()/2; i++)
    {
        packed_weights.push_back( gauss_weights[2*i+0] + gauss_weights[2*i+1]);
    }
    packed_weights.push_back(gauss_weights[gauss_weights.size()-1]);

    return packed_weights;
}

std::string COMMON31::GaussBlurHelper::GaussFloatListToString(std::vector<float> values)
{
    std::stringstream sstream;
    std::vector<float>::iterator it = values.begin();

    sstream<<std::fixed<<std::setprecision(10)<<*it;
    it++;

    for( ; it != values.end(); it++)
    {
        sstream<<", "<<*it;
    }

    return sstream.str();
}

std::string COMMON31::GaussBlurHelper::GaussVector2DListToString(const std::vector<KCL::Vector2D> & values)
{
    std::stringstream sstream;
    std::vector<KCL::Vector2D>::const_iterator it = values.begin();

    sstream<<std::fixed<<std::setprecision(10)<<"vec2("<<it->x<<","<<it->y<<")";
    it++;

    for( ; it != values.end(); it++)
    {
        sstream<<", "<<"vec2("<<it->x<<","<<it->y<<")";
    }

    return sstream.str();
}
