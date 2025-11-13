/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "qualitymatch.h"
#include "ng/json.h"

using namespace GLB;
using namespace KCL;


QualityMatch::QualityMatch(const GlobalTestEnvironment* const gte,TestBase *wrappedTest, const std::string& path_prefix, bool skip_init = 0) :
	TestWrapper(gte,wrappedTest), qm_fbo(0), m_score(-1), m_skip_initalize( skip_init)
	, m_min_frame_count(1)
{
	TestDescriptor ts(*gte->GetTestDescriptor()) ;

	nnm = NewNotificationManager::NewInstance();
	m_settings = new TestDescriptor(ts);
	m_metric = "PSNR";
	char filename[100];
	if (path_prefix.empty())
	{
		sprintf(filename, "common/ref_%s.png", GetSetting().m_name.c_str());
	}
	else
	{
		m_path_prefix = path_prefix;
		sprintf(filename, "common/%s/ref_%s_%d.png", path_prefix.c_str(), GetSetting().m_name.c_str(), GetSetting().qm_compare_frame);
	}
	m_reference_filename = filename;
	m_save_image = ts.m_qm_save_image;
//#define AUTOCREATE_MISSING_REF_IMAGES
#ifdef AUTOCREATE_MISSING_REF_IMAGES
	if (!KCL::File::Exists(filename))
	{
		m_save_image = 1;
	}
#endif
	if (!ts.m_qm_metric.empty())
	{
		m_metric = ts.m_qm_metric;
	}
	if (!ts.m_qm_reference_filename.empty())
	{
		m_reference_filename = ts.m_qm_reference_filename;
	}

#if defined QC_TEST_IMAGE
	delete m_wrappedTest;
	m_wrappedTest=NULL;
#endif
}

QualityMatch::~QualityMatch()
{
	delete nnm;
	delete qm_fbo;
	delete m_settings;
	nnm = nullptr;
	m_settings = nullptr;
	qm_fbo = nullptr;
	if(m_skip_initalize)
	{
		m_wrappedTest = nullptr;
	}
}

KCL::KCL_Status QualityMatch::init0()
{
	m_window_width = m_settings->m_viewport_width;
	m_window_height = m_settings->m_viewport_height;

	if (!getWrappedTest())
	{
		return KCL_TESTERROR_NOERROR;
	}

	int width;
	int height;

	if (m_referenceImage.load(m_reference_filename.c_str()))
	{
		if (!m_referenceImage.convertTo(KCL::Image_RGB888))
		{
			SetRuntimeError(KCL_TESTERROR_UNKNOWNERROR);
			return KCL_TESTERROR_UNKNOWNERROR;
		}

		width = m_referenceImage.getWidth();
		height = m_referenceImage.getHeight();
	}
	else
	{
		if (!m_save_image)
		{
			//INFO("WARN: ref image missing");
			SetRuntimeError(KCL_TESTERROR_FILE_NOT_FOUND);
			return KCL_TESTERROR_FILE_NOT_FOUND;
		}

		width = GetSetting().m_test_width;
		height = GetSetting().m_test_height;
	}

#ifdef HAVE_DX
	qm_fbo = new GLB::FBO();
	qm_fbo->init(width,height,GLB::RGBA8888_Nearest,GLB::DEPTH_None);
#elif defined USE_METAL
    qm_fbo = FBO::CreateFBO(m_gte, width, height, 0, GLB::RGBA8888_Nearest, GLB::DEPTH_None, "QualityMatch") ; ;
#else
	qm_fbo = new FBO(width, height, 0, RGBA8888_Nearest, DEPTH_None, "QualityMatch");
	//qm_fbo = new FBO(width, height, 0, RGB565_Nearest, DEPTH_None, "QualityMatch");
#endif
    getWrappedTest()->SetSetting().SetSingleFrame(GetSetting().m_single_frame).SetScreenMode(SMode_Onscreen).SetWidth(width).SetHeight(height).SetVirtualResolution(false);
	SetSetting().SetSingleFrame(-1).SetPlayTime(3000).SetStartAnimationTime(0).SetScreenMode(SMode_Onscreen);
	KCL::KCL_Status t;
	t = KCL::KCL_TESTERROR_NOERROR;
	if (!m_skip_initalize)
	{
		KCL::KCL_Status t = getWrappedTest()->init0();
		SetRuntimeError(t);
	}
	return t;
}

