/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TEST_BASE0_H
#define TEST_BASE0_H

#include "kcl_base.h"
#include "printable_result.h"
#include "test_descriptor.h"
#include "assert.h"
#include "graphics/graphicscontext.h"

class GlobalTestEnvironment ;
class TestDescriptor;

namespace GLB
{

class TestBase0
{
public:
    TestBase0(const TestDescriptor &ts) : m_frames(0), m_loadingProgressPtr(NULL), m_settings( &ts), m_cursorX(0), m_cursorY(0), m_is_warmup(false)
    {
//#if defined _DEBUG
        memset(m_keys_current, 0, sizeof(m_keys_current));
        memset(m_keys_last, 0, sizeof(m_keys_last));

        memset(m_buttons_current, 0, sizeof(m_buttons_current));
        memset(m_buttons_last, 0, sizeof(m_buttons_last));
//#endif
    }
	virtual ~TestBase0() {}
	virtual KCL::KCL_Status init0 () = 0;
	virtual bool animate (const int time) = 0;
	virtual bool render0 (const char* screenshotName = NULL) = 0;
    virtual void finishTest() {}
	virtual int result(std::vector<PrintableResult> *results) const = 0;
    virtual void setLoadingProgressPtr(float* ptr) {m_loadingProgressPtr = ptr;}
    virtual void setElapsedTime(long elapsedTime) {};
	inline unsigned int getFrames () const { return m_frames; }
	inline const TestDescriptor& GetSetting() const { return *m_settings; }
	virtual bool resize(int width, int height) { return false; }
    virtual void saveStatistics() {};
    virtual void logStatistics(double frameTime) {};

	virtual void onCheckboxChanged(int id, bool state) {}
    virtual void onSliderChanged(int id, float value) {}


//#if defined _DEBUG && (defined WIN32 || defined _METRO_)
    inline void setKeysCurrent(int key, bool value) { m_keys_current[key] = value;}
    inline void setMouseCurrent(int key, bool value) { m_buttons_current[key] = value;}
    inline void setCursor(int X, int Y) { m_cursorX = X; m_cursorY = Y;}

    inline void IncFrameCounter() { ++m_frames; }
	inline void ResetFrameCounter() { m_frames = 0; }

    bool WasKeyPressed(int id)
    {
        assert(id < MAX_KEY_COUNT);
        return ( !m_keys_last[id] && m_keys_current[id]);
    }

    bool IsKeyPressed(int id)
    {
        assert(id < MAX_KEY_COUNT);
        return m_keys_current[id];
    }

    bool WasButtonPressed(int id)
    {
        assert(id < MAX_BUTTON_COUNT);
        return ( !m_buttons_last[id] && m_buttons_current[id]);
    }

    bool IsButtonPressed(int id)
    {
        assert(id < MAX_BUTTON_COUNT);
        return m_buttons_current[id];
    }

    void GetCursor(int& X, int& Y)
    {
        X = m_cursorX; Y = m_cursorY;
    }

    //bool IsToggled(int id)
    //{
    //    assert(id < MAX_KEY_COUNT);
    //    return isToggled[id];
    //}
//#endif
    bool m_is_warmup;
protected:
	unsigned int m_frames;
    float* m_loadingProgressPtr;
	const TestDescriptor* m_settings;

//#if defined _DEBUG && (defined WIN32 || defined _METRO_)
    static const int MAX_KEY_COUNT = 512;
    bool m_keys_current[MAX_KEY_COUNT];
	bool m_keys_last[MAX_KEY_COUNT];

    static const int MAX_BUTTON_COUNT = 10;
    bool m_buttons_current[MAX_BUTTON_COUNT];
    bool m_buttons_last[MAX_BUTTON_COUNT];

    KCL::uint32 m_cursorX;
    KCL::uint32 m_cursorY;
//#endif
};

bool CreateGlobals(const char* data, const char* datarw,const GlobalTestEnvironment * const gte);
void DestroyGlobals(const GlobalTestEnvironment * const gte);
}

#endif
