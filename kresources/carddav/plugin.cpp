/*=========================================================================
| KCardDAV
|--------------------------------------------------------------------------
| (c) 2010  Timothy Pearson
|
| This project is released under the GNU General Public License.
| Please see the file COPYING for more details.
|--------------------------------------------------------------------------
| CardDAV resource factory.
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

using namespace KABC;

/*=========================================================================
| CLASS
 ========================================================================*/

// Creates the resource factory.

typedef KRES::PluginFactory<ResourceCardDav, ResourceCardDavConfig> CardDavFactory;

extern "C"
{
  void *init_kabc_carddav()
  {
    KGlobal::locale()->insertCatalogue( "kdepimresources" );
    KGlobal::locale()->insertCatalogue( "kres_caldav" );
    return new CardDavFactory;
  }
}

// EOF ========================================================================
