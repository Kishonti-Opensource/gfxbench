/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef VARIANT_H
#define VARIANT_H

#include "cursor.h"

#include "ng/format.h"

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>



struct Variant {
    Variant() : type(Cursor::FIELD_TYPE_NULL), integer(0) {}
    Variant(bool value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value ? 1 : 0) {}
    Variant(const char* value) : type(Cursor::FIELD_TYPE_STRING), text(value) {}
    Variant(const std::string& value) : type(Cursor::FIELD_TYPE_STRING), text(value) {}
    Variant(const std::string& value, bool) : type(Cursor::FIELD_TYPE_BLOB), text(value) {}
    Variant(float value) : type(Cursor::FIELD_TYPE_FLOAT), floating(value) {}
    Variant(double value) : type(Cursor::FIELD_TYPE_FLOAT), floating(value) {}
    Variant(short value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value) {}
    Variant(unsigned short value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value) {}
    Variant(int value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value) {}
    Variant(unsigned value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value) {}
    Variant(long value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value) {}
    Variant(unsigned long value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value) {}
    Variant(long long value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value) {}
    Variant(unsigned long long value) : type(Cursor::FIELD_TYPE_INTEGER), integer(value) {}
    bool toBoolean() const {
        switch (type) {
        case Cursor::FIELD_TYPE_NULL:
            return false;
        case Cursor::FIELD_TYPE_INTEGER:
            return integer != 0;
        case Cursor::FIELD_TYPE_FLOAT:
            return floating != 0.0;
        case Cursor::FIELD_TYPE_STRING:
        case Cursor::FIELD_TYPE_BLOB:
            return !text.empty() && (text != "no");
        default:
            assert(false);
            return false;
        }
    }
    long long toLong() const {
        switch (type) {
        case Cursor::FIELD_TYPE_NULL:
            return 0;
        case Cursor::FIELD_TYPE_INTEGER:
            return integer;
        case Cursor::FIELD_TYPE_FLOAT:
            return static_cast<long long>(floating);
        case Cursor::FIELD_TYPE_STRING:
        case Cursor::FIELD_TYPE_BLOB:
            return 0;
        default:
            assert(false);
            return 0;
        }
    }
    double toDouble() const {
        switch (type) {
        case Cursor::FIELD_TYPE_NULL:
            return 0.0;
        case Cursor::FIELD_TYPE_INTEGER:
            return static_cast<double>(integer);
        case Cursor::FIELD_TYPE_FLOAT:
            return floating;
        case Cursor::FIELD_TYPE_STRING:
        case Cursor::FIELD_TYPE_BLOB:
            return 0.0;
        default:
            assert(false);
            return 0.0;
        }
    }
    std::string toString() const {
        switch (type) {
        case Cursor::FIELD_TYPE_NULL:
            return "";
        case Cursor::FIELD_TYPE_INTEGER: {
            std::ostringstream oss;
            oss << integer;
            return oss.str();
        }
        case Cursor::FIELD_TYPE_FLOAT:
            return ng::formatNumberMetric(floating);
        case Cursor::FIELD_TYPE_STRING:
        case Cursor::FIELD_TYPE_BLOB:
            return text;
        default:
            assert(false);
            return "";
        }
    }

    Cursor::Type type;
    int64_t integer;
    double floating;
    std::string text;
};



#endif // VARIANT_H
