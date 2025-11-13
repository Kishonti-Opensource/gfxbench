/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gl_precision.h"
#include "kcl_image.h"
#include "kcl_io.h"
#include "platform.h"
#include "opengl/glbshader.h"
#include "opengl/glb_opengl_state_manager.h"
#include "kcl_math3d.h"
#include "opengl/misc2_opengl.h"
#include "opengl/fbo.h"

#include "qualitymatch.h"
#include "ng/log.h"


#define SAVE_REFERENCE_IMPLEMENTATION 0 // only works with C++11

void SaveImage(KCL::Image &img, const char* name)
{
	std::string path = KCL::File::GetDataRWPath();
	char filename[2048];
	sprintf(filename, "%s/%s", path.c_str(), name);
	img.saveTga(filename, true);
}

#if 0
void DumpImage(KCL::Image &img, const char* name)
{
	SaveImage(img, name);
}
#else
void DumpImage(KCL::Image &img, const char* name) {}
#endif


PrecisionTest::PrecisionTest(const GlobalTestEnvironment* const gte) : TestBase(gte),
m_vertexBuffer(0),
m_indexBuffer(0),
m_shader_bars(0),
m_shader_alu_mediump(0),
m_shader_alu_highp(0),
m_resolution(0.0f,0.0f),
m_denormal_test_supported(true),
m_quadrantScaleBias(1.0f, 1.0f, 0.0f, 0.0f)
{
    m_precisionScores[0].m_name = "float_precision_bits";
    m_precisionScores[0].m_uom = "";

    m_precisionScores[1].m_name = "denormal_bits";
    m_precisionScores[1].m_uom = "";

    m_precisionScores[2].m_name = "mediump_precision_PSNR";
    m_precisionScores[2].m_uom = "mB";

    m_precisionScores[3].m_name = "highp_precision_PSNR";
    m_precisionScores[3].m_uom = "mB";
}


PrecisionTest::~PrecisionTest()
{
    FreeResources();
}

