#ifndef RMM_UTILITY_H
#define RMM_UTILITY_H

#include <RMM_Enum.h>

namespace RMM {
    
QCString    toCrLfEol    (const QCString &);
QCString    toLfEol      (const QCString &);
QCString    toCrEol      (const QCString &);

QCString    encodeBase64    (const QByteArray &);
QByteArray  decodeBase64    (const QCString &);

QCString    encodeQuotedPrintable    (const QByteArray &);
QByteArray  decodeQuotedPrintable    (const QCString &);

}
#endif
// vim:ts=4:sw=4:tw=78