bool QualityMatch::animate(const int time)
{
#if defined QC_TEST_IMAGE
	SetAnimationTime(time);
#endif
	if (!getWrappedTest())
	{
		SetSetting().SetScreenMode(SMode_Offscreen);
		return false;
	}
	if (GetRuntimeError() != KCL_TESTERROR_NOERROR)
	{
		SetSetting().SetScreenMode(SMode_Offscreen);
		return false;
	}
	bool success = getWrappedTest()->animate(getWrappedTest()->GetSetting().m_single_frame);

	int timed = GetSetting().m_play_time;
	int framed = getFrames();
	if ((time > GetSetting().m_play_time) && (getFrames() > m_min_frame_count))
	{
		SetSetting().SetScreenMode(SMode_Offscreen);
		return false;
	}
	return true;
}

bool QualityMatch::GetQCValues(const KCL::Image& refImg, const KCL::Image& actImg, QCRESULTSTRUCT& result)
{
	KCL::uint32 w = actImg.getWidth();
	KCL::uint32 h = actImg.getHeight();
	if ((w != refImg.getWidth()) ||
		(h != refImg.getHeight()) ||
		(refImg.getFormat() != actImg.getFormat()))
	{
		return false;
	}

	switch (refImg.getFormat())
	{
	case KCL::Image_LUMINANCE_L8:
	case KCL::Image_LUMINANCE_ALPHA_LA88:
	case KCL::Image_RGB888:
	case KCL::Image_RGBA8888:
		// Format supported, no problem...
		break;

	default:
		//Unsupported format.
		return false;
	}

	int B = refImg.getBpp() / 8;

	KCL::uint8 *actPtr = (KCL::uint8*)actImg.getData();
	KCL::uint8 *refPtr = (KCL::uint8*)refImg.getData();

	KCL::uint64 sumSqrDiff = 0;
	KCL::uint64 sumDiff = 0;
	double sumSqrtDiff = 0;
	KCL::uint32 cntDiff = 0;

	for (KCL::uint32 i = 0; i < w * h; i++)
	{
		bool pixelDiffers = false;
		for (int j = 0; j < B; j++, actPtr++, refPtr++)
		{
			KCL::uint8 actPx = *actPtr;
			KCL::uint8 refPx = *refPtr;

			KCL::uint8 diff;
			if (actPx > refPx)
			{
				diff = actPx - refPx;
				pixelDiffers = true;
			}
			else if (actPx < refPx)
			{
				diff = refPx - actPx;
				pixelDiffers = true;
			}
			else
			{
				diff = 0;
			}

			sumSqrDiff += diff * diff;
			sumDiff += diff;
			sumSqrtDiff += sqrt(diff / 255.0);
		}

		if (pixelDiffers)
		{
			cntDiff++;
		}
	}

	result.MSE = (double)sumSqrDiff / 65025 / B / w / h;
	result.ME = (double)sumDiff / 255 / B / w / h;
	result.MRE = sumSqrtDiff / B / w / h;
	result.ERRCNT = (double)cntDiff / w / h;
	
	if (result.MSE > 0.0)
	{
		result.PSNR = 1000 * log10(1.0 / result.MSE);	// Multiply by 1000 for mB (it would be 10 for dB)
	}
	else
	{
		result.PSNR = 20000;
	}

	return true;
}

