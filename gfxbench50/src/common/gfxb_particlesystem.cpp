/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_particlesystem.h"

#include "ngl.h"
#include "gfxb_shader.h"
#include "gfxb_mesh.h"
#include "gfxb_mesh_shape.h"
#include "gfxb_material.h"
#include "kcl_scene_handler.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace GFXB;

// local functions -----------------------------

KCL::Vector3D Sine(const KCL::Vector3D &v)
{
	return KCL::Vector3D(sin(v.x), sin(v.y), sin(v.z));
}

KCL::Vector3D Wave3D(float time, KCL::Vector3D amplitude, KCL::Vector3D phase, KCL::Vector3D frequency)
{
	return Sine((time * frequency + phase) * 2.0f * 3.1415926f) * amplitude;
}

void ExtractOrientation(const KCL::Matrix4x4 &pom, KCL::Matrix4x4 &orientation_matrix)
{
	using namespace KCL;
	orientation_matrix = pom;
	reinterpret_cast<Vector3D&>(orientation_matrix.v11).normalize();
	reinterpret_cast<Vector3D&>(orientation_matrix.v21).normalize();
	reinterpret_cast<Vector3D&>(orientation_matrix.v31).normalize();
	reinterpret_cast<Vector4D&>(orientation_matrix.v41) = Vector4D(0, 0, 0, 1);
}

float Mix(float a, float b, float t)
{
	return (1.0f - t) * a + t * b;
}

KCL::Vector3D Mix(KCL::Vector3D a, KCL::Vector3D b, float t)
{
	return (1.0f - t) * a + t * b;
}

inline void InterpolatePOM(const KCL::Matrix4x4 & pom1, const KCL::Matrix4x4 & pom2, float t, KCL::Matrix4x4& result)
{
	result = pom2;
}

float EvaluateCurveWithinDomain(float time, const KCL::Vector2D *curve, int curve_point_count)
{
	for (int i = 0; i < curve_point_count - 1; i++)
	{
		if (time < curve[i + 1].x)
		{
			// lerp
			return ((curve[i + 1].x - time) * curve[i].y + (time - curve[i].x) * curve[i + 1].y) / (curve[i + 1].x - curve[i].x);
		}
	}

	return 0;
}

inline float EvaluateCurve(float time, const KCL::Vector2D *curve, int curve_point_count)
{
	if (time < curve[0].x)
	{
		return curve[0].y;
	}
	else
	{
		if (time >= curve[curve_point_count - 1].x)
		{
			return curve[curve_point_count - 1].y;
		}
		else
		{
			return EvaluateCurveWithinDomain(time, curve, curve_point_count);
		}
	}
}

bool CompareParticleRefs(ParticleParams *a, ParticleParams *b)
{
	return a->m_depth_position > b->m_depth_position;
}

inline std::string GetParticleStateFilename(KCL::uint32 animation_time)
{
	std::stringstream s;
	s << "particle_state_" << std::setfill('0') << std::setw(6) << animation_time << ".bin";
	return s.str();
}


// ParticleSystemSerializer class ///////////////////////////////////////////////////////////////////////////////////

template<bool is_writing, size_t element_size>
inline void SerializeElementPtr(KCL::File &file, void *member, size_t element_index)
{
	assert(false);
}

template<>
inline void SerializeElementPtr<true, 4>(KCL::File &file, void *member, size_t element_index)
{
	uint32_t &ivalue = ((uint32_t*)member)[element_index];
	uint8_t c[4];

	c[0] = ivalue & 255;
	c[1] = (ivalue >> 8) & 255;
	c[2] = (ivalue >> 16) & 255;
	c[3] = (ivalue >> 24) & 255;

	file.Write(c, 4, 1);
}

template<>
inline void SerializeElementPtr<false, 4>(KCL::File &file, void *member, size_t element_index)
{
	uint32_t &ivalue = ((uint32_t*)member)[element_index];
	uint8_t c[4];

	file.Read(c, 4, 1);

	ivalue = (uint32_t)c[0]
		+ ((uint32_t)c[1] << 8)
		+ ((uint32_t)c[2] << 16)
		+ ((uint32_t)c[3] << 24);
}

template<bool is_writing, size_t element_size, typename T>
inline void SerializeElement(KCL::File &file, T &member, size_t element_index = 0)
{
	SerializeElementPtr<is_writing, element_size>(file, (void*)&member, element_index);
}

template<bool is_writing, size_t element_size, typename T>
inline void SerializeMember(KCL::File &file, T &member)
{
	size_t element_count = sizeof(T) / element_size;
	for (size_t i = 0; i < element_count; i++)
	{
		SerializeElement<is_writing, element_size, T>(file, member, i);
	}
}

template<>
inline void SerializeMember<true, 4, bool>(KCL::File &file, bool &member)
{
	uint32_t ivalue = member ? ~0 : 0;
	SerializeElement<true, 4, uint32_t>(file, ivalue);
}

template<>
inline void SerializeMember<false, 4, bool>(KCL::File &file, bool &member)
{
	uint32_t ivalue = 0;
	SerializeElement<false, 4, uint32_t>(file, ivalue);
	member = ivalue > 0;
}

