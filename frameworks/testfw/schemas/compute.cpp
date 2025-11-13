/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compute.h"

using namespace tfw;

ng::JsonValue Compute::toJsonValue() const
{
	ng::JsonValue jcompute;
	{
		switch (type())
		{
		case Compute::UNSET:
			jcompute["type"] = "UNSET";
			break;
		case Compute::CL:
			jcompute["type"] = "CL";
			break;
		case Compute::CS:
			jcompute["type"] = "CS";
			break;
		case Compute::CU:
			jcompute["type"] = "CU";
			break;
		case Compute::MTL:
			jcompute["type"] = "MTL";
			break;
		case Compute::RS:
			jcompute["type"] = "RS";
			break;
		}
		jcompute["config_index"] = configIndex();
		jcompute["iter_count"] = iterCount();
		jcompute["dump"] = dump();
		jcompute["warmup"] = warmup();
		jcompute["fastmath"] = fastmath();
		jcompute["verify"] = verify();
		jcompute["verifyEps"] = verifyEps();

	}
	return jcompute;
}

void Compute::fromJsonValue(const ng::JsonValue& value)
{
	if (!value["type"].isNull())
	{
		if (value["type"].isString())
		{
			std::string type = value["type"].string();
			if (type == "CL")
			{
				setType(Compute::CL);
			}
			else if (type == "CS")
			{
				setType(Compute::CS);
			}
			else if (type == "CU")
			{
				setType(Compute::CU);
			}
			else if (type == "RS")
			{
				setType(Compute::RS);
			}
			else if (type == "MTL")
			{
				setType(Compute::MTL);
			}
			else if (type == "UNSET")
			{
				setType(Compute::UNSET);
			}
			else
			{
				throw std::runtime_error("Value of required field is incorrect: 'type'");
			}
		}
		else
		{
			throw std::runtime_error("Type of required field is incorrect: 'type'");
		}
	}
	else
	{
		throw std::runtime_error("Required field is null: 'type'");
	}

	setConfigIndex((int)value["config_index"].numberD(0));
	setIterCount((int)value["iter_count"].numberD(-1));
	setDump((bool)value["dump"].booleanD(false));
	setWarmup((bool)value["warmup"].booleanD(true));
	setFastmath((bool)value["fastmath"].booleanD(true));
	setVerify((bool)value["verify"].booleanD(false));
	setVerifyEps((float)value["verifyEps"].numberD(0.0));
}
