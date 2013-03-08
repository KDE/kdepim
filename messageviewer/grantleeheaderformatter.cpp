/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "grantleeheaderformatter.h"

#include <grantlee/templateloader.h>

namespace MessageViewer {

class GrantleeHeaderFormatter::Private
{
public:
    Private( const QString &path )
        : templatePath(path)
    {
        engine = new Grantlee::Engine;
        templateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );
        templateLoader->setTemplateDirs( QStringList() << templatePath );
        templateLoader->setTheme( QLatin1String( "default" ) );
        engine->addTemplateLoader( templateLoader );
        headerTemplate = mEngine->loadByName( "header.html" );
        if ( headerTemplate->error() ) {
          errorMessage += headerTemplate->errorString();
        }

    }
    ~Private()
    {
        delete engine;
    }

    Grantlee::Template headerTemplate;
    QString templatePath;
    QString errorMessage;
    Grantlee::FileSystemTemplateLoader::Ptr templateLoader;
    Grantlee::Engine *engine;
};

GrantleeHeaderFormatter::GrantleeHeaderFormatter(const QString &templatePath)
    : d(new GrantleeHeaderFormatter::Private(templatePath))
{
}

GrantleeHeaderFormatter::~GrantleeHeaderFormatter()
{
    delete d;
}

QString GrantleeHeaderFormatter::toHtml() const
{
    if ( !d->errorMessage.isEmpty() ) {
      return d->errorMessage;
    }
    //TODO
    return QString();
}

}
