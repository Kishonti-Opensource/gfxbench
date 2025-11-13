%module(directors=1) BenchmarkServiceGlobal

#pragma SWIG nowarn=516

%include <enums.swg>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_vector.i>
%include <windows.i>

%{
#include "../testfw/graphics/graphicscontext.h"
#include "../testfw/schemas/apidefinition.h"
#include "../testfw/schemas/configuration.h"
#include "../testfw/schemas/compute.h"
#include "../testfw/schemas/graphics.h"
#include "../testfw/schemas/environment.h"
#include "../testfw/schemas/descriptors.h"
#include "../testfw/schemas/chart.h"
#include "../testfw/schemas/computeresult.h"
#include "../testfw/schemas/gfxresult.h"
#include "../testfw/schemas/result.h"
#include "../testfw/schemas/testinfo.h"
#include "../testfw/schemas/testitem.h"
#include "../testfw/schemas/session.h"
#include "../testfw/deviceinfo/runtimeinfo.h"
#include "../testfw/deviceinfo/linuxruntimeinfo.h"
#include "benchmarkservice.h"
#include "cursor.h"
%}

#if defined(SWIGJAVA)

    %pragma(java) jniclasscode=%{
        static {
            // update benchmarkservice.i interface file with native dependencies, and regenerate swig
            System.loadLibrary("c++_shared");
            System.loadLibrary("benchmarkservice_d");
        }
    %}
#endif


namespace tfw {
    class Serializable
    {
    public:
        virtual std::string toString() const = 0;
    };
}
#define BENCHMARK_SERVICE_EXPORT
#define BENCHMARK_SERVICE_API


typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef long long uint64_t;
typedef long long size_t;

// Do not create interfaces for ng::JsonValue type
// Serializable objects in schemas
%ignore *::toJsonValue() const;
%ignore *::fromJsonValue(const ng::JsonValue&);

namespace std {
  %template(StringVector)           vector<std::string>;
  %template(BoolVector)             vector<bool>;
  %template(DoubleVector)           vector<double>;
  %template(IntVector)              vector<int>;
  %template(ResultVector)           vector<tfw::Result>;
  %template(ApiDefinitionVector)    vector<tfw::ApiDefinition>;
  %template(ChartVector)            vector<tfw::Chart>;
  %template(ConfigurationVector)    vector<tfw::Configuration>;
  %template(DescriptorVector)       vector<tfw::Descriptor>;
  %template(SessionVector)          vector<tfw::Session>;
  %template(TestItemVector)         vector<tfw::TestItem>;
  %template(ResultInfoVector)       vector<tfw::ResultInfo>;
  %template(IntVectorVector)        vector<vector<int> >;
  %template(SamplesVector)          vector<tfw::Samples>;
}

%shared_ptr(BenchmarkService)
%ignore BenchmarkService::release;
%ignore BenchmarkService::destroy;

%feature("director") CursorCallback;
%feature("director") BenchmarkServiceCallback;

%shared_ptr(Cursor)
%ignore Cursor::getCursorPointer;
%ignore Cursor::getBlobPointer;
%ignore Cursor::release;
%ignore Cursor::destroy;


#if defined(SWIGJAVA)

    %exception {
        try {
            $action
        } catch (const std::exception& e) {
            jenv->ThrowNew(jenv->FindClass("java/lang/RuntimeException"), e.what());
            return $null;
        }
    }
    %javaconst(1);
    %typemap(javabase) Cursor "android.database.AbstractCursor";
    %javamethodmodifiers Cursor::moveToPosition "protected";
    %rename(moveToPositionImpl) Cursor::moveToPosition;
    %ignore Cursor::getPosition;
    %javamethodmodifiers Cursor::setCallback "protected";

    %typemap(jtype) std::string Cursor::getBlob "byte[]"
    %typemap(jstype) std::string Cursor::getBlob "byte[]"
    %typemap(jni) std::string Cursor::getBlob "jbyteArray"
    %typemap(out) std::string Cursor::getBlob {
      size_t size;
      const char* pointer;
      arg1->getBlobPointer(arg2, &pointer, &size);
      $result = JCALL1(NewByteArray, jenv, size);
      JCALL4(SetByteArrayRegion, jenv, $result, 0, size, reinterpret_cast<const jbyte*>(pointer));
      return $result;
    }

    %typemap(javacode) Cursor %{
        private class Callback extends CursorCallback {
            public Callback() { super(); }

            @Override
            public void dataSetWillChange(int first, int last) {}
            @Override
            public void dataSetChanged(int first, int last) { onChange(false); }
            @Override
            public void dataSetWillBeInvalidated() {}
            @Override
            public void dataSetInvalidated() { onChange(false); }
        }

        @Override
        public String[] getColumnNames() {
            int n = getCount();
            String[] result = new String[n];
            for (int i = 0; i < n; ++i) {
                result[i] = getColumnName(i);
            }
            return result;
        }

        @Override
        public void close() {
            setCallback(null);
            super.close();
        }

        @Override
        public boolean onMove(int oldPosition, int newPosition) {
            moveToPositionImpl(newPosition);
            return true;
        }
    %}

    %typemap(javafinalize) Cursor %{
        protected void finalize() {
            try {
                setCallback(null);
                delete();
            } finally {
                super.finalize();
            }
        }
    %}
#elif defined(SWIGCSHARP)
    %exception {
        try {
            $action
        } catch (const std::exception& e) {
            SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.what());
        }
    }
#endif

%include "../testfw/graphics/graphicscontext.h"
%include "../testfw/schemas/apidefinition.h"
%include "../testfw/schemas/configuration.h"
%include "../testfw/schemas/compute.h"
%include "../testfw/schemas/graphics.h"
%include "../testfw/schemas/environment.h"
%include "../testfw/schemas/descriptors.h"
%include "../testfw/schemas/chart.h"
%include "../testfw/schemas/computeresult.h"
%include "../testfw/schemas/gfxresult.h"
%include "../testfw/schemas/result.h"
%include "../testfw/schemas/testinfo.h"
%include "../testfw/schemas/testitem.h"
%include "../testfw/schemas/session.h"
%include "../testfw/deviceinfo/runtimeinfo.h"
%include "../testfw/deviceinfo/linuxruntimeinfo.h"
%include "src/benchmarkservice.h"
%include "src/cursor.h"
#if defined(SWIGJAVA)
    %include "../testfw/deviceinfo/linuxruntimeinfo.h"
#endif


