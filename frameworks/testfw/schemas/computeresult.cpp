/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "computeresult.h"

using namespace tfw;

void ComputeResult::fromJsonValue(const ng::JsonValue &jvalue)
{
	if (jvalue.isNull())
	{
		return;
	}

	if (!jvalue.isObject())
	{
		throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result'");
	}

	const ng::JsonValue &jconfigId = jvalue["configId"];
	if (!jconfigId.isNull())
	{
		if (jconfigId.isNumber())
		{
			setConfigId((int)jconfigId.number());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result.configId'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].compute_result.configId'");
	}

	const ng::JsonValue &jplatformId = jvalue["platformId"];
	if (!jplatformId.isNull())
	{
		if (jplatformId.isNumber())
		{
			setPlatformId((int)jplatformId.number());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result.platformId'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].compute_result.platformId'");
	}

	const ng::JsonValue &jideviceId = jvalue["deviceId"];
	if (!jideviceId.isNull())
	{
		if (jideviceId.isNumber())
		{
			setDeviceId((int)jideviceId.number());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result.deviceId'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].compute_result.deviceId'");
	}

	const ng::JsonValue &jiterationCount = jvalue["iteration_count"];
	if (!jiterationCount.isNull())
	{
		if (jiterationCount.isNumber())
		{
			setIterationCount((int)jiterationCount.number());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result.iteration_count'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].compute_result.iteration_count'");
	}

	const ng::JsonValue &jdeviceVendor = jvalue["device_vendor"];
	if (!jdeviceVendor.isNull())
	{
		if (jdeviceVendor.isString())
		{
			setDeviceVendor(jdeviceVendor.string());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result.device_vendor'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].compute_result.device_vendor'");
	}

	const ng::JsonValue &jdeviceName = jvalue["device_name"];
	if (!jdeviceName.isNull())
	{
		if (jdeviceName.isString())
		{
			setDeviceName(jdeviceName.string());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result.device_name'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].compute_result.device_name'");
	}

	const ng::JsonValue &jdeviceType = jvalue["device_type"];
	if (!jdeviceType.isNull())
	{
		if (jdeviceType.isString())
		{
			setDeviceType(jdeviceType.string());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result.device_type'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].compute_result.device_type'");
	}

	const ng::JsonValue &jdeviceDriver = jvalue["device_driver"];
	if (!jdeviceDriver.isNull())
	{
		if (jdeviceDriver.isString())
		{
			setDeviceDriver(jdeviceDriver.string());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].compute_result.device_driver'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].compute_result.device_driver'");
	}
}

ng::JsonValue ComputeResult::toJsonValue() const
{
	ng::JsonValue jvalue;
	if (isEmpty())
	{
		return jvalue;
	}

	jvalue["configId"] = configId();
	jvalue["platformId"] = platformId();
	jvalue["deviceId"] = deviceId();
	jvalue["iteration_count"] = iterationCount();
	jvalue["device_vendor"] = deviceVendor();
	jvalue["device_name"] = deviceName();
	jvalue["device_type"] = deviceType();
	jvalue["device_driver"] = deviceDriver();

	return jvalue;
}