KCL::KCL_Status PrecisionTest::init()
{
    KCL::AssetFile vertex_file("shaders_40/lowlevel4/precision.vs");
    KCL::AssetFile fragment_file_bars("shaders_40/lowlevel4/precision_bars.fs");
    KCL::AssetFile fragment_file_alu("shaders_40/lowlevel4/precision_alu.fs");
    KCL::AssetFile fragment_file_fsq("shaders_40/lowlevel4/precision_fsq.fs");

	bool ok = (vertex_file.GetLastError() == KCL::KCL_IO_NO_ERROR) && (fragment_file_alu.GetLastError() == KCL::KCL_IO_NO_ERROR) &&
		(fragment_file_bars.GetLastError() == KCL::KCL_IO_NO_ERROR) && (fragment_file_fsq.GetLastError() == KCL::KCL_IO_NO_ERROR);

	if (!ok)
	{
		NGLOG_ERROR("Precision test. Shader not found.");
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

    const char *vertexShader = vertex_file.GetBuffer();
    const char *fragmentShader_bars = fragment_file_bars.GetBuffer();
    const char *fragmentShader_alu = fragment_file_alu.GetBuffer();
    std::string fragmentShader_fsq = fragment_file_fsq.GetBuffer();

	std::string fragmentShader_bars_with_denormals = std::string("#define TEST_DENORMALS\n") + fragmentShader_bars;
	std::string fragmentShader_bars_without_denormals = fragmentShader_bars;

	m_shader_bars = initProgram(vertexShader, fragmentShader_bars_with_denormals.c_str(), true); //force highp
	if (m_shader_bars == 0)
	{
		NGLOG_INFO("Denormal test unsupported");
		m_denormal_test_supported = false;
		m_shader_bars = initProgram(vertexShader, fragmentShader_bars_without_denormals.c_str(), true); //force highp
	}

    m_shader_alu_mediump = initProgram(vertexShader, fragmentShader_alu, false); //keep mediump
    m_shader_alu_highp = initProgram(vertexShader, fragmentShader_alu, true); //force highp
    
    bool landscape = m_window_height <= m_window_width || (GetSetting().GetScreenMode() != 0) || GetSetting().m_virtual_resolution;
    if (!landscape)
    {
        fragmentShader_fsq = std::string("#define ROTATE_RESULTS 1\n") + fragmentShader_fsq ;
    }
    m_shader_fsq = initProgram(vertexShader, fragmentShader_fsq.c_str(), false); //just for display, precision irrelevant here

    if (!m_shader_bars || !m_shader_alu_mediump || !m_shader_alu_highp || !m_shader_fsq)
    {
		NGLOG_ERROR("Precision test. Shader compile error.");
        return KCL::KCL_TESTERROR_SHADER_ERROR;
    }

    m_floatMinLimitsX = -152.0;
    m_floatMinLimitsY = -120.0f;

    m_resolution = KCL::Vector2D(1920, 1080); // m_settings->m_viewport_width, m_settings->m_viewport_height);

    m_uniResLocation[2] = glGetUniformLocation(m_shader_bars, "resolution");
    u_quadrantScaleBiasLocation[2] = glGetUniformLocation(m_shader_bars, "u_quadrantScaleBias");
    m_expLimitLocation = glGetUniformLocation(m_shader_bars, "exp_limits");

    m_uniResLocation[0] = glGetUniformLocation(m_shader_alu_mediump, "resolution");
    m_uniTimeLocation[0] = glGetUniformLocation(m_shader_alu_mediump, "u_time");
    m_uniLightDirLocation[0] = glGetUniformLocation(m_shader_alu_mediump, "u_lightDir");
    m_uniPositionLocation[0] = glGetUniformLocation(m_shader_alu_mediump, "u_eyePosition");
    m_uniOrientationLocation[0] = glGetUniformLocation(m_shader_alu_mediump, "u_orientation");
    u_quadrantScaleBiasLocation[0] = glGetUniformLocation(m_shader_alu_highp, "u_quadrantScaleBias");

    m_uniResLocation[1] = glGetUniformLocation(m_shader_alu_highp, "resolution");
    m_uniTimeLocation[1] = glGetUniformLocation(m_shader_alu_highp, "u_time");
    m_uniLightDirLocation[1] = glGetUniformLocation(m_shader_alu_highp, "u_lightDir");
    m_uniPositionLocation[1] = glGetUniformLocation(m_shader_alu_highp, "u_eyePosition");
    m_uniOrientationLocation[1] = glGetUniformLocation(m_shader_alu_highp, "u_orientation");
    u_quadrantScaleBiasLocation[1] = glGetUniformLocation(m_shader_alu_highp, "u_quadrantScaleBias");


    m_fsqSamplerLocation = glGetUniformLocation(m_shader_fsq, "texIn");
    u_quadrantScaleBiasLocation[3] = glGetUniformLocation(m_shader_fsq, "u_quadrantScaleBias");

    m_1080pFBO = new GLB::FBO(m_resolution.x, m_resolution.y, 0, GLB::RGB888_Nearest, GLB::DEPTH_None, "");

    if (glGetError())
    {
        return KCL::KCL_TESTERROR_SHADER_ERROR;
    }

    //bottom-left quadrant
    static const float screenBillboard[] =
    {
        -1.0, -1.0, 0,
        -1.0, 0.0, 0,
        0.0, -1.0, 0,
        0.0, 0.0, 0,
    };

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData_chunked(GL_ARRAY_BUFFER, sizeof(float) * 12, (void*)screenBillboard, GL_STATIC_DRAW);
    if (glGetError())
    {
        return KCL::KCL_TESTERROR_VBO_ERROR;
    }

    static const KCL::uint16 billboardIndices[] = { 0, 1, 2, 1, 2, 3 };
    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData_chunked(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint16) * 6, (void*)billboardIndices, GL_STATIC_DRAW);
    if (glGetError())
    {
        return KCL::KCL_TESTERROR_VBO_ERROR;
    }

    //load ref image
    if (m_referenceImage.load("common/ref_precision.png"))
    {
        if (!m_referenceImage.convertTo(KCL::Image_RGB888))
        {
            SetRuntimeError(KCL::KCL_TESTERROR_UNKNOWNERROR);
            return KCL::KCL_TESTERROR_UNKNOWNERROR;
        }
    }

	m_referenceImage.flipY();

    return KCL::KCL_TESTERROR_NOERROR;
}


