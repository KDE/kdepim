/**
   KDE Certificate Manager
   
   by Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se> and Jesper
   K. Pedersen <blackie@klaralvdalens-datakonsult.se>
   
   Copyright (C) 2002 by Klarälvdalens Datakonsult AB

   This software is licensed under the GPL.
*/

#include <kapplication.h>
#include "certmanager.h"
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <cryptplugwrapper.h>

CryptPlugWrapper* pWrapper;

int main( int argc, char** argv )
{
    KCmdLineArgs::init(argc, argv, "Certificate Manager","","");
    static KCmdLineOptions options[] =
        {
            { "+name", I18N_NOOP("The name of the plugin"), 0 },
            { "+lib" , I18N_NOOP("The library of the plugin"), 0 },
            { "external" , I18N_NOOP("Search for external certificates initially"), 0 },
            { "query " , I18N_NOOP("Initial query string"), 0 },
            { 0, 0, 0 } // End of options.
        };
    KCmdLineArgs::addCmdLineOptions( options );
          
    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if( args->count() < 2 ) {
        KMessageBox::error( 0,
                            i18n( "<qt>Certificate Manager called incorrectly.<br>Usage: certmanager <em>plugin-name</em> <em>plugin-lib</em><br>Certificate Manager will terminate now.</qt>" ),
                            i18n( "Certificate Manager Error" ) );
        return -1;
    }
  
    QString pluginName = QString::fromLocal8Bit( args->arg( 0 ) );
    QString pluginLib = QString::fromLocal8Bit( args->arg( 1 ) );
  
    pWrapper = new CryptPlugWrapper( 0, pluginName, pluginLib,
                                     QString::null, true );
    CryptPlugWrapper::InitStatus initStatus;
    QString errorText;
    if( !pWrapper->initialize( &initStatus, &errorText ) ) {
        KMessageBox::error( 0,
                            i18n( "<qt>The crypto plugin could not be initialized; the error message was %1.<br>Certificate Manager will terminate now.</qt>" ).arg( errorText ),
                            i18n( "Certificate Manager Error" ) );
        return -2;
    }
    CertManager* manager = new CertManager( args->isSet("external"), 
					    QString::fromLocal8Bit(args->getOption("query")) );
    args->clear();
    manager->show();   
  
    QObject::connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
    int ret = app.exec();
    delete pWrapper;
        
    return ret;
}