class GFXB::ParticleSystemSerializer
{
private:
	template<bool is_writing>
	static inline void SerializeEmitterState(KCL::File &file, ParticleEmitter &emitter)
	{
		// ParticleEmitter members

		SerializeMember<is_writing, 4>(file, emitter.m_max_particle_count);
		SerializeMember<is_writing, 4>(file, emitter.m_new_particle_base_index);
		SerializeMember<is_writing, 4>(file, emitter.m_active_particle_count);
		SerializeMember<is_writing, 4>(file, emitter.m_active_range_start);
		SerializeMember<is_writing, 4>(file, emitter.m_active_range_size);

		SerializeMember<is_writing, 4>(file, emitter.m_is_first_run);
		SerializeMember<is_writing, 4>(file, emitter.m_seed);
		SerializeMember<is_writing, 4>(file, emitter.m_track_emit_rate);
		SerializeMember<is_writing, 4>(file, emitter.m_override_track_emit_rate);

		SerializeMember<is_writing, 4>(file, emitter.m_step_rate);
		SerializeMember<is_writing, 4>(file, emitter.m_step_time_ms);
		SerializeMember<is_writing, 4>(file, emitter.m_step_time);

		SerializeMember<is_writing, 4>(file, emitter.m_last_animation_time_ms);
		SerializeMember<is_writing, 4>(file, emitter.m_last_time_residue_ms);
		SerializeMember<is_writing, 4>(file, emitter.m_emits_queued);
		SerializeMember<is_writing, 4>(file, emitter.m_emit_timer);
		SerializeMember<is_writing, 4>(file, emitter.m_last_pom);

		// EmitterParams members

		EmitterParams &emitter_params = emitter.m_emitter;
		uint32_t emitter_id = emitter_params.m_id;

		SerializeMember<is_writing, 4>(file, emitter_params.m_pom);
		SerializeMember<is_writing, 4>(file, emitter_params.m_id);

		SerializeMember<is_writing, 4>(file, emitter_params.m_emit_rate);
		SerializeMember<is_writing, 4>(file, emitter_params.m_lifespan);

		SerializeMember<is_writing, 4>(file, emitter_params.m_base_size);
		SerializeMember<is_writing, 4>(file, emitter_params.m_focus_distance);
		SerializeMember<is_writing, 4>(file, emitter_params.m_emit_velocity);
		SerializeMember<is_writing, 4>(file, emitter_params.m_emit_rotation);
		SerializeMember<is_writing, 4>(file, emitter_params.m_angular_velocity);
		SerializeMember<is_writing, 4>(file, emitter_params.m_aperture);
		SerializeMember<is_writing, 4>(file, emitter_params.m_frequency);
		SerializeMember<is_writing, 4>(file, emitter_params.m_amplitude);

		SerializeMember<is_writing, 4>(file, emitter_params.m_gravity);
		SerializeMember<is_writing, 4>(file, emitter_params.m_wind_velocity);
		SerializeMember<is_writing, 4>(file, emitter_params.m_air_resistance);
		SerializeMember<is_writing, 4>(file, emitter_params.m_external_velocity_inheritance);

		SerializeMember<is_writing, 4>(file, emitter_params.m_size_curve);
		SerializeMember<is_writing, 4>(file, emitter_params.m_opacity_curve);
		SerializeMember<is_writing, 4>(file, emitter_params.m_additive_factor);
		SerializeMember<is_writing, 4>(file, emitter_params.m_roundness);
		SerializeMember<is_writing, 4>(file, emitter_params.m_shade_factor);
		SerializeMember<is_writing, 4>(file, emitter_params.m_base_opacity);

		SerializeMember<is_writing, 4>(file, emitter_params.m_orientation_matrix);
		SerializeMember<is_writing, 4>(file, emitter_params.m_external_velocity);

		if (emitter_id != emitter_params.m_id)
		{
			INFO("PS: Emitter id doesn't match restored id. Try again with a freshly saved state file.");
			assert(false);
		}

		// ParticleParams members

		uint32_t particle_count = (uint32_t)emitter.m_particles.size();
		SerializeElement<is_writing, sizeof(particle_count)>(file, particle_count);
		assert(particle_count == (uint32_t)emitter.m_particles.size());

		bool is_active = false;

		for (uint32_t i = 0; i < particle_count; i++)
		{
			ParticleParams &particle = emitter.m_particles[i];

			SerializeMember<is_writing, 4>(file, particle.m_lifespan);
			SerializeMember<is_writing, 4>(file, particle.m_base_size);
			SerializeMember<is_writing, 4>(file, particle.m_turbulence_base);
			SerializeMember<is_writing, 4>(file, particle.m_frequency);
			SerializeMember<is_writing, 4>(file, particle.m_amplitude);
			SerializeMember<is_writing, 4>(file, particle.m_phase);
			
			// vector<bool> returns proxy, not a real element (isn't even single byte per element)
			is_active = emitter.m_particles_active[i];
			SerializeMember<is_writing, 4>(file, is_active);
			emitter.m_particles_active[i] = is_active;

			SerializeMember<is_writing, 4>(file, particle.m_age);
			SerializeMember<is_writing, 4>(file, particle.m_position);
			SerializeMember<is_writing, 4>(file, particle.m_velocity);
			SerializeMember<is_writing, 4>(file, particle.m_angular_velocity);
			SerializeMember<is_writing, 4>(file, particle.m_flipbook_start_frame);

			SerializeMember<is_writing, 4>(file, particle.m_opacity);
			SerializeMember<is_writing, 4>(file, particle.m_tint);
			SerializeMember<is_writing, 4>(file, particle.m_size);
			SerializeMember<is_writing, 4>(file, particle.m_rotation);
			SerializeMember<is_writing, 4>(file, particle.m_flipbook_blend_factor);
			SerializeMember<is_writing, 4>(file, particle.m_flipbook_coords_a);
			SerializeMember<is_writing, 4>(file, particle.m_depth_position);

			particle.m_emitter_ref = &emitter_params; // restore emitter reference
		}
	}

public:
	template<bool is_writing>
	static inline void SerializeManagerState(KCL::File &file, ParticleSystemManager *manager)
	{
		uint32_t emitter_count = (uint32_t)manager->m_emitters.size();
		SerializeElement<is_writing, sizeof(emitter_count)>(file, emitter_count);
		assert(emitter_count == (uint32_t)manager->m_emitters.size());

		for (std::list<ParticleEmitter*>::iterator i = manager->m_emitters.begin(); i != manager->m_emitters.end(); i++)
		{
			ParticleEmitter &emitter = **i;

			SerializeEmitterState<is_writing>(file, emitter);
		}
	}
};


// ParticleEmitter class ///////////////////////////////////////////////////////////////////////////////////

ParticleEmitter::ParticleEmitter(ParticleSystemManager *manager, const std::string &name, Node* parent, Object *owner)
	: KCL::Node(name, KCL::EMITTER5, parent, owner)
{
	m_manager = manager;
	m_emit_rate_track = nullptr;

	m_is_first_run = true;
	m_active_particle_count = 0;
	m_last_animation_time_ms = 0;
	m_last_time_residue_ms = 0;
	m_step_rate = 0;
	m_step_time = 0;
	m_emits_queued = 0;
	m_emit_timer = 0;
	if (m_manager != nullptr)
	{
		m_seed = manager->GetParticleEmitterSeed();
	}

	SetDefaults();
}

GFXB::ParticleEmitter::~ParticleEmitter()
{
	if (m_manager != nullptr)
	{
		m_manager->UnregisterParticleEmitter(this);
	}

	delete m_emit_rate_track;
}

bool ParticleEmitter::Init()
{
	m_is_first_run = true;
	m_particles.resize(m_max_particle_count);
	m_particles_active.resize(m_max_particle_count);
	Reset();

	if (m_manager != nullptr)
	{
		KCL::SceneHandler *scene = m_manager->GetScene();
		if (scene != nullptr)
		{
			scene->ReadAnimation(m_emit_rate_track, m_name + "_rate_track");
		}
	}

	return true;
}

void ParticleEmitter::SetDefaults()
{
	SetStepRate(48.0f);

	m_max_particle_count = 1000;
	m_new_particle_base_index = 0;
	m_flipbook_randomize_start_frame = true;
	Reset(m_max_particle_count, false);

	m_track_emit_rate = 0.0f;
	m_override_track_emit_rate = false;

	m_emitter.m_emit_rate = 1.0f;
	m_emitter.m_aperture = KCL::Vector3D(1, 1, 1) * 0.3f;
	m_emitter.m_base_size[0] = 1.0f;
	m_emitter.m_base_size[1] = 1.0f;
	m_emitter.m_focus_distance = 10;
	m_emitter.m_lifespan[0] = 5.0f;
	m_emitter.m_lifespan[1] = 5.0f;
	m_emitter.m_gravity = KCL::Vector3D(0, 0, 0);
	m_emitter.m_additive_factor = 0.0f;
	m_emitter.m_roundness = 0.0f;
	m_emitter.m_shade_factor = 0.0f;
	m_emitter.m_base_opacity = 1.0f;
	m_emitter.m_emit_velocity[0] = 1.6f;
	m_emitter.m_emit_velocity[1] = 1.6f;
	m_emitter.m_emit_rotation[0] = -20.6f;
	m_emitter.m_emit_rotation[1] = 20.6f;
	m_emitter.m_angular_velocity[0] = -10.6f;
	m_emitter.m_angular_velocity[1] = 10.6f;
	m_emitter.m_air_resistance = 1.0f;
	m_emitter.m_wind_velocity = KCL::Vector3D(0, 0, 0);
	m_emitter.m_frequency[0] = KCL::Vector3D(1, 1, 1);
	m_emitter.m_frequency[1] = KCL::Vector3D(1, 1, 1);
	m_emitter.m_amplitude[0] = KCL::Vector3D(3, 3, 3);
	m_emitter.m_amplitude[1] = KCL::Vector3D(3, 3, 3);
	m_emitter.m_external_velocity_inheritance = 0.0f;
	m_emitter.m_size_curve[0] = KCL::Vector2D(0, 0);
	m_emitter.m_size_curve[1] = KCL::Vector2D(0.33f, 1);
	m_emitter.m_size_curve[2] = KCL::Vector2D(0.66f, 1);
	m_emitter.m_size_curve[3] = KCL::Vector2D(1, 0);
	m_emitter.m_opacity_curve[0] = KCL::Vector2D(0, 0);
	m_emitter.m_opacity_curve[1] = KCL::Vector2D(0.33f, 1);
	m_emitter.m_opacity_curve[2] = KCL::Vector2D(0.66f, 1);
	m_emitter.m_opacity_curve[3] = KCL::Vector2D(1, 0);

	m_color_texture_filename = "";
	m_normal_texture_filename = "";
	m_flipbook.Init(4, 4);

	EnforceParamRanges();
}

