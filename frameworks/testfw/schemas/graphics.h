/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_SCHEMAS_GRAPHICS_H
#define TFW_SCHEMAS_GRAPHICS_H

#include "apidefinition.h"
#include "serializable.h"

#include <string>
#include <vector>

namespace tfw {
	class Config : public Serializable
	{
	public:
		Config() :
			red_size_(5),
			green_size_(6),
			blue_size_(5),
			depth_size_(16),
			alpha_size_(-1),
			stencil_size_(-1),
			samples_(-1),
			exact_(false) {}

		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		int red() const { return red_size_; }
		void setRed(int red_size) { red_size_ = red_size; }
		int green() const { return green_size_; }
		void setGreen(int green_size) { green_size_ = green_size; }
		int blue() const { return blue_size_; }
		void setBlue(int blue_size) { blue_size_ = blue_size; }
		int depth() const { return depth_size_; }
		void setDepth(int depth_size) { depth_size_ = depth_size; }
		int alpha() const { return alpha_size_; }
		void setAlpha(int alpha_size) { alpha_size_ = alpha_size; }
		int stencil() const { return stencil_size_; }
		void setStencil(int stencil_size) { stencil_size_ = stencil_size; }
		int samples() const { return samples_; }
		void setSamples(int samples) { samples_ = samples; }
		bool isExact() const { return exact_; }
		void setExact(bool exact) { exact_ = exact; }
		bool isVsync() const { return vsync_; }
		void setVsync( bool vsync ) { vsync_ = vsync; }

	private:
		int red_size_;
		int green_size_;
		int blue_size_;
		int depth_size_;
		int alpha_size_;
		int stencil_size_;
		int samples_;
		bool exact_;
		bool vsync_;
	};

	class Graphics : public Serializable
	{
	public:
		Graphics() :
			fullScreen_(false) {}

		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		const std::vector<ApiDefinition>& versions() const { return versions_; }
		void setVersions(const std::vector<ApiDefinition> &versions) { versions_ = versions; }
		const Config& config() const { return config_; }
		void setConfig(const Config &config) { config_ = config; }
		bool isFullScreen() const { return fullScreen_; }
		void setFullScreen(bool fullScreen) { fullScreen_ = fullScreen; }
#if defined(__QNX__)
		std::string getPixelFormat() { return pixelFormat_; }
		void setPixelFormat(const std::string& pixelFormat) { pixelFormat_ = pixelFormat; }
		int getNumBuffers() { return nbuffers_; }
		void setNumBuffers(int nbuffers) { nbuffers_ = nbuffers; }
		int getInterval() { return interval_; }
		void setInterval(int interval) { interval_ = interval; }
#endif
		std::string deviceId() const { return deviceid_; }
		void setDeviceId(const std::string& deviceid) { deviceid_ = deviceid; }
        int deviceIndex() const { return device_index_; }
        void setDeviceIndex(const int device_index) { device_index_ = device_index; }
	private:
		std::vector<ApiDefinition> versions_;
        std::string deviceid_;
        int device_index_ = -1;
		Config config_;
		bool fullScreen_;
#if defined(__QNX__)
		std::string pixelFormat_;
		int nbuffers_;
		int interval_;
#endif
	};

}

#endif
