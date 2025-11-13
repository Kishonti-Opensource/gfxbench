/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "csvstream.h"


void csvistream::scan_ws () const 
{
    while (m_is.good())
	{
        int c = m_is.peek();
		if (isspace(c) == 0)
		{
			break; 
		}
		m_newline = (c == '\n');
        m_is.get();
    }
}
void csvistream::scan_comment () const 
{
	if (!m_commentable) 
		return;

    while (m_is.good()) 
	{
        int c = m_is.peek();
		if (m_newline && c=='#') 
		{ 
			std::string comment; 
			std::getline(m_is, comment); 
		}
		else 
		{
			m_newline = false;
			break;
		}
    }
}
void csvistream::scan (std::string *s) const 
{
    std::string ws;
    int c = m_is.get();
    if (m_is.good()) 
	{
        do 
		{
            if (c == m_separator) 
				break;

			if (c == '\n') 
			{ 
				m_newline = true; 
				break;
			}
            if (s) 
			{
                ws += c;
                if (isspace(c) == 0) 
				{
                    *s += ws;
                    ws.clear();
                }
            }
            c = m_is.get();
        } while (m_is.good());

        if (m_is.eof()) 
			m_is.clear();
    }
}

csvistream::csvistream (std::istream &is, const char separator, const bool commentable) : 
	m_is(is), 
	m_separator(separator), 
	m_commentable(commentable), 
	m_newline(true) 
{
}
   
const csvistream & csvistream::operator >> (std::string &v) const 
{
    v.clear();
    scan_ws();
	scan_comment();
    scan_ws();
    if (m_is.peek() != '"') 
	{
		scan(&v);
	}
	else 
	{
        std::string tmp;
        m_is.get();
        std::getline(m_is, tmp, '"');
        while (m_is.peek() == '"') 
		{
            v += tmp;
            v += m_is.get();
            std::getline(m_is, tmp, '"');
        }
        v += tmp;
        scan();
    }
    return *this;
}

csvistream::operator bool () const 
{ 
	return !m_is.fail(); 
}

const csvostream::modifiers csvostream::newrow;

void csvostream::put(std::string s)
{
	if (!m_first)
	{
		m_os << m_separator;
	}
	m_first = false;

	if(s.find('"')!=std::string::npos)
	{
		for(std::string::size_type n=0;(n=s.find('"',n))!=std::string::npos;n+=2) 
			s.replace(n,1,"\"\"");

		m_os << "\"" << s << "\""; 
	}
	else if(s.find(m_separator)!=std::string::npos)
	{
		m_os << "\"" << s << "\""; 
	}
	else
	{
		m_os << s; 
	}
	
}

csvostream::csvostream (std::ostream &os, const char separator) : 
	m_os(os), 
	m_separator(separator),
	m_first(true)
{
}

csvostream & csvostream::operator << (const modifiers &v)
{
	if (&v == &csvostream::newrow)
	{
		m_first = true;
		m_os << "\n";
	}
	return *this;
}

csvostream::operator bool () const 
{ 
	return !m_os.fail(); 
}