/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_WRAPPER_PRIVATE_H
#define GL_WRAPPER_PRIVATE_H

#include "gl_wrapper_gl31_functions.h"
#include "../gl_wrapper.h"
#include <map>
#include <set>
#include <string>
#include <vector>

//NOTE: see platform.h on how statistics works
#define ENABLE_QUICK_STATS 0

namespace GL_Impl
{
	#include "oglx/gl.h"
}

// Forward declarations
namespace GLB
{
	class ContextLimits;
}

class GLWrapperImpl : public GLWrapper
{
public:
    enum GlObjectType
    {
        TYPE_BUFFER = 0,
        TYPE_SHADER,
        TYPE_PROGRAM,
        TYPE_PROGRAM_PIPELINE,
        TYPE_TEXTURE,
        TYPE_SAMPLER,
        TYPE_RENDERBUFFER,
        TYPE_FRAMEBUFFER,
        TYPE_VERTEX_ARRAY,
        TYPE_TRANSFORM_FEEDBACK,
        TYPE_QUERY,
        TYPE_SYNC,
	TYPE_COUNT
    };

	static GLWrapperImpl * Get();

	void OnBindSampler(GLuint unit, GLuint sampler);
	void OnBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
	void OnActiveTexture(GLenum texture);
	
	void OnBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
	void OnDeleteBuffers(GLsizei n, const GLuint *buffers);

	void OnDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);

	void CheckGLError(const char *fn);

	void OnFunctionCall(const char *name);
	void OnPreDrawCall(GLenum mode, GLsizei vertex_count, GLsizei instance_count);
	void OnPostDrawCall();

    void OnGenObject(GlObjectType type, GLsizei n, const GLuint *ids);
    void OnDeleteObject(GlObjectType type, GLsizei n, const GLuint *ids);

    // GLWrapper interface implementation
	virtual const KCL::uint32 BeginMeasure(const unsigned int flags);
	virtual MeasureResults EndMeasure(const KCL::uint32 id);

    virtual void BeginMeasureQuick();
    virtual void EndMeasureQuick(KCL::uint32& o_qd, KCL::uint32& o_qp, KCL::uint32& o_qpq);

    virtual void PauseMeasurement();
    virtual void ResumeMeasurement();
    virtual bool IsMeasurementPaused();

	virtual void CheckLeaks() ;

private:
	GLWrapperImpl();
	~GLWrapperImpl();

	GLWrapperImpl(const GLWrapperImpl &copy){}

	GLB::ContextLimits * GetLimits();

	std::map<std::string, unsigned int> num_fn_called;
	void collectActiveTextures(DrawCallStatistics &draw_call);


	GLuint m_query_samples;
	GLuint m_query_primitives;

    GLuint m_query_pipeline[DrawCallStatistics::query_stats_count]; //using ARB_pipeline_statistics_query

	bool m_query_disabled;
    bool m_measurement_paused;

	std::map<GLuint,int> m_object_counter[GLWrapperImpl::TYPE_COUNT] ;

	static GLWrapperImpl * instance;
	static const char * GetErrorString(GLenum value);
    static GLuint GetBufferBinding(GLenum target);
};


std::string GLObjectTypeToString(int type) ;


#endif
