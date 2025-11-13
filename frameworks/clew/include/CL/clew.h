/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CLEW_H
#define CLEW_H

#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#include <CL/opencl.h>
#include <string>

#define OCLB_PROCESS_CL_1_0_FUNCTIONS(action) \
		action(clGetPlatformIDs) \
        action(clGetPlatformInfo) \
        action(clGetDeviceIDs) \
        action(clGetDeviceInfo) \
        action(clCreateContext) \
        action(clCreateContextFromType) \
        action(clRetainContext) \
        action(clReleaseContext) \
        action(clGetContextInfo) \
        action(clCreateCommandQueue) \
        action(clRetainCommandQueue) \
        action(clReleaseCommandQueue) \
        action(clGetCommandQueueInfo) \
        action(clCreateBuffer) \
        action(clCreateImage2D) \
        action(clCreateImage3D) \
        action(clRetainMemObject) \
        action(clReleaseMemObject) \
        action(clGetSupportedImageFormats) \
        action(clGetMemObjectInfo) \
        action(clGetImageInfo) \
        action(clCreateSampler) \
        action(clRetainSampler) \
        action(clReleaseSampler) \
        action(clGetSamplerInfo) \
        action(clCreateProgramWithSource) \
        action(clCreateProgramWithBinary) \
        action(clRetainProgram) \
        action(clReleaseProgram) \
        action(clBuildProgram) \
        action(clGetProgramInfo) \
        action(clGetProgramBuildInfo) \
        action(clCreateKernel) \
        action(clCreateKernelsInProgram) \
        action(clRetainKernel) \
        action(clReleaseKernel) \
        action(clSetKernelArg) \
        action(clGetKernelInfo) \
        action(clGetKernelWorkGroupInfo) \
        action(clWaitForEvents) \
        action(clGetEventInfo) \
        action(clRetainEvent) \
        action(clReleaseEvent) \
        action(clGetEventProfilingInfo) \
        action(clFlush) \
        action(clFinish) \
        action(clEnqueueReadBuffer) \
        action(clEnqueueWriteBuffer) \
        action(clEnqueueCopyBuffer) \
        action(clEnqueueReadImage) \
        action(clEnqueueWriteImage) \
        action(clEnqueueCopyImage) \
        action(clEnqueueCopyImageToBuffer) \
        action(clEnqueueCopyBufferToImage) \
        action(clEnqueueMapBuffer) \
        action(clEnqueueMapImage) \
        action(clEnqueueUnmapMemObject) \
        action(clEnqueueNDRangeKernel) \
        action(clEnqueueNativeKernel) \
        action(clEnqueueMarker) \
        action(clEnqueueWaitForEvents) \
        action(clEnqueueBarrier) \
        action(clEnqueueTask) \
        action(clUnloadCompiler) \
        action(clGetExtensionFunctionAddress)

#define OCLB_PROCESS_CL_1_1_FUNCTIONS(action) \
        action(clCreateSubBuffer) \
        action(clSetMemObjectDestructorCallback) \
        action(clCreateUserEvent) \
        action(clSetUserEventStatus) \
        action(clSetEventCallback) \
        action(clEnqueueReadBufferRect) \
        action(clEnqueueWriteBufferRect) \
        action(clEnqueueCopyBufferRect)

#define OCLB_PROCESS_CL_1_2_FUNCTIONS(action) \
        action(clCreateSubDevices) \
        action(clRetainDevice) \
        action(clReleaseDevice) \
        action(clCreateImage) \
        action(clCreateProgramWithBuiltInKernels) \
        action(clCompileProgram) \
        action(clLinkProgram) \
        action(clUnloadPlatformCompiler) \
        action(clGetKernelArgInfo) \
        action(clEnqueueFillBuffer) \
        action(clEnqueueFillImage) \
        action(clEnqueueMigrateMemObjects) \
        action(clEnqueueMarkerWithWaitList) \
        action(clEnqueueBarrierWithWaitList) \
        action(clGetExtensionFunctionAddressForPlatform)

#define OCLB_PROCESS_CL_2_0_FUNCTIONS(action) \
        action(clCreateCommandQueueWithProperties) \
        action(clCreatePipe) \
        action(clGetPipeInfo) \
        action(clSVMAlloc) \
        action(clSVMFree) \
        action(clCreateSamplerWithProperties) \
        action(clSetKernelArgSVMPointer) \
        action(clSetKernelExecInfo) \
        action(clEnqueueSVMFree) \
        action(clEnqueueSVMMemcpy) \
        action(clEnqueueSVMMemFill) \
        action(clEnqueueSVMMap) \
        action(clEnqueueSVMUnmap)

#define OCLB_PROCESS_CL_1_0_GL_FUNCTIONS(action) \
        action(clCreateFromGLBuffer) \
		action(clCreateFromGLRenderbuffer) \
		action(clGetGLObjectInfo) \
		action(clGetGLTextureInfo) \
		action(clEnqueueAcquireGLObjects) \
		action(clEnqueueReleaseGLObjects) \
		action(clCreateFromGLTexture2D) \
		action(clCreateFromGLTexture3D)

#define OCLB_PROCESS_CL_1_0_GL_KHR_FUNCTIONS(action) \
        action(clGetGLContextInfoKHR)

#define OCLB_PROCESS_CL_1_2_GL_FUNCTIONS(action) \
		action(clCreateFromGLTexture)

#define OCLB_PROCESS_CL_1_0_EGL_KHR_FUNCTIONS(action) \
        action(clCreateFromEGLImageKHR) \
		action(clEnqueueAcquireEGLObjectsKHR) \
		action(clEnqueueReleaseEGLObjectsKHR) \
		action(clCreateEventFromEGLSyncKHR)

#include "clew_function_types.h"

#define OCLB_DECLARE_POINTER(function) extern PROC_##function FUNC_##function;
#define OCLB_DECLARE_POINTER_VOID(function) extern void* FUNC_##function;

	enum CLVersion
	{
		CL_1_0, CL_1_1, CL_1_2, CL_2_0, CL_UNSET
	};

    OCLB_PROCESS_CL_1_0_FUNCTIONS(OCLB_DECLARE_POINTER)
    OCLB_PROCESS_CL_1_1_FUNCTIONS(OCLB_DECLARE_POINTER)
    OCLB_PROCESS_CL_1_2_FUNCTIONS(OCLB_DECLARE_POINTER)
    OCLB_PROCESS_CL_2_0_FUNCTIONS(OCLB_DECLARE_POINTER)

	OCLB_PROCESS_CL_1_0_GL_FUNCTIONS(OCLB_DECLARE_POINTER)
	OCLB_PROCESS_CL_1_0_GL_KHR_FUNCTIONS(OCLB_DECLARE_POINTER)
	OCLB_PROCESS_CL_1_2_GL_FUNCTIONS(OCLB_DECLARE_POINTER)

	OCLB_PROCESS_CL_1_0_EGL_KHR_FUNCTIONS(OCLB_DECLARE_POINTER)

#include "clew_function_defines.h"

    int clewInit(std::string* oclPathIn = 0, std::string* oclPathOut = 0);

	int clewRelease();

	bool clewInitialized();

	CLVersion clewHighestVersionAvailable();
	CLVersion clewHighestGLExtensionsAvailable();

	void clewHighestVersionAvailable(int* major, int* minor);
	void clewHighestGLExtensionsAvailable(int* major, int* minor);

#endif