void GFXB::ParticleEmitter::EnforceParamRanges()
{
	m_emitter.m_roundness = std::min(1.0f, std::max(0.0f, m_emitter.m_roundness));
	m_emitter.m_additive_factor = std::min(1.0f, std::max(0.0f, m_emitter.m_additive_factor));
	m_emitter.m_shade_factor = std::min(1.0f, std::max(0.0f, m_emitter.m_shade_factor));
	m_emitter.m_lifespan[0] = std::max(0.0f, m_emitter.m_lifespan[0]);
	m_emitter.m_lifespan[1] = std::max(0.0f, m_emitter.m_lifespan[1]);
	m_emitter.m_base_size[0] = std::max(0.0f, m_emitter.m_base_size[0]);
	m_emitter.m_base_size[1] = std::max(0.0f, m_emitter.m_base_size[1]);
	m_emitter.m_air_resistance = std::max(0.0f, m_emitter.m_air_resistance);
	m_emitter.m_base_opacity = std::max(0.0f, m_emitter.m_base_opacity);

	m_emitter.m_aperture.x = std::max(0.0f, m_emitter.m_aperture.x);
	m_emitter.m_aperture.y = std::max(0.0f, m_emitter.m_aperture.y);
	m_emitter.m_aperture.z = std::max(0.0f, m_emitter.m_aperture.z);

	m_emitter.m_frequency[0].x = std::max(0.0f, m_emitter.m_frequency[0].x);
	m_emitter.m_frequency[0].y = std::max(0.0f, m_emitter.m_frequency[0].y);
	m_emitter.m_frequency[0].z = std::max(0.0f, m_emitter.m_frequency[0].z);
	m_emitter.m_frequency[1].x = std::max(0.0f, m_emitter.m_frequency[1].x);
	m_emitter.m_frequency[1].y = std::max(0.0f, m_emitter.m_frequency[1].y);
	m_emitter.m_frequency[1].z = std::max(0.0f, m_emitter.m_frequency[1].z);

	m_emitter.m_amplitude[0].x = std::max(0.0f, m_emitter.m_amplitude[0].x);
	m_emitter.m_amplitude[0].y = std::max(0.0f, m_emitter.m_amplitude[0].y);
	m_emitter.m_amplitude[0].z = std::max(0.0f, m_emitter.m_amplitude[0].z);
	m_emitter.m_amplitude[1].x = std::max(0.0f, m_emitter.m_amplitude[1].x);
	m_emitter.m_amplitude[1].y = std::max(0.0f, m_emitter.m_amplitude[1].y);
	m_emitter.m_amplitude[1].z = std::max(0.0f, m_emitter.m_amplitude[1].z);
}

void ParticleEmitter::UpdateEmitter(const KCL::Matrix4x4 &new_pom, KCL::uint32 animation_time)
{
	using namespace KCL;

	const Vector3D old_pos = reinterpret_cast<const Vector3D&>(m_emitter.m_pom.v41);
	m_emitter.m_pom = new_pom;
	ExtractOrientation(new_pom, m_emitter.m_orientation_matrix);
	m_emitter.m_external_velocity = (reinterpret_cast<const Vector3D&>(new_pom.v41) - old_pos) * (1.0f / m_step_time);
	
	if (m_emit_rate_track != nullptr)
	{
		float animation_time_sec = animation_time / 1000.0f;
		float animation_time_base_sec = 0.0f;

		KCL::Vector4D result;
		KCL::_key_node::Get(result, m_emit_rate_track, animation_time_sec, animation_time_base_sec, false);

		m_track_emit_rate = result.x;
	}
}

void ParticleEmitter::EmitParticle(ParticleParams &particle)
{
	using namespace KCL;

	particle.m_emitter_ref = &m_emitter;
	particle.m_age = 0.0f;
	particle.m_lifespan = Mix(m_emitter.m_lifespan[0], m_emitter.m_lifespan[1], Math::randomf(&m_seed));
	particle.m_base_size = Mix(m_emitter.m_base_size[0], m_emitter.m_base_size[1], Math::randomf(&m_seed));

	particle.m_turbulence_base = m_emitter.m_orientation_matrix;

	particle.m_phase.x = Math::randomf(&m_seed);
	particle.m_phase.y = Math::randomf(&m_seed);
	particle.m_phase.z = Math::randomf(&m_seed);

	Vector3D random;
	random.x = Math::randomf(&m_seed) * 2.0f - 1.0f;
	random.x = Math::randomf(&m_seed) * 2.0f - 1.0f;
	random.z = Math::randomf(&m_seed) * 2.0f - 1.0f;

	Vector3D emitOffset = m_emitter.m_aperture * 0.5f * random;

	particle.m_position = Vector3D(m_emitter.m_pom * Vector4D(emitOffset, 1.0f));

	Vector3D denorm_dir_y = reinterpret_cast<Vector3D&>(m_emitter.m_pom.v21);
	Vector3D dir_y = Vector3D::normalize(denorm_dir_y);
	Vector3D denorm_emit_dir = dir_y * m_emitter.m_focus_distance + reinterpret_cast<Vector3D&>(m_emitter.m_pom.v41) - particle.m_position;
	Vector3D emit_dir = Vector3D::normalize(denorm_emit_dir);

	particle.m_velocity =
		emit_dir * Mix(m_emitter.m_emit_velocity[0], m_emitter.m_emit_velocity[1], Math::randomf(&m_seed))
		+ m_emitter.m_external_velocity * m_emitter.m_external_velocity_inheritance;

	particle.m_frequency = Mix(m_emitter.m_frequency[0], m_emitter.m_frequency[1], Math::randomf(&m_seed));
	particle.m_amplitude = Mix(m_emitter.m_amplitude[0], m_emitter.m_amplitude[1], Math::randomf(&m_seed));

	particle.m_rotation = Mix(m_emitter.m_emit_rotation[0], m_emitter.m_emit_rotation[1], Math::randomf(&m_seed));
	particle.m_angular_velocity = Mix(m_emitter.m_angular_velocity[0], m_emitter.m_angular_velocity[1], Math::randomf(&m_seed));

	particle.m_size = EvaluateCurve(particle.m_age / particle.m_lifespan, m_emitter.m_size_curve, 4) * particle.m_base_size;
	particle.m_opacity = EvaluateCurve(particle.m_age / particle.m_lifespan, m_emitter.m_opacity_curve, 4);

	if (m_flipbook_randomize_start_frame)
	{
		particle.m_flipbook_start_frame = (uint32)(Math::randomf(&m_seed) * (m_flipbook.GetFrameCount() + 0.99f));
	}
	else
	{
		particle.m_flipbook_start_frame = 0;
		Math::randomf(&m_seed); // call randomf so the seed doesn't get out of sync
	}
}

