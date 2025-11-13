/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "result.h"

using namespace tfw;

void Result::fromJsonValue(const ng::JsonValue& jvalue)
{
	const ng::JsonValue &jversion = jvalue["version"];
	if (!jversion.isNull())
	{
		if (jversion.isNumber())
		{
			setVersion((int)jversion.number());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].version'");
		}

	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].version'");
	}

	if(version_ > 1) {
		const ng::JsonValue &jbenchmarkVersion = jvalue["benchmark_version"];
		if (!jbenchmarkVersion.isNull())
		{
			if (jbenchmarkVersion.isString())
			{
				setBenchmarkVersion(jbenchmarkVersion.string());
			}
			else
			{
				throw std::runtime_error("Type of required field  is incorrect: 'results[].benchmark_version'");
			}

		}
		else
		{
			throw std::runtime_error("Value of required field is null: 'results[].benchmark_version'");
		}
	}

	const ng::JsonValue jtestId = jvalue["test_id"];
	if (!jtestId.isNull())
	{
		if (jtestId.isString())
		{
			setTestId(jtestId.string());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].test_id'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].test_id'");
	}

	const ng::JsonValue jresultId = jvalue["result_id"];
	if (!jresultId.isNull())
	{
		if (jresultId.isString())
		{
			setResultId(jresultId.string());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].result_id'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].result_id'");
	}

	const ng::JsonValue jstatus = jvalue["status"];
	if (!jstatus.isNull())
	{
		if (jstatus.isString())
		{
			std::string status = jstatus.string();
			if (status == "OK")
			{
				setStatus(Result::OK);
			}
			else if (status == "INCOMPATIBLE")
			{
				setStatus(Result::INCOMPATIBLE);
			}
			else if (status == "CANCELLED")
			{
				setStatus(Result::CANCELLED);
			}
			else if (status == "FAILED")
			{
				setStatus(Result::FAILED);
			}
			else
			{
				throw std::runtime_error("Value of required field is incorrect: 'results[].status'");
			}
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].status'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].status'");
	}

	const ng::JsonValue jerrorString = jvalue["error_string"];
	if (!jerrorString.isNull())
	{
		if (jerrorString.isString())
		{
			setErrorString(jerrorString.string());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].error_string'");
		}
	}
	else
	{
		throw std::runtime_error("Value of required field is null: 'results[].error_string'");
	}

	setScore(jvalue["score"].numberD(-1.0));
	setLoadTime((int)jvalue["load_time"].numberD(-1));
	setElapsedTime((int)jvalue["elapsed_time"].numberD(-1));
	setMeasuredTime((int)jvalue["measured_time"].numberD(-1));
	setUnit(jvalue["unit"].stringD(""));

	const ng::JsonValue &jcompute = jvalue["compute_result"];
	if (!jcompute.isNull())
	{
		computeResult().fromJsonValue(jcompute);
	}

	const ng::JsonValue &jgfx = jvalue["gfx_result"];
	if (!jgfx.isNull())
	{
		gfxResult().fromJsonValue(jgfx);
	}

    const ng::JsonValue &jdesc = jvalue["desc"];
    if (!jdesc.isNull())
    {
        descriptor().fromJsonValue(jdesc);
    }
}

ng::JsonValue Result::toJsonValue() const
{
	ng::JsonValue jresult;

	jresult["version"] = version();
	jresult["benchmark_version"] = benchmarkVersion();
	jresult["test_id"] = testId();
	jresult["result_id"] = resultId();
	jresult["status"] = statusString();
	jresult["error_string"] = errorString();
	jresult["score"] = score();
	jresult["unit"] = unit();
	jresult["load_time"] = loadTime();
	jresult["elapsed_time"] = elapsedTime();
	jresult["measured_time"] = measuredTime();

	const ComputeResult &computeRes = computeResult();
	if (!computeRes.isEmpty())
	{
		jresult["compute_result"] = ng::JsonValue();
		ng::JsonValue &jcomputeRes = jresult["compute_result"];
		jcomputeRes = computeRes.toJsonValue();
	}

	const GfxResult &gfxRes = gfxResult();
	if (!gfxRes.isEmpty())
	{
		jresult["gfx_result"] = ng::JsonValue();
		ng::JsonValue &jgfxRes = jresult["gfx_result"];
		jgfxRes = gfxRes.toJsonValue();
    }

    const Descriptor &desc = descriptor();
    jresult["desc"] = ng::JsonValue();
    ng::JsonValue &jdesc = jresult["desc"];
    jdesc = desc.toJsonValue();

	return jresult;
}

