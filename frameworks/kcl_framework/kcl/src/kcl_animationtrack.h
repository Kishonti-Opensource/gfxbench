/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_ANIMATIONTRACK_H
#define KCL_ANIMATIONTRACK_H

#include <kcl_object.h>
#include <kcl_math3d.h>
#include <kcl_keyframesequence.h>

namespace KCL
{
	struct AnimationTrack : public Object
	{
		enum Constants
		{
			PROPERTY_NULL	= 0,
			ALPHA			= 256,
			AMBIENT_COLOR	= 257,
			COLOR			= 258,
			CROP			= 259,
 			DENSITY			= 260,
 			DIFFUSE_COLOR	= 261,
 			EMISSIVE_COLOR	= 262,
 			FAR_DISTANCE	= 263,
 			FIELD_OF_VIEW	= 264,
 			INTENSITY		= 265,
 			MORPH_WEIGHTS	= 266,
 			NEAR_DISTANCE	= 267,
 			ORIENTATION		= 268,
 			PICKABILITY		= 269,
 			SCALE			= 270,
 			SHININESS		= 271,
 			SPECULAR_COLOR	= 272,
 			SPOT_ANGLE		= 273,
 			SPOT_EXPONENT	= 274,
 			TRANSLATION		= 275,
 			VISIBILITY		= 276
		};
	
		AnimationTrack ();

		~AnimationTrack()
		{
			delete m_keyframeSequence;
		}

		KeyframeSequence::AnimationState isAnimated (int time);
		KCL::Quaternion getRotationKeyframe (int time);
		KCL::Vector3D getTranslationKeyframe (int time);
		KCL::Vector3D getScaleKeyframe (int time);
		bool getVisibilityKeyframe (int time);
		KCL::uint32 getProperty ()
		{
			return m_property;
		}

		int getLocalTime (int time)
		{
			return time - m_activeIntervalStart;
		}

	private:
		AnimationTrack(const AnimationTrack&);
		AnimationTrack& operator=(const AnimationTrack&);
	public:

		KCL::uint32 m_property;
		float  m_speed;
		float  m_weight;
		int    m_activeIntervalStart;
		int    m_activeIntervalEnd;
		float  m_referenceSequenceTime;
		KCL::int32  m_referenceWorldTime;

		KeyframeSequence *m_keyframeSequence;
	};
}

#endif
