/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "notificationman.h"


#include "texture.h"
#include "platform.h"
#include <kcl_os.h>

using namespace GLB;


const KCL::Vector4D NotificationManager::DefaultColor(1,1,1,1);


//these depend on the fps_number_palette.png, fps_bg.png, vsync_logo.png files:
const int DIGIT_W = 22;
const int DIGIT_H = 30;
const float FPS_DIGIT_GAP = DIGIT_W + 1;
const int DIGIT_CLIP_HORIZONTAL_OFFSET = 1;
const int DIGIT_CLIP_VERTICAL_OFFSET = 1;
const int DIGIT_CLIP_VERTICAL_BEGIN = 3;

const int FPS_BG_CLIP_X = 119;
const int FPS_BG_CLIP_MINUS_Y = 10;
const float DIGIT_POSITION_FPS_BG_1_X = 10;
const float DIGIT_POSITION_FPS_BG___Y = 15;
const int VSYNC_CLIP_MINUS_Y = 10;

const float DIGIT_POSITION_BATTERY_______DIGIT___Y = 15;
const float DIGIT_POSITION_BATTERY_TIME__DIGIT_1_X = 44;
const float DIGIT_POSITION_BATTERY_TIME__COLON_GAP = 11;
const float DIGIT_POSITION_BATTERY_LEVEL_DIGIT_1_X = 242;

const int BATTERY_COLOR_CLIP_X = 3;
const int BATTERY_COLOR_CLIP_Y = 22;
const int BATTERY_COLOR_CLIP_W = 10;
const int BATTERY_COLOR_CLIP_H = 18;
const float BATTERY_COLOR_POSITION_X = 226;
const float BATTERY_COLOR_POSITION_Y = 22;

const float BATTERY_BG_CLIP_W = 362;
const float BATTERY_BG_CLIP_H = 54;


NotificationManager::NotificationManager()
{
	m_inited = false;
}


NotificationManager::~NotificationManager()
{
	Clear();

	//delete m_GLState;
}


