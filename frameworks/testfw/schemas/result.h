/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_SCHEMAS_RESULT_H
#define TFW_SCHEMAS_RESULT_H

#include "serializable.h"
#include <vector>
#include "chart.h"
#include "gfxresult.h"
#include "computeresult.h"
#include "descriptors.h"

#ifdef Status
#undef Status
#endif

namespace tfw {
	class Result : public Serializable
	{
	public:
		enum Status
		{
			OK,
			INCOMPATIBLE,
			CANCELLED,
			FAILED
		};

		Result() :
			version_(3),
			benchmark_version_("1.0.0"),
			status_(FAILED),
			score_(-1.0),
			load_time_(-1),
			elapsed_time_(-1),
			measured_time_(-1) {}

		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		int version() const { return version_; }
		void setVersion(int version) { version_ = version; }

		const std::string& benchmarkVersion() const { return benchmark_version_; }
		void setBenchmarkVersion(const std::string &benchmark_version) { benchmark_version_ = benchmark_version; }

		const std::string& testId() const { return test_id_; }
		void setTestId(const std::string &test_id) { test_id_ = test_id; }

		double score() const { return score_; }
		void setScore(double score) { score_ = score; }

		const std::string& unit() const { return unit_; }
		void setUnit(const std::string &unit) { unit_ = unit; }

		const std::string& resultId() const { return result_id_; }
		void setResultId(const std::string& result_id) { result_id_ = result_id; }

		int loadTime() const { return load_time_; }
		void setLoadTime(int load_time) { load_time_ = load_time; }

		int elapsedTime() const { return elapsed_time_; }
		void setElapsedTime(int elapsed_time) { elapsed_time_ = elapsed_time; }

		int measuredTime() const { return measured_time_; }
		void setMeasuredTime(int measured_time) { measured_time_ = measured_time; }

		Status status() const { return status_; }
		void setStatus(Status status) { status_ = status; }
		std::string statusString() const;

		const std::string& errorString() const { return error_string_; }
		void setErrorString(const std::string &error_string) { error_string_ = error_string; }

		const GfxResult& gfxResult() const { return gfx_result_; }
		GfxResult& gfxResult() { return gfx_result_; }
		const ComputeResult& computeResult() const { return compute_result_; }
		ComputeResult& computeResult() { return compute_result_; }

        const Descriptor& descriptor() const { return desc_; }
        Descriptor& descriptor() { return desc_; }
        void setDescriptor(const Descriptor& desc) { desc_ = desc; }

	private:
		int version_;
		std::string benchmark_version_;
		std::string test_id_;
		std::string result_id_;
		Status status_;
		std::string error_string_;

		double score_;
		std::string unit_;

		int load_time_;
		int elapsed_time_;
		int measured_time_;

		GfxResult gfx_result_;
		ComputeResult compute_result_;
        Descriptor desc_;
	};

	class ResultGroup : public Serializable
	{
	public:
		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		void setTestId(const std::string &testId) { testId_ = testId; }
		std::string testId() const { return testId_; }
		long long rowId() const { return rowId_; }
		void setRowId(long long rowId) { rowId_ = rowId; }
		const std::string& configuration() const { return configuration_; }
		void setConfiguration(const std::string& configuration) { configuration_ = configuration; }
		const std::vector<Result>& results() const { return results_; }
		std::vector<Result>& results() { return results_; }
		const std::vector<Chart>& charts() const { return charts_; }
		std::vector<Chart>& charts() { return charts_; }
		std::vector<std::string>& flags() { return flags_; }
		const std::vector<std::string>& flags() const { return flags_; }

		void addResult(const Result& result);
        void addChart(const Chart& chart);
        void merge(const ResultGroup& other);
	private:
		long long rowId_;
		std::string configuration_;
		std::vector<Result> results_;
		std::vector<Chart> charts_;
		std::vector<std::string> flags_;
		std::string testId_;
	};

	typedef std::vector<ResultGroup> ResultGroupList;
}

#endif
