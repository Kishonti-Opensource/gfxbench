/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_SCENE_HANLDER_H
#define KCL_SCENE_HANLDER_H

#include <kcl_material.h>
#include <kcl_image.h>
#include <kcl_aabb.h>
#include <kcl_light2.h>
#include <kcl_actor.h>
#include "kcl_mesh.h"
#include <kcl_cube_envmap_descriptor.h>
#include <kcl_scene_version.h>

#include <tinyxml.h>
#include <vector>
#include <string>

namespace KCL // KCL - Kishonti Common Library
{
	#define MAX_PARTICLE_PER_EMITTER 1000
	#define MAX_DEFAULT_JOINT_NUM_PER_MESH 32
	#define BUFF_SIZE 4096

	class LightShape;
	class EnvProbe;

	struct MeshToActorRef
	{
		Node *m_parent;
		Mesh* m_mesh;
		std::string m_geometry_name;
		std::string m_material_name;
		bool m_is_bezier;
	};

	class SceneHandler
	{
		friend class AnimatedEmitter;
		friend class SceneXML;
	public:
		bool InitSceneVersion(const char* scenefile);
		KCLFactories& GetFactoryInstance();

		virtual KCL::TextureFactory &TextureFactory() = 0;
		virtual KCL::Mesh3Factory &Mesh3Factory() = 0;
		virtual KCL::MeshFactory &GetMeshFactory();

		SceneHandler();
		virtual ~SceneHandler();
		KCL_Status Process_Load( uint32 max_uniform_num, const char* path);
		KCL_Status Process_Load_RTRT( uint32 max_uniform_num, const char* path, int readingType);

        void SetProgressPtr(float* ptr) { m_progressPtr = ptr; }
        void IncrementProgressTo(float target);

		void AddGlow(KCL::Light* light, KCL::Actor* actor);

		std::string ImagesDirectory() const;
		std::string EnvmapsDirectory() const;

		virtual void Animate(); // !!!
		void AnimateCamera( KCL::uint32 time, Matrix4x4 &m, float &fov, Vector3D &fp);

		void Move( float delta);
		void Strafe( float delta);
		void StrafeUp(float delta);
		void Elevate( float delta);
		void Rotate( float delta);
		void Tilt( float delta);
		void SelectNextCamera(bool *is_freecamera = 0);

		KCL_Status LoadRooms();
		KCL_Status LoadActors();
		KCL_Status LoadCubeLinks();
        KCL_Status LoadMeshFlags();

		KCL::Material* CreateFlameMaterial();
		KCL::Material* CreateGlowMaterial();
		KCL::Material* CreateShadowCasterMaterial();
		KCL::Material* CreateShadowReceiverMaterial();
		KCL::Material* CreateOmniLightMaterial();
		KCL::Material* CreatePlanarReflectionMaterial();
		KCL::Material* CreateLensFlareMaterial();
		KCL::Material* CreateEnvMapGenMaterial();
		KCL::Material* CreateSmokeMaterial();
		KCL::Material* CreateSteamMaterial();
		KCL::Material* CreateMaterial( const char *name);
		bool ParseMaterial( KCL::Material *material, const char *name);
		bool ParseMaterialConfig( KCL::Material *material, const char *filename);
		bool ParseMaterialTexture( KCL::Material *material, const std::vector<std::string> &tokens);
		KCL::Image2D* CreateLightmap( std::string &name);
		void InitMaterial( KCL::Material* m);

		KCL::Node* ReadEmitter( const std::string &filename, Node *parent, Actor *actor);
		Light* ReadLight( const std::string &filename, Node *parent, Actor *actor);
		Light* ReadLight( const std::string &node, const std::string &shape, Node *parent, Actor *actor);
		Mesh3* ReadGeometry( const std::string &filename, Node *parent, Actor *actor, bool is_bezier, bool show_error);
		void ReadMeshGeometry(KCL::Mesh3 * mesh3, KCL::Actor *actor, AssetFile & file);
		void ReadBezierMeshGeometry(KCL::Mesh3 * mesh3, KCL::Actor *actor, AssetFile & file);
		KCL_Status ReadEntities(int readingType=0);
		void CreateActor( KCL::Actor *actor, std::vector<MeshToActorRef> &mesh_refs);

		void ConnectRooms( const char *r0, const char *r1, const char *p);
		void ConnectRoomsVisible( const char *r0, const char *r1);
		XRoom* SearchMyRoom( const KCL::Vector3D &p);
		XRoom* SearchMyRoomWithPlanes(const KCL::Vector3D &p);
		bool IsPossiblyVisible( XRoom *where, AABB &aabb);

		void setTextureCompressionType(const std::string &texture_compression_type);

        KCL::Node* SearchNode( const char *name) const;
        KCL::Node* SearchNodePartialMatch( const char *name) const;

		bool m_soft_shadow_enabled;
		std::string m_shadow_method_str;

	    bool m_tessellation_enabled;

		bool m_free_camera;
		KCL::uint32 m_play_time;
		KCL::uint32 m_animation_time;
		Camera2 *m_active_camera;
		Camera2 *m_fps_camera;
		Camera2 *m_animated_camera;