void NotificationManager::Init( int currentViewPortWidth, int currentViewPortHeight, int orientation)
{
	if(m_inited)
	{
		Clear();
	}
	m_inited = true;

	m_defaultViewPortWidth = currentViewPortWidth;
	m_defaultViewPortHeight = currentViewPortHeight;
	if(orientation == -1)
		m_is_rotated = m_defaultViewPortWidth < m_defaultViewPortHeight;
	else
		m_is_rotated = orientation > 0;

	DXB::Image2D *image = 0;

	image = new DXB::Image2D();
	image->load ("loading_glb_square.png");
	image->commit ();
    delete image;

	m_img_load = new Texture2D (image);
	m_img_load->setFiltering( Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_load->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);

	image = new DXB::Image2D();
	image->load ("glb_splashscreen.png");
	image->commit ();
    delete image;

	m_img_splash = new Texture2D (image);
	m_img_splash->setFiltering( Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_splash->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);

	image = new DXB::Image2D();
	image->load ("running_glb_square.png");
	image->commit ();
    delete image;

	m_img_running = new Texture2D (image);
	m_img_running->setFiltering( Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_running->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);


	image = new DXB::Image2D();
	image->load ("kishonti_logo_512.png");
	//image->load ("glb2logo.png");
	image->commit ();
    delete image;

	m_img_logo = new Texture2D (image);
	m_img_logo->setFiltering( Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_logo->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);


	image = new DXB::Image2D();
	image->load ("vsync_logo.png");
	image->commit ();
    delete image;

	m_img_vsync = new Texture2D (image);
	m_img_vsync->setFiltering( Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_vsync->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);


	image = new DXB::Image2D();
	image->load ("fps_number_palette.png");
	image->commit ();
    delete image;

	m_img_digits = new Texture2D( image);
	m_img_digits->setFiltering (Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_digits->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);


	image = new DXB::Image2D();
	image->load ("fps_bg.png");
	image->commit ();
    delete image;

	m_img_fps_background = new Texture2D (image);
	m_img_fps_background->setFiltering( Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_fps_background->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);


	image = new DXB::Image2D();
	image->load ("battery_info.png");
	image->commit ();
    delete image;

	m_img_battery_background = new Texture2D (image);
	m_img_battery_background->setFiltering( Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_battery_background->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);


	image = new DXB::Image2D();
	image->load ("battery_color.png");
	image->commit ();
    delete image;

	m_img_battery_color = new Texture2D (image);
	m_img_battery_color->setFiltering( Texture::FILTER_BASE_LEVEL, Texture::FILTER_LINEAR);
	m_img_battery_color->setWrapping( Texture::WRAP_CLAMP, Texture::WRAP_CLAMP);


	m_clips_digits[0][0].x = 0.0f;
	m_clips_digits[0][0].y = DIGIT_CLIP_VERTICAL_BEGIN;
	m_clips_digits[0][0].width = DIGIT_W;
	m_clips_digits[0][0].height = DIGIT_H;

	m_clips_digits[1][0].x = 0.0f;
	m_clips_digits[1][0].y = m_clips_digits[0][0].y + DIGIT_H + DIGIT_CLIP_VERTICAL_OFFSET;
	m_clips_digits[1][0].width = DIGIT_W;
	m_clips_digits[1][0].height = DIGIT_H;

	for(size_t i=1; i<10; ++i)
	{
		m_clips_digits[0][i].x = m_clips_digits[0][i-1].x + DIGIT_W + DIGIT_CLIP_HORIZONTAL_OFFSET;
		m_clips_digits[0][i].y = m_clips_digits[0][i-1].y;
		m_clips_digits[0][i].width = DIGIT_W;
		m_clips_digits[0][i].height = DIGIT_H;

		m_clips_digits[1][i].x = m_clips_digits[1][i-1].x + DIGIT_W + DIGIT_CLIP_HORIZONTAL_OFFSET;
		m_clips_digits[1][i].y = m_clips_digits[1][i-1].y;
		m_clips_digits[1][i].width = DIGIT_W;
		m_clips_digits[1][i].height = DIGIT_H;
	}


	m_clip_splash.width            = (float)m_img_splash->getImage()->getWidth ();
	m_clip_splash.height           = (float)m_img_splash->getImage()->getHeight();

	m_clip_load.width            = (float)m_img_load          ->getImage()->getWidth ();
	m_clip_load.height           = (float)m_img_load          ->getImage()->getHeight();
	m_clip_running.width         = (float)m_img_running       ->getImage()->getWidth ();
	m_clip_running.height        = (float)m_img_running       ->getImage()->getHeight();

	m_clip_logo.width            = (float)m_img_logo          ->getImage()->getWidth ();
	m_clip_logo.height           = (float)m_img_logo          ->getImage()->getHeight();

	m_clip_vsync.width           = (float)m_img_vsync         ->getImage()->getWidth ();
	m_clip_vsync.height          = (float)m_img_vsync         ->getImage()->getHeight() - VSYNC_CLIP_MINUS_Y;

	m_clip_fps_background.x = FPS_BG_CLIP_X;
	m_clip_fps_background.y = 0;
	m_clip_fps_background.width  = m_img_fps_background->getImage()->getWidth () - m_clip_fps_background.x;
	m_clip_fps_background.height = m_img_fps_background->getImage()->getHeight() - FPS_BG_CLIP_MINUS_Y;

	m_clip_battery_background.x      = 0;
	m_clip_battery_background.y      = 0;
	m_clip_battery_background.width  = BATTERY_BG_CLIP_W;
	m_clip_battery_background.height = BATTERY_BG_CLIP_H;

	m_clip_battery_color.x      = BATTERY_COLOR_CLIP_X;
	m_clip_battery_color.y      = BATTERY_COLOR_CLIP_Y;
	m_clip_battery_color.width  = BATTERY_COLOR_CLIP_W;
	m_clip_battery_color.height = BATTERY_COLOR_CLIP_H;

}



void NotificationManager::Clear()
{
	if(!m_inited)
	{
		return;
	}

	delete m_img_splash;
	m_img_splash = 0;

	delete m_img_load;
	m_img_load = 0;

	delete m_img_running;
	m_img_running = 0;

	delete m_img_logo;
	m_img_logo = 0;

	delete m_img_vsync;
	m_img_vsync = 0;

	delete m_img_digits;
	m_img_digits = 0;

	delete m_img_fps_background;
	m_img_fps_background = 0;

	delete m_img_battery_background;
	m_img_battery_background = 0;

	delete m_img_battery_color;
	m_img_battery_color = 0;

	m_inited = false;
}


void NotificationManager::Prerender() const
{
}


void NotificationManager::Postrender() const
{
}


