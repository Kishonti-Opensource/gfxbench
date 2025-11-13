/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CUDAW_FUNCTION_TYPES_H
#define CUDAW_FUNCTION_TYPES_H

#include <cuda.h>


/*--- 3.0 -----------------------------------------------------------*/

typedef CUresult (CUDAAPI *PROC_cuInit)(unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuDriverGetVersion)(int *driverVersion);
typedef CUresult (CUDAAPI *PROC_cuDeviceGet)(CUdevice *device, int ordinal);
typedef CUresult (CUDAAPI *PROC_cuDeviceGetCount)(int *count);
typedef CUresult (CUDAAPI *PROC_cuDeviceGetName)(char *name, int len, CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuDeviceGetAttribute)(int *pi, CUdevice_attribute attrib, CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuDeviceGetProperties)(CUdevprop *prop, CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuDeviceComputeCapability)(int *major, int *minor, CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuCtxGetDevice)(CUdevice *device);
typedef CUresult (CUDAAPI *PROC_cuCtxSynchronize)(void);
typedef CUresult (CUDAAPI *PROC_cuCtxAttach)(CUcontext *pctx, unsigned int flags);
typedef CUresult (CUDAAPI *PROC_cuCtxDetach)(CUcontext ctx);
typedef CUresult (CUDAAPI *PROC_cuModuleLoad)(CUmodule *module, const char *fname);
typedef CUresult (CUDAAPI *PROC_cuModuleLoadData)(CUmodule *module, const void *image);
typedef CUresult (CUDAAPI *PROC_cuModuleLoadDataEx)(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues);
typedef CUresult (CUDAAPI *PROC_cuModuleLoadFatBinary)(CUmodule *module, const void *fatCubin);
typedef CUresult (CUDAAPI *PROC_cuModuleUnload)(CUmodule hmod);
typedef CUresult (CUDAAPI *PROC_cuModuleGetFunction)(CUfunction *hfunc, CUmodule hmod, const char *name);
typedef CUresult (CUDAAPI *PROC_cuModuleGetTexRef)(CUtexref *pTexRef, CUmodule hmod, const char *name);
typedef CUresult (CUDAAPI *PROC_cuMemFreeHost)(void *p);
typedef CUresult (CUDAAPI *PROC_cuMemHostAlloc)(void **pp, size_t bytesize, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuMemHostGetFlags)(unsigned int *pFlags, void *p);
typedef CUresult (CUDAAPI *PROC_cuArrayDestroy)(CUarray hArray);
typedef CUresult (CUDAAPI *PROC_cuStreamCreate)(CUstream *phStream, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuStreamQuery)(CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuStreamSynchronize)(CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuEventCreate)(CUevent *phEvent, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuEventRecord)(CUevent hEvent, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuEventQuery)(CUevent hEvent);
typedef CUresult (CUDAAPI *PROC_cuEventSynchronize)(CUevent hEvent);
typedef CUresult (CUDAAPI *PROC_cuEventElapsedTime)(float *pMilliseconds, CUevent hStart, CUevent hEnd);
typedef CUresult (CUDAAPI *PROC_cuFuncGetAttribute)(int *pi, CUfunction_attribute attrib, CUfunction hfunc);
typedef CUresult (CUDAAPI *PROC_cuFuncSetCacheConfig)(CUfunction hfunc, CUfunc_cache config);
typedef CUresult (CUDAAPI *PROC_cuFuncSetBlockShape)(CUfunction hfunc, int x, int y, int z);
typedef CUresult (CUDAAPI *PROC_cuFuncSetSharedSize)(CUfunction hfunc, unsigned int bytes);
typedef CUresult (CUDAAPI *PROC_cuParamSetSize)(CUfunction hfunc, unsigned int numbytes);
typedef CUresult (CUDAAPI *PROC_cuParamSeti)(CUfunction hfunc, int offset, unsigned int value);
typedef CUresult (CUDAAPI *PROC_cuParamSetf)(CUfunction hfunc, int offset, float value);
typedef CUresult (CUDAAPI *PROC_cuParamSetv)(CUfunction hfunc, int offset, void *ptr, unsigned int numbytes);
typedef CUresult (CUDAAPI *PROC_cuLaunch)(CUfunction f);
typedef CUresult (CUDAAPI *PROC_cuLaunchGrid)(CUfunction f, int grid_width, int grid_height);
typedef CUresult (CUDAAPI *PROC_cuLaunchGridAsync)(CUfunction f, int grid_width, int grid_height, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuParamSetTexRef)(CUfunction hfunc, int texunit, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetArray)(CUtexref hTexRef, CUarray hArray, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetFormat)(CUtexref hTexRef, CUarray_format fmt, int NumPackedComponents);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetAddressMode)(CUtexref hTexRef, int dim, CUaddress_mode am);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetFilterMode)(CUtexref hTexRef, CUfilter_mode fm);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetFlags)(CUtexref hTexRef, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetArray)(CUarray *phArray, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetAddressMode)(CUaddress_mode *pam, CUtexref hTexRef, int dim);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetFilterMode)(CUfilter_mode *pfm, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetFormat)(CUarray_format *pFormat, int *pNumChannels, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetFlags)(unsigned int *pFlags, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefCreate)(CUtexref *pTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefDestroy)(CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuGraphicsUnregisterResource)(CUgraphicsResource resource);
typedef CUresult (CUDAAPI *PROC_cuGraphicsSubResourceGetMappedArray)(CUarray *pArray, CUgraphicsResource resource, unsigned int arrayIndex, unsigned int mipLevel);
typedef CUresult (CUDAAPI *PROC_cuGraphicsResourceSetMapFlags)(CUgraphicsResource resource, unsigned int flags);
typedef CUresult (CUDAAPI *PROC_cuGraphicsMapResources)(unsigned int count, CUgraphicsResource *resources, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuGraphicsUnmapResources)(unsigned int count, CUgraphicsResource *resources, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuGetExportTable)(const void **ppExportTable, const CUuuid *pExportTableId);
typedef CUresult (CUDAAPI *PROC_cuDeviceTotalMem)(size_t *bytes, CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuCtxCreate)(CUcontext *pctx, unsigned int flags, CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuModuleGetGlobal)(CUdeviceptr *dptr, size_t *bytes, CUmodule hmod, const char *name);
typedef CUresult (CUDAAPI *PROC_cuMemGetInfo)(size_t *free, size_t *total);
typedef CUresult (CUDAAPI *PROC_cuMemAlloc)(CUdeviceptr *dptr, size_t bytesize);
typedef CUresult (CUDAAPI *PROC_cuMemAllocPitch)(CUdeviceptr *dptr, size_t *pPitch, size_t WidthInBytes, size_t Height, unsigned int ElementSizeBytes);
typedef CUresult (CUDAAPI *PROC_cuMemFree)(CUdeviceptr dptr);
typedef CUresult (CUDAAPI *PROC_cuMemGetAddressRange)(CUdeviceptr *pbase, size_t *psize, CUdeviceptr dptr);
typedef CUresult (CUDAAPI *PROC_cuMemAllocHost)(void **pp, size_t bytesize);
typedef CUresult (CUDAAPI *PROC_cuMemHostGetDevicePointer)(CUdeviceptr *pdptr, void *p, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuMemcpyHtoD)(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpyDtoH)(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpyDtoD)(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpyDtoA)(CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpyAtoD)(CUdeviceptr dstDevice, CUarray srcArray, size_t srcOffset, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpyHtoA)(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpyAtoH)(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpyAtoA)(CUarray dstArray, size_t dstOffset, CUarray srcArray, size_t srcOffset, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpy2D)(const CUDA_MEMCPY2D *pCopy);
typedef CUresult (CUDAAPI *PROC_cuMemcpy2DUnaligned)(const CUDA_MEMCPY2D *pCopy);
typedef CUresult (CUDAAPI *PROC_cuMemcpy3D)(const CUDA_MEMCPY3D *pCopy);
typedef CUresult (CUDAAPI *PROC_cuMemcpyHtoDAsync)(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemcpyDtoHAsync)(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemcpyDtoDAsync)(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemcpyHtoAAsync)(CUarray dstArray, size_t dstOffset, const void *srcHost, size_t ByteCount, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemcpyAtoHAsync)(void *dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemcpy2DAsync)(const CUDA_MEMCPY2D *pCopy, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemcpy3DAsync)(const CUDA_MEMCPY3D *pCopy, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemsetD8)(CUdeviceptr dstDevice, unsigned char uc, size_t N);
typedef CUresult (CUDAAPI *PROC_cuMemsetD16)(CUdeviceptr dstDevice, unsigned short us, size_t N);
typedef CUresult (CUDAAPI *PROC_cuMemsetD32)(CUdeviceptr dstDevice, unsigned int ui, size_t N);
typedef CUresult (CUDAAPI *PROC_cuMemsetD2D8)(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height);
typedef CUresult (CUDAAPI *PROC_cuMemsetD2D16)(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height);
typedef CUresult (CUDAAPI *PROC_cuMemsetD2D32)(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height);
typedef CUresult (CUDAAPI *PROC_cuArrayCreate)(CUarray *pHandle, const CUDA_ARRAY_DESCRIPTOR *pAllocateArray);
typedef CUresult (CUDAAPI *PROC_cuArrayGetDescriptor)(CUDA_ARRAY_DESCRIPTOR *pArrayDescriptor, CUarray hArray);
typedef CUresult (CUDAAPI *PROC_cuArray3DCreate)(CUarray *pHandle, const CUDA_ARRAY3D_DESCRIPTOR *pAllocateArray);
typedef CUresult (CUDAAPI *PROC_cuArray3DGetDescriptor)(CUDA_ARRAY3D_DESCRIPTOR *pArrayDescriptor, CUarray hArray);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetAddress)(size_t *ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, size_t bytes);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetAddress2D)(CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR *desc, CUdeviceptr dptr, size_t Pitch);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetAddress)(CUdeviceptr *pdptr, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuGraphicsResourceGetMappedPointer)(CUdeviceptr *pDevPtr, size_t *pSize, CUgraphicsResource resource);
typedef CUresult (CUDAAPI *PROC_cuCtxDestroy)(CUcontext ctx);
typedef CUresult (CUDAAPI *PROC_cuCtxPushCurrent)(CUcontext ctx);
typedef CUresult (CUDAAPI *PROC_cuCtxPopCurrent)(CUcontext *pctx);
typedef CUresult (CUDAAPI *PROC_cuStreamDestroy)(CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuEventDestroy)(CUevent hEvent);


