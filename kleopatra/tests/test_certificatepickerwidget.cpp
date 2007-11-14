
#include <QApplication>

#include "uiserver/certificatepickerwidget.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    Kleo::CertificatePickerWidget picker;
    QStringList list;
    list << "Frank Osterfeld" << "Kleopatra" << "Christian" << "KDE";
    picker.setIdentifiers( list );
    picker.show();
    return app.exec();
}

