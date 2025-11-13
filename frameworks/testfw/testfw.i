%module(directors=1) testfw

#pragma SWIG nowarn=516

%{
#include "testfw.h"
#include "graphics/graphicscontext.h"
#include "tfwmessagequeue.h"
#include "schemas/serializable.h"
#include "schemas/apidefinition.h"
#include "schemas/chart.h"
#include "schemas/gfxresult.h"
#include "schemas/computeresult.h"
#include "schemas/result.h"
#include "schemas/descriptors.h"
#include "schemas/compositedescriptor.h"
#include "schemas/compareitem.h"
#include "schemas/compareresult.h"
#include "schemas/configuration.h"
#include "schemas/compute.h"
#include "schemas/graphics.h"
#include "schemas/environment.h"
#include "schemas/jsonutils.h"
#include "schemas/resultitem.h"
#include "schemas/session.h"
#include "schemas/testinfo.h"
#include "schemas/testitem.h"
#include "schemas/testrepository.h"
%}

#ifndef SWIGCSHARP
%{
    #include "videostream.h"
    #include "deviceinfo/runtimeinfo.h"
    #include "deviceinfo/nullruntimeinfo.h"
    #include "deviceinfo/linuxruntimeinfo.h"
    #include "deviceinfo/clinfo.h"
    #include "deviceinfo/glinfo.h"
    #include "deviceinfo/cudainfo.h"

#ifdef WITH_OVR_SDK
    #include "ovr/Include/VrApi.h"
    #include "ovr/Include/VrApi_Config.h"
    #include "ovr/Include/VrApi_Helpers.h"
    #include "ovr/Include/VrApi_LocalPrefs.h"
    #include "ovr/Include/VrApi_Types.h"
    #include "ovr/Include/VrApi_Version.h"
#endif

%}
#endif

#ifdef SWIGJAVA
%{
#ifdef ANDROID
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include "graphics/vulkangraphicscontext.h"
#include "graphics/AndroidVulkanGraphicsContext.h"
#include "graphics/eglgraphicscontext.h"
#include "graphics/ovrgraphicscontext.h"
#include <stdexcept>
#include "ScopedLocalRef.h"
#endif // ANDROID

JavaVM *cached_jvm = NULL;

using namespace tfw;

#include <iostream>

JNIEnv * JNU_GetEnv() {
    JNIEnv *env = 0;
    jint rc = cached_jvm->GetEnv((void **)&env, JNI_VERSION_1_2);
    if (rc == JNI_EDETACHED)
    {
        JavaVMAttachArgs attach_args = { JNI_VERSION_1_2, NULL, NULL };
#ifdef ANDROID
        rc = cached_jvm->AttachCurrentThread(&env, &attach_args);
#else // ANDROID
        rc = cached_jvm->AttachCurrentThread((void**)&env, &attach_args);
#endif // ANDROID
        if (rc != 0)
        {
            throw std::runtime_error("jni attach thread failed");
        }
    }
    if (rc == JNI_EVERSION)
    {
        std::cerr << "error 2" << std::endl;
        throw std::runtime_error("jni version not supported");
    }
    return env;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *) {
	cached_jvm = jvm;
	return JNI_VERSION_1_2;
}

JavaVM * JNU_GetJVM() {
    return cached_jvm;
}



%}

%pragma(java) jniclasscode=%{
        static {
                // update testfw.i interface file with native dependencies, and regenerate swig
                System.loadLibrary("c++_shared");
                System.loadLibrary("testfw_jni");
        }
%}

%exception {
    try {
        $action
    } catch (const std::exception &e) {
        jenv->ThrowNew(jenv->FindClass("java/lang/RuntimeException"), e.what());
        return $null;
    }
}

%typemap(javainterfaces) tfw::TestBase "net.kishonti.testfw.JTestInterface";
%typemap(javacode) tfw::TestBase %{
// this method is required for TestBase to implement JTestInterface
public void setContext(android.content.Context context) {
    // ignore context for native testfw
}
%}

