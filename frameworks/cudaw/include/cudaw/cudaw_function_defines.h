/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CUDAW_FUNCTION_DEFINES_H
#define CUDAW_FUNCTION_DEFINES_H

/*--- 3.0 -----------------------------------------------------------*/

#undef cuInit
#undef cuDriverGetVersion
#undef cuDeviceGet
#undef cuDeviceGetCount
#undef cuDeviceGetName
#undef cuDeviceGetAttribute
#undef cuDeviceGetProperties
#undef cuDeviceComputeCapability
#undef cuCtxGetDevice
#undef cuCtxSynchronize
#undef cuCtxAttach
#undef cuCtxDetach
#undef cuModuleLoad
#undef cuModuleLoadData
#undef cuModuleLoadDataEx
#undef cuModuleLoadFatBinary
#undef cuModuleUnload
#undef cuModuleGetFunction
#undef cuModuleGetTexRef
#undef cuMemFreeHost
#undef cuMemHostAlloc
#undef cuMemHostGetFlags
#undef cuArrayDestroy
#undef cuStreamCreate
#undef cuStreamQuery
#undef cuStreamSynchronize
#undef cuEventCreate
#undef cuEventRecord
#undef cuEventQuery
#undef cuEventSynchronize
#undef cuEventElapsedTime
#undef cuFuncGetAttribute
#undef cuFuncSetCacheConfig
#undef cuFuncSetBlockShape
#undef cuFuncSetSharedSize
#undef cuParamSetSize
#undef cuParamSeti
#undef cuParamSetf
#undef cuParamSetv
#undef cuLaunch
#undef cuLaunchGrid
#undef cuLaunchGridAsync
#undef cuParamSetTexRef
#undef cuTexRefSetArray
#undef cuTexRefSetFormat
#undef cuTexRefSetAddressMode
#undef cuTexRefSetFilterMode
#undef cuTexRefSetFlags
#undef cuTexRefGetArray
#undef cuTexRefGetAddressMode
#undef cuTexRefGetFilterMode
#undef cuTexRefGetFormat
#undef cuTexRefGetFlags
#undef cuTexRefCreate
#undef cuTexRefDestroy
#undef cuGraphicsUnregisterResource
#undef cuGraphicsSubResourceGetMappedArray
#undef cuGraphicsResourceSetMapFlags
#undef cuGraphicsMapResources
#undef cuGraphicsUnmapResources
#undef cuGetExportTable
#undef cuDeviceTotalMem
#undef cuCtxCreate
#undef cuModuleGetGlobal
#undef cuMemGetInfo
#undef cuMemAlloc
#undef cuMemAllocPitch
#undef cuMemFree
#undef cuMemGetAddressRange
#undef cuMemAllocHost
#undef cuMemHostGetDevicePointer
#undef cuMemcpyHtoD
#undef cuMemcpyDtoH
#undef cuMemcpyDtoD
#undef cuMemcpyDtoA
#undef cuMemcpyAtoD
#undef cuMemcpyHtoA
#undef cuMemcpyAtoH
#undef cuMemcpyAtoA
#undef cuMemcpy2D
#undef cuMemcpy2DUnaligned
#undef cuMemcpy3D
#undef cuMemcpyHtoDAsync
#undef cuMemcpyDtoHAsync
#undef cuMemcpyDtoDAsync
#undef cuMemcpyHtoAAsync
#undef cuMemcpyAtoHAsync
#undef cuMemcpy2DAsync
#undef cuMemcpy3DAsync
#undef cuMemsetD8
#undef cuMemsetD16
#undef cuMemsetD32
#undef cuMemsetD2D8
#undef cuMemsetD2D16
#undef cuMemsetD2D32
#undef cuArrayCreate
#undef cuArrayGetDescriptor
#undef cuArray3DCreate
#undef cuArray3DGetDescriptor
#undef cuTexRefSetAddress
#undef cuTexRefSetAddress2D
#undef cuTexRefGetAddress
#undef cuGraphicsResourceGetMappedPointer
#undef cuCtxDestroy
#undef cuCtxPushCurrent
#undef cuCtxPopCurrent
#undef cuStreamDestroy
#undef cuEventDestroy


