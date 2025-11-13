/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NOTIFICATIONMAN_H
#define NOTIFICATIONMAN_H

#include <kcl_math3d.h>
#include "rectangle.h"

namespace GLB
{
	class Texture2D;
}


class NotificationManager
{
public:
	enum Corner {UpperLeft, UpperRight, LowerLeft, LowerRight};

	NotificationManager();
	virtual ~NotificationManager();

	virtual void Init( int currentViewPortWidth, int currentViewPortHeight, int orientation);

	virtual void Resize( int currentViewPortWidth, int currentViewPortHeight)
	{
		m_defaultViewPortWidth = currentViewPortWidth;
		m_defaultViewPortHeight = currentViewPortHeight;
		m_is_rotated = m_defaultViewPortWidth < m_defaultViewPortHeight;
	}

	virtual void Clear();
	virtual void Prerender() const;
	virtual void Postrender() const;


	/****************************************************************************/
	/*
	   IF batchDrawModeEnabled == true:
	   1.) call Prerender
	   2.)a) call any number of Draw*(...) methods using batchDrawModeEnabled = true
	   2.)b) NEVER call Draw*(...) methods using batchDrawModeEnabled = false, including DrawLoadingScreen/DrawRunningScreen
	   3.) call Postrender
	*/
	/****************************************************************************/
	virtual void DrawLogo  (                            bool batchDrawModeEnabled = false, float scale = 1.0f, Corner corner = LowerRight, float paddingX = 10.0f, float paddingY = 10.0f, bool blend = true, const KCL::Vector4D &color = DefaultColor) const;

	virtual void DrawVsync (                            bool batchDrawModeEnabled = false, float scale = 1.0f, Corner corner = UpperRight, float paddingX = 10.0f, float paddingY = 0.0f, bool blend = true, const KCL::Vector4D &color = DefaultColor) const;

	virtual void DrawFPS   (int fps, bool vsyncLimited, bool batchDrawModeEnabled = false, float scale = 1.0f, Corner corner = UpperRight, float paddingX = 10.0f, float paddingY = 0.0f, bool blend = true, const KCL::Vector4D &color = DefaultColor) const;

	virtual void DrawSplashScreen(bool blend) const;

	virtual void DrawLoadingScreen(bool blend = true) const;
	
	virtual void DrawRunningScreen(bool blend = true) const;
	
	virtual void DrawTexture( const GLB::Texture2D &texture, const GLB::Rectangle &clip, float scale, float x, float y, bool blend = true, const KCL::Vector4D &color = DefaultColor, bool batchDrawModeEnabled = false) const;
	
	virtual void DrawTextureInCorner( const GLB::Texture2D &texture, const GLB::Rectangle &clip, float scale, Corner corner, float paddingX, float paddingY, bool blend = true, const KCL::Vector4D &color = DefaultColor, bool batchDrawModeEnabled = false) const;

	virtual void DrawBatteryStatistics (int batteryLevel, int hour, int minute, int secundum, bool batchDrawModeEnabled = false, float scale = 1.0f, Corner corner = UpperRight, float paddingX = 10.0f, float paddingY = 0.0f, bool blend = true, const KCL::Vector4D &color = DefaultColor) const;
protected:
	static const KCL::Vector4D DefaultColor;
private:

	NotificationManager(const NotificationManager&);
	NotificationManager& operator=(const NotificationManager&);


	bool IsRotated() const { return m_is_rotated; }

	bool m_inited;
	unsigned int m_notice_prog[2];
	int m_color_uniform_location[2];
	int m_texture_uniform_location[2];
	bool m_is_rotated;
	int m_defaultViewPortWidth;
	int m_defaultViewPortHeight;

	GLB::Texture2D *m_img_logo;
	GLB::Texture2D *m_img_vsync;
	GLB::Texture2D *m_img_load;
	GLB::Texture2D *m_img_splash;
	GLB::Texture2D *m_img_running;
	GLB::Texture2D *m_img_fps_background;
	GLB::Texture2D *m_img_digits;
	GLB::Texture2D *m_img_battery_background;
	GLB::Texture2D *m_img_battery_color;

	GLB::Rectangle m_clip_logo;
	GLB::Rectangle m_clip_vsync;
	GLB::Rectangle m_clip_splash;
	GLB::Rectangle m_clip_load;
	GLB::Rectangle m_clip_running;
	GLB::Rectangle m_clip_fps_background;
	GLB::Rectangle m_clips_digits[2][10];
	GLB::Rectangle m_clip_battery_background;
	GLB::Rectangle m_clip_battery_color;
	
};


class NotificationManagerDummy : public NotificationManager
{
public:
	NotificationManagerDummy(){}
	~NotificationManagerDummy(){}

	void Init( int currentViewPortWidth, int currentViewPortHeight, int orientation){}
	void Resize( int currentViewPortWidth, int currentViewPortHeight)
	{
	}

	void Clear(){}
	void Prerender() const{}
	void Postrender() const{}


	void DrawLogo  (                            bool batchDrawModeEnabled = false, float scale = 0.5f, Corner corner = LowerRight, float paddingX = 10.0f, float paddingY = 10.0f, bool blend = true, const KCL::Vector4D &color = DefaultColor) const{}
	void DrawVsync (                            bool batchDrawModeEnabled = false, float scale = 1.0f, Corner corner = UpperRight, float paddingX = 10.0f, float paddingY = 0.0f, bool blend = true, const KCL::Vector4D &color = DefaultColor) const{}
	void DrawFPS   (int fps, bool vsyncLimited, bool batchDrawModeEnabled = false, float scale = 1.0f, Corner corner = UpperRight, float paddingX = 10.0f, float paddingY = 0.0f, bool blend = true, const KCL::Vector4D &color = DefaultColor) const{}
	void DrawSplashScreen(bool blend) const{}
	void DrawLoadingScreen(bool blend = true) const{}
	void DrawRunningScreen(bool blend = true) const{}
	void DrawTexture( const GLB::Texture2D &texture, const GLB::Rectangle &clip, float scale, float x, float y, bool blend = true, const KCL::Vector4D &color = DefaultColor, bool batchDrawModeEnabled = false) const{}
	void DrawTextureInCorner( const GLB::Texture2D &texture, const GLB::Rectangle &clip, float scale, Corner corner, float paddingX, float paddingY, bool blend = true, const KCL::Vector4D &color = DefaultColor, bool batchDrawModeEnabled = false) const{}
	void DrawBatteryStatistics (int batteryLevel, int hour, int minute, int secundum, bool batchDrawModeEnabled = false, float scale = 1.0f, Corner corner = UpperRight, float paddingX = 10.0f, float paddingY = 0.0f, bool blend = true, const KCL::Vector4D &color = DefaultColor) const{}
private:
};


#endif //NOTIFICATIONMAN_H
