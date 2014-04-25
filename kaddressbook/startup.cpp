#include "startup.h"
#include <KLocale>
#include <KGlobal>

namespace KAddressBook {
void insertLibraryCatalogues()
{
    static const char * const catalogs[] = {
        "libkdepim",
        "kabc",
        "libakonadi",
        "kabcakonadi",
        "akonadicontact",
        "libpimcommon"
    };

    KLocale * l = KGlobal::locale();
    for ( unsigned int i = 0 ; i < sizeof catalogs / sizeof *catalogs; ++i ) {
      //QT5
      //l->insertCatalog( QLatin1String(catalogs[i]) );
    }
}

}