void ParticleEmitter::SimulateParticle(ParticleParams & particle)
{
	using namespace KCL;

	Vector3D delta_wave = Wave3D(particle.m_age, particle.m_amplitude, particle.m_phase, particle.m_frequency)
		- Wave3D(particle.m_age - m_step_time, particle.m_amplitude, particle.m_phase, particle.m_frequency);

	Vector3D turbulence_velocity = Vector3D(particle.m_turbulence_base * Vector4D(delta_wave, 1));
	particle.m_velocity += m_emitter.m_gravity * m_step_time;
	particle.m_position += (particle.m_velocity + m_emitter.m_wind_velocity + turbulence_velocity) * m_step_time;
	particle.m_rotation += particle.m_angular_velocity;

	float air_resistance_factor = powf(1.0f / m_emitter.m_air_resistance, m_step_time);
	particle.m_velocity *= air_resistance_factor;
	particle.m_amplitude *= air_resistance_factor; // hack, replace when turbulence semantics change
	particle.m_angular_velocity *= air_resistance_factor;

	particle.m_flipbook_coords_a = *(Vector4D*)m_flipbook.GetFrame(particle.m_flipbook_start_frame);

	particle.m_size = EvaluateCurve(particle.m_age / particle.m_lifespan, m_emitter.m_size_curve, 4) * particle.m_base_size;
	particle.m_opacity = EvaluateCurve(particle.m_age / particle.m_lifespan, m_emitter.m_opacity_curve, 4);

	particle.m_age += m_step_time;
}

void ParticleEmitter::Reset()
{
	const size_t count = m_particles_active.size();
	for (size_t i = 0; i < count; i++)
	{
		m_particles_active[i] = false;
	}

	m_new_particle_base_index = 0;
	m_active_range_start = 0;
	m_active_range_size = 0;
	m_emits_queued = 0;
	m_emit_timer = 0;
}

void ParticleEmitter::Reset(KCL::uint32 max_particle_count, bool signal_allocate)
{
	if (max_particle_count > GFXB_PARTICLE_SYSTEM_MAX_PARTICLE_PER_EMITTER)
	{
		max_particle_count = GFXB_PARTICLE_SYSTEM_MAX_PARTICLE_PER_EMITTER;
	}
	m_particles.resize(max_particle_count);
	m_particles_active.resize(max_particle_count);
	m_max_particle_count = max_particle_count;
	Reset();

	if (m_manager != nullptr && signal_allocate)
	{
		m_manager->Allocate();
	}
}

void ParticleEmitter::Animate(KCL::uint32 animation_time)
{
	KCL::uint32 delta_time_ms = 0;
	KCL::uint32 max_lifetime_ms = (KCL::uint32)(1000.0f * m_emitter.m_lifespan[1]);

	// if delta is negative or above max lifespan, editor is seeking
	if (m_last_animation_time_ms > animation_time || animation_time - m_last_animation_time_ms > max_lifetime_ms)
	{
		delta_time_ms = 0;
		//m_emit_timer = 0;
		m_last_time_residue_ms = 0;
	}
	else
	{
		if (m_last_animation_time_ms == animation_time && !m_is_first_run) // editor is paused
		{
			delta_time_ms = 40;

			if (m_manager != nullptr && m_manager->GetIsPaused())
			{
				delta_time_ms = 0;
			}
		}
		else
		{
			delta_time_ms = animation_time - m_last_animation_time_ms;
		}
	}

	delta_time_ms += m_last_time_residue_ms;

	KCL::uint32 steps = delta_time_ms / m_step_time_ms;

	EnforceParamRanges();

	float animate_progress;
	KCL::Matrix4x4 new_pom;
	const size_t particle_count = m_particles.size();
	const bool emit = !(particle_count == 0 || particle_count < m_max_particle_count);
	for (KCL::uint32 i = 1; i <= steps; i++)
	{
		animate_progress = (float)(i * m_step_time_ms) / delta_time_ms;
		InterpolatePOM(m_last_pom, m_world_pom, animate_progress, new_pom);
		UpdateEmitter(new_pom, m_last_animation_time_ms + i * m_step_time_ms);

		if (emit)
		{
			Emit();
		}

		Simulate();
	}

	m_last_animation_time_ms = animation_time;
	m_last_time_residue_ms = delta_time_ms % m_step_time_ms;

	m_last_pom = m_world_pom;

	m_is_first_run = false;
}

void ParticleEmitter::Emit()
{
	m_new_particle_base_index = m_new_particle_base_index % m_max_particle_count;

	float emit_rate = m_emit_rate_track != nullptr && !m_override_track_emit_rate ? m_track_emit_rate : m_emitter.m_emit_rate;
	m_emit_timer += ((m_step_time) * emit_rate);
	if (m_emit_timer >= 1.0f)
	{
		m_emits_queued += (int)m_emit_timer;

		for (KCL::uint32 i = 0; i < m_emits_queued; i++)
		{
			ParticleParams &particle = m_particles[m_new_particle_base_index];
			EmitParticle(particle);
			m_particles_active[m_new_particle_base_index] = true;

			m_new_particle_base_index = (m_new_particle_base_index + 1) % m_max_particle_count;
			m_active_range_size++;
		}
		m_emit_timer -= m_emits_queued;
		m_emits_queued = 0;
	}
}

void ParticleEmitter::Simulate()
{
	if (m_active_range_size > m_max_particle_count)
	{
		m_active_range_start = (m_active_range_start + (m_active_range_size % m_max_particle_count)) % m_max_particle_count;
		m_active_range_size = m_max_particle_count;
	}

	m_active_particle_count = 0;
	const KCL::uint32 range_size = m_active_range_size;
	for (KCL::uint32 i = m_active_range_start, j = 0; j < range_size; i++, j++)
	{
		if (i >= m_max_particle_count)
		{
			i = i % m_max_particle_count;
		}

		if (m_particles_active[i])
		{
			ParticleParams &particle = m_particles[i];

			if (particle.m_age + m_step_time <= particle.m_lifespan) // avoid function call for simple check
			{
				SimulateParticle(particle);
				m_active_particle_count++;
			}
			else
			{
				m_particles_active[i] = false;
			}
		}
		else if (m_active_range_start == i)
		{
			m_active_range_start = (m_active_range_start + 1) % m_max_particle_count;
			m_active_range_size--;
			assert(m_active_range_size <= GFXB_PARTICLE_SYSTEM_MAX_PARTICLE_PER_EMITTER);
		}
	}
}

