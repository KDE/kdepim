
#include "converterbase.h"

using namespace Vr3;


ConverterBase::ConverterBase( KSync::KonnectorUIDHelper* helper,
                              const QString& zone ) {
    m_tz = zone;
    m_helper = helper;
}
ConverterBase::~ConverterBase() {
}
time_t ConverterBase::toUTC( const QDateTime& ) {
    time_t t;
    return t;
}
QDateTime ConverterBase::fromUTC( time_t ) {
    return QDateTime::currentDateTime(); // fixme
}
KTempFile* ConverterBase::file() {
    return 0l;
}
QString ConverterBase::timeZone() const{
    return m_tz;
}
QString ConverterBase::konnectorId( const QString& appName, const QString& uid ) {
    return QString::null;
}
QString ConverterBase::kdeId( const QString& appName, const QString& uid ) {
    return QString::null;
}