/*--- 3.1 -----------------------------------------------------------*/


#undef cuModuleGetSurfRef
#undef cuSurfRefSetArray
#undef cuSurfRefGetArray
#undef cuCtxSetLimit
#undef cuCtxGetLimit


/*--- 3.2 -----------------------------------------------------------*/


#undef cuStreamWaitEvent
#undef cuCtxGetCacheConfig
#undef cuCtxSetCacheConfig
#undef cuCtxGetApiVersion
#undef cuMemsetD8Async
#undef cuMemsetD16Async
#undef cuMemsetD32Async
#undef cuMemsetD2D8Async
#undef cuMemsetD2D16Async
#undef cuMemsetD2D32Async


/*--- 4.0 -----------------------------------------------------------*/


#undef cuCtxSetCurrent
#undef cuCtxGetCurrent
#undef cuMemHostRegister
#undef cuMemHostUnregister
#undef cuMemcpy
#undef cuMemcpyPeer
#undef cuMemcpy3DPeer
#undef cuMemcpyAsync
#undef cuMemcpyPeerAsync
#undef cuMemcpy3DPeerAsync
#undef cuPointerGetAttribute
#undef cuLaunchKernel
#undef cuDeviceCanAccessPeer
#undef cuCtxEnablePeerAccess
#undef cuCtxDisablePeerAccess


/*--- 4.1 -----------------------------------------------------------*/


#undef cuDeviceGetByPCIBusId
#undef cuDeviceGetPCIBusId
#undef cuIpcGetEventHandle
#undef cuIpcOpenEventHandle
#undef cuIpcGetMemHandle
#undef cuIpcOpenMemHandle
#undef cuIpcCloseMemHandle


/*--- 4.2 -----------------------------------------------------------*/


#undef cuCtxGetSharedMemConfig
#undef cuCtxSetSharedMemConfig
#undef cuFuncSetSharedMemConfig


/*--- 5.0 -----------------------------------------------------------*/


#undef cuTexRefSetMipmappedArray
#undef cuTexRefSetMipmapFilterMode
#undef cuTexRefSetMipmapLevelBias
#undef cuTexRefSetMipmapLevelClamp
#undef cuTexRefSetMaxAnisotropy
#undef cuTexRefGetMipmappedArray
#undef cuTexRefGetMipmapFilterMode
#undef cuTexRefGetMipmapLevelBias
#undef cuTexRefGetMipmapLevelClamp
#undef cuTexRefGetMaxAnisotropy
#undef cuStreamAddCallback
#undef cuMipmappedArrayCreate
#undef cuMipmappedArrayGetLevel
#undef cuMipmappedArrayDestroy
#undef cuTexObjectCreate
#undef cuTexObjectDestroy
#undef cuTexObjectGetResourceDesc
#undef cuTexObjectGetTextureDesc
#undef cuTexObjectGetResourceViewDesc
#undef cuSurfObjectCreate
#undef cuSurfObjectDestroy
#undef cuSurfObjectGetResourceDesc
#undef cuGraphicsResourceGetMappedMipmappedArray


/*--- 5.5 -----------------------------------------------------------*/


#undef cuCtxGetStreamPriorityRange
#undef cuStreamCreateWithPriority
#undef cuStreamGetPriority
#undef cuStreamGetFlags
#undef cuLinkCreate
#undef cuLinkAddData
#undef cuLinkAddFile
#undef cuLinkComplete
#undef cuLinkDestroy


/*--- 6.0 -----------------------------------------------------------*/

#undef cuGetErrorString
#undef cuGetErrorName
#undef cuMemAllocManaged
#undef cuPointerSetAttribute
#undef cuStreamAttachMemAsync