void ParticleEmitter::Serialize(JsonSerializer &s)
{
	if (s.IsWriter)
	{
		EnforceParamRanges();
	}

	float step_rate = m_step_rate;
	s.Serialize("step_rate", step_rate);
	if (!s.IsWriter)
	{
		SetStepRate(step_rate);
	}

	s.Serialize("max_particle_count", m_max_particle_count);
	if (!s.IsWriter)
	{
		Reset(m_max_particle_count, false);
	}

	s.Serialize("flipbook_randomize_start_frame", m_flipbook_randomize_start_frame);

	// don't serialize: s.Serialize("new_particle_base_index", m_new_particle_base_index);
	s.Serialize("emit_rate", m_emitter.m_emit_rate);
	s.Serialize("aperture", m_emitter.m_aperture);
	s.Serialize("base_size_min", m_emitter.m_base_size[0]);
	s.Serialize("base_size_max", m_emitter.m_base_size[1]);
	s.Serialize("focus_distance", m_emitter.m_focus_distance);
	s.Serialize("lifespan_min", m_emitter.m_lifespan[0]);
	s.Serialize("lifespan_max", m_emitter.m_lifespan[1]);
	s.Serialize("gravity", m_emitter.m_gravity);
	s.Serialize("emit_velocity_min", m_emitter.m_emit_velocity[0]);
	s.Serialize("emit_velocity_max", m_emitter.m_emit_velocity[1]);
	s.Serialize("emit_rotation_min", m_emitter.m_emit_rotation[0]);
	s.Serialize("emit_rotation_max", m_emitter.m_emit_rotation[1]);
	s.Serialize("angular_velocity_min", m_emitter.m_angular_velocity[0]);
	s.Serialize("angular_velocity_max", m_emitter.m_angular_velocity[1]);
	s.Serialize("air_resistance", m_emitter.m_air_resistance);
	s.Serialize("wind_velocity", m_emitter.m_wind_velocity);
	s.Serialize("turbulence_frequency_min", m_emitter.m_frequency[0]);
	s.Serialize("turbulence_frequency_max", m_emitter.m_frequency[1]);
	s.Serialize("turbulence_amplitude_min", m_emitter.m_amplitude[0]);
	s.Serialize("turbulence_amplitude_max", m_emitter.m_amplitude[1]);
	s.Serialize("external_velocity_inheritance", m_emitter.m_external_velocity_inheritance);
	s.Serialize("size_curve_0", m_emitter.m_size_curve[0]);
	s.Serialize("size_curve_1", m_emitter.m_size_curve[1]);
	s.Serialize("size_curve_2", m_emitter.m_size_curve[2]);
	s.Serialize("size_curve_3", m_emitter.m_size_curve[3]);
	s.Serialize("opacity_curve_0", m_emitter.m_opacity_curve[0]);
	s.Serialize("opacity_curve_1", m_emitter.m_opacity_curve[1]);
	s.Serialize("opacity_curve_2", m_emitter.m_opacity_curve[2]);
	s.Serialize("opacity_curve_3", m_emitter.m_opacity_curve[3]);
	s.Serialize("additive_factor", m_emitter.m_additive_factor);
	s.Serialize("roundness", m_emitter.m_roundness);
	s.Serialize("shade_factor", m_emitter.m_shade_factor);
	s.Serialize("base_opacity", m_emitter.m_base_opacity);

	if (m_manager != nullptr)
	{
		m_color_texture_filename = m_manager->GetColorTextureFilename();
		m_normal_texture_filename = m_manager->GetNormalTextureFilename();
	}
	s.Serialize("color_texture_filename", m_color_texture_filename);
	s.Serialize("normal_texture_filename", m_normal_texture_filename);
	if (m_manager != nullptr && !s.IsWriter)
	{
		m_manager->SetColorTexture(m_color_texture_filename);
		m_manager->SetNormalTexture(m_normal_texture_filename);
	}

	KCL::uint32 flipbook_rows = m_flipbook.GetRows();
	KCL::uint32 flipbook_cols = m_flipbook.GetCols();
	KCL::uint32 flipbook_start_cell = m_flipbook.GetStartCell();
	KCL::uint32 flipbook_end_cell = m_flipbook.GetEndCell();
	float flipbook_framerate = m_flipbook.GetFramerate();
	s.Serialize("flipbook_rows", flipbook_rows);
	s.Serialize("flipbook_cols", flipbook_cols);
	s.Serialize("flipbook_start_cell", flipbook_start_cell);
	s.Serialize("flipbook_end_cell", flipbook_end_cell);
	s.Serialize("flipbook_framerate", flipbook_framerate);
	if (!s.IsWriter)
	{
		m_flipbook.Init(flipbook_rows, flipbook_cols, flipbook_start_cell, flipbook_end_cell, flipbook_framerate);
	}

	if (!s.IsWriter)
	{
		EnforceParamRanges();
	}
}

std::string GFXB::ParticleEmitter::GetParameterFilename() const
{
	return "emitters/" + m_name + ".json";
}

void ParticleEmitter::SetStepRate(float stepRate)
{
	m_step_rate = stepRate;
	m_step_time_ms = (KCL::uint32)(1000.f / stepRate);
	if (m_step_time_ms == 0)
	{
		m_step_time_ms = 1;
	}
	m_step_time = 0.001f * m_step_time_ms;
}

KCL::uint32 GFXB::ParticleEmitter::GetActiveParticleCount()
{
	return m_active_particle_count;
}

bool GFXB::ParticleEmitter::GetHasEmitRateTrack()
{
	return m_emit_rate_track != nullptr;
}

void GFXB::ParticleEmitter::SetOverrideTrackEmitRate(bool value)
{
	m_override_track_emit_rate = value;
}

bool GFXB::ParticleEmitter::GetOverrideTrackEmitRate()
{
	return m_override_track_emit_rate;
}

float GFXB::ParticleEmitter::GetTrackEmitRate()
{
	return m_track_emit_rate;
}

float GFXB::ParticleEmitter::GetCurrentEmitRate()
{
	return m_emit_rate_track != nullptr && !m_override_track_emit_rate ? m_track_emit_rate : m_emitter.m_emit_rate;
}

void GFXB::ParticleEmitter::SetColorTexture(std::string filename)
{
	m_color_texture_filename = filename;
}

void GFXB::ParticleEmitter::SetNormalTexture(std::string filename)
{
	m_normal_texture_filename = filename;
}

void GFXB::ParticleEmitter::InvalidateManager()
{
	m_manager = nullptr;
}

ParticleSystemManager * GFXB::ParticleEmitter::GetManager()
{
	return m_manager;
}


// ParticleSystemManager class ///////////////////////////////////////////////////////////////////////////////////

#define PARTICLE_BATCH_RENDER

GFXB::ParticleSystemManager::ParticleSystemManager()
{
	m_scene = nullptr;

	m_ref_pool_target_size = 0;
	m_default_mesh_object = nullptr;
	m_default_mesh3 = nullptr;
	m_default_shader = 0;
	m_wireframe_shader = 0;
	m_color_texture = nullptr;
	m_normal_texture = nullptr;

	m_next_emitter_id = 0;
	m_active_particle_count = 0;

	m_display_wireframe = false;
	m_is_paused = true;

	m_has_restored_state = false;
	m_last_animation_time = 0;
}

