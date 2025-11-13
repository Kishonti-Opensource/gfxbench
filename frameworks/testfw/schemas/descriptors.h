/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#ifdef _MSC_VER
#pragma warning(disable: 4275)
#endif

#include <algorithm>
#include <vector>
#include <string>
#include <stdint.h>
#include "ng/json.h"
#include "serializable.h"
#include "environment.h"

namespace tfw
{
    class JUnit
    {
    public:
		const std::string& pkg() const { return pkg_; }
        void setPkg(const std::string &pkg) { pkg_ = pkg;}

        const std::string& cls() const { return cls_; }
        void setCls(const std::string &cls) { cls_ = cls;}

        const std::string& test() const { return test_; }
        void setTest(const std::string &test) { test_ = test;}

    private:
        std::string pkg_;
        std::string cls_;
        std::string test_;
    };

	class Descriptor : public Serializable
	{
	public:
        Descriptor() :
            version_(1) {}

		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

        int version() const { return version_; }
        void setVersion(int version) { version_ = version; }

		const std::string& testId() const { return test_id_; }
		void setTestId(const std::string &test_id) { test_id_ = test_id;}

        const std::string& factoryMethod() const { return factory_method_; }
        void setFactoryMethod(const std::string &factory_method) { factory_method_ = factory_method; }

        const std::string& jclass() const { return jclass_; }
        void setJclass(const std::string &jclass) { jclass_ = jclass;}

		const Environment& env() const { return env_; }
		Environment& env() { return env_; }
		void setEnv(const Environment &env) { env_ = env; }
		const std::string &dataPrefix() const { return data_prefix_; }
		void setDataPrefix(const std::string &data_prefix) { data_prefix_ = data_prefix; }

		void setPreloadLibs(const std::vector<std::string> &preload_libs) { preload_libs_ = preload_libs; }
		const std::vector<std::string> &preloadLibs() const { return preload_libs_; }

		bool rawConfigHasKey(const std::string &key) const;

		double rawConfign(const std::string &key) const;
		bool rawConfigb(const std::string &key) const;
		std::string rawConfigs(const std::string &key) const;
		std::vector<std::string> rawConfigsv(const std::string &key) const;
		std::vector<double> rawConfignv(const std::string &key) const;
		std::vector<bool> rawConfigbv(const std::string &key) const;

		double rawConfign(const std::string &key, double def) const;
		bool rawConfigb(const std::string &key, bool def) const;
		std::string rawConfigs(const std::string &key, const std::string &def) const;
		template<typename T>
		void setRawConfig(const std::string &key, const T& val)
		{
			raw_config_[key.c_str()] = val;
		}

        const JUnit& junit() const { return junit_; }
        JUnit& junit() { return junit_; }
        void setJUnit(const JUnit &junit) { junit_ = junit; }

	private:
		int version_;
		std::string test_id_;
		std::string factory_method_;
        std::string jclass_;
		Environment env_;
		ng::JsonValue raw_config_;
		std::string data_prefix_;
		std::vector<std::string> preload_libs_;
        JUnit junit_;
	};

	class DescriptorList : public Serializable
	{
	public:
		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		const std::vector<Descriptor>& descriptors() const { return descriptors_; }
		std::vector<Descriptor>& descriptors() { return descriptors_; }

	private:
		std::vector<Descriptor> descriptors_;
	};
}

#endif