/*--- 6.5 -----------------------------------------------------------*/


#undef cuOccupancyMaxActiveBlocksPerMultiprocessor
#undef cuOccupancyMaxPotentialBlockSize


/*--- 7.0 -----------------------------------------------------------*/


#undef cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags
#undef cuOccupancyMaxPotentialBlockSizeWithFlags
#undef cuDevicePrimaryCtxRetain
#undef cuDevicePrimaryCtxRelease
#undef cuDevicePrimaryCtxSetFlags
#undef cuDevicePrimaryCtxGetState
#undef cuDevicePrimaryCtxReset
#undef cuCtxGetFlags
#undef cuPointerGetAttributes

/*--- GL -----------------------------------------------------------*/

/*--- 3.0 -----------------------------------------------------------*/

#undef cuGraphicsGLRegisterBuffer
#undef cuGraphicsGLRegisterImage

#ifdef _WIN32
#undef cuWGLGetDevice
#endif

#undef cuGLInit
#undef cuGLRegisterBufferObject
#undef cuGLUnmapBufferObject
#undef cuGLUnregisterBufferObject
#undef cuGLSetBufferObjectMapFlags
#undef cuGLUnmapBufferObjectAsync
#undef cuGLCtxCreate
#undef cuGLMapBufferObject
#undef cuGLMapBufferObjectAsync


/*--- 4.1 -----------------------------------------------------------*/


#undef cuGLGetDevices

















/*--- 3.0 -----------------------------------------------------------*/

