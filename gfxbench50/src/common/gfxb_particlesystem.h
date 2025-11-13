/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_PARTICLESYSTEM_H
#define GFXB_PARTICLESYSTEM_H

#include <kcl_base.h>
#include <kcl_math3d.h>
#include <kcl_node.h>
#include <kcl_camera2.h>
#include "gfxb_material.h"
#include "gfxb_mesh.h"
#include "gfxb_mesh_shape.h"
#include "gfxb_texture.h"
#include "gfxb_texture_flipbook.h"
#include <vector>

#define GFXB_PARTICLE_SYSTEM_MAX_PARTICLE_PER_EMITTER 5000

namespace GFXB
{
	class SceneBase;
	class ParticleSystemManager;
	struct EmitterParams;
	class ParticleSystemSerializer;

	struct ParticleShaderParams
	{
		KCL::Vector4D m_particle_position; // w is yet unused (age?)
		KCL::Vector4D m_particle_size_rotation_opacity; // w is yet unused (frameblend?)
		KCL::Vector4D m_flipbook_frame;
		KCL::Vector4D m_particle_additive_roundness_shade;
	};

	struct ParticleParams
	{
		// static params (set only on emit)
		EmitterParams *m_emitter_ref;
		float m_base_size; // global scale
		KCL::Matrix4x4 m_turbulence_base; // aligned with the emitter at the moment of emission
		KCL::Vector3D m_frequency;
		KCL::Vector3D m_amplitude; // global scale
		KCL::Vector3D m_phase;
		float m_lifespan;

		// dynamic params (updated on Simulate())
		float m_age;
		KCL::Vector3D m_position; // global scale
		KCL::Vector3D m_velocity; // global scale
		float m_angular_velocity;
		KCL::uint32 m_flipbook_start_frame;

		// implicit params (driven by other parameters and falloff curves)
		float m_opacity; // curve driven
		KCL::Vector4D m_tint; //  curve driven
		float m_size; // curve driven
		float m_rotation; // param driven (initialRotation, angularVelocity, ~age)
		float m_flipbook_blend_factor; // param driven (flipbookStartFrame, ~age)
		KCL::Vector4D m_flipbook_coords_a; // param driven (flipbookStartFrame, ~age)
		//KCL::Vector2D m_flipbook_coords_b; // param driven (flipbookStartFrame, ~age)
		float m_depth_position; // param driven (position, view_dir)
	};

	struct EmitterParams
	{
		KCL::Matrix4x4 m_pom;
		uint32_t m_id;

		float m_emit_rate;
		float m_lifespan[2];

		float m_base_size[2]; // global scale
		float m_focus_distance; // global scale
		float m_emit_velocity[2]; // global scale
		float m_emit_rotation[2];
		float m_angular_velocity[2];
		KCL::Vector3D m_aperture; // global scale
		KCL::Vector3D m_frequency[2];
		KCL::Vector3D m_amplitude[2]; // global scale

		KCL::Vector3D m_gravity; // global scale
		KCL::Vector3D m_wind_velocity; // global scale
		float m_air_resistance;
		float m_external_velocity_inheritance;

		KCL::Vector2D m_size_curve[4];
		KCL::Vector2D m_opacity_curve[4];
		//KCL::Vector2D tintCurve[4]; // TODO
		float m_additive_factor; // 0: alpha blended, 1: additive
		float m_roundness; // 0: flat normals, 1: spherical normals
		float m_shade_factor; // 0: shadeless, 1: shaded
		float m_base_opacity;

		// implicit params
		KCL::Matrix4x4 m_orientation_matrix;
		KCL::Vector3D m_external_velocity; // emitter position delta * steptime
	};

	class ParticleEmitter : public KCL::Node ///////////////////////////////////////////
	{
		friend class ParticleSystemSerializer;

	public:
		EmitterParams m_emitter;
		std::vector<ParticleParams> m_particles;
		std::vector<bool> m_particles_active;
		TextureFlipbook m_flipbook;
		bool m_flipbook_randomize_start_frame;

	private:
		ParticleSystemManager *m_manager;
		KCL::_key_node *m_emit_rate_track;

		// binary serialization members block start
		KCL::uint32 m_max_particle_count;
		KCL::uint32 m_new_particle_base_index;
		KCL::uint32 m_active_particle_count;
		KCL::uint32 m_active_range_start;
		KCL::uint32 m_active_range_size;

		bool m_is_first_run;
		int m_seed;
		float m_track_emit_rate;
		bool m_override_track_emit_rate;

		float m_step_rate;
		KCL::uint32 m_step_time_ms;
		float m_step_time;

