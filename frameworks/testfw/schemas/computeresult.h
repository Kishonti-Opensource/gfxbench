/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_SCHEMAS_COMPUTERESULT_H
#define TFW_SCHEMAS_COMPUTERESULT_H

#include "serializable.h"
#include <string>

namespace tfw
{
	class ComputeResult : public Serializable
	{
	public:
		ComputeResult() :
			configId_(-1),
			platformId_(-1),
			deviceId_(-1),
			iteration_count_(-1) {}

		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		int configId() const { return configId_; }
		void setConfigId(int configId) { configId_ = configId; }

		int platformId() const { return platformId_; }
		void setPlatformId(int platformId) { platformId_ = platformId; }

		int deviceId() const { return deviceId_; }
		void setDeviceId(int deviceId) { deviceId_ = deviceId; }

		int iterationCount() const { return iteration_count_; }
		void setIterationCount(int iteration_count) { iteration_count_ = iteration_count; }

		const std::string& deviceVendor() const { return device_vendor_; }
		void setDeviceVendor(const std::string &device_vendor) { device_vendor_ = device_vendor; }
		const std::string& deviceName() const { return device_name_; }
		void setDeviceName(const std::string &device_name) { device_name_ = device_name; }
		const std::string& deviceType() const { return device_type_; }
		void setDeviceType(const std::string &device_type) { device_type_ = device_type; }
		const std::string& deviceDriver() const { return device_driver_; }
		void setDeviceDriver(const std::string &device_driver) { device_driver_ = device_driver; }

		bool isEmpty() const { return iteration_count_ == -1 && device_name_.empty() && device_driver_.empty(); }

	private:
		int configId_;
		int platformId_;
		int deviceId_;

		int iteration_count_;

		std::string device_vendor_;
		std::string device_name_;
		std::string device_type_;
		std::string device_driver_;
	};
}

#endif