KCL::uint32 PrecisionTest::initProgram(const char *vertSrc, const char *fragSrc, bool force_highp)
{
	std::string vertSrcString;
	std::string fragSrcString;
	if (force_highp)
	{
		vertSrcString = fragSrcString = "#ifdef GL_ES\nprecision highp float;\n#endif\n#define HIGHP\n";
	}
	vertSrcString += vertSrc;
	fragSrcString += fragSrc;
	
	return GLB::initProgram(vertSrcString.c_str(), fragSrcString.c_str(), true, force_highp);
}


void PrecisionTest::renderFBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_stride, 0);

    /////////////////////////////////////////
    //DRAW BARS
    /////////////////////////////////////////
    GLB::OpenGLStateManager::GlUseProgram(m_shader_bars);

    glUniform2fv(m_uniResLocation[2], 1, m_resolution.v);
    glUniform2fv(m_expLimitLocation, 1, KCL::Vector2D(m_floatMinLimitsX, m_floatMinLimitsY).v);

    //left bars
    m_quadrantScaleBias = KCL::Vector4D(1.0, 1.0, 0.0, 1.0);
    glUniform4fv(u_quadrantScaleBiasLocation[2], 1, m_quadrantScaleBias.v);
    GLB::OpenGLStateManager::Commit();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    //right bars
    m_quadrantScaleBias = KCL::Vector4D(1.0, 1.0, 1.0, 1.0);
    glUniform4fv(u_quadrantScaleBiasLocation[2], 1, m_quadrantScaleBias.v);

    GLB::OpenGLStateManager::Commit();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    /////////////////////////////////////////
    //      DRAW MEDIUMP ALU
    /////////////////////////////////////////
    GLB::OpenGLStateManager::GlUseProgram(m_shader_alu_mediump);

    glUniform2fv(m_uniResLocation[0], 1, m_resolution.v);
    int fixtime = 11800;

    float time = (fixtime % 1000) / 1000.0f;
    glUniform1f(m_uniTimeLocation[0], time);

    float lightTime = fixtime * 0.0003;
    m_lightDir = KCL::Vector3D(sin(lightTime), 0.5 + 0.5 * cos(lightTime), 0.5 - 0.5 * cos(lightTime));
    m_lightDir.normalize();
    glUniform3fv(m_uniLightDirLocation[0], 1, m_lightDir);

    float flyTime = fixtime * 0.0005;
    m_eyePosition = KCL::Vector3D(10 * sin(flyTime), sin(flyTime * 0.5) * 2 + 0.1, 10 * cos(flyTime));
    glUniform3fv(m_uniPositionLocation[0], 1, m_eyePosition.v);

    KCL::Matrix4x4 yaw, pitch, roll;
    KCL::Matrix4x4::RotateY(yaw, -cos(flyTime) * 28);
    KCL::Matrix4x4::RotateX(pitch, -cos(flyTime * 0.5) * 28);
    KCL::Matrix4x4::RotateZ(roll, -sin(flyTime) * 28);

    m_orientation = pitch * roll * yaw;

    glUniformMatrix4fv(m_uniOrientationLocation[0], 1, false, m_orientation);

    //left alu @mediump
    m_quadrantScaleBias = KCL::Vector4D(1.0, 1.0, 0.0, 0.0);
    glUniform4fv(u_quadrantScaleBiasLocation[0], 1, m_quadrantScaleBias.v);

    GLB::OpenGLStateManager::Commit();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);


    /////////////////////////////////////////
    //      DRAW HIGHP ALU
    /////////////////////////////////////////
    GLB::OpenGLStateManager::GlUseProgram(m_shader_alu_highp);
    glUniform2fv(m_uniResLocation[1], 1, m_resolution.v);
    glUniform1f(m_uniTimeLocation[1], time);
    glUniform3fv(m_uniLightDirLocation[1], 1, m_lightDir);
    glUniform3fv(m_uniPositionLocation[1], 1, m_eyePosition.v);
    glUniformMatrix4fv(m_uniOrientationLocation[1], 1, false, m_orientation);

    //right alu @highp
    m_quadrantScaleBias = KCL::Vector4D(1.0, 1.0, 1.0, 0.0);
    glUniform4fv(u_quadrantScaleBiasLocation[1], 1, m_quadrantScaleBias.v);

    GLB::OpenGLStateManager::Commit();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

