/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef TFW_SCHEMAS_COMPUTE_H
#define TFW_SCHEMAS_COMPUTE_H

#include "serializable.h"

#ifdef verify
#undef verify
#endif

namespace tfw
{
	class Compute : public Serializable
	{
	public:
		enum Type
		{
			UNSET,
			CL,		// Khronos OpenCL
			CS,		// Khronos OpenGL ES 3.1 Compute Shaders
			CU,		// Nvidia CUDA
			MTL,	// iOS Metal
			RS		// Google Android RenderScript
		};

		Compute() :
			type_(UNSET),
			config_index_(0),
			iter_count_(-1),
			dump_(false),
			warmup_(true),
			fastmath_(true),
			verify_(false),
			verifyEps_(0.0f) {}

		virtual ng::JsonValue toJsonValue() const;
		virtual void fromJsonValue(const ng::JsonValue& value);

		Type type() const { return type_; }
		void setType(Type type) { type_ = type; }
		int configIndex() const { return config_index_; }
		void setConfigIndex(int config_index) { config_index_ = config_index; }
		int iterCount() const { return iter_count_; }
		void setIterCount(int iter_count) { iter_count_ = iter_count; }
		bool dump() const { return dump_; }
		void setDump(bool dump) { dump_ = dump; }
		bool warmup() const { return warmup_; }
		void setWarmup(bool warmup) { warmup_ = warmup; }
		bool fastmath() const { return fastmath_; }
		void setFastmath(bool fastmath) { fastmath_ = fastmath; }
		bool verify() const { return verify_; }
		void setVerify(bool verify) { verify_ = verify; }
		float verifyEps() const { return verifyEps_; }
		void setVerifyEps(float verifyEps) { verifyEps_ = verifyEps; }

	private:
		Type type_;
		int config_index_;
		int iter_count_;
		bool dump_;
		bool warmup_;
		bool fastmath_;
		bool verify_;
		float verifyEps_;
	};
}

#endif