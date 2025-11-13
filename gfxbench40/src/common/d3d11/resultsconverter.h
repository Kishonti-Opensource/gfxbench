/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESULTSCONVERTER_H
#define RESULTSCONVERTER_H

#include <map>
#include <string>
#include "results.h"

class ResultsConverter
{
private:
	virtual std::string ConvertTo(const ResultsContainer &results) = 0;
	virtual ResultsContainer ConvertFrom(const std::string &input) = 0;
	std::vector<char> Encrypt(const std::vector<char> &input);
	virtual std::vector<char> Decrypt(const std::vector<char> &input);
public:
	std::string ToString(const ResultsContainer &results);
	std::vector<char> ToStringCrypted(const ResultsContainer &results);
	ResultsContainer FromString(const std::string &input);
	ResultsContainer FromStringCrypted(const std::vector<char> &input);
};

class ResultsToCSV : public ResultsConverter
{
private:
	std::string ConvertTo(const ResultsContainer &results);
	ResultsContainer ConvertFrom(const std::string &input);
};

class ResultsToUpload : public ResultsConverter
{
private:
	std::string m_username;
	std::string m_password;
	const std::map<std::string, std::string> &m_sysinfo;

	static std::string ResultsToUpload::urlencode(const std::string &s);
	std::string ConvertTo(const ResultsContainer &results);
	ResultsContainer ConvertFrom(const std::string &input);

public:
	ResultsToUpload(
			std::string username,
			std::string password,
			const std::map<std::string, std::string> &sysinfo
		);
};


#endif //RESULTSCONVERTER_H
