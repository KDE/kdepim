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
#include <QDir>
#include <QDebug>

#include <grantlee/context.h>
#include <grantlee/engine.h>
#include "grantlee_paths.h"

MailWindow::MailWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // widgets in which is embedded a theme
    splitter = new QSplitter( Qt::Vertical,this );
    webView = new QWebView( splitter );

    // The start to create and configure template objects
    engine = new Grantlee::Engine();

    // allows to load templates from the file system
    Grantlee::FileSystemTemplateLoader::Ptr loader(new Grantlee::FileSystemTemplateLoader());

    // add the TemplateLoader object to the engine
    engine->addTemplateLoader( loader );

    // set the directories where the templates will be located.
    loader->setTemplateDirs( QStringList() << GRANTLEE_TEMPLATE_PATH << (QDir::currentPath() + "/images/"));

    // set the templates directory to find template tags and filters
    engine->setPluginPaths( QStringList() << GRANTLEE_PLUGIN_PATH );

    // create the combo and insert some items.
    label = new QLabel( "Theme" );
    comboThemes = new QComboBox();
    comboThemes->addItem( "kde" );
    comboThemes->addItem( "nokia" );
    comboThemes->addItem( "opensuse" );
    comboThemes->addItem( "konsole" );

    // load a new theme when the combo current item changed.
    connect( comboThemes , SIGNAL( activated( const QString )),
                this, SLOT( renderMail( const QString & )));

    // theme loaded by default
    renderMail( "kde" );

    setCentralWidget( splitter );
}

void MailWindow::renderMail( const QString &themeName )
{
    // load the template according to his themeName
    Grantlee::Template t = engine->loadByName( themeName + ".html" );

    QDateTime dateTime = QDateTime::currentDateTime();

    // mail message
    QString message;
    message += "I've created the branch branches/work/soc-pim &quot;Kmail &amp; Co&quot; ";
    message += "The branch is intended to be shared by all PIM-related GSoC projects,";
    message += "Happy Summer of Code to all the students.\n";

    QVariantList attachments;
    attachments << "screenshot.png" << "screenshot2.png" ;

    // Insert all the data that will be called on themes.
    QVariantHash data;
    data.insert( "subject" , " [Kde-pim] Work branches for GSoC " );
    data.insert( "from" , " PIM Hacker <pimhacker@kde.org> " );
    data.insert( "to" , " KDE PIM <kde-pim@kde.org> " );
    data.insert( "date" , dateTime );
    data.insert( "message" , message );
    data.insert( "attachments" , attachments );

    // holds the context (data) to render in the template
    Grantlee::Context c( data );
    c.setRelativeMediaPath("images/");
    QString content = t->render( &c );

    // qDebug() << content;

    webView->setHtml( content , QUrl::fromLocalFile(QDir::currentPath()) );
    splitter->addWidget( label );
    splitter->addWidget( comboThemes );
    splitter->addWidget( webView );
}

MailWindow::~MailWindow()
{

}

#include "mailwindow.moc"
