#include <klocale.h>

#include "stderror.h"

using namespace KSync;

Error StdError::connectionLost() {
    return Error( Error::ConnectionLost,  i18n("The connections was lost.") );
}
Error StdError::wrongPassword() {
    return Error( Error::WrongPassword,  i18n("The password was wrong.") );
}
Error StdError::authenticationError() {
    return Error( Error::Authentication, i18n("Could not authenticate.") );
}
Error StdError::wrongUser( const QString& user ) {
    return Error( Error::WrongUser,  i18n("The username '%1' was wrong").arg(user) );
}
Error StdError::wrongIP() {
    return Error( Error::WrongIP, i18n("The entered IP address was wrong.") );
}
Error StdError::couldNotConnect() {
    return Error( Error::CouldNotConnect, i18n("Could not connect.") );
}
Error StdError::downloadError(const QString& file) {
    return Error( Error::DownloadError, i18n("Could not download '%1'").arg(file) );
}
Error StdError::uploadError(const QString& file) {
    return Error( Error::UploadError, i18n("Could not upload '%1'").arg(file) );
}
Error StdError::konnectorDoesNotExist( const QString& udi ) {
    return Error( Error::KonnectorNotExist, i18n("The Konnector with the UDI %1 does not exist").arg(udi) );
}
Error StdError::backupNotSupported() {
    return Error( Error::BackupNotSupported, i18n("Backing up is currently not supported.") );
}
Error StdError::restoreNotSupported() {
    return Error( Error::RestoreNotSupported, i18n("Restoring is currently not supported.") );
}
Error StdError::downloadNotSupported() {
    return Error( Error::DownloadNotSupported, i18n("Downloading custom resources is currently not supported.") );
}
