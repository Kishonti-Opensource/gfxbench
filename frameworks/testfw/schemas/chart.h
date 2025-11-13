/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_SCHEMAS_CHART_H
#define TFW_SCHEMAS_CHART_H

#include "serializable.h"
#include <vector>

namespace tfw {

	class Samples : public Serializable
	{
	public:
		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);
	
		void setName(std::string name) { name_ = name; }
		std::string name() const { return name_; }
		const std::vector<double>& values() const { return values_; }
		std::vector<double>& values() { return values_; }
        void setValues(const std::vector<double> &values) { values_ = values; }
        
        void addValue(double value) { values_.push_back(value); }
	private:
		std::vector<double> values_;
		std::string name_;
	};
	
	class Chart : public Serializable
	{
	public:
		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);
	
		const Samples& domain() const { return domain_; }
		Samples& domain() { return domain_; }
        void setDomain(const Samples &domain) { domain_ = domain; }
		const std::vector<Samples>& values() const { return values_; }
		std::vector<Samples>& values() { return values_; }
        void setValues(const std::vector<Samples> &values) { values_ = values; }
		void setChartID(std::string chart_id) { chart_id_ = chart_id; }
		std::string chartID() const { return chart_id_; }
		void setDomainAxis(std::string domain_axis) { domain_axis_ = domain_axis; }
		std::string domainAxis() const { return domain_axis_; }
		void setSampleAxis(std::string sample_axis) { sample_axis_ = sample_axis; }
		std::string sampleAxis() const { return sample_axis_; }
        
        void addDomainValue(double value) { domain_.addValue(value); }
        void addSampleValue(int samples_index, double value) { values_[samples_index].addValue(value); }
	private:
		Samples domain_;
		std::vector<Samples> values_;
		std::string chart_id_;
		std::string domain_axis_;
		std::string sample_axis_;
	};

}

#endif