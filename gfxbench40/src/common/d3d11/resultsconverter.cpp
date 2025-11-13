/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "resultsconverter.h"

#include <sstream>

#include "csvstream.h"

using namespace std;

namespace Crypto
{
	const unsigned char Key[] = { 10, 39, 180, 142, 251, 215, 46, 4, 158, 255, 87, 226, 252, 181, 120, 86, 44, 29, 85, 28, 156, 8, 17, 187, 150, 237, 79, 82, 56, 62, 166, 78 };
	const unsigned int KeyLength = 32;
}

vector<char> ResultsConverter::Encrypt(const vector<char> &input)
{
	vector<char> retval;
	retval.reserve(input.size());

	for(unsigned int i = 0; i < input.size(); i++)
	{
		int keyIndex = i % Crypto::KeyLength;
		retval.push_back(input[i] ^ Crypto::Key[keyIndex]);
	}
	return retval;
}

vector<char> ResultsConverter::Decrypt(const vector<char> &input)
{
	//Symmetric
	return Encrypt(input);
}

string ResultsConverter::ToString(const ResultsContainer &results)
{
	return ConvertTo(results);
}

vector<char> ResultsConverter::ToStringCrypted(const ResultsContainer &results)
{
	string converted = ConvertTo(results);
	vector<char> crypt(converted.begin(), converted.end());
	return Encrypt(crypt);
}

ResultsContainer ResultsConverter::FromString(const string &input)
{
	return ConvertFrom(input);
}

ResultsContainer ResultsConverter::FromStringCrypted(const vector<char> &input)
{
	vector<char> crypt = Decrypt(input);
	string str(crypt.begin(), crypt.end());
	return ConvertFrom(str);
}

namespace StringConstants
{
	const char* Errors[] = 
	{
		"TESTERROR_NOERROR",
		"TESTERROR_NOFSAA",
		"TESTERROR_OUT_OF_VMEMORY",
		"TESTERROR_SHADER_ERROR",
		"TESTERROR_FILE_NOT_FOUND",
		"TESTERROR_UNKNOWNERROR",
		"TESTERROR_HIGH_NOT_SUPPORTED",
		"TESTERROR_OFFSCREEN_NOT_SUPPORTED",
		"TESTERROR_OFFSCREEN_PRETEST_FAILED",
		"TESTERROR_SKIPPED",
		"TESTERROR_CUSTOM",
		"TESTERROR_OUT_OF_MEMORY",
		"TESTERROR_Z24_NOT_SUPPORTED",
		"TESTERROR_BATTERYTEST_PLUGGED_ON_CHARGER",
		"TESTERROR_VBO_ERROR",
		"TESTERROR_UNKNOWN_TC_TYPE",
		"TESTERROR_UNSUPPORTED_TC_TYPE",
		"TESTERROR_OFFSCREEN_NOT_SUPPORTED_IN_MSAA",
		"TESTERROR_INVALID_SCREEN_RESOLUTION",
		"TESTERROR_BATTERYTEST_BRIGHTNESS_CHANGED",
		"TESTERROR_MOTIONBLUR_WITH_MSAA_NOT_SUPPORTED",
		"TESTERROR_MAX"
	};
	const char* CsvTitles[] = 
	{
		"test_id",
		"test_title",
		"test_type",
		"texture_type",
		"uom",
		"test_length",
		"frame_step_time",
		"error",
		"score",
		"fps",
		"minfps",
		"maxfps",
		"is_vsync_triggered",
		"is_warmup",
		"viewport_width",
		"viewport_height",
		"uploaded"
	};
}


string ResultsToCSV::ConvertTo(const ResultsContainer &results)
{
	std::stringstream ss;
	csvostream csv(ss, ';');
	
	int n = sizeof(StringConstants::CsvTitles)/sizeof(char*);
	for (int i=0;i<n;i++)
	{
		csv << StringConstants::CsvTitles[i];
	}
	csv << csvostream::newrow;
	
	for(ResultsContainer::const_iterator it = results.begin();it!=results.end();it++)
	{
		csv << it->GetTestId();
		csv << it->GetTestTitle();
		csv << it->GetTestType();
		csv << it->GetTextureType();
		csv << it->GetUom();
		csv << it->GetTestLength();
		csv << it->GetFrameStepTime();
		csv << StringConstants::Errors[it->GetError()];
		csv << it->GetScore();
		csv << it->GetFps();
		csv << it->GetMinFps();
		csv << it->GetMaxFps();
		csv << (int)it->IsVsyncTriggered();
		csv << (int)it->IsWarmup();
		csv << it->GetViewportWidth();
		csv << it->GetViewportHeight();
		csv << (int)it->IsUploaded();
		csv << csvostream::newrow;
	}

	return ss.str();
}