/*--- 3.1 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuModuleGetSurfRef)(CUsurfref *pSurfRef, CUmodule hmod, const char *name);
typedef CUresult (CUDAAPI *PROC_cuSurfRefSetArray)(CUsurfref hSurfRef, CUarray hArray, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuSurfRefGetArray)(CUarray *phArray, CUsurfref hSurfRef);
typedef CUresult (CUDAAPI *PROC_cuCtxSetLimit)(CUlimit limit, size_t value);
typedef CUresult (CUDAAPI *PROC_cuCtxGetLimit)(size_t *pvalue, CUlimit limit);


/*--- 3.2 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuStreamWaitEvent)(CUstream hStream, CUevent hEvent, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuCtxGetCacheConfig)(CUfunc_cache *pconfig);
typedef CUresult (CUDAAPI *PROC_cuCtxSetCacheConfig)(CUfunc_cache config);
typedef CUresult (CUDAAPI *PROC_cuCtxGetApiVersion)(CUcontext ctx, unsigned int *version);
typedef CUresult (CUDAAPI *PROC_cuMemsetD8Async)(CUdeviceptr dstDevice, unsigned char uc, size_t N, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemsetD16Async)(CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemsetD32Async)(CUdeviceptr dstDevice, unsigned int ui, size_t N, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemsetD2D8Async)(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemsetD2D16Async)(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemsetD2D32Async)(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height, CUstream hStream);


/*--- 4.0 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuCtxSetCurrent)(CUcontext ctx);
typedef CUresult (CUDAAPI *PROC_cuCtxGetCurrent)(CUcontext *pctx);
typedef CUresult (CUDAAPI *PROC_cuMemHostRegister)(void *p, size_t bytesize, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuMemHostUnregister)(void *p);
typedef CUresult (CUDAAPI *PROC_cuMemcpy)(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpyPeer)(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount);
typedef CUresult (CUDAAPI *PROC_cuMemcpy3DPeer)(const CUDA_MEMCPY3D_PEER *pCopy);
typedef CUresult (CUDAAPI *PROC_cuMemcpyAsync)(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemcpyPeerAsync)(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuMemcpy3DPeerAsync)(const CUDA_MEMCPY3D_PEER *pCopy, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuPointerGetAttribute)(void *data, CUpointer_attribute attribute, CUdeviceptr ptr);
typedef CUresult (CUDAAPI *PROC_cuLaunchKernel)(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void **kernelParams, void **extra);
typedef CUresult (CUDAAPI *PROC_cuDeviceCanAccessPeer)(int *canAccessPeer, CUdevice dev, CUdevice peerDev);
typedef CUresult (CUDAAPI *PROC_cuCtxEnablePeerAccess)(CUcontext peerContext, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuCtxDisablePeerAccess)(CUcontext peerContext);


/*--- 4.1 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuDeviceGetByPCIBusId)(CUdevice *dev, const char *pciBusId);
typedef CUresult (CUDAAPI *PROC_cuDeviceGetPCIBusId)(char *pciBusId, int len, CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuIpcGetEventHandle)(CUipcEventHandle *pHandle, CUevent event);
typedef CUresult (CUDAAPI *PROC_cuIpcOpenEventHandle)(CUevent *phEvent, CUipcEventHandle handle);
typedef CUresult (CUDAAPI *PROC_cuIpcGetMemHandle)(CUipcMemHandle *pHandle, CUdeviceptr dptr);
typedef CUresult (CUDAAPI *PROC_cuIpcOpenMemHandle)(CUdeviceptr *pdptr, CUipcMemHandle handle, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuIpcCloseMemHandle)(CUdeviceptr dptr);


/*--- 4.2 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuCtxGetSharedMemConfig)(CUsharedconfig *pConfig);
typedef CUresult (CUDAAPI *PROC_cuCtxSetSharedMemConfig)(CUsharedconfig config);
typedef CUresult (CUDAAPI *PROC_cuFuncSetSharedMemConfig)(CUfunction hfunc, CUsharedconfig config);


/*--- 5.0 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuTexRefSetMipmappedArray)(CUtexref hTexRef, CUmipmappedArray hMipmappedArray, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetMipmapFilterMode)(CUtexref hTexRef, CUfilter_mode fm);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetMipmapLevelBias)(CUtexref hTexRef, float bias);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetMipmapLevelClamp)(CUtexref hTexRef, float minMipmapLevelClamp, float maxMipmapLevelClamp);
typedef CUresult (CUDAAPI *PROC_cuTexRefSetMaxAnisotropy)(CUtexref hTexRef, unsigned int maxAniso);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetMipmappedArray)(CUmipmappedArray *phMipmappedArray, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetMipmapFilterMode)(CUfilter_mode *pfm, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetMipmapLevelBias)(float *pbias, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetMipmapLevelClamp)(float *pminMipmapLevelClamp, float *pmaxMipmapLevelClamp, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuTexRefGetMaxAnisotropy)(int *pmaxAniso, CUtexref hTexRef);
typedef CUresult (CUDAAPI *PROC_cuStreamAddCallback)(CUstream hStream, CUstreamCallback callback, void *userData, unsigned int flags);
typedef CUresult (CUDAAPI *PROC_cuMipmappedArrayCreate)(CUmipmappedArray *pHandle, const CUDA_ARRAY3D_DESCRIPTOR *pMipmappedArrayDesc, unsigned int numMipmapLevels);
typedef CUresult (CUDAAPI *PROC_cuMipmappedArrayGetLevel)(CUarray *pLevelArray, CUmipmappedArray hMipmappedArray, unsigned int level);
typedef CUresult (CUDAAPI *PROC_cuMipmappedArrayDestroy)(CUmipmappedArray hMipmappedArray);
typedef CUresult (CUDAAPI *PROC_cuTexObjectCreate)(CUtexObject *pTexObject, const CUDA_RESOURCE_DESC *pResDesc, const CUDA_TEXTURE_DESC *pTexDesc, const CUDA_RESOURCE_VIEW_DESC *pResViewDesc);
typedef CUresult (CUDAAPI *PROC_cuTexObjectDestroy)(CUtexObject texObject);
typedef CUresult (CUDAAPI *PROC_cuTexObjectGetResourceDesc)(CUDA_RESOURCE_DESC *pResDesc, CUtexObject texObject);
typedef CUresult (CUDAAPI *PROC_cuTexObjectGetTextureDesc)(CUDA_TEXTURE_DESC *pTexDesc, CUtexObject texObject);
typedef CUresult (CUDAAPI *PROC_cuTexObjectGetResourceViewDesc)(CUDA_RESOURCE_VIEW_DESC *pResViewDesc, CUtexObject texObject);
typedef CUresult (CUDAAPI *PROC_cuSurfObjectCreate)(CUsurfObject *pSurfObject, const CUDA_RESOURCE_DESC *pResDesc);
typedef CUresult (CUDAAPI *PROC_cuSurfObjectDestroy)(CUsurfObject surfObject);
typedef CUresult (CUDAAPI *PROC_cuSurfObjectGetResourceDesc)(CUDA_RESOURCE_DESC *pResDesc, CUsurfObject surfObject);
typedef CUresult (CUDAAPI *PROC_cuGraphicsResourceGetMappedMipmappedArray)(CUmipmappedArray *pMipmappedArray, CUgraphicsResource resource);


/*--- 5.5 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuCtxGetStreamPriorityRange)(int *leastPriority, int *greatestPriority);
typedef CUresult (CUDAAPI *PROC_cuStreamCreateWithPriority)(CUstream *phStream, unsigned int flags, int priority);
typedef CUresult (CUDAAPI *PROC_cuStreamGetPriority)(CUstream hStream, int *priority);
typedef CUresult (CUDAAPI *PROC_cuStreamGetFlags)(CUstream hStream, unsigned int *flags);
typedef CUresult (CUDAAPI *PROC_cuLinkCreate)(unsigned int numOptions, CUjit_option *options, void **optionValues, CUlinkState *stateOut);
typedef CUresult (CUDAAPI *PROC_cuLinkAddData)(CUlinkState state, CUjitInputType type, void *data, size_t size, const char *name, unsigned int numOptions, CUjit_option *options, void **optionValues);
typedef CUresult (CUDAAPI *PROC_cuLinkAddFile)(CUlinkState state, CUjitInputType type, const char *path, unsigned int numOptions, CUjit_option *options, void **optionValues);
typedef CUresult (CUDAAPI *PROC_cuLinkComplete)(CUlinkState state, void **cubinOut, size_t *sizeOut);
typedef CUresult (CUDAAPI *PROC_cuLinkDestroy)(CUlinkState state);


/*--- 6.0 -----------------------------------------------------------*/