#define cuInit                              FUNC_cuInit
#define cuDriverGetVersion                  FUNC_cuDriverGetVersion
#define cuDeviceGet                         FUNC_cuDeviceGet
#define cuDeviceGetCount                    FUNC_cuDeviceGetCount
#define cuDeviceGetName                     FUNC_cuDeviceGetName
#define cuDeviceGetAttribute				FUNC_cuDeviceGetAttribute
#define cuDeviceGetProperties				FUNC_cuDeviceGetProperties
#define cuDeviceComputeCapability           FUNC_cuDeviceComputeCapability
#define cuCtxGetDevice                      FUNC_cuCtxGetDevice
#define cuCtxSynchronize                    FUNC_cuCtxSynchronize
#define cuCtxAttach                         FUNC_cuCtxAttach
#define cuCtxDetach                         FUNC_cuCtxDetach
#define cuModuleLoad                        FUNC_cuModuleLoad
#define cuModuleLoadData                    FUNC_cuModuleLoadData
#define cuModuleLoadDataEx                  FUNC_cuModuleLoadDataEx
#define cuModuleLoadFatBinary				FUNC_cuModuleLoadFatBinary
#define cuModuleUnload                      FUNC_cuModuleUnload
#define cuModuleGetFunction                 FUNC_cuModuleGetFunction
#define cuModuleGetTexRef                   FUNC_cuModuleGetTexRef
#define cuMemFreeHost                       FUNC_cuMemFreeHost
#define cuMemHostAlloc                      FUNC_cuMemHostAlloc
#define cuMemHostGetFlags                   FUNC_cuMemHostGetFlags
#define cuArrayDestroy                      FUNC_cuArrayDestroy
#define cuStreamCreate                      FUNC_cuStreamCreate
#define cuStreamQuery                       FUNC_cuStreamQuery
#define cuStreamSynchronize                 FUNC_cuStreamSynchronize
#define cuEventCreate                       FUNC_cuEventCreate
#define cuEventRecord                       FUNC_cuEventRecord
#define cuEventQuery                        FUNC_cuEventQuery
#define cuEventSynchronize                  FUNC_cuEventSynchronize
#define cuEventElapsedTime                  FUNC_cuEventElapsedTime
#define cuFuncGetAttribute                  FUNC_cuFuncGetAttribute
#define cuFuncSetCacheConfig				FUNC_cuFuncSetCacheConfig
#define cuFuncSetBlockShape                 FUNC_cuFuncSetBlockShape
#define cuFuncSetSharedSize                 FUNC_cuFuncSetSharedSize
#define cuParamSetSize                      FUNC_cuParamSetSize
#define cuParamSeti                         FUNC_cuParamSeti
#define cuParamSetf                         FUNC_cuParamSetf
#define cuParamSetv                         FUNC_cuParamSetv
#define cuLaunch                            FUNC_cuLaunch
#define cuLaunchGrid                        FUNC_cuLaunchGrid
#define cuLaunchGridAsync                   FUNC_cuLaunchGridAsync
#define cuParamSetTexRef                    FUNC_cuParamSetTexRef
#define cuTexRefSetArray                    FUNC_cuTexRefSetArray
#define cuTexRefSetFormat                   FUNC_cuTexRefSetFormat
#define cuTexRefSetAddressMode				FUNC_cuTexRefSetAddressMode
#define cuTexRefSetFilterMode				FUNC_cuTexRefSetFilterMode
#define cuTexRefSetFlags                    FUNC_cuTexRefSetFlags
#define cuTexRefGetArray                    FUNC_cuTexRefGetArray
#define cuTexRefGetAddressMode				FUNC_cuTexRefGetAddressMode
#define cuTexRefGetFilterMode				FUNC_cuTexRefGetFilterMode
#define cuTexRefGetFormat                   FUNC_cuTexRefGetFormat
#define cuTexRefGetFlags                    FUNC_cuTexRefGetFlags
#define cuTexRefCreate                      FUNC_cuTexRefCreate
#define cuTexRefDestroy                     FUNC_cuTexRefDestroy
#define cuGraphicsUnregisterResource		FUNC_cuGraphicsUnregisterResource
#define cuGraphicsSubResourceGetMappedArray FUNC_cuGraphicsSubResourceGetMappedArray
#define cuGraphicsResourceSetMapFlags		FUNC_cuGraphicsResourceSetMapFlags
#define cuGraphicsMapResources				FUNC_cuGraphicsMapResources
#define cuGraphicsUnmapResources			FUNC_cuGraphicsUnmapResources
#define cuGetExportTable                    FUNC_cuGetExportTable
#define cuDeviceTotalMem                    FUNC_cuDeviceTotalMem
#define cuCtxCreate                         FUNC_cuCtxCreate
#define cuModuleGetGlobal                   FUNC_cuModuleGetGlobal
#define cuMemGetInfo                        FUNC_cuMemGetInfo
#define cuMemAlloc                          FUNC_cuMemAlloc
#define cuMemAllocPitch                     FUNC_cuMemAllocPitch
#define cuMemFree                           FUNC_cuMemFree
#define cuMemGetAddressRange				FUNC_cuMemGetAddressRange
#define cuMemAllocHost                      FUNC_cuMemAllocHost
#define cuMemHostGetDevicePointer			FUNC_cuMemHostGetDevicePointer
#define cuMemcpyHtoD                        FUNC_cuMemcpyHtoD
#define cuMemcpyDtoH                        FUNC_cuMemcpyDtoH
#define cuMemcpyDtoD                        FUNC_cuMemcpyDtoD
#define cuMemcpyDtoA                        FUNC_cuMemcpyDtoA
#define cuMemcpyAtoD                        FUNC_cuMemcpyAtoD
#define cuMemcpyHtoA                        FUNC_cuMemcpyHtoA
#define cuMemcpyAtoH                        FUNC_cuMemcpyAtoH
#define cuMemcpyAtoA                        FUNC_cuMemcpyAtoA
#define cuMemcpy2D                          FUNC_cuMemcpy2D
#define cuMemcpy2DUnaligned                 FUNC_cuMemcpy2DUnaligned
#define cuMemcpy3D                          FUNC_cuMemcpy3D
#define cuMemcpyHtoDAsync                   FUNC_cuMemcpyHtoDAsync
#define cuMemcpyDtoHAsync                   FUNC_cuMemcpyDtoHAsync
#define cuMemcpyDtoDAsync                   FUNC_cuMemcpyDtoDAsync
#define cuMemcpyHtoAAsync                   FUNC_cuMemcpyHtoAAsync
#define cuMemcpyAtoHAsync                   FUNC_cuMemcpyAtoHAsync
#define cuMemcpy2DAsync                     FUNC_cuMemcpy2DAsync
#define cuMemcpy3DAsync                     FUNC_cuMemcpy3DAsync
#define cuMemsetD8                          FUNC_cuMemsetD8
#define cuMemsetD16                         FUNC_cuMemsetD16
#define cuMemsetD32                         FUNC_cuMemsetD32
#define cuMemsetD2D8                        FUNC_cuMemsetD2D8
#define cuMemsetD2D16                       FUNC_cuMemsetD2D16
#define cuMemsetD2D32                       FUNC_cuMemsetD2D32
#define cuArrayCreate                       FUNC_cuArrayCreate
#define cuArrayGetDescriptor				FUNC_cuArrayGetDescriptor
#define cuArray3DCreate                     FUNC_cuArray3DCreate
#define cuArray3DGetDescriptor				FUNC_cuArray3DGetDescriptor
#define cuTexRefSetAddress                  FUNC_cuTexRefSetAddress
#define cuTexRefSetAddress2D				FUNC_cuTexRefSetAddress2D
#define cuTexRefGetAddress                  FUNC_cuTexRefGetAddress
#define cuGraphicsResourceGetMappedPointer  FUNC_cuGraphicsResourceGetMappedPointer
#define cuCtxDestroy                        FUNC_cuCtxDestroy
#define cuCtxPushCurrent                    FUNC_cuCtxPushCurrent
#define cuCtxPopCurrent                     FUNC_cuCtxPopCurrent
#define cuStreamDestroy                     FUNC_cuStreamDestroy
#define cuEventDestroy                      FUNC_cuEventDestroy


