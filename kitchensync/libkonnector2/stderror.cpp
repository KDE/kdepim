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
