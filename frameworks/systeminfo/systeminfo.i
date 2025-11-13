%ignore sysinf::Properties::set(std::string const &,long);
%ignore sysinf::Properties::set(std::string const &,long long);
%ignore sysinf::Properties::set(std::string const &,unsigned long);

%module(directors=1) systeminfolib
%{
#include "properties.h"

#ifndef SWIGCSHARP
#include "glinfo.h"
#include "deviceinfocollector.h"
#include "vulkaninfocollector.h"
#include "clinfo.h"
#include "cudainfo.h"
#include "metalinfo.h"
#ifndef SWIGJAVA
#include "directxinfo.h"
#endif
#endif

#include "vulkaninfo.h"
#include "dataformatter.h"
#include "SystemInfoCommonKeys.h"
#include "deviceinfo.h"
#include "systeminfo.h"
%}
#ifndef SWIGCSHARP
%feature("director") sysinf::DeviceInfoCollector;
#endif

%{
using sysinf::PropertyFeature;
%}


#if SWIG_VERSION >= 0x040000
%extend std::map {
%proxycode %{
// Old API, used when swig 4 run
public boolean empty() {
	return isEmpty();
}
public void set($typemap(jboxtype, K) key, $typemap(jboxtype, T) x) {
	put(key, x);
}
public void del($typemap(jboxtype, K) key) {
	remove(key);
}
public boolean has_key($typemap(jboxtype, K) key) {
	return containsKey(key);
}
%}
}
#endif


%pragma(java) jniclasscode=%{
	static {
		if (!android.os.Build.BRAND.toLowerCase().contains("vega"))
		{
			try { System.load("/system/lib/libOpenCL.so"); } catch (UnsatisfiedLinkError e) {
				try { System.load("/vendor/lib/libOpenCL.so"); } catch (UnsatisfiedLinkError f) {
					try { System.load("/system/vendor/lib/libOpenCL.so"); } catch (UnsatisfiedLinkError g) {
						try { System.load("/vendor/lib/egl/libGLES_mali.so"); } catch (UnsatisfiedLinkError h) {
							try { System.load("/system/vendor/lib/egl/libGLES_mali.so"); } catch (UnsatisfiedLinkError i) {}
						}
					}
				}
			}
		}
		System.loadLibrary("c++_shared");
		System.loadLibrary("systeminfo_jni");
	}
%}

SWIG_JAVABODY_PROXY(public, public, SWIGTYPE)
SWIG_JAVABODY_TYPEWRAPPER(public, public, public, SWIGTYPE)

%include "std_string.i"
%include "stdint.i"

%include "std_vector.i"
%include "std_map.i"
%include "std_pair.i"

%include "src/properties.h";
#ifndef SWIGCSHARP
%include "src/glinfo.h"
%include "src/vulkaninfo.h"
%include "src/clinfo.h"
%include "src/cudainfo.h"
%include "src/metalinfo.h"
#ifndef SWIGJAVA
%include "src/directxinfo.h"
#endif
#endif

%include "src/dataformatter.h";
%include "src/FormattedDeviceInfo.h";
%include "src/SystemInfoCommonKeys.h";
%include "src/deviceinfo.h"
%include "src/systeminfo.h"

#ifndef SWIGCSHARP
%include "src/deviceinfocollector.h"
%include "src/vulkaninfocollector.h"
#endif



%template(FormattedDeviceInfoVector) std::vector<sysinf::FormattedDeviceInfo>;
%template(PropertyFeatureVector) std::vector<sysinf::PropertyFeature>;
%template(ApiPlatformVector) std::vector<sysinf::ApiPlatform>;
%template(ApiDeviceVector) std::vector<sysinf::ApiDevice>;
%template(ApiInfoVector) std::vector<sysinf::ApiInfo>;
%template(StringVector) std::vector<std::string>;
%template(StringStringMap) std::map<std::string, std::string>;
%template(StringBoolMap) std::map<std::string, bool>;
%template(StringUnsignedLongLongMap) std::map<std::string, unsigned long long>;
%template(StringStringVectorMap) std::map<std::string, std::vector<std::string>>;

%template(StringIntPair) std::pair<std::string, int>;
%template(StringLongPair) std::pair<std::string, long>;
%template(StringLongLongPair) std::pair<std::string, long long>;
%template(StringFloatPair) std::pair<std::string, float>;
%template(StringBoolPair) std::pair<std::string, bool>;
%template(StringUnsignedIntPair) std::pair<std::string, unsigned int>;

%template(StringIntPairVector) std::vector<std::pair<std::string, int>>;
%template(StringLongLongPairVector) std::vector<std::pair<std::string, long long>>;
%template(StringFloatPairVector) std::vector<std::pair<std::string, float>>;
%template(StringBoolPairVector) std::vector<std::pair<std::string, bool>>;
%template(StringUnsignedIntPairVector) std::vector<std::pair<std::string, unsigned int>>;

%template(DisplayInfoVector) std::vector<sysinf::DisplayInfo>;
%template(CpuInfoVector) std::vector<sysinf::CpuInfo>;
%template(GpuInfoVector) std::vector<sysinf::GpuInfo>;
%template(StorageInfoVector) std::vector<sysinf::StorageInfo>;
%template(BatteryInfoVector) std::vector<sysinf::BatteryInfo>;
%template(CameraInfoVector) std::vector<sysinf::CameraInfo>;

#ifndef SWIGCSHARP
%template(CLPlatformInfoVector) std::vector<sysinf::CLPlatformInfo>;
%template(CLDeviceInfoVector) std::vector<sysinf::CLDeviceInfo>;
%template(VulkanDeviceInfoVector) std::vector<sysinf::VulkanDeviceInfo>;
%template(EGLConfigInfoVector) std::vector<sysinf::EGLConfigInfo>;
%template(MetalDeviceInfoVector) std::vector<sysinf::MetalDeviceInfo>;
%template(CLImageFormatVector) std::vector<sysinf::CLImageFormat>;
#ifndef SWIGJAVA
%template(DirectxDeviceInfoVector) std::vector<sysinf::DirectxDeviceInfo>;
#endif
%template(CudaDeviceInfoVector) std::vector<sysinf::CudaDeviceInfo>;
#endif