		float m_CullNear;
		float m_CullFar;
		KCL::uint32 m_viewport_x;
		KCL::uint32 m_viewport_y;
		KCL::uint32 m_viewport_width;
		KCL::uint32 m_viewport_height;
		bool m_is_portrait;

		float m_camera_focus_distance;

		KCL::Vector3D m_background_color;
		float m_fog_density;
		KCL::Vector3D m_light_dir;
        KCL::Vector3D m_light_dir_orig;
		KCL::Vector3D m_light_color;
		float m_camera_near;
		float m_animation_multiplier;
		KCL::uint32 m_frame;

		bool m_color_texture_is_srgb ;
		bool m_depth_of_field_enabled ;
		bool m_torches_enabled;


		void UpdateCamera();
		Matrix4x4 camTranslateMatrix;
		Matrix4x4 camRotateMatrix;
		Matrix4x4 camPitch;
		Matrix4x4 camYaw;

		KCL::Vector3D m_camera_position;
		KCL::Vector3D m_camera_ref;


		AABB m_shadow_focus_aabb;

		std::vector<KCL::LightShape*> m_light_shapes;
		std::vector<KCL::CubeEnvMapDescriptor> m_envmap_descriptors;
		std::vector<KCL::Mesh3*> m_meshes;
		std::vector<KCL::Material*> m_materials;
        std::vector<KCL::Texture*> m_textures;
		std::vector<KCL::Image2D*> m_lightmaps;
		std::vector<Mesh*> m_sky_mesh;
		std::vector<Mesh*> m_irradiance_meshes;
		std::vector<XRoom*> m_rooms;
		std::vector<XRoomConnection*> m_room_connections;
		std::vector<XPortal*> m_portals;
		std::vector<Actor*> m_actors;
		std::vector<MeshInstanceOwner2*> m_mios2;
		std::vector<Mesh*> m_occluders;
		std::vector<EnvProbe*> m_probes;

		bool **m_pvs;
		Mesh* m_lens_flare_mesh;
		KCL::Material* m_flameMaterial;
		KCL::Material* m_glowMaterial;
		KCL::Material* m_shadowCasterMaterial;
		KCL::Material* m_shadowStaticReceiverMaterial;
		KCL::Material* m_omniLightMaterial;
		KCL::Material* m_planarReflectionMaterial;
		KCL::Material* m_lensFlareMaterial;
		KCL::Material* m_envMapGenMaterial;
		KCL::Material* m_smokeMaterial;
		KCL::Material* m_steamMaterial;
		KCL::Material* m_mblurMaterial;

		KCL::Material* m_instanced_fire_material;
		KCL::Material* m_instanced_smoke_material;
		KCL::Material* m_instanced_spark_material;

		Actor* m_hud_target_actor;


		std::string m_texture_compression_type;
		float m_fps_cam_fov;
		int m_fboEnvMap_size;
		int m_fboShadowMap_size;
		KCL::uint32 m_num_shadow_maps;
		std::string m_tier_level_name;

		_key_node *m_HDR_exposure;
		_key_node *m_active_camera_index_track;
		_key_node *m_camera_position_tracks[32];
		_key_node *m_camera_orientation_tracks[32];
		_key_node *m_camera_fov_tracks[32];
		_key_node *m_camera_focus_position_focus_tracks[32];

		KCL::uint32 m_num_draw_calls;
		KCL::uint32 m_num_triangles;
		KCL::uint32 m_num_vertices;
		KCL::int32 m_num_used_texture;
		KCL::int32 m_num_samples_passed;
        KCL::int32 m_pixel_coverage_sampleCount;
        KCL::int32 m_pixel_coverage_primCount;
		KCL::int32 m_num_instruction;

		//Shader *m_spark_shader;
		//KCL::uint32 m_spark_geometry_vbo; //for instanced
		//KCL::uint32 m_spark_geometry_ebo; //for instanced
		//KCL::uint32 m_spark_indices_count;
		//GLB::Texture2D *m_spark_texture;

		Mesh3 *m_spark_geometry;
		Mesh3 *m_billboard_geometry;
		Mesh3 *m_particle_geometry;
		KCL::AABB m_aabb;
		KCL::Matrix4x4 m_world_fit_matrix;

		SceneVersion m_scene_version;
		bool loadStatic;
		KCL::Matrix4x4 m_prev_vp;
		bool m_mblur_enabled;
		uint32 m_max_joint_num_per_mesh;
		std::string GetVersionStr();
		std::set<KCL::Material*> m_materials_with_video;

	protected:
		void LoadParams(const char* xml_scene_settings_fileName);
		void Load(const TiXmlElement *element);

		virtual KCL_Status OnParamsLoaded() { return KCL_TESTERROR_NOERROR; }
		virtual void OnLoaded() {}
		KCLFactories m_factory;

        float* m_progressPtr;

		std::string m_secondary_path;

		void CreateMeshInstanceOwner2( KCL::Mesh *mesh, KCL::int32 max_instances = 0);
		bool CheckImage( const std::string &filename, const std::string &images_dir, std::string &assetname);

	private:
		DefaultMeshNodeFactory *m_default_mesh_node_factory;


	public:
		/*!
		*Deprecated
		*Use ReadAnimation instead.
		*/
		bool ReadAnimation(KCL::_key_node *&animation, const std::string &filename);
	};

}

#endif

