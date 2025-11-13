/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_OVR_SETTINGS_MANAGER_H
#define GFXB_OVR_SETTINGS_MANAGER_H

#include <map>
#include <string>
#include <assert.h>
#include "common/gfxb_shader.h"

namespace GFXB
{
	struct OvrOption
	{
		std::string m_name;
		float m_cost; //ms
		//[0...10]
		//0 means not visible
		//10 means huge effect on scene look
		int m_importance;
	protected:
		OvrOption()
		{
		}
	};

	struct OvrIntOption : public OvrOption
	{
		int m_option; //the actual option to set: #define NAME option

		OvrIntOption()
		{
		}
		OvrIntOption( const std::string& n, int o, float c, int i )
		{
			m_name = n;
			m_option = o;
			m_cost = c;
			m_importance = i;
		}
	};

	enum OvrOptimizationResult
	{
		FAILED = 0, //the device does not reach target framerate even if everything is turned off
		WARMUP_STAGE, //make sure shaders are loaded etc.
		MEASURED, //at this frame, we only measured (to accumulate 10 frames data)
		OPTIMIZED, //at this frame we turned off some option
		DONE //the device reached target framerate for some level of settings
	};

	class OvrSettingsManager
	{
		std::map<std::string, OvrIntOption> m_int_options;

		//these won't change at runtime
		std::map<std::string, OvrIntOption> m_static_int_options;

		bool m_has_changed; //did we change an option?
		int m_target_fps; //target fps we want to achieve
		unsigned m_current_option; //option we will change next time
		bool m_done; //is the optimization done?

		static bool Sorter( OvrIntOption* a, OvrIntOption* b )
		{
			//importance: [0...10]
			//0 means not at all visible/important
			//10 means it has a huge impact on the scene
			
			//cost: 
			//c: effect cost in ms
			//cost = c / target_budget_ms
			//so the importance is weighted by the cost of the effect
			//ie. we would like to turn off effects that have little importance
			//and are costly, and so on, so forth...
			return 1.0f/a->m_importance * a->m_cost > 1.0f/b->m_importance * b->m_cost;
		}

	public:
		OvrSettingsManager() : m_has_changed(false), m_target_fps(58), m_current_option(0), m_done(false) { }

		int GetScore() const
		{
			return m_current_option;
		}

