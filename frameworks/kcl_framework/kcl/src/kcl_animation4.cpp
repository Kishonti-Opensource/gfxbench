/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_io.h>
#include <kcl_animation4.h>
#include <stdint.h>

namespace KCL
{

_key_node::_key_node(): m_divider( 0.0f), m_data( 0)
{
	m_children[0] = 0;
	m_children[1] = 0;
}


_key_node::~_key_node()
{
	delete m_data;
	delete m_children[0];
	delete m_children[1];
}


_key_node* _key_node::Search( _key_node *root, float time)
{
	_key_node *node = root;

	while( node->m_children[0])
	{
		if( time >= node->m_divider)
		{
			node = node->m_children[1];
		}
		else
		{
			node = node->m_children[0];
		}
	}

	return node;
}


void Qslerp( Vector4D &q1, Vector4D &q2, float t, Vector4D &result)
{
	float dot, s1, s2, om, sinom;
	dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;

	if (dot < 0) {
		q1 = -q1;
		dot  = - dot;
	}
	if ((1.0f - dot) > 0.0015) { // spherical interpolation
		om    = acos (dot);
		sinom = sin (om);
		s1    = sin ((1.0f - t)*om) / sinom;
		s2    = sin (t*om)/ sinom;
	} else { // linear interpolation
		s1 = 1.0f - t;
		s2 = t;
	}
	result = s1*q1 + s2*q2;
}


float _key_node::Get( Vector4D &result, _key_node *root, float &time, float &time_base, bool repeat)
{
	float t = time - time_base;

	_key_node* node = Search( root, t);


	if( node->m_data->m_time > t)
	{
		t = node->m_data->m_time;
	}

	float interpolator = (t - node->m_data->m_time) * node->m_data->m_inv_time_length;
	float one_minus_interpolator = 1.0f - interpolator;

	switch( node->m_data->m_interpolation_type)
	{
	case KIT_FINISHED:
		{
			if( repeat)
			{
				time_base = time;
			}
			interpolator = -1.0f;
		}
	case KIT_NONE:
		{
			result = node->m_data->m_data;
			break;
		}
	case KIT_LRP1:
		{
			result.x = node->m_data->m_data.x * one_minus_interpolator + node->m_data->m_next_data.x * interpolator;
			break;
		}
	case KIT_LRP3:
		{
			result.x = node->m_data->m_data.x * one_minus_interpolator + node->m_data->m_next_data.x * interpolator;
			result.y = node->m_data->m_data.y * one_minus_interpolator + node->m_data->m_next_data.y * interpolator;
			result.z = node->m_data->m_data.z * one_minus_interpolator + node->m_data->m_next_data.z * interpolator;
			break;
		}
	case KIT_SLRP4:
		{
			Qslerp( node->m_data->m_data, node->m_data->m_next_data, interpolator, result);
			break;
		}
	case KIT_BEZIER1:
		{
			float A = one_minus_interpolator * one_minus_interpolator * one_minus_interpolator;
			float B = 3.0f * interpolator * one_minus_interpolator * one_minus_interpolator;
			float C = 3.0f * interpolator * interpolator * one_minus_interpolator;
			float D = interpolator * interpolator * interpolator;

			result.x = node->m_data->m_data.x * A + node->m_data->m_out_tangent * B + node->m_data->m_next_in_tangent * C + node->m_data->m_next_data.x * D;

			result.x = node->m_data->m_data.x * one_minus_interpolator + node->m_data->m_next_data.x * interpolator;
			result.y = node->m_data->m_data.y * one_minus_interpolator + node->m_data->m_next_data.y * interpolator;
			result.z = node->m_data->m_data.z * one_minus_interpolator + node->m_data->m_next_data.z * interpolator;

			break;
		}
	case KIT_MAX:
		{
			break;
		}
	}
	return interpolator;
}


void _key_node::Read( _key_node *&node, AssetFile &io)
{
	if (KCL::g_os->LoadingCallback(0)!=KCL_TESTERROR_NOERROR) return;
	bool data_exist;

	if (node != 0) 
	{
		INFO("WARNING! Animation node not null. Possible memory leak or uninitialized pointer!") ;
	}

	node = new _key_node;

	if( (io.Read(&node->m_divider, sizeof( float), 1) == 0) || (io.Read(&data_exist, sizeof( bool), 1) == 0) )
	{
		INFO("Read error!");
		return;
	}
	
	if( data_exist )
	{
		node->m_data = new _key_data;
		if ((io.Read( &node->m_data->m_data, sizeof( Vector4D), 1) == 0) ||
			(io.Read( &node->m_data->m_interpolation_type, sizeof( _key_interpolation_type), 1) == 0) ||
			(io.Read( &node->m_data->m_inv_time_length, sizeof( float), 1) == 0) ||
			(io.Read( &node->m_data->m_next_data, sizeof( Vector4D), 1) == 0) ||
			(io.Read( &node->m_data->m_next_in_tangent, sizeof( float), 1) == 0) ||
			(io.Read( &node->m_data->m_out_tangent, sizeof( float), 1) == 0) ||
			(io.Read( &node->m_data->m_time, sizeof( float), 1) == 0) ||
			(io.Read( &node->m_data->m_time_length, sizeof( float), 1) == 0))
		{
			INFO("Read error!");
			return;
		}
	}
	else
	{
		Read( node->m_children[0], io);
		Read( node->m_children[1], io);
	}
}


void _key_node::PrepareData(std::vector<_key_data> &data)
{
	for (size_t i = 0; i < data.size(); i++)
	{
		if (i == data.size() - 1)
		{
			data[i].m_time_length = 0.0f;
			data[i].m_inv_time_length = 0.0f;
			data[i].m_interpolation_type = KIT_FINISHED;
		}
		else
		{
			data[i].m_time_length = data[i + 1].m_time - data[i].m_time;
			data[i].m_inv_time_length = 1.0f / data[i].m_time_length;
			data[i].m_next_data = data[i+1].m_data;
			data[i].m_next_in_tangent = data[i + 1].m_next_in_tangent;
		}
	}
}

void _key_node::Create(_key_node *&node, std::vector<_key_data> &data, uint32 offset, uint32 size)
{
	if (node != 0) 
	{
		INFO("WARNING! Animation node not null. Possible memory leak or uninitialized pointer!") ;
	}

	node = new KCL::_key_node;

	if (size == 1)
	{
		node->m_data = new KCL::_key_data;
		memcpy(node->m_data, &data[offset], sizeof(KCL::_key_data));
		return;
	}
	else
	{
		uint32_t lower_half_size = size / 2;
		uint32_t upper_half_size = size - lower_half_size;

		node->m_divider = data[offset + lower_half_size].m_time;

		Create(node->m_children[0], data, offset, lower_half_size);
		Create(node->m_children[1], data, offset + lower_half_size, upper_half_size);
	}
}


void _key_node::Save(_key_node *node, File &io)
{
	bool zero = false;
	bool one = true;
	io.Write(&node->m_divider, sizeof(node->m_divider), 1);

	if (node->m_data)
	{
		io.Write(&one, sizeof(bool), 1);
		io.Write(&node->m_data->m_data, sizeof(Vector4D), 1);
		io.Write(&node->m_data->m_interpolation_type, sizeof(_key_interpolation_type), 1);
		io.Write(&node->m_data->m_inv_time_length, sizeof(float), 1);
		io.Write(&node->m_data->m_next_data, sizeof(Vector4D), 1);
		io.Write(&node->m_data->m_next_in_tangent, sizeof(float), 1);
		io.Write(&node->m_data->m_out_tangent, sizeof(float), 1);
		io.Write(&node->m_data->m_time, sizeof(float), 1);
		io.Write(&node->m_data->m_time_length, sizeof(float), 1);
	}
	else
	{
		io.Write(&zero, sizeof(bool), 1);
		Save(node->m_children[0], io);
		Save(node->m_children[1], io);
	}
}


bool ReadAnimation(_key_node *&animation, const std::string &filename_)
{
	std::string filename;
	if (filename_.find("animations/") == std::string::npos) //HACK
	{
		filename = "animations/" + filename_;
	}
	else
	{
		filename = filename_;
	}

	AssetFile file(filename);
	if (file.Opened())
	{
		_key_node::Read(animation, file);
		return true;
	}

	return false;
}

}