%javaexception("java.text.ParseException") tfw::TestRepository::loadTestsFromJsonString {
    try {
        $action
    } catch (const std::exception &e) {
        ScopedLocalRef<jclass> jclaz(jenv, jenv->FindClass("java/text/ParseException"));
        jmethodID jconsructor = jenv->GetMethodID(jclaz.get(), "<init>", "(Ljava/lang/String;I)V");
        ScopedLocalRef<jstring> jmsg(jenv, jenv->NewStringUTF(e.what()));
        ScopedLocalRef<jthrowable> jex(jenv, (jthrowable)jenv->NewObject(jclaz.get(), jconsructor, jmsg.get(), 0));
        jenv->Throw(jex.get());
        return $null;
    }
}

#endif // SWIGJAVA

typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef long long uint64_t;
typedef long long size_t;
%include "std_string.i"
%include "std_vector.i"

%newobject TestFactory::create_test;
%include "testfw.h";

#ifdef SWIGJAVA
%typemap(javaimports) AndroidVulkanGraphicsContext %{
import android.view.Surface;
%}
%typemap(javaimports) EGLGraphicsContext %{
import android.view.Surface;
%}
%typemap(javaimports) OVRGraphicsContext %{
import android.view.Surface;
%}

%typemap(jni) ANativeWindow* window "jobject";
%typemap(jtype) ANativeWindow* window "Object";
%typemap(jstype) ANativeWindow* window "Surface";
%typemap(javain) ANativeWindow* window "$javainput"
%typemap(in) ANativeWindow* {
    $1 = ANativeWindow_fromSurface(jenv, $input);
}


%typemap(jni) EGLNativeWindowType window "jobject";
%typemap(jtype) EGLNativeWindowType window "Object";
%typemap(jstype) EGLNativeWindowType window "Surface";
%typemap(javain) EGLNativeWindowType window "$javainput"
%typemap(in) EGLNativeWindowType {
	$1 = ANativeWindow_fromSurface(jenv, $input);
}
#endif
%include "graphics/graphicscontext.h"
#ifdef HAVE_EGL
%include "graphics/glformat.h"
%include "graphics/vulkangraphicscontext.h"
%include "graphics/AndroidVulkanGraphicsContext.h"
%include "graphics/eglgraphicscontext.h"
%include "graphics/ovrgraphicscontext.h"
#endif
%include "messagequeue.h"
%include "tfwmessagequeue.h"

// Do not create interfaces for ng::JsonValue type
// Serializable objects in schemas
%ignore *::toJsonValue() const;
%ignore *::fromJsonValue(const ng::JsonValue&);
// schemas/jsonutils.h
%ignore tfw::JsonUtils::parseString(const ng::JsonValue&, const std::string&, const char*);
%ignore tfw::JsonUtils::parseString(const ng::JsonValue&, const std::string&);
%ignore tfw::JsonUtils::parseStringVector(const ng::JsonValue&, const std::string&);
%ignore tfw::JsonUtils::parseNumber(const ng::JsonValue&, const std::string&, double);
%ignore tfw::JsonUtils::parseNumber(const ng::JsonValue&, const std::string&);
%ignore tfw::JsonUtils::parseNumberVector(const ng::JsonValue&, const std::string&);
%ignore tfw::JsonUtils::parseBool(const ng::JsonValue&, const std::string&, bool);
%ignore tfw::JsonUtils::parseBool(const ng::JsonValue&, const std::string&);
%ignore tfw::JsonUtils::parseBoolVector(const ng::JsonValue&, const std::string&);

// schemas/serializable.h fromJsonString/fromJsonFile error string pointer variants
%ignore tfw::Serializable::fromJsonString(const std::string&, std::string*);
%ignore tfw::Serializable::fromJsonFile(const std::string&, std::string*);

%include "schemas/serializable.h"
%include "schemas/apidefinition.h"
%include "schemas/testinfo.h"
%include "schemas/computeresult.h"
%include "schemas/gfxresult.h"
%include "schemas/chart.h"
%include "schemas/compute.h"
%include "schemas/graphics.h"
%include "schemas/configuration.h"
%include "schemas/environment.h"
%include "schemas/descriptors.h"
%include "schemas/result.h"
%include "schemas/compositedescriptor.h"

%include "schemas/jsonutils.h"
%include "schemas/compareresult.h"
%include "schemas/session.h"

%include "schemas/resultitem.h"
%include "schemas/compareitem.h"
%include "schemas/testitem.h"
%include "schemas/testrepository.h"