		//return if done or not
		OvrOptimizationResult Optimize( float current_fps )
		{
			//logic:
			//turn off effects starting from least important to most important
			//until fps >= target fps
			//aggregate 10 frame measure

			if( m_done )
			{
				return OvrOptimizationResult::DONE;
			}

			if( m_current_option >= m_int_options.size() )
			{
				INFO( "Optimization FAILED" );
				INFO( "Device can't reach target framerate: %i FPS", m_target_fps );
				return OvrOptimizationResult::FAILED;
			}

			if( current_fps < 0.0f )
				return OvrOptimizationResult::WARMUP_STAGE;

			/*const int aggregate_frames = 30;
			static int counter = 0;
			static float fps_array[aggregate_frames];
			if(counter < aggregate_frames )
			{ 
				fps_array[counter] = current_fps;
				counter++;
				return OvrOptimizationResult::MEASURED;
			}

			counter = 0;

			float avg_fps = 0.0f;
			for( int c = 0; c < aggregate_frames; ++c )
			{
				avg_fps += fps_array[c];
			}
			avg_fps /= aggregate_frames;*/

			float avg_fps = current_fps;

			if( avg_fps >= m_target_fps )
			{
				m_done = true;
				INFO("Optimization complete");
				INFO( "Target framerate: %i FPS", m_target_fps );
				INFO( "Optimized framerate: %.2f FPS", avg_fps );
				INFO( "Score [1...%i]: %i", m_int_options.size(), m_int_options.size() - m_current_option );
				return OvrOptimizationResult::DONE;
			}

			std::vector<OvrIntOption*> option_list;
			option_list.reserve( m_int_options.size() );
			
			for( std::map<std::string, OvrIntOption>::iterator it = m_int_options.begin(); it != m_int_options.end(); ++it )
			{
				option_list.push_back( &( it->second ) );
			}

			std::sort( option_list.begin(), option_list.end(), Sorter );

			//Hardcoded, so that we always try to make the shadow map smaller before actually disabling it...
			if( option_list[m_current_option]->m_name == "ENABLE_SHADOW_MAPPING" )
			{
				//first let's try to make the shadow map smaller before going all in
				if( m_int_options["SHADOW_MAP_SIZE"].m_option > 1024 )
				{
					SetIntOption( "SHADOW_MAP_SIZE", 1024 );
				}
				else if( m_int_options["SHADOW_MAP_SIZE"].m_option > 512 )
				{
					SetIntOption( "SHADOW_MAP_SIZE", 512 );
				}
				else
				{
					SetIntOption( "ENABLE_SHADOW_MAPPING", 0 ); //disable shadow maps as the last measure
					m_current_option++;
				}

				return OvrOptimizationResult::OPTIMIZED;
			}
			else if( option_list[m_current_option]->m_name == "SHADOW_MAP_SIZE" )
			{
				if( m_int_options["SHADOW_MAP_SIZE"].m_option > 1024 )
				{
					SetIntOption( "SHADOW_MAP_SIZE", 1024 );
				}
				else
				{
					SetIntOption( "SHADOW_MAP_SIZE", 512 );
				}

				m_current_option++;
				return OvrOptimizationResult::OPTIMIZED;
			}

			if( option_list[m_current_option]->m_name == "SSAO_ENABLED" )
			{
				//skip ssao, as it's now disabled by default...
				m_current_option++;
			}

			//if not shadow mapping, disable the desired option
			SetIntOption( option_list[m_current_option]->m_name, !option_list[m_current_option]->m_option );
			
			m_current_option++;

			return OvrOptimizationResult::OPTIMIZED;
		}

		void SetTargetFps( int f )
		{
			m_target_fps = f;
		}

		void SetIntOption( const std::string& str, int o )
		{
			m_int_options[str].m_option = o;
			m_has_changed = true;
		}

		void AddIntOption( const std::string& str, int o, float c, int i )
		{
			//only add once
			assert( m_int_options.find( str ) == m_int_options.end() );
			m_int_options[str] = OvrIntOption( str, o, c / (1.0f / m_target_fps * 1000.0f), i );
		}

		void AddStaticIntOption( const std::string& str, int o, float c = 0.0f, int i = 0 )
		{
			//only add once
			assert( m_static_int_options.find( str ) == m_static_int_options.end() );
			m_static_int_options[str] = OvrIntOption( str, o, c, i );
		}

		OvrIntOption getStaticIntOption( const std::string& str )
		{
			//only add once
			assert( m_static_int_options.find( str ) != m_static_int_options.end() );
			return m_static_int_options[str];
		}

		OvrIntOption getIntOption( const std::string& str )
		{
			//only add once
			assert( m_int_options.find( str ) != m_int_options.end() );
			return m_int_options[str];
		}

		void ApplyOptions()
		{
			//apply all options globally for all shaders
			//pray we don't break anything...
			ShaderFactory* shader_factory = ShaderFactory::GetInstance();

			for( std::map<std::string, OvrIntOption>::iterator it = m_static_int_options.begin(); it != m_static_int_options.end(); ++it )
			{
				shader_factory->AddGlobalDefineInt( it->first.c_str(), it->second.m_option );
			}

			for( std::map<std::string, OvrIntOption>::iterator it = m_int_options.begin(); it != m_int_options.end(); ++it )
			{
				shader_factory->AddGlobalDefineInt( it->first.c_str(), it->second.m_option );
			}

			m_has_changed = false;
		}

		bool GetHasChanged()
		{
			return m_has_changed;
		}
	};
}

#endif