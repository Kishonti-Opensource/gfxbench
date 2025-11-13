/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CUDAW_H
#define CUDAW_H

#include <string>

#define NVCU_PROCESS_3_0_FUNCTIONS(action) \
    action(cuInit) \
    action(cuDriverGetVersion) \
    action(cuDeviceGet) \
    action(cuDeviceGetCount) \
    action(cuDeviceGetName) \
    action(cuDeviceGetAttribute) \
    action(cuDeviceGetProperties) \
    action(cuDeviceComputeCapability) \
    action(cuCtxGetDevice) \
    action(cuCtxSynchronize) \
    action(cuCtxAttach) \
    action(cuCtxDetach) \
    action(cuModuleLoad) \
    action(cuModuleLoadData) \
    action(cuModuleLoadDataEx) \
    action(cuModuleLoadFatBinary) \
    action(cuModuleUnload) \
    action(cuModuleGetFunction) \
    action(cuModuleGetTexRef) \
    action(cuMemFreeHost) \
    action(cuMemHostAlloc) \
    action(cuMemHostGetFlags) \
    action(cuArrayDestroy) \
    action(cuStreamCreate) \
    action(cuStreamQuery) \
    action(cuStreamSynchronize) \
    action(cuEventCreate) \
    action(cuEventRecord) \
    action(cuEventQuery) \
    action(cuEventSynchronize) \
    action(cuEventElapsedTime) \
    action(cuFuncGetAttribute) \
    action(cuFuncSetCacheConfig) \
    action(cuFuncSetBlockShape) \
    action(cuFuncSetSharedSize) \
    action(cuParamSetSize) \
    action(cuParamSeti) \
    action(cuParamSetf) \
    action(cuParamSetv) \
    action(cuLaunch) \
    action(cuLaunchGrid) \
    action(cuLaunchGridAsync) \
    action(cuParamSetTexRef) \
    action(cuTexRefSetArray) \
    action(cuTexRefSetFormat) \
    action(cuTexRefSetAddressMode) \
    action(cuTexRefSetFilterMode) \
    action(cuTexRefSetFlags) \
    action(cuTexRefGetArray) \
    action(cuTexRefGetAddressMode) \
    action(cuTexRefGetFilterMode) \
    action(cuTexRefGetFormat) \
    action(cuTexRefGetFlags) \
    action(cuTexRefCreate) \
    action(cuTexRefDestroy) \
    action(cuGraphicsUnregisterResource) \
    action(cuGraphicsSubResourceGetMappedArray) \
    action(cuGraphicsResourceSetMapFlags) \
    action(cuGraphicsMapResources) \
    action(cuGraphicsUnmapResources) \
    action(cuGetExportTable) \
    action(cuDeviceTotalMem) \
    action(cuCtxCreate) \
    action(cuModuleGetGlobal) \
    action(cuMemGetInfo) \
    action(cuMemAlloc) \
    action(cuMemAllocPitch) \
    action(cuMemFree) \
    action(cuMemGetAddressRange) \
    action(cuMemAllocHost) \
    action(cuMemHostGetDevicePointer) \
    action(cuMemcpyHtoD) \
    action(cuMemcpyDtoH) \
    action(cuMemcpyDtoD) \
    action(cuMemcpyDtoA) \
    action(cuMemcpyAtoD) \
    action(cuMemcpyHtoA) \
    action(cuMemcpyAtoH) \
    action(cuMemcpyAtoA) \
    action(cuMemcpy2D) \
    action(cuMemcpy2DUnaligned) \
    action(cuMemcpy3D) \
    action(cuMemcpyHtoDAsync) \
    action(cuMemcpyDtoHAsync) \
    action(cuMemcpyDtoDAsync) \
    action(cuMemcpyHtoAAsync) \
    action(cuMemcpyAtoHAsync) \
    action(cuMemcpy2DAsync) \
    action(cuMemcpy3DAsync) \
    action(cuMemsetD8) \
    action(cuMemsetD16) \
    action(cuMemsetD32) \
    action(cuMemsetD2D8) \
    action(cuMemsetD2D16) \
    action(cuMemsetD2D32) \
    action(cuArrayCreate) \
    action(cuArrayGetDescriptor) \
    action(cuArray3DCreate) \
    action(cuArray3DGetDescriptor) \
    action(cuTexRefSetAddress) \
    action(cuTexRefSetAddress2D) \
    action(cuTexRefGetAddress) \
    action(cuGraphicsResourceGetMappedPointer) \
    action(cuCtxDestroy) \
    action(cuCtxPushCurrent) \
    action(cuCtxPopCurrent) \
    action(cuStreamDestroy) \
    action(cuEventDestroy)