std::string Result::statusString() const
{
	switch (status_)
	{
	case Result::OK:
		return "OK";
		break;
	case Result::INCOMPATIBLE:
		return "INCOMPATIBLE";
		break;
	case Result::CANCELLED:
		return "CANCELLED";
		break;
	case Result::FAILED:
		return "FAILED";
		break;
	default:
		return "";
		break;
	}
}

void ResultGroup::fromJsonValue(const ng::JsonValue& jvalue)
{
	results_.clear();
	if (jvalue.isNull())
	{
		throw std::runtime_error("Object is null: json for object 'TestResult'");
	}


	const ng::JsonValue &jresults = jvalue["results"];
	if (!jresults.isArray())
	{
		throw std::runtime_error("Type of required field  is incorrect: 'results'");
	}

	size_t count = jresults.size();
	const ng::JsonValue& jflags = jvalue["flags"];
	if (!jflags.isNull() && jflags.isArray())
	{
		flags_.resize(jflags.size());
		for (size_t i(0); i < jflags.size(); ++i)
		{
			flags_[i] = jflags[i].stringD("");
		}
	}

	results_.resize(count);
	for (size_t i = 0; i < count; ++i)
	{
		results_[i].fromJsonValue(jresults[i]);
	}

	const ng::JsonValue &jcharts = jvalue["charts"];
	if (!jcharts.isNull())
	{
		if (jcharts.isArray())
		{
			charts_.resize(jcharts.size());
			for (size_t i = 0; i < jcharts.size(); ++i)
			{
				charts_[i].fromJsonValue(jcharts[i]);
			}
		}
		else {
			throw std::runtime_error("results[].charts is not an array");
		}
	}

	const ng::JsonValue &jconfiguration = jvalue["configuration"];
	if (!jconfiguration.isNull())
	{
		if (jconfiguration.isString())
		{
			setConfiguration(jconfiguration.string());
		}
		else
		{
			throw std::runtime_error("Type of required field  is incorrect: 'results[].configuration'");
		}
	}
	const ng::JsonValue &jtestId = jvalue["test_id"];
	if (jtestId.isString())
	{
	    testId_ = jtestId.string();
	}
	else
	{
	    // handle backward compatiblity with existing serialized result group
	    testId_ = results_.at(0).testId();
	}
}

ng::JsonValue ResultGroup::toJsonValue() const
{
	ng::JsonValue jtestResults;

	jtestResults["configuration"] = configuration();

	jtestResults["results"] = ng::JsonValue();
	ng::JsonValue& jresults = jtestResults["results"];

	if (!testId_.empty())
	{
	    jtestResults["test_id"] = testId_;
	}
	else if(!results_.empty())
	{
	    jtestResults["test_id"] = results_.front().testId();
	}

	size_t count = results_.size();
	jresults.resize(count);

	for (size_t i = 0; i < count; ++i)
	{
		jresults[i] = results_[i].toJsonValue();
	}

	const std::vector<Chart>& Charts = charts();
	if (!Charts.empty())
	{
		jtestResults["charts"] = ng::JsonValue();
		ng::JsonValue &jcharts = jtestResults["charts"];
		jcharts.resize(Charts.size());
		for (size_t i = 0; i < Charts.size(); ++i)
		{
			jcharts[i] = Charts[i].toJsonValue();
		}
	}
	const std::vector<std::string> &Flags = flags();
	if (Flags.size() != 0)
	{
		jtestResults["flags"] = ng::JsonValue();
		ng::JsonValue &jflags = jtestResults["flags"];
		jflags.resize(Flags.size());
		for (size_t i(0); i < Flags.size(); ++i)
		{
			jflags[i] = Flags[i];
		}
	}
	return jtestResults;
}

void ResultGroup::addResult(const Result& result)
{
	results_.push_back(result);
}

void ResultGroup::addChart(const Chart& chart)
{
    charts_.push_back(chart);
}

void ResultGroup::merge(const ResultGroup& other)
{
    for (size_t i = 0; i < other.results_.size(); ++i) {
        bool found = false;
        for (size_t j = 0; j < results_.size(); ++j) {
            if (results_.at(j).resultId() == other.results_.at(i).resultId()) {
                found = true;
                break;
            }
        }
        if (!found) {
            results_.push_back(other.results_.at(i));
        }
    }

    for (size_t i = 0; i < other.charts_.size(); ++i) {
        bool found = false;
        for (size_t j = 0; j < charts_.size(); ++j) {
            if (charts_.at(j).chartID() == other.charts_.at(i).chartID()) {
                found = true;
                break;
            }
        }
        if (!found) {
            charts_.push_back(other.charts_.at(i));
        }
    }
}
