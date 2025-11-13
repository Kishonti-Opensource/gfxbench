/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SEMVER_H_
#define SEMVER_H_

#include <stdexcept>
#include <string>

typedef unsigned int uint;

class SemVersion
{
public:
    SemVersion(uint, uint, uint);
    SemVersion(uint, uint, uint, const char *, const char *);
    SemVersion(uint, uint, uint, const std::string &, const std::string &);
    SemVersion(const char *);
    SemVersion(const std::string &);

    bool operator==(const SemVersion &) const;
    bool operator>(const SemVersion &) const;

    void set(uint, uint, uint);
    void set(uint, uint, uint, const char *, const char *);
    void set(uint, uint, uint, const std::string &, const std::string &);
    void set(const char *);
    void set(const std::string &);

    std::string str(void) const;

    uint major;
    uint minor;
    uint patch;
    std::string pre_release;
    std::string build;
};

class bad_format_exception: public std::runtime_error
{
public:
    bad_format_exception(void): std::runtime_error("Bad version format.") {};
};

#endif /* SEMVER_H_ */
