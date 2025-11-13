/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_KEYFRAMESEQUENCE_H
#define KCL_KEYFRAMESEQUENCE_H

#include <kcl_object.h>
#include <kcl_math3d.h>

namespace KCL
{
	struct KeyframeSequence : public Object
	{
		enum AnimationState
		{
			PRE_ANIMATION = -1,
			ANIMATED = 0,
			POST_ANIMATION = 1
		};

		KeyframeSequence () : Object( "", KEYFRAMESEQUENCE)
		{
			m_keyframes = NULL;
			m_times = NULL;
			m_keyframeCount = 0;
			m_componentCount = 0;
			m_currentIndex = 0;
		}

		~KeyframeSequence ()
		{
			delete [] m_times;
			delete [] m_keyframes;
			m_keyframes = NULL;
			m_keyframeCount = 0;
			m_componentCount = 0;
		}

		void setKeyframe (int index, int time, float *keyframe)
		{
			m_times[index] = time;
			for (KCL::uint32 i = 0; i < m_componentCount; i++)
			{
				m_keyframes[index * m_componentCount + i] = keyframe[i];
			}
		}

		AnimationState isAnimated (int time)
		{
			AnimationState state = ANIMATED;
			if (time < m_times[0])
			{
				state = PRE_ANIMATION;
			}
			else if (time >= m_times[0] && time < m_times[m_keyframeCount-1])
			{
				state = ANIMATED;
			}
			else if (time >= m_times[m_keyframeCount-1])
			{
				state = POST_ANIMATION;
			}
			return state;
		}

		void getKeyframe (int time, float * &key1, float * &key2, float &t)
		{
			AnimationState state = isAnimated (time);
			if (state == PRE_ANIMATION)
			{
				key1 = &m_keyframes[0];
				key2 = &m_keyframes[0];
				t = 0;
			}
			else if (state == POST_ANIMATION)
			{
				key1 = &m_keyframes[(m_keyframeCount-1)*m_componentCount];
				key2 = &m_keyframes[(m_keyframeCount-1)*m_componentCount];
				t = 1;
			}
			else
			{
				int index1 = getIndex (time);
				key1 = &m_keyframes[index1*m_componentCount];
				key2 = &m_keyframes[index1*m_componentCount + m_componentCount];
				t = (float) (time - m_times[index1]) / (float)(m_times[index1 + 1] - m_times[index1]);
			}
		}
	
		int	getIndex (int time)
		{
			int i = m_currentIndex;
			if(time<=m_times[i])
				i=0;
			while (i < (int)m_keyframeCount-2 && m_times[i+1] < time)
			{
				i++;
			}
			m_currentIndex = i;
			return i;
		}

	private:
		KeyframeSequence(const KeyframeSequence&);
		KeyframeSequence& operator=(const KeyframeSequence&);
	public:

		int	   m_currentIndex;
		KCL::uint32 m_componentCount;
		KCL::uint32 m_keyframeCount;
		int   *m_times;
		float *m_keyframes;
	};
}

#endif