#define NVCU_PROCESS_3_1_FUNCTIONS(action) \
    action(cuModuleGetSurfRef) \
    action(cuSurfRefSetArray) \
    action(cuSurfRefGetArray) \
    action(cuCtxSetLimit) \
    action(cuCtxGetLimit) \


#define NVCU_PROCESS_3_2_FUNCTIONS(action) \
    action(cuStreamWaitEvent) \
    action(cuCtxGetCacheConfig) \
    action(cuCtxSetCacheConfig) \
    action(cuCtxGetApiVersion) \
    action(cuMemsetD8Async) \
    action(cuMemsetD16Async) \
    action(cuMemsetD32Async) \
    action(cuMemsetD2D8Async) \
    action(cuMemsetD2D16Async) \
    action(cuMemsetD2D32Async)

#define NVCU_PROCESS_4_0_FUNCTIONS(action) \
    action(cuCtxSetCurrent) \
    action(cuCtxGetCurrent) \
    action(cuMemHostRegister) \
    action(cuMemHostUnregister) \
    action(cuMemcpy) \
    action(cuMemcpyPeer) \
    action(cuMemcpy3DPeer) \
    action(cuMemcpyAsync) \
    action(cuMemcpyPeerAsync) \
    action(cuMemcpy3DPeerAsync) \
    action(cuPointerGetAttribute) \
    action(cuLaunchKernel) \
    action(cuDeviceCanAccessPeer) \
    action(cuCtxEnablePeerAccess) \
    action(cuCtxDisablePeerAccess)

#define NVCU_PROCESS_4_1_FUNCTIONS(action) \
    action(cuDeviceGetByPCIBusId) \
    action(cuDeviceGetPCIBusId) \
    action(cuIpcGetEventHandle) \
    action(cuIpcOpenEventHandle) \
    action(cuIpcGetMemHandle) \
    action(cuIpcOpenMemHandle) \
    action(cuIpcCloseMemHandle)

#define NVCU_PROCESS_4_2_FUNCTIONS(action) \
    action(cuCtxGetSharedMemConfig) \
    action(cuCtxSetSharedMemConfig) \
    action(cuFuncSetSharedMemConfig)

#define NVCU_PROCESS_5_0_FUNCTIONS(action) \
    action(cuTexRefSetMipmappedArray) \
    action(cuTexRefSetMipmapFilterMode) \
    action(cuTexRefSetMipmapLevelBias) \
    action(cuTexRefSetMipmapLevelClamp) \
    action(cuTexRefSetMaxAnisotropy) \
    action(cuTexRefGetMipmappedArray) \
    action(cuTexRefGetMipmapFilterMode) \
    action(cuTexRefGetMipmapLevelBias) \
    action(cuTexRefGetMipmapLevelClamp) \
    action(cuTexRefGetMaxAnisotropy) \
    action(cuStreamAddCallback) \
    action(cuMipmappedArrayCreate) \
    action(cuMipmappedArrayGetLevel) \
    action(cuMipmappedArrayDestroy) \
    action(cuTexObjectCreate) \
    action(cuTexObjectDestroy) \
    action(cuTexObjectGetResourceDesc) \
    action(cuTexObjectGetTextureDesc) \
    action(cuTexObjectGetResourceViewDesc) \
    action(cuSurfObjectCreate) \
    action(cuSurfObjectDestroy) \
    action(cuSurfObjectGetResourceDesc) \
    action(cuGraphicsResourceGetMappedMipmappedArray)

#define NVCU_PROCESS_5_5_FUNCTIONS(action) \
    action(cuCtxGetStreamPriorityRange) \
    action(cuStreamCreateWithPriority) \
    action(cuStreamGetPriority) \
    action(cuStreamGetFlags) \
    action(cuLinkCreate) \
    action(cuLinkAddData) \
    action(cuLinkAddFile) \
    action(cuLinkComplete) \
    action(cuLinkDestroy)

