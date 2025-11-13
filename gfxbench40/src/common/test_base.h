/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TEST_BASE_H
#define TEST_BASE_H

#include "render_statistics.h"
#include "kcl_os.h"
#include "kcl_base.h"
#include "printable_result.h"
#include "offscrman.h"
#include "test_base0.h"

#include "../gfxbench/global_test_environment.h"

#include <string>

namespace GLB {

class TestBase : public TestBase0
{
public:
	TestBase (const GlobalTestEnvironment* const gte);
	virtual ~TestBase() ;

	inline unsigned int GetAnimationTime() const { return m_time; }
	inline void SetAnimationTime(const int time) { m_time = time; }
    
    inline long getElapsedTime() const { return m_elapsedtime; }
    inline void setElapsedTime(long elapsedTime) { m_elapsedtime = elapsedTime; }
    

	inline void SetRuntimeError(KCL::KCL_Status error) { m_runtime_error = error; }
	inline KCL::KCL_Status GetRuntimeError() const { return m_runtime_error; }
	
	inline TestDescriptor& SetSetting()  { return *(TestDescriptor*)m_settings; }

	inline const int getViewportWidth() const { return m_settings->m_viewport_width; }
	inline const int getViewportHeight() const { return m_settings->m_viewport_height; }

	inline const std::string getTextureType() const { return m_settings->GetTextureType().c_str(); }

	/*
		Returns the result of the test.
		@results[out]	pointer to pointer to set to an array of result objects. If NULL, no result will be created, only the number of result objects is returned.
		@count[out]		number of result objects
	*/
	virtual void getTestResult(PrintableResult** results, int* count) const;
	virtual int result(std::vector<PrintableResult> *results) const;

	virtual float getScore () const = 0;

    virtual float getNormalizedScore() const;

	virtual const char* getUom() const = 0;
	
	virtual bool isWarmup() const = 0;

	inline bool isFixedTime() const { return m_settings->m_frame_step_time > 0; }

	virtual KCL::uint32 indexCount() const = 0;
	
	virtual void resetEnv();

	virtual bool resize(int width, int height);
	
	virtual bool render0(const char* screenshotName = NULL);

    virtual void finishTest();

	virtual KCL::KCL_Status init0();

	virtual bool isLowLevel() const;

	inline int GetFrameStepTime() const { return m_settings->m_frame_step_time; }

	virtual void FreeResources() = 0;

	virtual void initEffects (int , const char *const*);

	inline const unsigned int getTestLength() const { return m_settings->m_play_time; }

	virtual KCL::uint32 getRenderedVerticesCount() const;

	virtual KCL::uint32 getRenderedTrianglesCount() const;

	virtual KCL::uint32 getDrawCalls() const;	

	virtual KCL::int32 getUsedTextureCount() const;

	virtual KCL::int32 getSamplesPassed() const;

    virtual float getPixelCoverage() const;

	virtual KCL::int32 getInstructionCount() const;

	virtual void logStatistics(double frameTime);

	inline const StatisticsArray& GetStatisticsArray() const { return *m_statistics_array; }

	inline ScreenMode GetScreenMode() const { return m_settings->GetScreenMode(); };

	inline bool screenRefreshNeeded() { return m_screenRefreshNeeded;m_screenRefreshNeeded = false; }

    virtual void saveStatistics();

protected:
	// must implement in the subclass
	virtual KCL::KCL_Status init() = 0;
	virtual bool render() = 0;
	
	inline void setScreenRefreshNeeded() { m_screenRefreshNeeded = true; }
	int m_window_width;
	int m_window_height;
	OffscreenManager *m_offscreenmanager;
	StatisticsArray *m_statistics_array;
	KCL::uint32 m_time;
    long m_elapsedtime;
	KCL::KCL_Status m_runtime_error;
	bool m_screenRefreshNeeded;
	double m_last_swap_buffer_time;

    bool m_offscreen_virtual_res;

	const GlobalTestEnvironment* const m_gte;

#if defined HAVE_DX
    ID3D11Query* m_pEventQuery;
#endif

private:
#if defined USE_ANY_GL
    GLB::FBO *m_finish_fbo;
#endif
};

} // namespace GLB

#endif