/*--- 3.1 -----------------------------------------------------------*/


#define cuModuleGetSurfRef                  FUNC_cuModuleGetSurfRef
#define cuSurfRefSetArray                   FUNC_cuSurfRefSetArray
#define cuSurfRefGetArray                   FUNC_cuSurfRefGetArray
#define cuCtxSetLimit                       FUNC_cuCtxSetLimit
#define cuCtxGetLimit                       FUNC_cuCtxGetLimit


/*--- 3.2 -----------------------------------------------------------*/


#define cuStreamWaitEvent                   FUNC_cuStreamWaitEvent
#define cuCtxGetCacheConfig                 FUNC_cuCtxGetCacheConfig
#define cuCtxSetCacheConfig                 FUNC_cuCtxSetCacheConfig
#define cuCtxGetApiVersion                  FUNC_cuCtxGetApiVersion
#define cuMemsetD8Async                     FUNC_cuMemsetD8Async
#define cuMemsetD16Async                    FUNC_cuMemsetD16Async
#define cuMemsetD32Async                    FUNC_cuMemsetD32Async
#define cuMemsetD2D8Async                   FUNC_cuMemsetD2D8Async
#define cuMemsetD2D16Async                  FUNC_cuMemsetD2D16Async
#define cuMemsetD2D32Async                  FUNC_cuMemsetD2D32Async


/*--- 4.0 -----------------------------------------------------------*/