		KCL::uint32 m_last_animation_time_ms; // timestamp of the last Animate() call
		KCL::uint32 m_last_time_residue_ms; // timestamp of the last Animate() call minus the timestamp of the last finished step
		KCL::uint32 m_emits_queued;
		float m_emit_timer;
		KCL::Matrix4x4 m_last_pom;
		// binary serialization members block end

		std::string m_color_texture_filename;
		std::string m_normal_texture_filename;

	public:
		ParticleEmitter(ParticleSystemManager *manager, const std::string &name, KCL::Node *parent, KCL::Object *owner);
		~ParticleEmitter();

		bool Init();
		void SetDefaults();
		void EnforceParamRanges();
		void UpdateEmitter(const KCL::Matrix4x4 &new_pom, KCL::uint32 animation_time);
		void EmitParticle(ParticleParams &particle);
		void SimulateParticle(ParticleParams &particle);
		void Reset();
		void Reset(KCL::uint32 max_particle_count, bool signal_allocate = true);
		void Animate(KCL::uint32 animation_time);
		void Emit();
		void Simulate();
		virtual void Serialize(JsonSerializer& s) override;
		virtual std::string GetParameterFilename() const override;

		void SetStepRate(float stepRate);
		KCL::uint32 GetActiveParticleCount();
		bool GetHasEmitRateTrack();
		void SetOverrideTrackEmitRate(bool value);
		bool GetOverrideTrackEmitRate();
		float GetTrackEmitRate();
		float GetCurrentEmitRate();
		void SetColorTexture(std::string filename);
		void SetNormalTexture(std::string filename);
		void InvalidateManager();
		ParticleSystemManager* GetManager(); // can be nullptr!

		inline KCL::uint32 GetMaxParticleCount() const
		{
			return m_max_particle_count;
		}

		inline KCL::uint32 GetActiveRangeStart() const
		{
			return m_active_range_start;
		}

		inline KCL::uint32 GetActiveRangeSize() const
		{
			return m_active_range_size;
		}
	};

	class ParticleSystemManager //////////////////////////////////////////////////////////
	{
		friend class ParticleSystemSerializer;

		KCL::SceneHandler *m_scene;

		size_t m_ref_pool_target_size;

		KCL::Mesh *m_default_mesh_object;
		GFXB::Mesh3 *m_default_mesh3;
		KCL::uint32 m_default_shader;
		KCL::uint32 m_wireframe_shader;
		GFXB::Texture *m_color_texture;
		GFXB::Texture *m_normal_texture;

		std::list<ParticleEmitter*> m_emitters;
		uint32_t m_next_emitter_id;
		std::vector<ParticleParams*> m_particle_ref_pool;
		std::vector<ParticleShaderParams> m_particle_upload_pool;
		size_t m_active_particle_count;

		bool m_display_wireframe;
		bool m_is_paused;
		std::string m_color_texture_filename;
		std::string m_normal_texture_filename;

		bool m_has_restored_state;
		KCL::uint32 m_last_animation_time;

	public:
		ParticleSystemManager();
		~ParticleSystemManager();

		ParticleEmitter *CreateParticleEmitter(const std::string &name, KCL::Node *parent, KCL::Object *owner);
		void UnregisterParticleEmitter(ParticleEmitter *particle_emitter);
		void UnregisterAllParticleEmitters();

		KCL::SceneHandler *GetScene();
		int GetParticleEmitterSeed();
		void Init(KCL::SceneHandler *scene);
		void LoadMaterial();
		void Allocate();
		void Warmup(KCL::uint32 job, KCL::uint32 depth_texture, KCL::Camera2 *camera);
		void AnimateAll(KCL::uint32 animation_time, std::vector<int> &particle_save_frames);
		void RenderAll(KCL::uint32 job, KCL::uint32 depth_texture, KCL::Camera2 *camera);

		void SaveState(KCL::uint32 animation_time);
		void RestoreState(KCL::uint32 animation_time);

		void SetDisplayWireframe(bool display_wireframe);
		bool GetDisplayWireframe();
		void SetIsPaused(bool is_paused);
		bool GetIsPaused();
		void SetColorTexture(std::string filename);
		void SetNormalTexture(std::string filename);
		std::string GetColorTextureFilename();
		std::string GetNormalTextureFilename();

		void SetAllFlipbooks(KCL::uint32 rows, KCL::uint32 cols, std::set<KCL::Serializable *> &nodes);
		void SetAllColorTextures(std::string filename, std::set<KCL::Serializable *> &nodes);
		void SetAllNormalTextures(std::string filename, std::set<KCL::Serializable *> &nodes);

		size_t GetMaxParticleCount();
		size_t GetActiveParticleCount();
		size_t GetEmitterCount();
		size_t GetParticleCountPerBatch();
	};
}

#endif