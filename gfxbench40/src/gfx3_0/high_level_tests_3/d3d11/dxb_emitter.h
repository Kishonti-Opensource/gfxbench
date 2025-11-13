/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include "kcl_particlesystem.h"
#include <kcl_particlesystem2.h>
#include "d3d11/dxb_buffer.h"
#include "d3d11/shader.h"
#include "constant_buffer.h"

namespace DXB
{
	class DXBEmitter : public KCL::_emitter
	{
    friend class KCL::_emitter;

	public:
		// Keep in sync with m_animationDataLayout and m_streamOutputLayout declaration
		struct ParticleData
		{
			DirectX::XMFLOAT3 Position;
			float Age;
			float Speed;
			float Acceleration;
			DirectX::XMFLOAT3 Amplitude;
			DirectX::XMFLOAT3 Phase;
			DirectX::XMFLOAT3 Frequency;
			DirectX::XMFLOAT3 Tangent;
			DirectX::XMFLOAT3 Bitangent;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 Velocity;
		};

	private:
		static const UINT m_animationDataLayoutCount = 9;
		static const D3D11_INPUT_ELEMENT_DESC m_animationDataLayout[9];

		static const UINT m_renderVertexLayoutCount = 5;
		static const D3D11_INPUT_ELEMENT_DESC m_renderVertexLayout[5];

		static const UINT m_streamOutputLayoutCount = 9;
		static const D3D11_SO_DECLARATION_ENTRY m_streamOutputLayout[9];

		ID3D11InputLayout* m_animationLayout;
		ID3D11InputLayout* m_particleInstanceLayout;
		ID3D11DepthStencilState* m_disabledRasterizerState;
		KCL::ConstantBuffer* m_constantBuffer;
		KCL::VertexBuffer* m_geometryData;
		DXB::DXBVertexBuffer* m_instanceData[2];
		DXB::DXBVertexBuffer* m_animateSource;
		DXB::DXBVertexBuffer* m_animateTarget;
		DXB::DXBVertexBuffer* m_renderSource;
		Shader* m_animationShader;
		ID3D11GeometryShader* m_geometryShader;
		const Shader* m_renderShader;	// maintained outside

		KCL::int32 m_startBirthIdx;
		KCL::int32 m_endBirthIdx;
		float m_actual_rate;

	public:
		
		DXBEmitter(const std::string &name, KCL::ObjectType type, Node *parent, Object *owner);
		virtual ~DXBEmitter(void);

		/*override*/ void Process();
		/*override*/ void Simulate( KCL::uint32 time);
		/*override*/ KCL::uint32 Emit( KCL::uint32 time)	{ return 0; }

		void setConsts(ConstantBufferEmitter* buffer) const;

		/*
			Configures vertex layout according to specified shader
			@shader[in]		shader used to render particles
		*/
		void setRenderShader(const Shader* shader);

		inline void setConstantBuffer(KCL::ConstantBuffer* buffer)	{ m_constantBuffer = buffer; }

		void bindBuffers() const;

	private:
		void swapBuffers();
		void simulateSubStep();
	};

	class DXBAnimatedEmitterFactory : public KCL::AnimatedEmitterFactory
	{
	public:
		virtual KCL::AnimatedEmitter *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner);
	};
} //namespace DXB