#define cuCtxSetCurrent                     FUNC_cuCtxSetCurrent
#define cuCtxGetCurrent                     FUNC_cuCtxGetCurrent
#define cuMemHostRegister                   FUNC_cuMemHostRegister
#define cuMemHostUnregister                 FUNC_cuMemHostUnregister
#define cuMemcpy                            FUNC_cuMemcpy
#define cuMemcpyPeer                        FUNC_cuMemcpyPeer
#define cuMemcpy3DPeer                      FUNC_cuMemcpy3DPeer
#define cuMemcpyAsync                       FUNC_cuMemcpyAsync
#define cuMemcpyPeerAsync                   FUNC_cuMemcpyPeerAsync
#define cuMemcpy3DPeerAsync                 FUNC_cuMemcpy3DPeerAsync
#define cuPointerGetAttribute				FUNC_cuPointerGetAttribute
#define cuLaunchKernel                      FUNC_cuLaunchKernel
#define cuDeviceCanAccessPeer				FUNC_cuDeviceCanAccessPeer
#define cuCtxEnablePeerAccess				FUNC_cuCtxEnablePeerAccess
#define cuCtxDisablePeerAccess				FUNC_cuCtxDisablePeerAccess


/*--- 4.1 -----------------------------------------------------------*/


#define cuDeviceGetByPCIBusId				FUNC_cuDeviceGetByPCIBusId
#define cuDeviceGetPCIBusId                 FUNC_cuDeviceGetPCIBusId
#define cuIpcGetEventHandle                 FUNC_cuIpcGetEventHandle
#define cuIpcOpenEventHandle				FUNC_cuIpcOpenEventHandle
#define cuIpcGetMemHandle                   FUNC_cuIpcGetMemHandle
#define cuIpcOpenMemHandle                  FUNC_cuIpcOpenMemHandle
#define cuIpcCloseMemHandle                 FUNC_cuIpcCloseMemHandle


/*--- 4.2 -----------------------------------------------------------*/


#define cuCtxGetSharedMemConfig				FUNC_cuCtxGetSharedMemConfig
#define cuCtxSetSharedMemConfig				FUNC_cuCtxSetSharedMemConfig
#define cuFuncSetSharedMemConfig			FUNC_cuFuncSetSharedMemConfig


/*--- 5.0 -----------------------------------------------------------*/


#define cuTexRefSetMipmappedArray			FUNC_cuTexRefSetMipmappedArray
#define cuTexRefSetMipmapFilterMode			FUNC_cuTexRefSetMipmapFilterMode
#define cuTexRefSetMipmapLevelBias			FUNC_cuTexRefSetMipmapLevelBias
#define cuTexRefSetMipmapLevelClamp			FUNC_cuTexRefSetMipmapLevelClamp
#define cuTexRefSetMaxAnisotropy			FUNC_cuTexRefSetMaxAnisotropy
#define cuTexRefGetMipmappedArray			FUNC_cuTexRefGetMipmappedArray
#define cuTexRefGetMipmapFilterMode			FUNC_cuTexRefGetMipmapFilterMode
#define cuTexRefGetMipmapLevelBias			FUNC_cuTexRefGetMipmapLevelBias
#define cuTexRefGetMipmapLevelClamp			FUNC_cuTexRefGetMipmapLevelClamp
#define cuTexRefGetMaxAnisotropy			FUNC_cuTexRefGetMaxAnisotropy
#define cuStreamAddCallback                 FUNC_cuStreamAddCallback
#define cuMipmappedArrayCreate				FUNC_cuMipmappedArrayCreate
#define cuMipmappedArrayGetLevel			FUNC_cuMipmappedArrayGetLevel
#define cuMipmappedArrayDestroy				FUNC_cuMipmappedArrayDestroy
#define cuTexObjectCreate                   FUNC_cuTexObjectCreate
#define cuTexObjectDestroy                  FUNC_cuTexObjectDestroy
#define cuTexObjectGetResourceDesc			FUNC_cuTexObjectGetResourceDesc
#define cuTexObjectGetTextureDesc			FUNC_cuTexObjectGetTextureDesc
#define cuTexObjectGetResourceViewDesc		FUNC_cuTexObjectGetResourceViewDesc
#define cuSurfObjectCreate                  FUNC_cuSurfObjectCreate
#define cuSurfObjectDestroy                 FUNC_cuSurfObjectDestroy
#define cuSurfObjectGetResourceDesc			FUNC_cuSurfObjectGetResourceDesc
#define cuGraphicsResourceGetMappedMipmappedArray	FUNC_cuGraphicsResourceGetMappedMipmappedArray


