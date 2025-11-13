/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_SCHEMAS_GFXRESULT_H
#define TFW_SCHEMAS_GFXRESULT_H

#include "serializable.h"
#include <string>
#include <vector>

namespace tfw {
	class GfxResult : public Serializable
	{
	public:
		GfxResult() :
			fps_(-1.0),
			frame_count_(-1),
			egl_config_id_(-1),
			surface_width_(-1),
			surface_height_(-1),
			is_vsync_limited_(false) {}

		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		double fps() const { return fps_; }
		void setFps(double fps) { fps_ = fps; }
		int frameCount() const { return frame_count_; }
		void setFrameCount(int frame_count) { frame_count_ = frame_count; }
		int eglConfigId() const { return egl_config_id_; }
		void setEglConfigId(int egl_config_id) { egl_config_id_ = egl_config_id; }
		const std::string vendor() const { return vendor_; }
		void setVendor(const std::string &vendor) { vendor_ = vendor; }
		const std::string renderer() const { return renderer_; }
		void setRenderer(const std::string& renderer) { renderer_ = renderer; }
		const std::string graphicsVersion() const { return graphics_version_; }
		void setGraphicsVersion(const std::string &graphics_version) { graphics_version_ = graphics_version; }
		int surfaceWidth() const { return surface_width_; }
		void setSurfaceWidth(int surface_width) { surface_width_ = surface_width; }
		int surfaceHeight() const { return surface_height_; }
		void setSurfaceHeight(int surface_height) { surface_height_ = surface_height; }
		const std::vector<std::string>& extraData() const { return extra_data_; }
		std::vector<std::string>& extraData() { return extra_data_; }
		const std::vector<int>& getFrameTimes() const { return frame_times_; }
		std::vector<int>& frameTimes() { return frame_times_; }
		const std::vector<std::vector<int> >& getCPUfreqs() const { return cpu_freqs_; }
		std::vector<std::vector<int> >& CPUfreqs() { return cpu_freqs_; }
		bool isVSyncLimited() const { return is_vsync_limited_; }
		void setIsVSyncLimited(bool is_vsync_limited) { is_vsync_limited_ = is_vsync_limited; }

		bool isEmpty() const {
			return fps_ == -1.0 && frame_count_ == -1 && surface_width_ == -1 && surface_height_ == -1 &&
				egl_config_id_ == -1 && vendor_.empty() && renderer_.empty() && graphics_version_.empty() && extra_data_.empty();
		}

	private:
		double fps_;
		int frame_count_;
		int egl_config_id_;
		std::vector<int> frame_times_;
		std::string vendor_;
		std::string renderer_;
		std::string graphics_version_;
		int surface_width_;
		int surface_height_;
		std::vector<std::string> extra_data_;
		std::vector<std::vector<int> > cpu_freqs_;
		bool is_vsync_limited_;
	};


}

#endif