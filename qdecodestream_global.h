#ifndef QDECODESTREAM_GLOBAL_H
#define QDECODESTREAM_GLOBAL_H

#include <QtCore/qglobal.h>

#include "sys/msg.h"
#include "errno.h"

#if defined(QDECODESTREAM_LIBRARY)
#  define QDECODESTREAMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QDECODESTREAMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QDECODESTREAM_GLOBAL_H