ResultsContainer ResultsToCSV::ConvertFrom(const string &input)
{
	ResultsContainer retval;

	try
	{
		stringstream ss(input);
		csvistream csv(ss, ';');
	
		//headers
		std::string tmp;
		int n = sizeof(StringConstants::CsvTitles)/sizeof(char*);
		for (int i=0;i<n;i++)
		{
			csv >> tmp;
		}

		while (csv)
		{
			unsigned int test_id;
			std::string test_title;
			std::string test_type;
			std::string texture_type;
			std::string uom;
			float test_length;
			float frame_step_time;
			string error_str;
			KCL::KCL_Status error;
			float score;
			float fps;
			float minfps;
			float maxfps;
			bool is_vsync_triggered;
			bool is_warmup;
			unsigned int is_vsync_triggered_int;
			unsigned int is_warmup_int;
			unsigned int viewport_width;
			unsigned int viewport_height;
			bool is_uploaded;
			unsigned int is_uploaded_int;
			
			if (
				csv >> test_id &&
				csv >> test_title &&
				csv >> test_type &&
				csv >> texture_type &&
				csv >> uom &&
				csv >> test_length &&
				csv >> frame_step_time &&
				csv >> error_str &&
				csv >> score &&
				csv >> fps &&
				csv >> minfps &&
				csv >> maxfps &&
				csv >> is_vsync_triggered_int &&
				csv >> is_warmup_int &&
				csv >> viewport_width &&
				csv >> viewport_height &&
				csv >> is_uploaded_int
				)
			{

				error = KCL::KCL_TESTERROR_UNKNOWNERROR;
				int n = sizeof(StringConstants::Errors)/sizeof(char*);
				for (int i=0;i<n;i++)
				{
					if (StringConstants::Errors[i]==error_str)
					{
						error = (KCL::KCL_Status)i;
						break;
					}
				}
			
				is_vsync_triggered = (bool)is_vsync_triggered_int;
				is_warmup = (bool)is_warmup_int;
				is_uploaded = (bool)is_uploaded_int;


				retval.push_back(
					Result(
						test_id,
						test_title,
						test_type,
						texture_type,
						uom,
						test_length,
						frame_step_time,
						error,
						score,
						fps,
						minfps,
						maxfps,
						is_vsync_triggered,
						is_warmup,
						viewport_width,
						viewport_height,
						is_uploaded
					));
			}
		}
	}
	catch (...)
	{
	}

	return retval;
}

ResultsToUpload::ResultsToUpload(
	string username,
	string password,
	const map<string, string> &sysinfo
)
:
m_username(username),
m_password(password),
m_sysinfo(sysinfo)
{
}

std::string ResultsToUpload::urlencode(const std::string &s)
{
    //RFC 3986 section 2.3 Unreserved Characters (January 2005)
    const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

    std::string escaped="";
    for(size_t i=0; i<s.length(); i++)
    {
        if (unreserved.find_first_of(s[i]) != std::string::npos)
        {
            escaped.push_back(s[i]);
        }
        else
        {
            escaped.append("%");
            char buf[3];
            sprintf(buf, "%.2X", s[i]);
            escaped.append(buf);
        }
    }
    return escaped;
}

string ResultsToUpload::ConvertTo(const ResultsContainer &results)
{
	std::stringstream ss;
	
	//login credentials
	ss << "movie=" << urlencode(m_username) << "&cinema=" << urlencode(m_password);
	
	//system info
	for (std::map<std::string, std::string>::const_iterator it = m_sysinfo.begin(); it != m_sysinfo.end(); it++)
	{
		ss << "&" << it->first << "=" << urlencode(it->second);
	}

	//results
	for(ResultsContainer::const_iterator it = results.begin();it!=results.end();it++)
	{
		std::string frame_times;
		char key[256] = {0};
		char* value = new char[1024 + frame_times.length()];

		sprintf (key, "DXB_TEST%d", it->GetTestId());
		sprintf (value, "%d;%d;%0.0f;%d;%d;%d;%d;%s",
			it->GetError(),
			0,
			it->GetScore(),
			0,//startup
			0,
			it->GetViewportWidth(),
			it->GetViewportHeight(),
			frame_times.c_str()
			);
		
		ss << "&" << key << "=" << urlencode(value);

		delete[] value;
	}


	return ss.str();
}

ResultsContainer ResultsToUpload::ConvertFrom(const string &input)
{
	return ResultsContainer();
}