bool PrecisionTest::render()
{
    GLB::OpenGLStateManager::DisableAllCapabilites();
    GLB::OpenGLStateManager::DisableAllVertexAttribs();
    GLB::OpenGLStateManager::GlEnableVertexAttribArray(0);
    GLB::OpenGLStateManager::GlDisable(GL_BLEND);
    GLB::OpenGLStateManager::Commit();

    GLB::FBO::bind(m_1080pFBO);
    glViewport(0, 0, m_1080pFBO->getWidth(), m_1080pFBO->getHeight());
    glClearColor(0.25f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderFBO();

    // rendering main
    GLB::FBO::bind(0);
    glViewport(0, 0, GetSetting().m_viewport_width, GetSetting().m_viewport_height);
    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_stride, 0);

    GLB::OpenGLStateManager::GlUseProgram(m_shader_fsq);
    GLB::OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_1080pFBO->getTextureName());
    glUniform1i(m_fsqSamplerLocation, 0);
    m_quadrantScaleBias = KCL::Vector4D(2.0, 2.0, 1.0, 1.0); //make the final quad cover the whole display
    //m_quadrantScaleBias = KCL::Vector4D(1.0, 1.0, 0.0, 0.0);
    glUniform4fv(u_quadrantScaleBiasLocation[3], 1, m_quadrantScaleBias.v);

    GLB::OpenGLStateManager::Commit();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLB::OpenGLStateManager::GlDisableVertexAttribArray(0);

    return true;
}

void PrecisionTest::getTestResult(PrintableResult** results, int* count) const
{
    *count = m_numScores;
    if (!results)
    {
        return;
    }

    *results = new PrintableResult[m_numScores];

    for (int i = 0; i < m_numScores; ++i)
    {
        PrintableResult performanceResult(
            getTextureType(),
            m_precisionScores[i].m_value,
            KCL_Status_To_Cstr(m_runtime_error),
            -1.0f,
            m_precisionScores[i].m_uom,
            false,
            false,
            m_precisionScores[i].m_name,
            GetFrameStepTime(),
            m_settings->GetScreenMode() ? m_settings->GetTestWidth() : getViewportWidth(),
            m_settings->GetScreenMode() ? m_settings->GetTestHeight() : getViewportHeight(),
            -1,
            -1,
            -1.0f,
            m_runtime_error,
            std::vector<std::string>());

        (*results)[i] = performanceResult;
    }
}