GFXB::ParticleSystemManager::~ParticleSystemManager()
{
	delete m_default_mesh_object;
	delete m_default_mesh3;
	delete m_color_texture;
	delete m_normal_texture;
}

ParticleEmitter *GFXB::ParticleSystemManager::CreateParticleEmitter(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
	ParticleEmitter *new_particle_emitter = new ParticleEmitter(this, name, parent, owner);
	new_particle_emitter->m_emitter.m_id = m_next_emitter_id;
	m_next_emitter_id++;
	
	m_emitters.push_back(new_particle_emitter);
	return new_particle_emitter;
}

void GFXB::ParticleSystemManager::UnregisterParticleEmitter(ParticleEmitter *particle_emitter)
{
	if (particle_emitter != nullptr)
	{
		particle_emitter->InvalidateManager();
		m_emitters.remove(particle_emitter);
	}
}

void GFXB::ParticleSystemManager::UnregisterAllParticleEmitters()
{
	for (std::list<ParticleEmitter*>::iterator i = m_emitters.begin(); i != m_emitters.end(); i++)
	{
		ParticleEmitter *particle_emitter = *i;

		if (particle_emitter == nullptr)
			continue;

		particle_emitter->InvalidateManager();
	}

	m_emitters.clear();
}

KCL::SceneHandler * GFXB::ParticleSystemManager::GetScene()
{
	return m_scene;
}

int GFXB::ParticleSystemManager::GetParticleEmitterSeed()
{
	return (int)m_emitters.size();
}

void GFXB::ParticleSystemManager::Init(KCL::SceneHandler *scene)
{
	using namespace KCL;

	m_scene = scene;

	int max_allowed_uniform_bytes = 4096;
	max_allowed_uniform_bytes -= 2 * 64; // mvp and view matrices
	max_allowed_uniform_bytes -= 4 * 16; // 2 * float4 for texture samplers (might be smaller but that makes no difference)
	int upload_pool_size = max_allowed_uniform_bytes / sizeof(ParticleShaderParams);
	int upload_struct_size = sizeof(ParticleShaderParams) / sizeof(Vector4D);

	INFO("Particle struct size: %d * float4", upload_struct_size);
	INFO("Particle batch size: %d", upload_pool_size);
	INFO("Particle batch aux bytes: %d", 4096 - max_allowed_uniform_bytes);
	INFO("Particle batch unused bytes: %d", max_allowed_uniform_bytes - upload_pool_size * sizeof(ParticleShaderParams));


	// Create mesh

	m_default_mesh3 = new GFXB::Mesh3("particle_shape");
	if (m_default_mesh3 == nullptr)
	{
		INFO("Couldn't create Mesh3: \"particle_shape\"");
		return;
	}

#ifdef PARTICLE_BATCH_RENDER
	float left = -0.5f, right = 0.5f, bottom = -0.5f, top = 0.5f, z_offset = 0.0f;

	for (int i = 0; i < upload_pool_size; i++)
	{
		m_default_mesh3->m_vertex_indices[0].push_back((i * 4) + 0);
		m_default_mesh3->m_vertex_indices[0].push_back((i * 4) + 1);
		m_default_mesh3->m_vertex_indices[0].push_back((i * 4) + 2);

		m_default_mesh3->m_vertex_indices[0].push_back((i * 4) + 0);
		m_default_mesh3->m_vertex_indices[0].push_back((i * 4) + 2);
		m_default_mesh3->m_vertex_indices[0].push_back((i * 4) + 3);

		m_default_mesh3->m_vertex_attribs3[0].push_back(Vector3D(left, bottom, z_offset));
		m_default_mesh3->m_vertex_attribs3[0].push_back(Vector3D(right, bottom, z_offset));
		m_default_mesh3->m_vertex_attribs3[0].push_back(Vector3D(right, top, z_offset));
		m_default_mesh3->m_vertex_attribs3[0].push_back(Vector3D(left, top, z_offset));

		m_default_mesh3->m_vertex_attribs2[0].push_back(Vector2D(0.0f, 1.0f));
		m_default_mesh3->m_vertex_attribs2[0].push_back(Vector2D(1.0f, 1.0f));
		m_default_mesh3->m_vertex_attribs2[0].push_back(Vector2D(1.0f, 0.0f));
		m_default_mesh3->m_vertex_attribs2[0].push_back(Vector2D(0.0f, 0.0f));

		m_default_mesh3->m_vertex_attribs2[1].push_back(Vector2D(-1.0f, -1.0f));
		m_default_mesh3->m_vertex_attribs2[1].push_back(Vector2D(1.0f, -1.0f));
		m_default_mesh3->m_vertex_attribs2[1].push_back(Vector2D(1.0f, 1.0f));
		m_default_mesh3->m_vertex_attribs2[1].push_back(Vector2D(-1.0f, 1.0f));
	}
#else
	m_default_mesh3->ConvertToBillboard(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f);
#endif

	m_default_mesh3->UploadMesh();

	m_default_mesh_object = dynamic_cast<KCL::Mesh*>(m_scene->GetMeshFactory().Create("particle_mesh", nullptr, nullptr));
	if (m_default_mesh_object == nullptr)
	{
		INFO("Couldn't create Mesh: \"particle_mesh\"");
		return;
	}
	m_default_mesh_object->m_visible = false;
	m_default_mesh_object->m_mesh = m_default_mesh3;
	m_default_mesh_object->m_mesh_variants[0] = m_default_mesh3;
	m_default_mesh_object->CalculateStaticAABB();


	// Load shaders

	m_default_shader = ShaderFactory::GetInstance()->AddDescriptor(
		ShaderDescriptor("particle.shader", "particle.shader")
#ifdef PARTICLE_BATCH_RENDER
		.AddDefine("PARTICLE_BATCH")
#endif
		.AddDefineUInt("PARTICLE_UPLOAD_POOL_SIZE", upload_pool_size)
		.AddDefineUInt("PARTICLE_UPLOAD_STRUCT_SIZE", upload_struct_size)
	);

	m_wireframe_shader = ShaderFactory::GetInstance()->AddDescriptor(
		ShaderDescriptor("particle_wireframe.shader", "particle_wireframe.shader")
#ifdef PARTICLE_BATCH_RENDER
		.AddDefine("PARTICLE_BATCH")
#endif
		.AddDefineUInt("PARTICLE_UPLOAD_POOL_SIZE", upload_pool_size)
		.AddDefineUInt("PARTICLE_UPLOAD_STRUCT_SIZE", upload_struct_size)
	);


	// Load materials

	LoadMaterial();


	m_particle_upload_pool.resize(upload_pool_size);
}

void GFXB::ParticleSystemManager::LoadMaterial()
{
	if (m_scene != nullptr)
	{
		if (m_color_texture_filename == "")
		{
			INFO("No color texture specified");
			return;
		}
		if (m_normal_texture_filename == "")
		{
			INFO("No normal texture specified");
			return;
		}

		delete m_color_texture;
		delete m_normal_texture;

		m_color_texture = dynamic_cast<GFXB::Texture*>(m_scene->TextureFactory().CreateAndSetup(KCL::Texture_2D, (m_scene->ImagesDirectory() + "/" + m_color_texture_filename).c_str()));
		if (m_color_texture == nullptr)
		{
			std::string msg = "Couldn't load color texture: " + m_color_texture_filename;
			INFO(msg.c_str());
			return;
		}

		m_normal_texture = dynamic_cast<GFXB::Texture*>(m_scene->TextureFactory().CreateAndSetup(KCL::Texture_2D, (m_scene->ImagesDirectory() + "/" + m_normal_texture_filename).c_str()));
		if (m_normal_texture == nullptr)
		{
			std::string msg = "Couldn't load normal texture: " + m_normal_texture_filename;
			INFO(msg.c_str());
			return;
		}
	}
}

