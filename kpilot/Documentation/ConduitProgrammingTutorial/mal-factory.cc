/* Time-factory.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file defines the factory for the MAL-conduit plugin.
*/
 
#include "options.h" 

#include <kapplication.h>
#include <kinstance.h>
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


// A number of static variables; except for fAbout, they're 
// all KConfig group or entry keys.
//
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

    fInstance = new KInstance("MALconduit");
    fAbout = new KAboutData("MALconduit",
        I18N_NOOP("MAL Syncronization Conduit for KPilot"),
        KPILOT_VERSION,
        I18N_NOOP("Syncronizes the content from MAL Servers like AvantGo to the Handheld"),
        KAboutData::License_GPL,
        "(C) 2002, Reinhold Kainhofer");
    fAbout->addAuthor("Reinhold Kainhofer",
        I18N_NOOP("Primary Author"), "reinhold@kainhofer.com", "http://reinhold.kainhofer.com/");
    fAbout->addAuthor("Jason Day",
        I18N_NOOP("Author of libmal and the JPilot AvantGo conduit"), "jasonday@worldnet.att.net");
    fAbout->addAuthor("Tom Whittaker",
        I18N_NOOP("Author of syncmal"), "tom@tomw.org", "http://www.tomw.org/");
    fAbout->addAuthor("AvantGo, Inc.",
        I18N_NOOP("Authors of the malsync library (c) 1997-1999"), "www.avantgo.com", "http://www.avantgo.com/");
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
    DEBUGCONDUIT << fname
        << ": Creating object of class "
        << c
        << endl;
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
            kdError() << k_funcinfo
                << ": Couldn't cast parent to widget."
                << endl;
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
            kdError() << k_funcinfo
                << ": Couldn't cast parent to KPilotDeviceLink"
                << endl;
            return 0L;
        }
    }

    return 0L;
}

