/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_animationtrack.h>

using namespace KCL;

AnimationTrack::AnimationTrack () : Object( "", ANIMATIONTRACK)
{
	m_keyframeSequence = NULL;
	m_property = PROPERTY_NULL;
	m_speed = 1.0f;
	m_weight = 1.0f;
	m_activeIntervalStart = 0;
	m_activeIntervalEnd = 0;
	m_referenceSequenceTime = 0.0f;
	m_referenceWorldTime = 0;
}

KeyframeSequence::AnimationState AnimationTrack::isAnimated (int time)
{
	return m_keyframeSequence->isAnimated (time - m_activeIntervalStart);
}

KCL::Quaternion AnimationTrack::getRotationKeyframe (int time)
{
	if (m_property != ORIENTATION) return KCL::Quaternion();

	float *key1, *key2;
	float t;

	KCL::Quaternion result;
	m_keyframeSequence->getKeyframe (getLocalTime (time), key1, key2, t);
	KCL::Quaternion::Interpolate (KCL::Quaternion (key1), KCL::Quaternion(key2), t, result);
	return result;
}


KCL::Vector3D AnimationTrack::getTranslationKeyframe (int time)
{
	if (m_property != TRANSLATION) return KCL::kZeroVector3D;

	float *key1, *key2;
	float t;

	KCL::Vector3D result;
	m_keyframeSequence->getKeyframe (getLocalTime (time), key1, key2, t);
	return KCL::Vector3D::interpolate (KCL::Vector3D (key1), KCL::Vector3D (key2), t);
}


KCL::Vector3D AnimationTrack::getScaleKeyframe (int time)
{
	if (m_property != SCALE) return KCL::Vector3D (1, 1, 1);
	
	float *key1, *key2;
	float t;
	m_keyframeSequence->getKeyframe (getLocalTime (time), key1, key2, t);
	return KCL::Vector3D::interpolate (KCL::Vector3D (key1), KCL::Vector3D (key2), t);
}

bool AnimationTrack::getVisibilityKeyframe (int time)
{
	if (m_property != VISIBILITY) return true;
	float *key1, *key2;
	float t;
	m_keyframeSequence->getKeyframe (getLocalTime (time), key1, key2, t);
	return *key1 > 0.5;
}
