#if defined EPOXY_STATIC
  // This macro should be set when _using_ the epoxy, and epoxy is a static lib
  #define EPOXY_EXPORT
  #define EPOXY_NO_EXPORT
#elif defined _WIN32 || defined __CYGWIN__
  // epoxy_EXPORTS should be set when _building_ the epoxy as shared lib
  #ifdef epoxy_EXPORTS
    #define EPOXY_EXPORT __declspec(dllexport)
  #else
    #define EPOXY_EXPORT __declspec(dllimport)
  #endif
  // not needed, symbols are hidden by default
  #define EPOXY_NO_EXPORT
#elif __GNUC__ >= 4
  #define EPOXY_EXPORT __attribute__ ((visibility ("default")))
  #define EPOXY_NO_EXPORT __attribute__ ((visibility ("hidden")))
#else
  #define EPOXY_EXPORT
  #define EPOXY_NO_EXPORT
#endif

#if defined _WIN32 || __CYGWIN__
  #define EPOXY_DEPRECATED __declspec(deprecated)
#elif __GNUC__ >= 4
  #define EPOXY_DEPRECATED __attribute__ ((__deprecated__))
#else
  #define EPOXY_DEPRECATED
#endif

#ifndef EPOXY_DEPRECATED_EXPORT
#  define EPOXY_DEPRECATED_EXPORT EPOXY_EXPORT EPOXY_DEPRECATED
#endif

#ifndef EPOXY_DEPRECATED_NO_EXPORT
#  define EPOXY_DEPRECATED_NO_EXPORT EPOXY_NO_EXPORT EPOXY_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define EPOXY_NO_DEPRECATED
#endif
