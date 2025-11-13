/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "ng/json.h"
#include "kcl_io.h"
#include <time.h>
#include <cstdio>

template<class T>
bool equals(const T& value1, const T& value2, const T& tolerance = (T)0.01f)
{
	return value1 - value2 < tolerance && value1 - value2 > -tolerance;
}

template< class dummy>
class ProgressBarStatic
{
protected:
	/*!Displayed progress in percent
	*/
	static double m_currentProgress;

	/*!Last displayed progress in percent
	*/
	static double m_lastProgress;

	/*!Current value of progress
	*/
	static size_t m_progressIterator;

	/*!Last displayed progress
	*/
	static bool m_simulateProgress;

	/*!display every Nth progress
	*/
	static size_t m_progress_step;

	/*!Progress bar has started?
	*/
	static bool m_started;

	static std::vector<time_t> m_times;
	static std::vector<double> m_progress_values;
	static std::vector<double> m_progress_values_collector;
};

template < class dummy>
std::vector<double> ProgressBarStatic<dummy>::m_progress_values;

template < class dummy>
std::vector<double> ProgressBarStatic<dummy>::m_progress_values_collector;

template < class dummy>
std::vector<time_t> ProgressBarStatic<dummy>::m_times;

template < class dummy>
double ProgressBarStatic<dummy>::m_currentProgress;

template < class dummy>
double ProgressBarStatic<dummy>::m_lastProgress;

template < class dummy>
size_t ProgressBarStatic<dummy>::m_progressIterator;

template < class dummy>
bool ProgressBarStatic<dummy>::m_simulateProgress;

template < class dummy>
size_t ProgressBarStatic<dummy>::m_progress_step = 5;

template < class dummy>
bool ProgressBarStatic<dummy>::m_started;

/*!If you add or remove AdvanceProgress you have to erase the progress file
*/
class ProgressBar : public ProgressBarStatic<void>
{
public:
	static double GetProgress()
	{
		return m_currentProgress;
	}
	static void StartProgress()
	{
		ng::JsonValue jv;
		ng::Result r;
		
		jv.fromFile(KCL::File::GetScenePath() + "progress", r);
		if (r.ok() && !jv["m_times"].isNull() && jv["m_times"].isArray())
		{
			for (size_t i(0); i< jv["m_times"].size(); ++i)
			{
				m_progress_values.push_back(jv["m_times"][i].number());
			}
			m_simulateProgress = true;
		}
		m_started = true;
	}
	static void CalculateNormalziedProgress()
	{
		time_t elapsed = m_times[m_times.size() - 1] - m_times[0];
		time_t start_time = m_times[0];
		for (size_t i = 0; i < m_progress_values_collector.size(); ++i)
		{
			m_times[i] = m_times[i] - start_time;
			m_progress_values_collector[i] = m_times[i] / (float)elapsed * 100.0;
		}
	}
	static void EndProgress()
	{
		m_started = false;
		if (!m_simulateProgress)
		{
			CalculateNormalziedProgress();
		}

		ng::JsonValue jv;
		ng::Result r;
		jv.fromFile(KCL::File::GetScenePath() + "progress", r);//<<--check file existing
		if (m_progress_values.size() != m_progressIterator || r.error())
		{
			CalculateNormalziedProgress();
			ng::JsonValue jv;
			for (size_t i = 0; i < m_progress_values_collector.size(); ++i)
			{
				jv["m_times"].push_back(m_progress_values_collector[i]);
			}
			ng::Result r;
			jv.toFile(KCL::File::GetDataRWPath() + "progress", r);
		}
	}
	static void AdvanceProgress(const char* state = 0)
	{
		if (!m_started)
		{
			return;
		}

		time_t current;
		time(&current);
		m_times.push_back(current);
		if (m_simulateProgress)
		{
			m_progress_values_collector.push_back((double)m_progressIterator);
			if (m_progressIterator < m_progress_values.size())
			{
				m_currentProgress = m_progress_values[m_progressIterator];
			}
			else
			{
				m_currentProgress = 100.0;
			}
		}
		else
		{
			m_progress_values_collector.push_back((double)m_progressIterator);
			++m_currentProgress;
		}
		++m_progressIterator;

		if ((m_simulateProgress && m_currentProgress > 0) &&
			(!equals(m_currentProgress, m_lastProgress) || (state != nullptr)))
		{
			m_lastProgress = m_currentProgress;
			if (m_simulateProgress)
			{
				if (state)
				{
					INFO("Loading %3.0f%% [ %s ]", m_currentProgress, state);
				}
				else
				{
					INFO("Loading %3.0f%%", m_currentProgress);
				}
			}
		}
	}
};


#endif