typedef CUresult (CUDAAPI *PROC_cuGetErrorString)(CUresult error, const char **pStr);
typedef CUresult (CUDAAPI *PROC_cuGetErrorName)(CUresult error, const char **pStr);
typedef CUresult (CUDAAPI *PROC_cuMemAllocManaged)(CUdeviceptr *dptr, size_t bytesize, unsigned int flags);
typedef CUresult (CUDAAPI *PROC_cuPointerSetAttribute)(const void *value, CUpointer_attribute attribute, CUdeviceptr ptr);
typedef CUresult (CUDAAPI *PROC_cuStreamAttachMemAsync)(CUstream hStream, CUdeviceptr dptr, size_t length, unsigned int flags);


/*--- 6.5 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuOccupancyMaxActiveBlocksPerMultiprocessor)(int *numBlocks, CUfunction func, int blockSize, size_t dynamicSMemSize);
typedef CUresult (CUDAAPI *PROC_cuOccupancyMaxPotentialBlockSize)(int *minGridSize, int *blockSize, CUfunction func, CUoccupancyB2DSize blockSizeToDynamicSMemSize, size_t dynamicSMemSize, int blockSizeLimit);


/*--- 7.0 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags)(int *numBlocks, CUfunction func, int blockSize, size_t dynamicSMemSize, unsigned int flags);
typedef CUresult (CUDAAPI *PROC_cuOccupancyMaxPotentialBlockSizeWithFlags)(int *minGridSize, int *blockSize, CUfunction func, CUoccupancyB2DSize blockSizeToDynamicSMemSize, size_t dynamicSMemSize, int blockSizeLimit, unsigned int flags);
typedef CUresult (CUDAAPI *PROC_cuDevicePrimaryCtxRetain)(CUcontext *pctx, CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuDevicePrimaryCtxRelease)(CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuDevicePrimaryCtxSetFlags)(CUdevice dev, unsigned int flags);
typedef CUresult (CUDAAPI *PROC_cuDevicePrimaryCtxGetState)(CUdevice dev, unsigned int *flags, int *active);
typedef CUresult (CUDAAPI *PROC_cuDevicePrimaryCtxReset)(CUdevice dev);
typedef CUresult (CUDAAPI *PROC_cuCtxGetFlags)(unsigned int *flags);
typedef CUresult (CUDAAPI *PROC_cuPointerGetAttributes)(unsigned int numAttributes, CUpointer_attribute *attributes, void **data, CUdeviceptr ptr);

/*--- GL ------------------------------------------------------------*/

