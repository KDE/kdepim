#include <klocale.h>

#include "stdprogress.h"

using namespace KSync;

Progress StdProgress::connection() {
    return Progress( Progress::Connection, i18n("A connection was opened.") );
}
Progress StdProgress::connected() {
    return Progress( Progress::Connected, i18n("A connection was established.") );
}
Progress StdProgress::authenticated() {
    return Progress( Progress::Authenticated,  i18n("Successfully authenticated.") );
}
Progress StdProgress::syncing(const QString& str) {
    return Progress( Progress::Syncing, i18n("Currently synchronizing %1").arg(str) );
}
Progress StdProgress::downloading(const QString& str) {
    return Progress( Progress::Downloading, i18n("Currently downloading %1").arg(str) );
}
Progress StdProgress::uploading(const QString& str) {
    return Progress( Progress::Uploading, i18n("Currently uploading %1").arg(str) );
}
Progress StdProgress::converting(const QString& str) {
    return Progress( Progress::Converting,  i18n("Converting %1 to native format").arg(str) );
}
Progress StdProgress::reconverting( const QString& str) {
    return Progress( Progress::Reconverting, i18n("Reconverting %1 to remote format").arg(str) );
}
Progress StdProgress::done() {
    return Progress( Progress::Done, i18n("Done.") );
}