#define NVCU_PROCESS_6_0_FUNCTIONS(action) \
    action(cuGetErrorString) \
    action(cuGetErrorName) \
    action(cuMemAllocManaged) \
    action(cuPointerSetAttribute) \
    action(cuStreamAttachMemAsync)

#define NVCU_PROCESS_6_5_FUNCTIONS(action) \
    action(cuOccupancyMaxActiveBlocksPerMultiprocessor) \
    action(cuOccupancyMaxPotentialBlockSize)


#define NVCU_PROCESS_7_0_FUNCTIONS(action) \
    action(cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags) \
    action(cuOccupancyMaxPotentialBlockSizeWithFlags) \
    action(cuDevicePrimaryCtxRetain) \
    action(cuDevicePrimaryCtxRelease) \
    action(cuDevicePrimaryCtxSetFlags) \
    action(cuDevicePrimaryCtxGetState) \
    action(cuDevicePrimaryCtxReset) \
    action(cuCtxGetFlags) \
    action(cuPointerGetAttributes)

#define NVCU_PROCESS_3_0_GL_FUNCTIONS(action) \
    action(cuGraphicsGLRegisterBuffer) \
    action(cuGraphicsGLRegisterImage) \
    action(cuGLInit) \
    action(cuGLRegisterBufferObject) \
    action(cuGLUnmapBufferObject) \
    action(cuGLUnregisterBufferObject) \
    action(cuGLSetBufferObjectMapFlags) \
    action(cuGLUnmapBufferObjectAsync) \
    action(cuGLCtxCreate) \
    action(cuGLMapBufferObject) \
    action(cuGLMapBufferObjectAsync)

#define NVCU_PROCESS_3_0_WGL_FUNCTIONS(action) \
    action(cuWGLGetDevice)

#define NVCU_PROCESS_4_1_GL_FUNCTIONS(action) \
    action(cuGLGetDevices)

#if !defined(__GLEW_H__) && !defined(__glew_h__)
    typedef unsigned int GLenum;
    typedef unsigned int GLuint;
#endif

#include <cuda.h>
#include <cudaGL.h>
#include <cudaw/cudaw_function_types.h>

#define NVCU_DECLARE_POINTER(function) extern PROC_##function FUNC_##function;
#define NVCU_DECLARE_POINTER_VOID(function) extern void* FUNC_##function;

	enum CUVersion
	{
		CU_UNSET = 0,
        CU_3_0 = 3000,
        CU_3_1 = 3010,
        CU_3_2 = 3020,
        CU_4_0 = 4000,
        CU_4_1 = 4010,
        CU_4_2 = 4020,
        CU_5_0 = 5000,
        CU_5_5 = 5050,
        CU_6_0 = 6000,
        CU_6_5 = 6050,
        CU_7_0 = 7000
	};

    NVCU_PROCESS_3_0_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_3_1_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_3_2_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_4_0_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_4_1_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_4_2_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_5_0_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_5_5_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_6_0_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_6_5_FUNCTIONS(NVCU_DECLARE_POINTER)
    NVCU_PROCESS_7_0_FUNCTIONS(NVCU_DECLARE_POINTER)

    NVCU_PROCESS_3_0_GL_FUNCTIONS(NVCU_DECLARE_POINTER)
#ifdef _WIN32
    NVCU_PROCESS_3_0_WGL_FUNCTIONS(NVCU_DECLARE_POINTER)
#endif
    NVCU_PROCESS_4_1_GL_FUNCTIONS(NVCU_DECLARE_POINTER)


#include <cudaw/cudaw_function_defines.h>

    CUresult cudawInit(std::string* oclPathIn = 0, std::string* oclPathOut = 0);

	void cudawRelease();

	bool cudawInitialized();
#if defined(_DEBUG) || !defined(NDEBUG)
    void cudawPrintFunctions();
	void cudawPrintBoundFunctions();
#endif

	CUVersion cudawHighestVersionAvailable();
	CUVersion cudawHighestGLExtensionsAvailable();

	void cudawHighestVersionAvailable(int* major, int* minor);
	void cudawHighestGLExtensionsAvailable(int* major, int* minor);

#endif
