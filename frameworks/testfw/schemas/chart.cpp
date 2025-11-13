/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "chart.h"

using namespace tfw;

void Samples::fromJsonValue(const ng::JsonValue &jvalue)
{
	setName(jvalue["name"].stringD(""));
	const ng::JsonValue& jvalues = jvalue["values"];
	values().resize(jvalues.size());
	for (size_t i = 0; i < jvalues.size(); ++i)
	{
		values()[i] = jvalues[i].numberD(-1.0);
	}
}

ng::JsonValue Samples::toJsonValue() const
{
	ng::JsonValue jvalue;
	jvalue["name"] = name_;
	jvalue["values"] = ng::JsonValue();
	ng::JsonValue& jvalues = jvalue["values"];
	jvalues.resize(values_.size());
	for (size_t i = 0; i < values_.size(); ++i)
	{
		jvalues[i] = values_[i];
	}
	return jvalue;
}

void Chart::fromJsonValue(const ng::JsonValue &jvalue)
{
	setChartID(jvalue["chart_id"].stringD(""));
	setDomainAxis(jvalue["domain_axis"].stringD(""));
	setSampleAxis(jvalue["sample_axis"].stringD(""));
	domain().fromJsonValue(jvalue["domain"]);
	const ng::JsonValue &jvalues = jvalue["values"];
	values().resize(jvalues.size());
	for (size_t i = 0; i < jvalues.size(); ++i)
	{
		values()[i].fromJsonValue(jvalues[i]);
	}
}

ng::JsonValue Chart::toJsonValue() const
{
	ng::JsonValue jvalue;
	jvalue["chart_id"] = chart_id_;
	jvalue["domain_axis"] = domain_axis_;
	jvalue["sample_axis"] = sample_axis_;
	jvalue["domain"] = domain_.toJsonValue();

	jvalue["values"] = ng::JsonValue();
	ng::JsonValue &jvalues = jvalue["values"];
	jvalues.resize(values_.size());
	for (size_t i = 0; i < values_.size(); ++i)
	{
		jvalues[i] = values_[i].toJsonValue();
	}
	return jvalue;
}