/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef QUALITYMATCH_H
#define QUALITYMATCH_H

#include "test_wrapper.h"
#include "kcl_image.h"
#include "misc2.h"
#include "kcl_io.h"

#if defined USE_ANY_GL
#include "opengl/fbo.h"
#elif defined HAVE_DX
#include "d3d11/fbo3.h"
#elif defined USE_METAL
#include "metal/fbo.h"
#endif

#include "newnotificationmanager.h"


// Source is a png file, not an actual test image
//#define QC_TEST_IMAGE "allrefs\\ref_intel.png"

struct QCRESULTSTRUCT
{
    double MSE;
    double ME;
    double MRE;
    double PSNR;
    double ERRCNT;
};

class QualityMatch: public TestWrapper
{
public:
	QualityMatch(const GlobalTestEnvironment* const gte, TestBase *wrappedTest, const std::string& path_prefix, bool skip_init);
	~QualityMatch();

	const char* getUom() const { 
		if (m_metric == "MSE") return "% MSE";
		else if (m_metric == "ME") return "% ME";
		else if (m_metric == "MRE") return "% MRE";
		else if (m_metric == "ERRCNT") return "% ERRCNT";
		else return "mB PSNR";
	}
	inline float getScore () const { return m_score; }

	KCL::KCL_Status init0 ();
	bool animate (const int time);
	bool render0 (const char* screenshotName = NULL);

    static bool GetQCValues(const KCL::Image& refImg, const KCL::Image& actImg, QCRESULTSTRUCT& result);

private:
	std::string m_path_prefix;
	bool m_skip_initalize;
	bool m_save_image;
	std::string m_reference_filename;
	std::string m_metric;
	GLB::FBO* qm_fbo;
	KCL::Image m_referenceImage;
	double m_score;
	GLB::NewNotificationManager *nnm;
	unsigned int m_min_frame_count;
};

template <class T>
class QualityMatchA : public QualityMatch
{
public:
	QualityMatchA(const GlobalTestEnvironment* const gte) :QualityMatch(gte, new T(gte), "", 0)
	{
	}
};
#endif

