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

#include "grantleeprintstyle.h"
#include "contactfields.h"
#include "printingwizard.h"
#include "printprogress.h"
#include "printstyle.h"

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include <grantlee/templateloader.h>

#include <KABC/Addressee>

#include <KDebug>
#include <KLocale>

#include <QPrinter>
#include <QTextDocument>

using namespace KABPrinting;

static QString contactsToHtml( const KABC::Addressee::List &contacts )
{
    QString content;
    //TODO
    return content;
}

GrantleePrintStyle::GrantleePrintStyle( PrintingWizard *parent )
    : PrintStyle( parent )
{
    mEngine = new Grantlee::Engine;
    mTemplateLoader = Grantlee::FileSystemTemplateLoader::Ptr( new Grantlee::FileSystemTemplateLoader );
    //setPreview( QLatin1String("") );
    setPreferredSortOptions( ContactFields::FormattedName, Qt::AscendingOrder );
}

GrantleePrintStyle::~GrantleePrintStyle()
{
    delete mEngine;
}

void GrantleePrintStyle::print( const KABC::Addressee::List &contacts, PrintProgress *progress )
{
    QPrinter *printer = wizard()->printer();
    printer->setPageMargins( 20, 20, 20, 20, QPrinter::DevicePixel );

    progress->addMessage( i18n( "Setting up document" ) );

    const QString html = contactsToHtml( contacts );

    QTextDocument document;
    document.setHtml( html );

    progress->addMessage( i18n( "Printing" ) );

    document.print( printer );

    progress->addMessage( i18nc( "Finished printing", "Done" ) );
}

GrantleeStyleFactory::GrantleeStyleFactory( PrintingWizard *parent )
    : PrintStyleFactory( parent )
{
}

PrintStyle *GrantleeStyleFactory::create() const
{
    return new GrantleePrintStyle( mParent );
}

QString GrantleeStyleFactory::description() const
{
    return i18n( "Grantlee Printing Style" );
}