/*--- 5.5 -----------------------------------------------------------*/


#define cuCtxGetStreamPriorityRange			FUNC_cuCtxGetStreamPriorityRange
#define cuStreamCreateWithPriority			FUNC_cuStreamCreateWithPriority
#define cuStreamGetPriority                 FUNC_cuStreamGetPriority
#define cuStreamGetFlags                    FUNC_cuStreamGetFlags
#define cuLinkCreate                        FUNC_cuLinkCreate
#define cuLinkAddData                       FUNC_cuLinkAddData
#define cuLinkAddFile                       FUNC_cuLinkAddFile
#define cuLinkComplete                      FUNC_cuLinkComplete
#define cuLinkDestroy                       FUNC_cuLinkDestroy


/*--- 6.0 -----------------------------------------------------------*/

#define cuGetErrorString                    FUNC_cuGetErrorString
#define cuGetErrorName                      FUNC_cuGetErrorName
#define cuMemAllocManaged                   FUNC_cuMemAllocManaged
#define cuPointerSetAttribute				FUNC_cuPointerSetAttribute
#define cuStreamAttachMemAsync				FUNC_cuStreamAttachMemAsync


/*--- 6.5 -----------------------------------------------------------*/


#define cuOccupancyMaxActiveBlocksPerMultiprocessor FUNC_cuOccupancyMaxActiveBlocksPerMultiprocessor
#define cuOccupancyMaxPotentialBlockSize            FUNC_cuOccupancyMaxPotentialBlockSize


/*--- 7.0 -----------------------------------------------------------*/

#define cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags	FUNC_cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags
#define cuOccupancyMaxPotentialBlockSizeWithFlags				FUNC_cuOccupancyMaxPotentialBlockSizeWithFlags
#define cuDevicePrimaryCtxRetain                    FUNC_cuDevicePrimaryCtxRetain
#define cuDevicePrimaryCtxRelease                   FUNC_cuDevicePrimaryCtxRelease
#define cuDevicePrimaryCtxSetFlags                  FUNC_cuDevicePrimaryCtxSetFlags
#define cuDevicePrimaryCtxGetState                  FUNC_cuDevicePrimaryCtxGetState
#define cuDevicePrimaryCtxReset                     FUNC_cuDevicePrimaryCtxReset
#define cuCtxGetFlags                               FUNC_cuCtxGetFlags
#define cuPointerGetAttributes                      FUNC_cuPointerGetAttributes

/*--- GL -----------------------------------------------------------*/

/*--- 3.0 -----------------------------------------------------------*/

#define cuGraphicsGLRegisterBuffer				FUNC_cuGraphicsGLRegisterBuffer
#define cuGraphicsGLRegisterImage				FUNC_cuGraphicsGLRegisterImage

#ifdef _WIN32
#define cuWGLGetDevice                          FUNC_cuWGLGetDevice
#endif

#define cuGLInit                                FUNC_cuGLInit
#define cuGLRegisterBufferObject				FUNC_cuGLRegisterBufferObject
#define cuGLUnmapBufferObject                   FUNC_cuGLUnmapBufferObject
#define cuGLUnregisterBufferObject				FUNC_cuGLUnregisterBufferObject
#define cuGLSetBufferObjectMapFlags				FUNC_cuGLSetBufferObjectMapFlags
#define cuGLUnmapBufferObjectAsync				FUNC_cuGLUnmapBufferObjectAsync
#define cuGLCtxCreate                           FUNC_cuGLCtxCreate
#define cuGLMapBufferObject                     FUNC_cuGLMapBufferObject
#define cuGLMapBufferObjectAsync				FUNC_cuGLMapBufferObjectAsync


/*--- 4.1 -----------------------------------------------------------*/


#define cuGLGetDevices                          FUNC_cuGLGetDevices

#endif //CUDAW_FUNCTION_DEFINES_H