bool QualityMatch::render0(const char* screenshotName)
{
	KCL::Image actImage;

	if (getFrames() <= m_min_frame_count)
	{
#if defined QC_TEST_IMAGE
		actImage.load(QC_TEST_IMAGE);
		actImage.convertTo(KCL::Image_RGB888);
		nnm->UpdateLogo(&actImage);
#else
		if (!getWrappedTest())
		{
			return false;
		}

		FBO* backupfbo = FBO::GetGlobalFBO();
		FBO::SetGlobalFBO(qm_fbo);
		FBO::bind(0);

		getWrappedTest()->render0();

		if (getFrames() == m_min_frame_count)
		{
			int getScreenshotResult = -1;

#ifdef USE_ANY_GL 
			glFinish();
#endif

			getScreenshotResult = FBO::GetScreenshotImage(actImage);

			if (getScreenshotResult)
			{
				SetRuntimeError(KCL_TESTERROR_OUT_OF_MEMORY);
				return false;
			}

			actImage.convertTo(KCL::Image_RGB888);

			nnm->UpdateLogo(&actImage);
		}

		FBO::SetGlobalFBO(backupfbo);
		FBO::bind(0);
#endif
	}

#ifdef HAVE_DX
	CD3D11_VIEWPORT vp;
	vp.Width = (float)m_window_width;
	vp.Height = (float)m_window_height;

	vp.TopLeftX = vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	//DX::getContext()->RSSetViewports(1, &vp)
	DX::getStateManager()->SetViewport(vp);
#elif defined USE_METAL

	// Metal handle the viewport locally

#else
	glViewport(0,0,m_window_width,m_window_height);
#endif

	if (getFrames() >= m_min_frame_count)
	{
		nnm->ShowLogo(true, false);
	}

	if (getFrames()==m_min_frame_count)
	{
		if (m_save_image)
		{
			// Save quality match image
			std::string path = KCL::File::GetDataRWPath();
			char filename[255];
			if (m_path_prefix.empty())
			{
				sprintf(filename, "%sqc_%s_%s_%d_%dp.tga",
					path.c_str(),
					getWrappedTest()->GetSetting().m_name.c_str(),
					getTextureType().c_str(),
					getWrappedTest()->GetAnimationTime(),
					actImage.getHeight());
			}
			else
			{
				sprintf(filename, "%sref_%s_%d.tga",
					path.c_str(),
					getWrappedTest()->GetSetting().m_name.c_str(),
					getWrappedTest()->GetAnimationTime()
					);
			}
			FILE* f = fopen(filename, "rb");
			if (!f)
			{
				INFO("qm_image saved: %s", filename);
				//actImage.savePng(filename);
				actImage.saveTga(filename);
			}
			else
			{
				fclose(f);
			}
			if (!m_referenceImage.getData())
			{
				m_score = -1.0;
				SetRuntimeError(KCL_TESTERROR_FILE_NOT_FOUND);
				return false;
			}
		}
	
		QCRESULTSTRUCT result;
		if (!GetQCValues(m_referenceImage, actImage, result))
		{
			SetRuntimeError(KCL_TESTERROR_INVALID_SCREEN_RESOLUTION);
			return false;
		};
	
		//INFO("Quality check results:");
		//INFO("MSE  = %9.5f %%", result.MSE * 100);
		//INFO("ME   = %9.5f %%", result.ME * 100);
		//INFO("MRE  = %9.5f %%", result.MRE * 100);
		//INFO("ERR  = %9.5f %%", result.ERRCNT * 100);
		//INFO("PSNR = %9.5f dB", result.PSNR / 100);	// Convert mB to dB
#ifndef NDEBUG
		/*
		FILE* file;
		file = fopen("/data/local/tmp/qm_results.txt", "a+");
		if(!file)
		{
			file = fopen("y:/temp/qm_results_desktop.txt", "a+");
		}
		if(file)
		{
			fprintf(file, "PSNR = %9.5f mB\n", result.PSNR);
			fclose(file);
		}
		*/
#endif
		if (m_metric == "MSE") m_score = result.MSE*100;
		else if (m_metric == "ME") m_score = result.ME*100;
		else if (m_metric == "MRE") m_score = result.MRE*100;
		else if (m_metric == "ERRCNT") m_score = result.ERRCNT*100;
		else m_score = result.PSNR;
	}

	IncFrameCounter();
	return true;
}