void PrecisionTest::calcPrecisionBars(KCL::Image &img)
{
	const KCL::uint32 image_width = m_resolution.x;
	const KCL::uint32 image_height = m_resolution.y;

	m_precisionScores[PRECISION].m_value = 0;
	m_precisionScores[DENORMALS].m_value = 0;

	// calc floating point precision bars
	{
		const KCL::uint32 bar_count = 26;
		double bar_height = (1.0*image_height / 2.0) / bar_count;

		for (KCL::uint32 i = 0; i < bar_count; i++)
		{
			KCL::uint32 x = bar_height / 2.0;
			KCL::uint32 y = bar_height / 2.0 + i * bar_height;

			KCL::uint32 idx = y * img.getLinePitch() + x * img.getBpp() / 8; // left bars position

			KCL::uint8* img_data = (KCL::uint8*)img.getData();

			KCL::uint8 r = img_data[idx + 0];
			KCL::uint8 g = img_data[idx + 1];
			KCL::uint8 b = img_data[idx + 2];

			const int thres = 1;
			if ( !((r < thres) && (g < thres) && (b < thres)))
			{
				m_precisionScores[PRECISION].m_value++;
			}
		}
	}

	if (!m_denormal_test_supported)
	{
		return;
	}

	// calc denormal supported
	{
		const KCL::uint32 bar_count = m_floatMinLimitsY - m_floatMinLimitsX;
		double bar_height = (1.0*image_height / 2.0) / bar_count;

		for (KCL::uint32 i = 0; i < bar_count; i++)
		{
			KCL::uint32 x = bar_height / 2.0;
			KCL::uint32 y = bar_height / 2.0 + i * bar_height;

			KCL::uint32 idx = y * img.getLinePitch() + (x + image_width / 2) * img.getBpp() / 8; // left bars position

			KCL::uint8* img_data = (KCL::uint8*)img.getData();

			KCL::uint8 r = img_data[idx + 0];
			KCL::uint8 g = img_data[idx + 1];
			KCL::uint8 b = img_data[idx + 2];

			const int thres = 5;
			if ( !((r > 255 - thres) && (g < thres) && (b < thres)) )
			{
				m_precisionScores[DENORMALS].m_value++;
			}
		}
	}

	if (m_precisionScores[DENORMALS].m_value >= 6)
	{
		m_precisionScores[DENORMALS].m_value -= 6;
	}
	else
	{
		m_precisionScores[DENORMALS].m_value = 0;
	}
}


void PrecisionTest::finishTest()
{
	TestBase::finishTest();

	GLB::FBO* backupfbo = GLB::FBO::GetGlobalFBO();
	GLB::FBO::SetGlobalFBO(m_1080pFBO);
	GLB::FBO::bind(0);

	while (glGetError() != GL_NO_ERROR);

	// get precision and denormal support result
	{
		//only gather upper half
		KCL::Image upper_half;
		upper_half.Allocate2D(m_resolution.x, m_resolution.y / 2, KCL::Image_RGBA8888);
		glReadPixels(0, m_resolution.y / 2, m_resolution.x, m_resolution.y / 2, GL_RGBA, GL_UNSIGNED_BYTE, upper_half.getData());

		calcPrecisionBars(upper_half);

		DumpImage(upper_half, "qc_TEST_upper_half.tga");
	}

	// get mediump alu result
	{
		KCL::Image bottom_left_mediump_alu;
		bottom_left_mediump_alu.Allocate2D(m_resolution.x / 2, m_resolution.y / 2, KCL::Image_RGBA8888);
		glReadPixels(0, 0, m_resolution.x / 2, m_resolution.y / 2, GL_RGBA, GL_UNSIGNED_BYTE, bottom_left_mediump_alu.getData());
		DumpImage(bottom_left_mediump_alu, "qc_TEST_bottom_left_mediump_alu.tga");

		bottom_left_mediump_alu.convertTo(KCL::Image_RGB888);

		QCRESULTSTRUCT res;
		bool ok = QualityMatch::GetQCValues(m_referenceImage, bottom_left_mediump_alu, res);
		m_precisionScores[MEDP_PSNR].m_value = res.PSNR;

		if (!ok)
		{
			NGLOG_ERROR("Image compare failed! Precision mediump alu.");
		}
	}

	// get highp alu result
	{
		KCL::Image bottom_right_highp_alu;
		bottom_right_highp_alu.Allocate2D(m_resolution.x / 2, m_resolution.y / 2, KCL::Image_RGBA8888);
		glReadPixels(m_resolution.x / 2, 0, m_resolution.x / 2, m_resolution.y / 2, GL_RGBA, GL_UNSIGNED_BYTE, bottom_right_highp_alu.getData());
		DumpImage(bottom_right_highp_alu, "qc_TEST_bottom_right_highp_alu.tga");

		bottom_right_highp_alu.convertTo(KCL::Image_RGB888);

		QCRESULTSTRUCT res;
		bool ok = QualityMatch::GetQCValues(m_referenceImage, bottom_right_highp_alu, res);
		m_precisionScores[HIGHP_PSNR].m_value = res.PSNR;

		if (!ok)
		{
			NGLOG_ERROR("Image compare failed! Precision highp alu.");
		}
	}

	GLB::FBO::SetGlobalFBO(backupfbo);
	GLB::FBO::bind(0);

	saveReference();
}


