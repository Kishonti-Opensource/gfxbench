/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_PRECISION_H
#define GL_PRECISION_H

#include "test_base.h"
#include "kcl_base.h"
#include "kcl_math3d.h"
#include "opengl/glb_shader2.h"
#include "kcl_image.h"

class PrecisionTest : public GLB::TestBase
{
private:
	enum {
		PRECISION = 0,
		DENORMALS,
		MEDP_PSNR,
		HIGHP_PSNR
	};

    struct PrecisionScore
    {
        std::string m_name;
        std::string m_uom;
        KCL::uint32 m_value;

		PrecisionScore()
		{
			m_name = "";
			m_uom = "";
			m_value = 0;
		}
    };

    void saveReference();
    void renderFBO();

    static const int m_stride = 3 * sizeof(float); // distance between the beginning of vertices
    KCL::uint32 m_vertexBuffer;
    KCL::uint32 m_indexBuffer;
    
    //3 shaders
    KCL::uint32 m_shader_bars;
    KCL::uint32 m_shader_alu_mediump;
    KCL::uint32 m_shader_alu_highp;
	bool m_denormal_test_supported;

    //count of floating point precision bits
    //are denormals supported
    //mediump precision PSNR
    //highp precision PSNR
    static const KCL::uint32 m_numScores = 4;
    PrecisionScore m_precisionScores[m_numScores];
    
    KCL::Vector3D m_lightDir;
    KCL::Vector3D m_eyePosition;
    KCL::Matrix4x4 m_orientation;

    KCL::Image m_referenceImage;
    
    KCL::Vector4D m_quadrantScaleBias;

    double m_floatMinLimitsX;
    double m_floatMinLimitsY;

    KCL::int32 m_uniTimeLocation[2];
    KCL::int32 m_uniLightDirLocation[2];
    KCL::int32 m_uniPositionLocation[2];
    KCL::int32 m_uniOrientationLocation[2];
    KCL::int32 u_quadrantScaleBiasLocation[4];
    KCL::int32 m_uniResLocation[3]; //bar shader needs this too
    KCL::int32 m_expLimitLocation; //only bar shader needs this

    KCL::Vector2D m_resolution;


    GLB::FBO* m_1080pFBO;
    KCL::uint32 m_shader_fsq;
    KCL::int32  m_fsqSamplerLocation;
    KCL::uint32 m_sampler;

public:
    PrecisionTest(const GlobalTestEnvironment* const gte);
    virtual ~PrecisionTest();

protected:
    virtual const char* getUom() const { return "multiple"; }
    virtual float getScore() const { return 0.0; }
    virtual bool isWarmup() const { return false; }
    virtual KCL::uint32 indexCount() const { return 0; }

	void calcPrecisionBars(KCL::Image &img);

	KCL::uint32 initProgram(const char *vertSrc, const char *fragSrc, bool force_highp);
    virtual void finishTest();
    virtual KCL::KCL_Status init();
    virtual bool render();
    virtual bool animate(const int time) 
    { 
        return time < m_settings->m_play_time; 
    };

    virtual void getTestResult(PrintableResult** results, int* count) const;

    virtual void FreeResources();
};

#endif

