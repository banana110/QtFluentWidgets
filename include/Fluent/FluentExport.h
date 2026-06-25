#pragma once

#include <QtGlobal>

#if defined(FLUENT_BUILDING_LIBRARY)
#  define FLUENT_EXPORT Q_DECL_EXPORT
#else
#  define FLUENT_EXPORT Q_DECL_IMPORT
#endif