#if SAVE_REFERENCE_IMPLEMENTATION
#pragma optimize( "", off )

#define vec2 KCL::dVector2D
#define vec3 KCL::dVector3D
#define vec4 KCL::dVector4D

KCL::uint8 ConvertToByte(double di)
{
    di = fmin(di, 1.0); di = fmax(di, 0.0);

    //float-to-int rule according to ES 3.1 spec
    return (KCL::uint8)(round(di * 255.0) + 0.5);
}

double fract(double a)
{
    return a - floor(a);
}

double rand2D(vec2 co)
{
    return fract(sin(KCL::dVector2D::dot(co, vec2(2.9898, 8.233))) * 8.5453);
}

// Returns a pseudo-random number
double rand2D_C1(vec2 co)
{
    vec2 flr = vec2(floor(co.x), floor(co.y));
    vec2 frc = co - flr;
    double v0 = rand2D(flr);
    double v1 = rand2D(flr + vec2(1.0, 0.0));
    double mix1 = KCL::Math::interpolate(v0, v1, frc.x);
    double v2 = rand2D(flr + vec2(0.0, 1.0));
    double v3 = rand2D(flr + vec2(1.0, 1.0));
    double mix2 = KCL::Math::interpolate(v2, v3, frc.x);
    return KCL::Math::interpolate(mix1, mix2, frc.y);
}

double step(double edge, double x)
{
    if (x < edge)
        return 0.0;
    return 1.0;
}

vec3 getSkyColor(vec3 direction, vec3 lightdir)
{
    double cosDir = KCL::dVector3D::dot(direction, lightdir) * 0.5 + 0.5;
    vec3 horizon = pow(cosDir, 0.3) * KCL::dVector3D::interpolate(vec3(1.0, 0.75, 0.2), vec3(0.5, 0.9, 1.0), lightdir.y);
    vec3 zenit = KCL::dVector3D::interpolate(vec3(0.0, 0.2, 0.4), vec3(0.5, 0.9, 1.0), lightdir.y);
    vec3 sky = KCL::dVector3D::interpolate(horizon, zenit, direction.y);
    vec3 direct = vec3(1.0, 1.0, 1.0) * (0.1 * cosDir + 0.5 * pow(cosDir, 16.0) + step(0.999, cosDir));
    vec3 light = sky + direct;
    return light;
}

vec3 getWaterNormal(vec2 coords, double distance, double time)
{
    double t = time * 0.002;
    double r1 = rand2D_C1(coords * vec2(0.9, 1.8)) *6.28318;
    double r2 = rand2D_C1(coords * vec2(2.0, 4.0)) *6.28318;
    double nx = cos(r1 + t) + 0.5 * cos(r2 + t * 2.0);
    double ny = sin(r1 + t) + 0.5 * sin(r2 + t * 2.0) + 0.5 * sin(coords.y + cos(coords.x) - t);
    vec3 retval = vec3(nx, 0.01 + 0.1 * pow(distance, 1.5), ny);
    retval.normalize();
    return retval;
}

vec3 getGroundColor(vec2 coords)
{
    vec2 c;
    c.x = step(0.5, fract(coords.x));
    c.y = step(0.5, fract(coords.y));
    return KCL::dVector3D::interpolate(vec3(0.2, 0.8, 1.0), vec3(0.1, 0.4, 0.7), abs(c.x - c.y));
}

