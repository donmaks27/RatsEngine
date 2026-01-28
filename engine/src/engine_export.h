
#ifndef RATS_ENGINE_EXPORT_H
#define RATS_ENGINE_EXPORT_H

#ifdef RATS_ENGINE_STATIC_DEFINE
#  define RATS_ENGINE_EXPORT
#  define RATS_ENGINE_NO_EXPORT
#else
#  ifndef RATS_ENGINE_EXPORT
#    ifdef rats_engine_EXPORTS
        /* We are building this library */
#      define RATS_ENGINE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define RATS_ENGINE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef RATS_ENGINE_NO_EXPORT
#    define RATS_ENGINE_NO_EXPORT 
#  endif
#endif

#ifndef RATS_ENGINE_DEPRECATED
#  define RATS_ENGINE_DEPRECATED __declspec(deprecated)
#endif

#ifndef RATS_ENGINE_DEPRECATED_EXPORT
#  define RATS_ENGINE_DEPRECATED_EXPORT RATS_ENGINE_EXPORT RATS_ENGINE_DEPRECATED
#endif

#ifndef RATS_ENGINE_DEPRECATED_NO_EXPORT
#  define RATS_ENGINE_DEPRECATED_NO_EXPORT RATS_ENGINE_NO_EXPORT RATS_ENGINE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef RATS_ENGINE_NO_DEPRECATED
#    define RATS_ENGINE_NO_DEPRECATED
#  endif
#endif

#endif /* RATS_ENGINE_EXPORT_H */