#ifndef SWIGCSHARP
    %include "carrays.i"
    %array_class(float, FloatArray)

    %feature("director") VideoStream;
    %include "videostream.h"

	%feature("director") tfw::RuntimeInfo;
	%include "deviceinfo/runtimeinfo.h"
	%feature("director") tfw::NullRuntimeInfo;
	%include "deviceinfo/nullruntimeinfo.h"
	%feature("director") tfw::LinuxRuntimeInfo;
	%include "deviceinfo/linuxruntimeinfo.h"

    %immutable; // make these structs read-only in java
    %include "deviceinfo/clinfo.h"
    %ignore tfw::GLESInfo::aliasedLineWidthRange;
    %ignore tfw::GLESInfo::aliasedPointSizeRange;
    %ignore tfw::GLESInfo::maxComputeWorkGroupCount;
    %ignore tfw::GLESInfo::maxComputeWorkGroupSize;
    %ignore tfw::GLESInfo::maxViewportDims;
    %ignore tfw::GLESInfo::maxi;
    %include "deviceinfo/glinfo.h"
    %ignore tfw::CudaDeviceInfo::attributes;
    %include "deviceinfo/cudainfo.h"
    %mutable;
    %extend tfw::GLESInfo {
        // provide convenient accessors for C array members
        int getAliasedLineWidthRange(int i) { return $self->aliasedLineWidthRange[i]; }
        int getAliasedPointSizeRange(int i) { return $self->aliasedPointSizeRange[i]; }
        int getMaxComputeWorkGroupCount(int i) { return $self->maxComputeWorkGroupCount[i]; }
        int getMaxComputeWorkGroupSize(int i) { return $self->maxComputeWorkGroupSize[i]; }
        int getViewportDims(int i) { return $self->maxViewportDims[i]; }
        int getNumberOfMaxProperties() { return $self->maxi.size(); }
        std::string getMaxPropertyName(int i) { return $self->maxi[i].first; }
        int64_t getMaxPropertyValue(int i) { return $self->maxi[i].second; }
    }
    %extend tfw::CudaDeviceInfo {
        int getNumberOfAttributes() { return $self->attributes.size(); }
        std::string getAttributeName(int i) { return $self->attributes[i].first; }
        int64_t getAttributeValue(int i) { return $self->attributes[i].second; }
    }

    %ignore tfw::CompositeDescriptor::descriptors() const;
    %extend tfw::CompositeDescriptor {
        int size() { return $self->descriptors().size(); }
        Descriptor descriptor(int i) { return $self->descriptors()[i]; }
    }
    namespace std {
        %template(EGLConfigInfoVector)      vector<tfw::EGLConfigInfo>;
        %template(CLDeviceInfoVector)      vector<tfw::CLDeviceInfo>;
    }
#endif

namespace std {
	%template(StringVector)				vector<std::string>;
	%template(BoolVector) 				vector<bool>;
	%template(DoubleVector) 			vector<double>;
	%template(IntVector)	 			vector<int>;
	%template(ResultVector) 			vector<tfw::Result>;
	%template(ApiDefinitionVector)		vector<tfw::ApiDefinition>;
	%template(IntVectorVector)			vector<vector<int> >;
	%template(SamplesVector)			vector<tfw::Samples>;
	%template(ChartVector)				vector<tfw::Chart>;
	%template(CompareItemVector)		vector<tfw::CompareItem>;
	%template(CompareResultVector)		vector<tfw::CompareResult>;
	%template(ConfigurationVector)		vector<tfw::Configuration>;
	%template(DescriptorVector)			vector<tfw::Descriptor>;
	%template(ResultGroupVector)		vector<tfw::ResultGroup>;
	%template(ResultInfoVector)			vector<tfw::ResultInfo>;
	%template(ResultItemVector)			vector<tfw::ResultItem>;
	%template(ResultItemVectorVector)	vector<vector<tfw::ResultItem> >;
	%template(SessionVector)			vector<tfw::Session>;
	%template(TestItemVector)			vector<tfw::TestItem>;
	%template(TestItemVectorVector)		vector<vector<tfw::TestItem> >;
	%template(UnsignedVector)			vector<unsigned>;
}

namespace tfw {
	%copyctor TestItem;
	%copyctor ResultItem;
	%template(setRawConfigString)   Descriptor::setRawConfig<std::string>;
	%template(setRawConfigDouble)   Descriptor::setRawConfig<double>;
	%template(setRawConfigBool)     Descriptor::setRawConfig<bool>;
}