void PrecisionTest::saveReference()
{
    KCL::Image actImage;

    int img_width = 1920 / 2;
    int img_height = 1080 / 2;

    actImage.Allocate2D(img_width, img_height, KCL::Image_RGB888);

    KCL::uint8* data = (KCL::uint8*)actImage.getData();

    //cannot test precision of vertex pipe / rasterizer / interpolation / texture sampler..., just ALU + maybe RTs

    //need to render to FHD target, and present contents as a textured FSQ @native res

    double resolutionX = img_width; // (double)m_resolution.x;
    double resolutionY = img_height; // (double)m_resolution.y;

    KCL::dMatrix4x4 orientation_tp = m_orientation;
    orientation_tp.transpose();

    //run the shader on the CPU: i=0 is bottom row, j=0 is left coloumn
    for (int j = 0; j < img_height; ++j)
    {
        for (int i = 0; i < img_width; ++i)
        {
            double gl_FragCoordX = (double)i + 0.5;
            double gl_FragCoordY = (double)j + 0.5;

#if 0
            if (gl_FragCoordY / resolutionY > 0.5)
            {
                if (gl_FragCoordX / resolutionX < 0.5)
                {
                    double y = (gl_FragCoordY / resolutionY * 2.0 - 1.0) * 26.0;
                    double x = 1.0 - (gl_FragCoordX / (resolutionX * 0.5));
                    double p = pow(2.0, floor(y));
                    double b = fmin(fmax(p + x, p), p + 1.0); //avoid compiler optimization of (p + x) - p = x, we want underflow to happen here
                    double c = b - p;
                    if (fract(y) >= 0.9)
                        c = 0.0;

                    data[j * img_width * 3 + i * 3 + 0] = ConvertToByte(c);
                    data[j * img_width * 3 + i * 3 + 1] = ConvertToByte(c);
                    data[j * img_width * 3 + i * 3 + 2] = ConvertToByte(c);
                }
                else
                {
                    double y = (gl_FragCoordY / resolutionY * 2.0 - 1.0) * (m_floatMinLimitsY - m_floatMinLimitsX);
                    double x = (1.0 - (gl_FragCoordX / resolutionX));
                    x *= 2.0;
                    double row = floor(y) + (-m_floatMinLimitsY);
                    for (double c = 0.0; c < row; c = c + 1.0)
                    {
                        x = x / 2.0;
                    }
                    //x will underflow to 0 for higher rows (smaller exponents)
                    for (double c = 0.0; c < row; c = c + 1.0)
                    {
                        x = x * 2.0;
                    }

                    data[j * img_width * 3 + i * 3 + 0] = ConvertToByte(x);
                    data[j * img_width * 3 + i * 3 + 1] = ConvertToByte(x);
                    data[j * img_width * 3 + i * 3 + 2] = ConvertToByte(x);

                    if (x == 0.0)
                    {
                        data[j * img_width * 3 + i * 3 + 0] = ConvertToByte(1.0);
                        data[j * img_width * 3 + i * 3 + 1] = ConvertToByte(0.0);
                        data[j * img_width * 3 + i * 3 + 2] = ConvertToByte(0.0);
                    }
                    if (fract(y) > 0.9)
                    {
                        data[j * img_width * 3 + i * 3 + 0] = ConvertToByte(0.0);
                        data[j * img_width * 3 + i * 3 + 1] = ConvertToByte(0.0);
                        data[j * img_width * 3 + i * 3 + 2] = ConvertToByte(0.0);
                    }
                }
            }
            else
#endif

            KCL::dVector4D u_quadrantScaleBias(1.0,1.0,-1.0,-1.0);

            {
                vec2 xy = vec2(gl_FragCoordX / resolutionX, gl_FragCoordY / resolutionY);
				xy = (xy * vec2(u_quadrantScaleBias.x, u_quadrantScaleBias.y) + vec2(u_quadrantScaleBias.z, u_quadrantScaleBias.w) + vec2(1.0,1.0)) * vec2(0.5,0.5);
                double aspect = resolutionY / resolutionX;

                double fov = 2.5;
                double thf = tan(fov * 0.5);
                vec3 viewDirection = (vec3(xy.x * thf, xy.y * thf * aspect, 1.0)).normalize();
                vec4 tmp2;
                vec4 vD = vec4(viewDirection, 1.0);

                const KCL::dMatrix4x4 otp = orientation_tp;
                KCL::mult4x4(otp, vD, tmp2);
                viewDirection = vec3(tmp2.x, tmp2.y, tmp2.z);

                double targetHeight = (viewDirection.y >= 0.0) ? 5.0 : 0.0;
                vec3 eyeToPlane = viewDirection * ((m_eyePosition.y - targetHeight) / viewDirection.y);
                vec3 intersectPosition = m_eyePosition + eyeToPlane;
                vec3 planeNormal = getWaterNormal(vec2(intersectPosition.x, intersectPosition.z), KCL::dVector3D::length(eyeToPlane), m_time);

                vec4 gl_FragColor;

                if (viewDirection.y >= 0.0)
                {
                    vec3 skyColor = getSkyColor(viewDirection, m_lightDir);
                    double cloud = fmax(KCL::dVector3D::dot(planeNormal, viewDirection), 0.3 - 0.3 * planeNormal.y);
                    gl_FragColor = vec4(KCL::dVector3D::interpolate(skyColor, vec3(1.0, 1.0, 1.0), cloud), 1.0);
                }
                else
                {
                    vec3 refractedViewDir = viewDirection - 0.25 * planeNormal;
                    vec3 waterToGround = refractedViewDir * (1.0 / refractedViewDir.y);
                    vec3 groundPosition = intersectPosition + waterToGround;
                    vec3 refractedLightDir = (KCL::dVector3D(m_lightDir) + planeNormal).normalize();
                    vec3 groundColor = getGroundColor(vec2(groundPosition.x, groundPosition.z));
                    groundColor *= refractedLightDir.y;	// dot(refractedLightDir, (0, 1, 0)
                    groundColor = KCL::dVector3D::interpolate(vec3(0.0, 0.2, 0.4), groundColor, abs(KCL::dVector3D::dot(planeNormal, viewDirection)));

                    vec3 reflVec = viewDirection - 2.0 * KCL::dVector3D::dot(planeNormal, viewDirection) * planeNormal;
                    vec3 reflectedView;
                    reflectedView.x = fmax(-1.0, reflVec.x);
                    reflectedView.y = fmax(0.0, reflVec.y);
                    reflectedView.z = fmax(-1.0, reflVec.z);
                    vec3 reflectedSky = 0.25 * getSkyColor(reflectedView, m_lightDir);

                    gl_FragColor = vec4(groundColor + reflectedSky, 1.0);
                }

                data[j * img_width * 3 + i * 3 + 0] = ConvertToByte(gl_FragColor.x);
                data[j * img_width * 3 + i * 3 + 1] = ConvertToByte(gl_FragColor.y);
                data[j * img_width * 3 + i * 3 + 2] = ConvertToByte(gl_FragColor.z);

                if (gl_FragColor.x < 0.1)
                {
                    data[j * img_width * 3 + i * 3 + 0] = ConvertToByte(0.25);
                    data[j * img_width * 3 + i * 3 + 1] = ConvertToByte(0.3);
                    data[j * img_width * 3 + i * 3 + 2] = ConvertToByte(0.3);
                }
            }
        }
    }

    // Save quality match image
	SaveImage(actImage, "ref_precision.tga");
}
#pragma optimize( "", on )
#else

void PrecisionTest::saveReference()
{

}

#endif


void PrecisionTest::FreeResources()
{
    if (m_shader_bars)
    {
        glDeleteProgram(m_shader_bars);
        m_shader_bars = 0;
    }
    if (m_shader_alu_mediump)
    {
        glDeleteProgram(m_shader_alu_mediump);
        m_shader_alu_mediump = 0;
    }
    if (m_shader_alu_highp)
    {
        glDeleteProgram(m_shader_alu_highp);
        m_shader_alu_highp = 0;
    }

    if (m_vertexBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &m_vertexBuffer);
        m_vertexBuffer = 0;
    }

    if (m_indexBuffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &m_indexBuffer);
        m_indexBuffer = 0;
    }

    delete m_1080pFBO;
    m_1080pFBO = 0;
}
