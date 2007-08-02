/* Time-factory.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the MAL-conduit plugin.
*/
 
#include "options.h" 

#include <kcomponentdata.h>
#include <kaboutdata.h>

#include <time.h> // Needed by pilot-link include
#include "mal-conduit.h"
#include "mal-setup.h"

#include "mal-factory.moc"


extern "C"
{

void *init_libmalconduit()
{
    return new MALConduitFactory;
}

} ;


// A number of static variables
KAboutData *MALConduitFactory::fAbout = 0L;
const char *MALConduitFactory::fGroup = "MAL-conduit";
const char *MALConduitFactory::fLastSync = "Last MAL Sync";
const char *MALConduitFactory::fSyncTime = "Sync Frequency";
const char *MALConduitFactory::fProxyType = "Proxy Type";
const char *MALConduitFactory::fProxyServer = "Proxy Server";
const char *MALConduitFactory::fProxyPort = "Proxy Port";
const char *MALConduitFactory::fProxyUser = "Proxy User";
const char *MALConduitFactory::fProxyPassword = "Proxy Password";

MALConduitFactory::MALConduitFactory(QObject *p, const char *n) :
    KLibFactory(p,n)
{
    FUNCTIONSETUP;

    fInstance("MALconduit");
    fAbout = new KAboutData("MALconduit", 0,
        ki18n("MAL Synchronization Conduit for KPilot"),
        KPILOT_VERSION,
        ki18n("Synchronizes the content from MAL Servers like AvantGo to the Handheld"),
        KAboutData::License_GPL,
        ki18n("(C) 2002, Reinhold Kainhofer"));
    fAbout->addAuthor(ki18n("Reinhold Kainhofer"),
        ki18n("Primary Author"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/");
    fAbout->addAuthor(ki18n("Jason Day"),
        ki18n("Author of libmal and the JPilot AvantGo conduit"), "jasonday@worldnet.att.net");
    fAbout->addAuthor(ki18n("Tom Whittaker"),
        ki18n("Author of syncmal"), "tom@tomw.org", "http://www.tomw.org/");
    fAbout->addAuthor(ki18n("AvantGo, Inc."),
        ki18n("Authors of the malsync library (c) 1997-1999"), "www.avantgo.com", "http://www.avantgo.com/");
}

MALConduitFactory::~MALConduitFactory()
{
    FUNCTIONSETUP;

    KPILOT_DELETE(fInstance);
    KPILOT_DELETE(fAbout);
}

/* virtual */ QObject *MALConduitFactory::createObject( QObject *p,
    const char *n,
    const char *c,
    const QStringList &a)
{
    FUNCTIONSETUP;

#ifdef DEBUG
    DEBUGKPILOT << fname
        << ": Creating object of class "
        << c;
#endif

    if (qstrcmp(c,"ConduitConfig")==0)
    {
        QWidget *w = dynamic_cast<QWidget *>(p);

        if (w)
        {
            return new MALWidgetSetup(w,n,a);
        }
        else 
        {
            kError() << k_funcinfo
                << ": Couldn't cast parent to widget.";
            return 0L;
        }
    }

    if (qstrcmp(c,"SyncAction")==0)
    { 
        KPilotDeviceLink *d = dynamic_cast<KPilotDeviceLink *>(p);

        if (d)
        {
            return new MALConduit(d,n,a);
        }
        else
        {
            kError() << k_funcinfo
                << ": Couldn't cast parent to KPilotDeviceLink";
            return 0L;
        }
    }

    return 0L;
}

