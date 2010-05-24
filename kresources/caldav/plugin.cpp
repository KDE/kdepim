/*=========================================================================
| KCalDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
| (c) 2009  Kumaran Santhanam (initial KDE4 version)
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| CalDAV resource factory.
 ========================================================================*/

/*=========================================================================
| INCLUDES
 ========================================================================*/

#include "resource.h"
#include "config.h"
#include "export.h"

#include <kglobal.h>
#include <klocale.h>

/*=========================================================================
| NAMESPACE
 ========================================================================*/

using namespace KCal;

/*=========================================================================
| CLASS
 ========================================================================*/

// Creates the resource factory.
//EXPORT_KRESOURCES_PLUGIN2( ResourceCalDav, ResourceCalDavConfig, "libkcal", "kres_caldav" )

typedef KRES::PluginFactory<ResourceCalDav, ResourceCalDavConfig> CalDavFactory;

extern "C"
{
  void *init_kcal_caldav()
  {
    KGlobal::locale()->insertCatalogue( "libkcal" );
    KGlobal::locale()->insertCatalogue( "kres_caldav" );
    return new CalDavFactory;
  }
}

// EOF ========================================================================