void GFXB::ParticleSystemManager::Allocate()
{
	m_ref_pool_target_size = 0;
	for (std::list<ParticleEmitter*>::iterator i = m_emitters.begin(); i != m_emitters.end(); i++)
	{
		ParticleEmitter &emitter = **i;
		m_ref_pool_target_size += emitter.GetMaxParticleCount();
	}

	if (m_particle_ref_pool.size() < m_ref_pool_target_size)
	{
		m_particle_ref_pool.resize(m_ref_pool_target_size);
	}
}

void GFXB::ParticleSystemManager::Warmup(KCL::uint32 job, KCL::uint32 depth_texture, KCL::Camera2 *camera)
{
	using namespace KCL;

	m_active_particle_count = 0;

	uint32 shader = m_default_shader;
	if (m_display_wireframe)
	{
		shader = m_wireframe_shader;
	}

	if (m_scene != nullptr
		&& m_default_mesh3 != nullptr
		&& m_default_mesh_object != nullptr
		&& m_color_texture != nullptr
		&& m_normal_texture != nullptr
		&& shader != 0
		)
	{
		const KCL::Matrix4x4& mv = camera->GetView();
		const KCL::Matrix4x4& mvp = camera->GetViewProjection();

		const void *p[UNIFORM_MAX];

		p[UNIFORM_MV] = mv.v; // used as view matrix (positions a world coords -> model is identity)
		p[UNIFORM_MVP] = mvp.v;

		p[UNIFORM_GBUFFER_DEPTH_TEX] = &depth_texture;
		p[UNIFORM_COLOR_TEX] = &m_color_texture->m_id;
		p[UNIFORM_NORMAL_TEX] = &m_normal_texture->m_id;
		p[UNIFORM_DEPTH_PARAMETERS] = camera->m_depth_linearize_factors.v;

		Vector4D light_pos = Vector4D(50.0f, 1.5f, 0, 1);
		p[UNIFORM_LIGHT_POS] = &light_pos;

		{
			// pack particles

			KCL::uint32 upload_size = 0;
			std::vector<ParticleShaderParams>::iterator pack_iterator;
			for (pack_iterator = m_particle_upload_pool.begin(); pack_iterator != m_particle_upload_pool.end(); pack_iterator++)
			{
				ParticleShaderParams &shader_params = *pack_iterator;
				memset(&shader_params, 0, sizeof(shader_params));
				upload_size++;
			}

			// upload particles

			upload_size = 0; // we don't need to actually draw any particles
			p[UNIFORM_PARTICLE_UPLOAD_POOL] = &m_particle_upload_pool[0];
			p[UNIFORM_PARTICLE_UPLOAD_SIZE] = &upload_size;
			nglDraw(job, NGL_TRIANGLES, shader, 1, &m_default_mesh3->m_vbid, m_default_mesh3->m_ibid, NGL_FRONT_SIDED, p);
		}
	}
}

void GFXB::ParticleSystemManager::AnimateAll(KCL::uint32 animation_time, std::vector<int> &particle_save_frames)
{
	using namespace KCL;

	// save state
	{
		const size_t count = particle_save_frames.size();
		for (size_t i = 0; i < count; i++)
		{
			if (particle_save_frames[i] >= 0 
				&& (uint32)particle_save_frames[i] > m_last_animation_time 
				&& (uint32)particle_save_frames[i] <= animation_time)
			{
				SaveState((uint32)particle_save_frames[i]);
			}
		}
	}

	// animate particles

	for (std::list<ParticleEmitter*>::iterator i = m_emitters.begin(); i != m_emitters.end(); i++)
	{
		ParticleEmitter &emitter = **i;
		emitter.Animate(animation_time);
	}

	m_last_animation_time = animation_time;
}

void GFXB::ParticleSystemManager::RenderAll(KCL::uint32 job, KCL::uint32 depth_texture, KCL::Camera2 *camera)
{
	using namespace KCL;

	m_active_particle_count = 0;

	uint32 shader = m_default_shader;
	if (m_display_wireframe)
	{
		shader = m_wireframe_shader;
	}

	if (m_scene != nullptr
		&& m_default_mesh3 != nullptr
		&& m_default_mesh_object != nullptr
		&& m_color_texture != nullptr
		&& m_normal_texture != nullptr
		&& shader != 0
		)
	{
		Vector3D view_dir = KCL::Vector3D(-camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10]).normalize();
		std::vector<ParticleParams*>::iterator p_ref = m_particle_ref_pool.begin();
		for (std::list<ParticleEmitter*>::iterator i = m_emitters.begin(); i != m_emitters.end(); i++)
		{
			ParticleEmitter &emitter = **i;

			const uint32 active_range_start = emitter.GetActiveRangeStart();
			const uint32 active_range_size = emitter.GetActiveRangeSize();
			const uint32 particle_count = emitter.GetMaxParticleCount();
			for (uint32 j = active_range_start, k = 0; k < active_range_size; j++, k++)
			{
				if (j >= particle_count)
				{
					j = j % particle_count;
				}

				if (emitter.m_particles_active[j])
				{
					ParticleParams &particle = emitter.m_particles[j];
					particle.m_depth_position = Vector3D::dot(particle.m_position, view_dir);
					*p_ref = &particle;
					p_ref++;
					m_active_particle_count++;
				}

				if (p_ref == m_particle_ref_pool.end())
				{
					break;
				}
			}

			if (p_ref == m_particle_ref_pool.end())
			{
				break;
			}
		}

		std::stable_sort(m_particle_ref_pool.begin(), p_ref, CompareParticleRefs);

		const KCL::Matrix4x4& mv = camera->GetView();
		const KCL::Matrix4x4& mvp = camera->GetViewProjection();

		const void *p[UNIFORM_MAX];

		p[UNIFORM_MV] = mv.v; // used as view matrix (positions a world coords -> model is identity)
		p[UNIFORM_MVP] = mvp.v;

		p[UNIFORM_GBUFFER_DEPTH_TEX] = &depth_texture;
		p[UNIFORM_COLOR_TEX] = &m_color_texture->m_id;
		p[UNIFORM_NORMAL_TEX] = &m_normal_texture->m_id;
		p[UNIFORM_DEPTH_PARAMETERS] = camera->m_depth_linearize_factors.v;

		static const Vector4D light_pos = Vector4D(50.0f, 1.5f, 0, 1);// Vector4D(reinterpret_cast<Vector3D&>((*this->m_emitters.begin())->m_world_pom.v41), 1);
		p[UNIFORM_LIGHT_POS] = &light_pos;

		for (std::vector<ParticleParams*>::iterator i = m_particle_ref_pool.begin(); i != p_ref; )
		{
			// pack particles

			KCL::uint32 upload_size = 0;
			std::vector<ParticleShaderParams>::iterator pack_iterator;
			for (pack_iterator = m_particle_upload_pool.begin(); pack_iterator != m_particle_upload_pool.end() && i != p_ref; pack_iterator++, i++)
			{
				ParticleParams &particle = **i;
				ParticleShaderParams &shader_params = *pack_iterator;
				EmitterParams &emitter = *particle.m_emitter_ref;
				reinterpret_cast<Vector3D&>(shader_params.m_particle_position) = particle.m_position;
				shader_params.m_particle_size_rotation_opacity.x = particle.m_size;
				shader_params.m_particle_size_rotation_opacity.y = particle.m_rotation / 180.0f * 3.1415926f;
				shader_params.m_particle_size_rotation_opacity.z = particle.m_opacity * emitter.m_base_opacity;
				shader_params.m_flipbook_frame = particle.m_flipbook_coords_a;
				shader_params.m_particle_additive_roundness_shade.x = emitter.m_additive_factor;
				shader_params.m_particle_additive_roundness_shade.y = emitter.m_roundness;
				shader_params.m_particle_additive_roundness_shade.z = emitter.m_shade_factor;
				upload_size++;
			}

			//for (; pack_iterator != m_particle_upload_pool.end(); pack_iterator++)
			//{
			//	ParticleShaderParams &shader_params = *pack_iterator;
			//	shader_params.m_particle_position = Vector4D(0, 0, 0, 0);
			//	shader_params.m_particle_size_rotation_opacity = Vector4D(0, 0, 0, 0);
			//	shader_params.m_flipbook_frame = Vector4D(0, 0, 0, 0);
			//	shader_params.m_particle_additive_roundness_shade = Vector4D(0, 0, 0, 0);
			//}

			// upload particles

#ifdef PARTICLE_BATCH_RENDER
			p[UNIFORM_PARTICLE_UPLOAD_POOL] = &m_particle_upload_pool[0];
			p[UNIFORM_PARTICLE_UPLOAD_SIZE] = &upload_size;
			nglDraw(job, NGL_TRIANGLES, shader, 1, &m_default_mesh3->m_vbid, m_default_mesh3->m_ibid, NGL_FRONT_SIDED, p);
#else
			for (std::vector<ParticleShaderParams>::iterator j = m_particle_upload_pool.begin(); j != pack_iterator; j++)
			{
				ParticleShaderParams &shader_params = *j;
				p[UNIFORM_PARTICLE_POSITION] = &shader_params.m_particle_position;
				p[UNIFORM_PARTICLE_SIZE_ROTATION_OPACITY] = &shader_params.m_particle_size_rotation_opacity;
				p[UNIFORM_FLIPBOOK_FRAME] = &shader_params.m_flipbook_frame;

				nglDraw(job, NGL_TRIANGLES, shader, 1, &m_default_mesh3->m_vbid, m_default_mesh3->m_ibid, false ? NGL_TWO_SIDED : NGL_FRONT_SIDED, p);
			}
#endif
		}
	}
}