//#include <cudaGL.h>

/*--- 3.0 -----------------------------------------------------------*/

typedef CUresult (CUDAAPI *PROC_cuGraphicsGLRegisterBuffer)(CUgraphicsResource *pCudaResource, GLuint buffer, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuGraphicsGLRegisterImage)(CUgraphicsResource *pCudaResource, GLuint image, GLenum target, unsigned int Flags);

#ifdef _WIN32
typedef CUresult (CUDAAPI *PROC_cuWGLGetDevice)(CUdevice *pDevice, HGPUNV hGpu);
#endif

typedef CUresult (CUDAAPI *PROC_cuGLInit)(void);
typedef CUresult (CUDAAPI *PROC_cuGLRegisterBufferObject)(GLuint buffer);
typedef CUresult (CUDAAPI *PROC_cuGLUnmapBufferObject)(GLuint buffer);
typedef CUresult (CUDAAPI *PROC_cuGLUnregisterBufferObject)(GLuint buffer);
typedef CUresult (CUDAAPI *PROC_cuGLSetBufferObjectMapFlags)(GLuint buffer, unsigned int Flags);
typedef CUresult (CUDAAPI *PROC_cuGLUnmapBufferObjectAsync)(GLuint buffer, CUstream hStream);
typedef CUresult (CUDAAPI *PROC_cuGLCtxCreate)(CUcontext *pCtx, unsigned int Flags, CUdevice device );
typedef CUresult (CUDAAPI *PROC_cuGLMapBufferObject)(CUdeviceptr *dptr, size_t *size,  GLuint buffer);
typedef CUresult (CUDAAPI *PROC_cuGLMapBufferObjectAsync)(CUdeviceptr *dptr, size_t *size,  GLuint buffer, CUstream hStream);


/*--- 4.1 -----------------------------------------------------------*/


typedef CUresult (CUDAAPI *PROC_cuGLGetDevices)(unsigned int *pCudaDeviceCount, CUdevice *pCudaDevices, unsigned int cudaDeviceCount, CUGLDeviceList deviceList);


#endif //CUDAW_FUNCTION_TYPES_H