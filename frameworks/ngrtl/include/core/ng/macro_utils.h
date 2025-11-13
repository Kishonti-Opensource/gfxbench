/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_MACRO_UTILS_H_
#define NG_MACRO_UTILS_H_

#include <string>
#include <sstream>
#include <vector>

namespace ng {
	namespace macro_utils {

		struct TableItem
		{
			const char *name;
			int val;
		};
		inline std::string findItemNameByVal(const TableItem *t, int val) {
			const TableItem *i = t;
			while (i->name) {
				if (i->val == val) {
					return i->name;
				}
				++i;
			}
			std::ostringstream os;
			os << val;
			return os.str();
		}
		inline int findItemValByName(const TableItem *t, const std::string &name, int def)
		{
		    int val = def;
		    const TableItem *i = t;
		    while (i->name) {
		        std::string n = i->name;
		        if (n == name) {
		            return i->val;
		        }
		        ++i;
		    }
		     return val;
		}
		template<typename T>
		inline std::string formatBitmask(const TableItem *t, T flags, std::string sep = "|") {
			std::ostringstream os;
			bool first = true;
			for (size_t i = 0; i < sizeof(T)*8; ++i) {
				if ((flags >> i) & 1) {
					if (!first) {
						os << sep;
					}
					os << findItemNameByVal(t, 1<<i);
					first = false;
				}
			}
			return os.str();
		}
		template<typename T>
		inline std::vector<std::string> bitmaskArray(const TableItem *t, T flags) {
			std::vector<std::string> result;
			for (size_t i = 0; i < sizeof(T) * 8; ++i) {
				if ((flags >> i) & 1) {
					 result.push_back(findItemNameByVal(t, 1 << i));
				}
			}
			return result;
		}
		class Table {
		public:
			Table(const ng::macro_utils::TableItem *t) : items_(t) {}
			std::string operator()(int val) const { return ng::macro_utils::findItemNameByVal(items_, val); }
			int operator()(const std::string &name, int def) const { return ng::macro_utils::findItemValByName(items_, name, def); }
			std::string bitmask(long flags) const { return ng::macro_utils::formatBitmask(items_, flags); }
			std::vector<std::string> bitmaskArray(long flags) const { return ng::macro_utils::bitmaskArray(items_, flags); }
		private:
			const ng::macro_utils::TableItem *items_;
		};

	}
}
#define NG_TABLE_START(var) static const ng::macro_utils::TableItem ITEMS_##var[] = {
#define NG_TABLE_ITEM0(item) { #item, item },
#define NG_TABLE_ITEM1(item, itemval) { item, itemval },
#define NG_TABLE_END(var) {0, 0} };\
	static const ::ng::macro_utils::Table var(ITEMS_##var);

#endif  // NG_MACRO_UTILS_H_
