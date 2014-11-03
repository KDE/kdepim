#include <libkleopatraclient/core/decryptverifyfilescommand.h>

#include "test_util.h"

#include <QApplication>
#include <QMessageBox>

using namespace KleopatraClientCopy;

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    DecryptVerifyFilesCommand cmd;
    cmd.setFilePaths(filePathsFromArgs(argc, argv));

    app.connect(&cmd, SIGNAL(finished()), SLOT(quit()));

    cmd.start();

    int rc = app.exec();

    if (cmd.error() && !cmd.wasCanceled())
        QMessageBox::information(0, QLatin1String("Kleopatra Error"),
                                 QString::fromLatin1("There was an error while connecting to Kleopatra: %1")
                                 .arg(cmd.errorString()));

    return rc;

}
