/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <string>
#include <map>
#include <stdint.h>
#include "systeminfo.h"

namespace sysinf
{
#ifndef DISABLE_COLLECTORS
    class DeviceInfoCollector;
#endif
	class Property
	{
		std::string value_;
		std::string type_;
	public:
		Property(const char *value, const char *type)
			: value_(value), type_(type)
		{}
		Property() {}
		bool getBool() const;
		int32_t getInt() const;
		int64_t getLong() const;
		double getDouble() const;
		const std::string &getString() const;
		const std::string &getType() const;

		template<typename T> void get(T &result) const;
		template<typename T> void set(T &value);
	};

	class PropertyIter;
	class Properties
	{
		friend class PropertyIter;
		typedef std::map<std::string, Property> map_t;
		map_t value_;
	public:
		const Property *get(const std::string &name) const;

		bool getBool(const std::string &name, bool defaultValue = false) const;
		void setBool(const std::string &name, bool value);
        void set(const std::string &name, bool value);

		int32_t getInt(const std::string &name, int32_t defaultValue = 0) const;
		void setInt(const std::string &name, int32_t value);
        void set(const std::string &name, int value);
        void set(const std::string &name, unsigned value);

		int64_t getLong(const std::string &name, int64_t defaultValue = 0) const;
		void setLong(const std::string &name, int64_t value);
        void set(const std::string &name, long value);
        void set(const std::string &name, unsigned long value);
        void set(const std::string &name, long long value);
        void set(const std::string &name, unsigned long long value);
        
		double getDouble(const std::string &name, double defaultValue = 0.0) const;
		void setDouble(const std::string &name, double value);
        void set(const std::string &name, double value);

		std::string getString(const std::string &name, std::string defaultValue = "") const;
		void setString(const std::string &name, const std::string &value);
        void set(const std::string &name, const std::string &value);
		
		const std::string toJsonString(bool compact = true) const;
		void updateFromJsonString(const std::string &s);
		void updateFromFolder(const std::string &folder);

		bool has(const std::string &name) const;
		PropertyIter iterator() const;
		PropertyIter groupIterator(const std::string &namePrefix) const;
#ifndef DISABLE_COLLECTORS
        void collect(DeviceInfoCollector& deviceInfoCollector, sysinf::SystemInfo& systeminfo);
#endif
	};

	class PropertyIter
	{
		typedef Properties::map_t::const_iterator const_it;
		const Properties::map_t &values_;
		const_it iter_;
		const std::string group_;

		friend class Properties;
		PropertyIter(const Properties::map_t &values_, const std::string &group = "");
		const_it findPrefix(const Properties::map_t &values_, const std::string &group = "") const;
	public:
		void first();
		void next();
		bool done() const;

		const std::string &name() const;
		const Property &value() const;
	};
}

#endif