void GFXB::ParticleSystemManager::SaveState(KCL::uint32 animation_time)
{
	std::string filename = GetParticleStateFilename(animation_time);

	INFO("Saving particle state...");

	KCL::File file(filename, KCL::Write, KCL::RWDir, true);

	if (!file.Opened())
	{
		INFO("Cannot save particle state. Could not open '%s'", filename.c_str());
		return;
	}

	ParticleSystemSerializer::SerializeManagerState<true>(file, this);

	file.Close();
}

void GFXB::ParticleSystemManager::RestoreState(KCL::uint32 animation_time)
{
	std::string filename = GetParticleStateFilename(animation_time);

	if (!KCL::AssetFile::Exists(filename))
	{
		INFO("No particle state file found for frame. Skipping restore. (%s)", filename.c_str());
		return;
	}

	INFO("Restoring particle state for frame %d...", animation_time);

	KCL::AssetFile file(filename);

	if (!file.Opened())
	{
		INFO("Cannot restore particle state. Could not open '%s'", filename.c_str());
		m_has_restored_state = false;
		return;
	}

	ParticleSystemSerializer::SerializeManagerState<false>(file, this);

	file.Close();

	m_has_restored_state = true;
}

void GFXB::ParticleSystemManager::SetDisplayWireframe(bool display_wireframe)
{
	m_display_wireframe = display_wireframe;
}

bool GFXB::ParticleSystemManager::GetDisplayWireframe()
{
	return m_display_wireframe;
}

void GFXB::ParticleSystemManager::SetIsPaused(bool is_paused)
{
	m_is_paused = is_paused;
}

bool GFXB::ParticleSystemManager::GetIsPaused()
{
	return m_is_paused;
}

void GFXB::ParticleSystemManager::SetColorTexture(std::string filename)
{
	m_color_texture_filename = filename;
}

void GFXB::ParticleSystemManager::SetNormalTexture(std::string filename)
{
	m_normal_texture_filename = filename;
}

std::string GFXB::ParticleSystemManager::GetColorTextureFilename()
{
	return m_color_texture_filename;
}

std::string GFXB::ParticleSystemManager::GetNormalTextureFilename()
{
	return m_normal_texture_filename;
}

void GFXB::ParticleSystemManager::SetAllFlipbooks(KCL::uint32 rows, KCL::uint32 cols, std::set<KCL::Serializable *> &nodes)
{
	KCL::uint32 flipbook_start_cell;
	KCL::uint32 flipbook_end_cell;
	float flipbook_framrate;

	for (std::list<ParticleEmitter*>::iterator i = m_emitters.begin(); i != m_emitters.end(); i++)
	{
		ParticleEmitter &emitter = **i;

		flipbook_start_cell = emitter.m_flipbook.GetStartCell();
		flipbook_end_cell = emitter.m_flipbook.GetEndCell();
		flipbook_framrate = emitter.m_flipbook.GetFramerate();

		emitter.m_flipbook.Init(rows, cols, flipbook_start_cell, flipbook_end_cell, flipbook_framrate);

		nodes.insert(&emitter);
	}
}

void GFXB::ParticleSystemManager::SetAllColorTextures(std::string filename, std::set<KCL::Serializable*>& nodes)
{
	m_color_texture_filename = filename;

	for (std::list<ParticleEmitter*>::iterator i = m_emitters.begin(); i != m_emitters.end(); i++)
	{
		ParticleEmitter &emitter = **i;
		emitter.SetColorTexture(filename);
		nodes.insert(&emitter);
	}

	LoadMaterial();
}

void GFXB::ParticleSystemManager::SetAllNormalTextures(std::string filename, std::set<KCL::Serializable*>& nodes)
{
	m_normal_texture_filename = filename;

	for (std::list<ParticleEmitter*>::iterator i = m_emitters.begin(); i != m_emitters.end(); i++)
	{
		ParticleEmitter &emitter = **i;
		emitter.SetNormalTexture(filename);
		nodes.insert(&emitter);
	}

	LoadMaterial();
}

size_t GFXB::ParticleSystemManager::GetMaxParticleCount()
{
	return m_particle_ref_pool.size();
}

size_t GFXB::ParticleSystemManager::GetActiveParticleCount()
{
	return m_active_particle_count;
}

size_t GFXB::ParticleSystemManager::GetEmitterCount()
{
	return m_emitters.size();
}

size_t GFXB::ParticleSystemManager::GetParticleCountPerBatch()
{
	return m_particle_upload_pool.size();
}
