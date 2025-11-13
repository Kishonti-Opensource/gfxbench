/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CSVISTREAM_H
#define CSVISTREAM_H

#include <iostream>
#include <sstream>
#include <string>

class csvistream
{
private:
    std::istream &m_is;
    const char m_separator;
    const bool m_commentable;
	mutable bool m_newline;

	void scan_ws () const;
	void scan_comment () const;
    void scan (std::string *s = 0) const;

    template <typename T> struct set_value 
	{
        void operator () (std::string in, T &v) const 
		{
            std::istringstream(in) >> v;
        }
    };
public:
	csvistream (std::istream &is, const char separator = ',', const bool commentable = false);
    
    const csvistream & operator >> (std::string &v) const;
    operator bool () const;
    template <typename T> const csvistream & operator >> (T &v) const
	{
		std::string tmp;
		*this >> tmp;
		set_value<T>()(tmp, v);
		return *this;
	}
    template <typename T> const csvistream & operator >> (T &(*manip)(T &)) const
	{
		m_is >> manip;
		return *this;
	}
};

class csvostream
{
private:
    std::ostream &m_os;
    const char m_separator;
	bool m_first;
	void put(std::string s);
public:
	class modifiers { };
	static const modifiers newrow;

	csvostream (std::ostream &os, const char separator = ',');
    
    operator bool () const;
	csvostream & operator << (const modifiers &v);
    template <typename T> csvostream & operator << (const T &v)
	{
		std::ostringstream tmp;
		tmp << v;
		put(tmp.str());
		return *this;
	}
    template <typename T> csvostream & operator << (const T &(*manip)(const T &))
	{
		m_os << manip;
		return *this;
	}
};

#endif