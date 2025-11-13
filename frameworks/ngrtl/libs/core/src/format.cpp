/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/format.h"
#include <cstdio>
#include <algorithm>
#include <assert.h>

#include "ng/math.h"

namespace ng
{

format::
	format()
{
}

format::
	format(const char* formatString)
	: _formatString(formatString)
{
}

std::string format::
	str() const
{
	ParsedArgs::const_iterator pait = _parsedArgs.begin();
	ParsedArgs::const_iterator paend = _parsedArgs.end();

	std::string out;

	std::string::const_iterator sit = _formatString.begin();
	std::string::const_iterator send = _formatString.end();
	for(; sit != send; ++sit)
	{
		if ( sit + 1 != send && *sit == '%')
		{
			if ( sit[1] == '%' )
			{
				out += '%';
			} else if ( sit[1] == 's' )
			{
				if ( pait != paend )
				{
					out += *pait;
					++pait;
				}
			} else
			{
				out += *sit;
				out += sit[1];
			}
			++sit;
		} else
			out += *sit;
	}
	return out;
}

std::string formatDurationSec(double dsec)
{
	const char* sign = dsec < 0 ? "-" : "";
	unsigned isec = (unsigned)round(fabs(dsec));
	unsigned h = isec / 3600;
	isec -= h * 3600;
	unsigned m = isec / 60;
	isec -= m * 60;
	unsigned s = isec;
	assert(0 <= s && s < 60);
	char buf[32];
	if ( h == 0 && m == 0 )
		sprintf(buf, "%s%us", sign, s);
	else if ( h == 0 )
		sprintf(buf, "%s%u:%02us", sign, m, s);
	else
		sprintf(buf, "%s%u:%02u:%02us", sign, h, m, s);
	return buf;
}

std::string
	formatNumberMetric(double d, int nDigitsNeeded)
{
	if ( d < 0 )
		return std::string("-") + formatNumberMetric(-d, nDigitsNeeded);

	if ( d == 0 ) return "0";
#if (_MSC_VER == 1700 ) || __APPLE__
    if (isnan(d)) return "NaN";
#else
	if ( std::isnan(d) ) return "NaN";
#endif
	if ( d == std::numeric_limits<double>::infinity() ) return "Inf";
	if ( d == -std::numeric_limits<double>::infinity() ) return "-Inf";

	const int c_nPrefixes = 8;
	const char* ppos = "kMGTPEZY"; //must be c_nPrefixes characters
	const char* pneg = "munpfazy"; //must be c_nPrefixes characters
	int e = (int)floor(log10(d));
	d = round(d * pow(10.0, nDigitsNeeded - 1 - e)) * pow(10.0, e - (nDigitsNeeded - 1));
	e = (int)floor(log10(d));
	int targete = (e >= 0) ? (e % 3) : (((e % 3) + 3) % 3); //simple modulus, fixed for negative numbers
	int prefixe = e - targete;
	assert(prefixe % 3 == 0);
	int prefixe3 = clamp(prefixe / 3, -c_nPrefixes, c_nPrefixes);
	targete = e - prefixe3 * 3;
	double d2 = round(d * pow(10.0, nDigitsNeeded - 1 - e)) * pow(10.0, targete - (nDigitsNeeded - 1));
	int digitsBeforeDot = targete + 1;
	int digitsNeededAfterDot = std::max(nDigitsNeeded - digitsBeforeDot, 0);
	while(digitsNeededAfterDot > 0 && frac(round(d2 * pow(10.0, digitsNeededAfterDot))/10) == 0.0)
		--digitsNeededAfterDot;
	char buf1[10], buf2[32];
	sprintf(buf1, "%%.%df", digitsNeededAfterDot);
	sprintf(buf2, buf1, d2);
	std::string result = buf2;
	if ( prefixe3 != 0 )
		result += prefixe3 > 0 ? ppos[prefixe3-1] : pneg[-prefixe3-1];
	return result;
}

template<typename INTTYPE>
std::string formatInt(INTTYPE i, const char* formatString, int width)
{
	if(i == 0 && width == 0)
		return "";

    bool bx = formatString && ::strchr(formatString, 'x') != 0;
    bool bX = formatString && ::strchr(formatString, 'X') != 0;
	std::ostringstream oss;
    if (bx || bX) oss << std::hex;
    if (bX) oss << std::uppercase;
	oss << i;
	std::string s = oss.str();

	if(i >= 0 && formatString)
	{
		if(::strchr(formatString, '+'))
			s.insert(s.begin(), '+');
		else if (::strchr(formatString, ' '))
			s.insert(s.begin(), ' ');
	}
	if(0 < width && static_cast<size_t>(width) < s.size())
	{
		if(formatString && ::strchr(formatString, '-'))
			s.append(width - s.size(), ' ');
		else if (formatString && ::strchr(formatString, '0'))
			s.insert(s.begin(), width - s.size(), '0');
		else
			s.insert(s.begin(), width - s.size(), ' ');
	}
	return s;
}

#define INSTANTIATE_FORMATINT(X) template std::string formatInt<X>(X, const char*, int);
INSTANTIATE_FORMATINT(signed char)
INSTANTIATE_FORMATINT(unsigned char)
INSTANTIATE_FORMATINT(short)
INSTANTIATE_FORMATINT(unsigned short)
INSTANTIATE_FORMATINT(int)
INSTANTIATE_FORMATINT(unsigned)
INSTANTIATE_FORMATINT(long)
INSTANTIATE_FORMATINT(unsigned long)
INSTANTIATE_FORMATINT(long long)
#undef INSTANTIATE_FORMATINT


std::string sprintfDouble(const char* formatString, double d)
{
	const int kBufSize = 256;
	char buf[kBufSize];

	int written = sprintf(buf, formatString, d);
	if (written < 0)
		return std::string();
	else
		return buf;
}

std::string sprintfDouble(const char* formatString, int widthOrPrecision, double d)
{
	const int kBufSize = 256;
	char buf[kBufSize];

	int written = sprintf(buf, formatString, widthOrPrecision, d);
	if (written < 0)
		return std::string();
	else
		return buf;
}

std::string sprintfDouble(const char* formatString, int width, int precision, double d)
{
	const int kBufSize = 256;
	char buf[kBufSize];

	int written = sprintf(buf, formatString, width, precision, d);
	if (written < 0)
		return std::string();
	else
		return buf;
}

}