void NotificationManager::DrawTexture( const GLB::Texture2D &texture, const GLB::Rectangle &clip, float scale, float x, float y, bool blend, const KCL::Vector4D &color, bool batchDrawModeEnabled) const
{
	if(!batchDrawModeEnabled)
	{
		Prerender();
	}

	float tx_min = clip.x / texture.getImage()->getWidth();
	float ty_min = clip.y / texture.getImage()->getHeight();
	float tx_max = tx_min + clip.width / texture.getImage()->getWidth();
	float ty_max = ty_min + clip.height / texture.getImage()->getHeight();

	float vertices[] =
	{
		-1.0f, -1.0f, tx_min, ty_max,
		1.0f, -1.0f, tx_max, ty_max,
		-1.0f,  1.0f, tx_min, ty_min,
		1.0f,  1.0f, tx_max, ty_min
	};

	if(!batchDrawModeEnabled)
	{
		Postrender();
	}
}


//void NotificationManager::DrawTexture(const GLB::Texture2D* texture, const GLB::Rectangle &clip, float scale, float x, float y, bool blend, const KCL::Vector4D &color, bool batchDrawModeEnabled = false) const
//{
//	DrawTexture( texture, clip, scale, x, y, blend, color, batchDrawModeEnabled);
//}


void NotificationManager::DrawTextureInCorner(const GLB::Texture2D &texture, const GLB::Rectangle &clip, float scale, Corner corner, float paddingX, float paddingY, bool blend, const KCL::Vector4D &color, bool batchDrawModeEnabled) const
{
	float x;
	float y;

	//Remember: enum Corner {UpperLeft, UpperRight, LowerLeft, LowerRight};
	if(IsRotated())
	{
		if(corner % 2) // on the right side --> up
		{
			y = m_defaultViewPortHeight - paddingX - clip.width * scale;
			//x = m_defaultViewPortWidth - paddingX - clip.width * scale;
		}
		else // on the left side --> down
		{
			y = paddingX;
			//x = paddingX;
		}
		if(corner < 2) // upper --> left
		{
			x = paddingY;
			//y = m_defaultViewPortHeight - paddingY - clip.height * scale;
		}
		else // lower -->right
		{
			x = m_defaultViewPortWidth - paddingY - clip.height * scale;
			//y = paddingY;
		}
	}
	else{
		if(corner % 2) // on the right side
		{
			x = m_defaultViewPortWidth - paddingX - clip.width * scale;
		}
		else // on the left side
		{
			x = paddingX;
		}
		if(corner < 2) // upper
		{
			y = m_defaultViewPortHeight - paddingY - clip.height * scale;
		}
		else // lower
		{
			y = paddingY;
		}
	}

	DrawTexture( texture, clip, scale, x, y, blend, color, batchDrawModeEnabled);
}


//void NotificationManager::DrawTextureInCorner(const GLB::Texture2D* texture, const GLB::Rectangle &clip, float scale, Corner corner, float paddingX, float paddingY, bool blend, const KCL::Vector4D &color, bool batchDrawModeEnabled) const
//{
//	DrawTextureInCorner( texture, texture->getImage()->getWidth(), texture->getImage()->getHeight(), clip, scale, corner, paddingX, paddingY, blend, color, batchDrawModeEnabled);
//}


void NotificationManager::DrawLogo (bool batchDrawModeEnabled, float scale, Corner corner, float paddingX, float paddingY, bool blend, const KCL::Vector4D &color) const
{
	DrawTextureInCorner(*m_img_logo, m_clip_logo, scale, corner, paddingX, paddingY, blend, color, batchDrawModeEnabled);
}


void NotificationManager::DrawVsync (bool batchDrawModeEnabled, float scale, Corner corner, float paddingX, float paddingY, bool blend, const KCL::Vector4D &color) const
{
	DrawTextureInCorner(*m_img_vsync, m_clip_vsync, scale, corner, paddingX, paddingY, blend, color, batchDrawModeEnabled);
}


