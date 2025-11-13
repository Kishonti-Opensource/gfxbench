/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_SCHEMAS_ENVIRONMENT_H
#define TFW_SCHEMAS_ENVIRONMENT_H

#include "serializable.h"
#include "graphics.h"
#include "compute.h"

namespace tfw
{
	class Environment : public Serializable
	{
	public:
		Environment() :
			width_(0),
			height_(0) {}

		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		int width() const { return width_; }
		void setWidth(int width) { width_ = width; }
		int height() const { return height_; }
		void setHeight(int height) { height_ = height; }
		const std::string& readPath() const { return read_path_; }
		void setReadPath(const std::string &read_path) { read_path_ = read_path; }
		const std::string& writePath() const { return write_path_; }
		void setWritePath(const std::string &write_path) { write_path_ = write_path; }
		const Graphics& graphics() const { return graphics_; }
		Graphics& graphics() { return graphics_; }
		void setGraphics(const Graphics &graphics) { graphics_ = graphics; }
		const Compute& compute() const { return compute_; }
		Compute& compute() { return compute_; }
		void setCompute(const Compute &compute) { compute_ = compute; }
		const std::string& locale() const { return locale_; }
		void setLocale(const std::string &locale) { locale_ = locale; }
        
        const std::string& device() const { return device_; }
        void setDevice(const std::string &device) { device_ = device; }
        const std::string& os() const { return os_; }
        void setOs(const std::string &os) { os_ = os; }
        const std::string& timestamp() const { return timestamp_; }
        void setTimestamp(const std::string &timestamp) { timestamp_ = timestamp; }

	private:
		int width_;
		int height_;
		std::string read_path_;
		std::string write_path_;
		Graphics graphics_;
		Compute compute_;
		std::string locale_;
        
        std::string device_;
        std::string os_;
        std::string timestamp_;
	};
}

#endif