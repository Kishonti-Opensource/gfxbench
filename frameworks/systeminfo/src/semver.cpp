/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <cstdlib>
#include <string>
#include <sstream>
#include "semver.h"

SemVersion::SemVersion(uint major, uint minor, uint patch)
{
    this->set(major, minor, patch);
}

SemVersion::SemVersion(uint major, uint minor, uint patch, const char *pre_release, const char *build)
{
    this->set(major, minor, patch, pre_release, build);
}

SemVersion::SemVersion(uint major, uint minor, uint patch, const std::string &pre_release, const std::string &build)
{
    this->set(major, minor, patch, pre_release, build);
}

SemVersion::SemVersion(const char *version)
{
    this->set(version);
}

SemVersion::SemVersion(const std::string &version)
{
    this->set(version);
}

bool SemVersion::operator==(const SemVersion &other) const
{
    return (this->str() == other.str());
}

bool SemVersion::operator>(const SemVersion &other) const
{
    if (this->major > other.major)
    {
        return true;
    }
    else if (this->major < other.major)
    {
        return false;
    }

    if (this->minor > other.minor)
    {
        return true;
    }
    else if (this->minor < other.minor)
    {
        return false;
    }

    if (this->patch > other.patch)
    {
        return true;
    }
    else if (this->patch < other.patch)
    {
        return false;
    }

    return false;
}

void SemVersion::set(uint major, uint minor, uint patch)
{
    this->major = major;
    this->minor = minor;
    this->patch = patch;
    this->pre_release = std::string();
    this->build = std::string();
}

void SemVersion::set(uint major, uint minor, uint patch, const char *pre_release, const char *build)
{
    this->set(major, minor, patch);
    this->pre_release = std::string(pre_release);
    this->build = std::string(build);
}

void SemVersion::set(uint major, uint minor, uint patch, const std::string &pre_release, const std::string &build)
{
    this->set(major, minor, patch);
    this->pre_release = std::string(pre_release);
    this->build = std::string(build);
}

void SemVersion::set(const char *version)
{
    std::string strVersion(version);

    this->set(strVersion);
}

void SemVersion::set(const std::string &version)
{
    size_t firstDotPos;
    size_t secondDotPos;
    size_t dashPos;
    size_t plusPos;

    firstDotPos = version.find('.');
    secondDotPos = version.find('.', firstDotPos + 1);
    dashPos = version.find('-', secondDotPos + 1);
    plusPos = version.find('+', secondDotPos + 1);

    if (firstDotPos == std::string::npos || secondDotPos == std::string::npos)
    {
        throw bad_format_exception();
    }

    this->major = std::atoi(version.substr(0, firstDotPos).c_str());
    this->minor = std::atoi(version.substr(firstDotPos + 1, secondDotPos - firstDotPos).c_str());
    if (dashPos == std::string::npos && plusPos == std::string::npos)
    {
        this->patch = std::atoi(version.substr(secondDotPos + 1).c_str());
        this->pre_release = std::string();
        this->build = std::string();
    }
    else if (plusPos == std::string::npos)
    {
        this->patch = std::atoi(version.substr(secondDotPos + 1, dashPos - secondDotPos).c_str());
        this->pre_release = std::string(version.substr(dashPos + 1));
        this->build = std::string();
    }
    else if (dashPos == std::string::npos)
    {
        this->patch = std::atoi(version.substr(secondDotPos + 1, plusPos - secondDotPos).c_str());
        this->pre_release = std::string();
        this->build = std::string(version.substr(plusPos + 1));
    }
    else
    {
        if (plusPos < dashPos)
        {
            throw bad_format_exception();
        }

        this->patch = std::atoi(version.substr(secondDotPos + 1, dashPos - secondDotPos).c_str());
        this->pre_release = std::string(version.substr(dashPos + 1, plusPos - dashPos));
        this->build = std::string(version.substr(plusPos));
    }
}

std::string SemVersion::str() const
{
    std::ostringstream stream;

    stream << this->major << '.' << this->minor << '.' << this->patch;

    if (this->pre_release.length() > 0)
    {
        stream << '-' << this->pre_release;
    }
    if (this->build.length() > 0)
    {
        stream << '+' << this->build;
    }

    return stream.str();
}