void NotificationManager::DrawFPS (int fps, bool vsyncLimited, bool batchDrawModeEnabled, float scale, Corner corner, float paddingX, float paddingY, bool blend, const KCL::Vector4D &color) const
{
	float paddingDigit_1_x = 0.0f;
	float paddingDigit_2_x = 0.0f;
	float paddingDigit_3_x = 0.0f;
	float paddingDigit___y = 0.0f;

	if(corner % 2) // on the right side
	{
		paddingDigit_1_x = paddingX + (m_clip_fps_background.width - DIGIT_POSITION_FPS_BG_1_X - DIGIT_W) * scale;
		paddingDigit_2_x = paddingDigit_1_x - FPS_DIGIT_GAP * scale;
		paddingDigit_3_x = paddingDigit_2_x - FPS_DIGIT_GAP * scale;
	}
	else // on the left side
	{
		paddingDigit_1_x = paddingX + DIGIT_POSITION_FPS_BG_1_X * scale;
		paddingDigit_2_x = paddingDigit_1_x + FPS_DIGIT_GAP * scale;
		paddingDigit_3_x = paddingDigit_2_x + FPS_DIGIT_GAP * scale;
	}
	if(corner < 2) // upper
	{
		paddingDigit___y = paddingY + DIGIT_POSITION_FPS_BG___Y * scale;
	}
	else // lower
	{
		paddingDigit___y = paddingY + (m_clip_fps_background.height - DIGIT_POSITION_FPS_BG___Y - DIGIT_H) * scale;
	}

	int digit_1 = 0;
	int digit_2 = 0;
	int digit_3 = 0;

	if( fps > 999)
	{
		digit_1 = 9;
		digit_2 = 9;
		digit_3 = 9;
	}
	else if( fps > 99)
	{
		digit_1 = fps / 100;
		digit_2 = (fps % 100) / 10;
		digit_3 = (fps % 100) % 10;
	}
	else if( fps > 9)
	{
		digit_2 = fps / 10;
		digit_3 = fps % 10;
	}
	else if( fps > 0)
	{
		digit_3 = fps;
	}

	DrawTextureInCorner(*m_img_fps_background, m_clip_fps_background, scale, corner, paddingX, paddingY, blend, color, batchDrawModeEnabled);

	if(digit_1 > 0)
	{
		DrawTextureInCorner(*m_img_digits, m_clips_digits[vsyncLimited][digit_1], scale, corner, paddingDigit_1_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
		DrawTextureInCorner(*m_img_digits, m_clips_digits[vsyncLimited][digit_2], scale, corner, paddingDigit_2_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	}
	else if(digit_2 > 0)
	{
		DrawTextureInCorner(*m_img_digits, m_clips_digits[vsyncLimited][digit_2], scale, corner, paddingDigit_2_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	}
	DrawTextureInCorner(*m_img_digits, m_clips_digits[vsyncLimited][digit_3], scale, corner, paddingDigit_3_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
}


void NotificationManager::DrawSplashScreen(bool blend) const
{
	float X = IsRotated() ? (m_defaultViewPortWidth  - (int)m_clip_splash.height) / 2 : (m_defaultViewPortWidth  - (int)m_clip_splash.width ) / 2;
	float Y = IsRotated() ? (m_defaultViewPortHeight - (int)m_clip_splash.width ) / 2 : (m_defaultViewPortHeight - (int)m_clip_splash.height) / 2;
	DrawTexture(*m_img_splash, m_clip_splash, 1.0f, X, Y, blend);
}


void NotificationManager::DrawLoadingScreen(bool blend) const
{
	float X = IsRotated() ? (m_defaultViewPortWidth  - (int)m_clip_load.height) / 2 : (m_defaultViewPortWidth  - (int)m_clip_load.width ) / 2;
	float Y = IsRotated() ? (m_defaultViewPortHeight - (int)m_clip_load.width ) / 2 : (m_defaultViewPortHeight - (int)m_clip_load.height) / 2;
	DrawTexture(*m_img_load, m_clip_load, 1.0f, X, Y, blend);
}


void NotificationManager::DrawRunningScreen(bool blend) const
{
	float X = IsRotated() ? (m_defaultViewPortWidth  - (int)m_clip_running.height) / 2 : (m_defaultViewPortWidth  - (int)m_clip_running.width ) / 2;
	float Y = IsRotated() ? (m_defaultViewPortHeight - (int)m_clip_running.width ) / 2 : (m_defaultViewPortHeight - (int)m_clip_running.height) / 2;
	DrawTexture(*m_img_running, m_clip_running, 1.0f, X, Y, blend);
}


void NotificationManager::DrawBatteryStatistics (int batteryLevel, int hour, int minute, int secundum, bool batchDrawModeEnabled, float scale, Corner corner, float paddingX, float paddingY, bool blend, const KCL::Vector4D &color) const
{
	int digit_hour_____1 = 0;
	int digit_hour_____2 = 0;
	int digit_minute___1 = 0;
	int digit_minute___2 = 0;
	int digit_secundum_1 = 0;
	int digit_secundum_2 = 0;
	bool too_much_time = false;

	if(hour > 99)
	{
		too_much_time = true;
		digit_hour_____1 = 9;
		digit_hour_____2 = 9;
		digit_minute___1 = 9;
		digit_minute___2 = 9;
		digit_secundum_1 = 9;
		digit_secundum_2 = 9;
	}
	else
	{
		digit_hour_____1 = hour / 10;
		digit_hour_____2 = hour % 10;
		digit_minute___1 = minute / 10;
		digit_minute___2 = minute % 10;
		digit_secundum_1 = secundum / 10;
		digit_secundum_2 = secundum % 10;
	}

	int digit_battery_1 = 0;
	int digit_battery_2 = 0;
	int digit_battery_3 = 0;
	float scale_for_batteryLevel_H = 1.0f;


	if(batteryLevel > 99)
	{
		batteryLevel = 100;
		digit_battery_1 = 1;
	}
	else if(batteryLevel > 9)
	{
		digit_battery_2 = batteryLevel / 10;
		digit_battery_3 = batteryLevel % 10;

		scale_for_batteryLevel_H = (1 + batteryLevel / 10) / 10.0f;
	}
	else if(batteryLevel > 0)
	{
		digit_battery_3 = batteryLevel;
		scale_for_batteryLevel_H = 0.1f;
	}
	else
	{
		batteryLevel = 0;
	}


	float scaled_batteryLevel_H = m_clip_battery_color.height * scale_for_batteryLevel_H;
	float shifted_batteryLevel_Y = m_clip_battery_color.y + (m_clip_battery_color.height - scaled_batteryLevel_H);

	GLB::Rectangle percentClip(m_clip_battery_color.x, shifted_batteryLevel_Y, m_clip_battery_color.width, scaled_batteryLevel_H);


	float paddingDigit_hour_____1_x = 0.0f;
	float paddingDigit_hour_____2_x = 0.0f;
	float paddingDigit_minute___1_x = 0.0f;
	float paddingDigit_minute___2_x = 0.0f;
	float paddingDigit_secundum_1_x = 0.0f;
	float paddingDigit_secundum_2_x = 0.0f;

	float paddingDigit_battery_1_x = 0.0f;
	float paddingDigit_battery_2_x = 0.0f;
	float paddingDigit_battery_3_x = 0.0f;

	float paddingDigit___y = 0.0f;

	float paddingBatteryColor_x = 0.0f;
	float paddingBatteryColor_y = 0.0f;

	if(corner % 2) // on the right side
	{
		paddingDigit_hour_____1_x = paddingX + (m_clip_battery_background.width - DIGIT_POSITION_BATTERY_TIME__DIGIT_1_X - DIGIT_W) * scale;
		paddingDigit_hour_____2_x = paddingDigit_hour_____1_x - DIGIT_W * scale;

		paddingDigit_minute___1_x = paddingDigit_hour_____2_x - (DIGIT_POSITION_BATTERY_TIME__COLON_GAP + DIGIT_W) * scale;
		paddingDigit_minute___2_x = paddingDigit_minute___1_x - DIGIT_W * scale;

		paddingDigit_secundum_1_x = paddingDigit_minute___2_x - (DIGIT_POSITION_BATTERY_TIME__COLON_GAP + DIGIT_W) * scale;
		paddingDigit_secundum_2_x = paddingDigit_secundum_1_x - DIGIT_W * scale;

		paddingDigit_battery_1_x = paddingX + (m_clip_battery_background.width - DIGIT_POSITION_BATTERY_LEVEL_DIGIT_1_X - DIGIT_W) * scale;
		paddingDigit_battery_2_x = paddingDigit_battery_1_x - DIGIT_W * scale;
		paddingDigit_battery_3_x = paddingDigit_battery_2_x - DIGIT_W * scale;

		paddingBatteryColor_x = paddingX + (m_clip_battery_background.width - BATTERY_COLOR_POSITION_X - BATTERY_COLOR_CLIP_W) * scale;
	}
	else // on the left side
	{
		paddingDigit_hour_____1_x = paddingX + DIGIT_POSITION_BATTERY_TIME__DIGIT_1_X * scale;
		paddingDigit_hour_____2_x = paddingDigit_hour_____1_x + DIGIT_W * scale;

		paddingDigit_minute___1_x = paddingDigit_hour_____2_x + (DIGIT_POSITION_BATTERY_TIME__COLON_GAP + DIGIT_W) * scale;
		paddingDigit_minute___2_x = paddingDigit_minute___1_x + DIGIT_W * scale;

		paddingDigit_secundum_1_x = paddingDigit_minute___2_x + (DIGIT_POSITION_BATTERY_TIME__COLON_GAP + DIGIT_W) * scale;
		paddingDigit_secundum_2_x = paddingDigit_secundum_1_x + DIGIT_W * scale;

		paddingDigit_battery_1_x = paddingX + DIGIT_POSITION_BATTERY_LEVEL_DIGIT_1_X * scale;
		paddingDigit_battery_2_x = paddingDigit_battery_1_x + DIGIT_W * scale;
		paddingDigit_battery_3_x = paddingDigit_battery_2_x + DIGIT_W * scale;

		paddingBatteryColor_x = paddingX + BATTERY_COLOR_POSITION_X * scale;
	}

	//scaled_batteryLevel_H 
	//shifted_batteryLevel_Y
	if(corner < 2) // upper
	{
		paddingDigit___y = paddingY + DIGIT_POSITION_BATTERY_______DIGIT___Y * scale;
		paddingBatteryColor_y = paddingY + shifted_batteryLevel_Y * scale;
	}
	else // lower
	{
		paddingDigit___y = paddingY + (m_clip_battery_background.height - DIGIT_POSITION_BATTERY_______DIGIT___Y - DIGIT_H) * scale;
		//paddingBatteryColor_y = paddingY + (m_clip_battery_background.height - BATTERY_COLOR_CLIP_Y - BATTERY_COLOR_CLIP_H) * scale;
		paddingBatteryColor_y = paddingY + (m_clip_battery_background.height - shifted_batteryLevel_Y - scaled_batteryLevel_H) * scale;
	}


	DrawTextureInCorner(*m_img_battery_background, m_clip_battery_background, scale, corner, paddingX, paddingY, blend, color, batchDrawModeEnabled);

	DrawTextureInCorner(*m_img_digits, m_clips_digits[too_much_time][digit_hour_____1], scale, corner, paddingDigit_hour_____1_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	DrawTextureInCorner(*m_img_digits, m_clips_digits[too_much_time][digit_hour_____2], scale, corner, paddingDigit_hour_____2_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	DrawTextureInCorner(*m_img_digits, m_clips_digits[too_much_time][digit_minute___1], scale, corner, paddingDigit_minute___1_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	DrawTextureInCorner(*m_img_digits, m_clips_digits[too_much_time][digit_minute___2], scale, corner, paddingDigit_minute___2_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	DrawTextureInCorner(*m_img_digits, m_clips_digits[too_much_time][digit_secundum_1], scale, corner, paddingDigit_secundum_1_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	DrawTextureInCorner(*m_img_digits, m_clips_digits[too_much_time][digit_secundum_2], scale, corner, paddingDigit_secundum_2_x, paddingDigit___y, blend, color, batchDrawModeEnabled);

	float shifted_batteryLevel = batteryLevel - 50;

	KCL::Vector4D batteryColor((100.0f - 1.5f * shifted_batteryLevel) / 100.0f, (100.0f + 1.99f * shifted_batteryLevel) / 100.0f, 0, 1);
	if(batteryLevel)
	{
		DrawTextureInCorner(*m_img_battery_color, percentClip, scale, corner, paddingBatteryColor_x, paddingBatteryColor_y, blend, batteryColor, batchDrawModeEnabled);
	}

	if(digit_battery_1 > 0)
	{
		DrawTextureInCorner(*m_img_digits, m_clips_digits[false][digit_battery_1], scale, corner, paddingDigit_battery_1_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
		DrawTextureInCorner(*m_img_digits, m_clips_digits[false][digit_battery_2], scale, corner, paddingDigit_battery_2_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	}
	else if(digit_battery_2 > 0)
	{
		DrawTextureInCorner(*m_img_digits, m_clips_digits[false][digit_battery_2], scale, corner, paddingDigit_battery_2_x, paddingDigit___y, blend, color, batchDrawModeEnabled);
	}
	DrawTextureInCorner(*m_img_digits, m_clips_digits[digit_battery_1 + digit_battery_2 == 0][digit_battery_3], scale, corner, paddingDigit_battery_3_x, paddingDigit___y, blend, color, batchDrawModeEnabled);

}


