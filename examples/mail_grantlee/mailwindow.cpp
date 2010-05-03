/*
  Example of theming using Grantlee.

  Copyright (c) 2010 Ronny Yabar Aizcorbe <ronnycontacto@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 3 only, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License version 3 for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "mailwindow.h"

#include <QDateTime>

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include "grantlee_paths.h"

MailWindow::MailWindow(QWidget *parent)
    : QMainWindow(parent)
{
    splitter = new QSplitter(Qt::Horizontal,this);
    webView = new QWebView( splitter );

    engine = new Grantlee::Engine();
    Grantlee::FileSystemTemplateLoader::Ptr loader(new Grantlee::FileSystemTemplateLoader());

    loader->setTemplateDirs( QStringList() << GRANTLEE_TEMPLATE_PATH );

    engine->addTemplateLoader( loader );
    engine->setPluginPaths( QStringList() << GRANTLEE_PLUGIN_PATH );

    renderMail();
    setCentralWidget( splitter );
}

void MailWindow::renderMail()
{
    QString themeName = "kde.html";
    Grantlee::Template t = engine->loadByName( themeName );

    QString message;
    message += "Hello KDE and PIM community, let us have fun.\n";
    message += "Happy Summer of Code to all the students.\n";

    QDateTime dateTime = QDateTime::currentDateTime();

    QVariantHash data;
    data.insert( "subject" , " [Kde-pim] Work branches for GSoC " );
    data.insert( "from" , " PIM Hacker <pimhacker@kde.org> " );
    data.insert( "to" , " KDE PIM <kde-pim@kde.org> " );
    data.insert( "date" , dateTime );
    data.insert( "message" , message);

    Grantlee::Context c( data );
    QString content = t->render(&c);

    webView->setHtml( content );
    splitter->addWidget( webView );
}

MailWindow::~MailWindow()
{

}

#include "mailwindow.moc"
