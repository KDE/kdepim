/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "filterimporterexportergui.h"
#include "mailcommon/filter/filterimporterexporter.h"

#include <qapplication.h>
#include <QVBoxLayout>
#include <QMenu>
#include <mailfilter.h>
#include <qmenubar.h>
Q_DECLARE_METATYPE(MailCommon::FilterImporterExporter::FilterType)
FilterImporterExporterGui::FilterImporterExporterGui(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QMenuBar *menuBar = new QMenuBar(this);
    mainLayout->addWidget(menuBar);
    QMenu *menu = new QMenu(this);
    QMenu *menuFilter = menuBar->addMenu(QLatin1String("filter"));
    menuFilter->addMenu(menu);
    QAction *act = new QAction( QLatin1String( "KMail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::KMailFilter) );
    menu->addAction( act );

    act = new QAction( QLatin1String( "Thunderbird filters" ), this );

    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::ThunderBirdFilter) );
    menu->addAction( act );

    act = new QAction( QLatin1String( "Evolution filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::EvolutionFilter) );
    menu->addAction( act );

    act = new QAction( QLatin1String( "Sylpheed filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::SylpheedFilter) );
    menu->addAction( act );

    act = new QAction( QLatin1String( "Procmail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::ProcmailFilter) );
    menu->addAction( act );

    act = new QAction( QLatin1String( "Balsa filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::BalsaFilter) );
    menu->addAction( act );

    act = new QAction( QLatin1String( "Claws Mail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::ClawsMailFilter) );
    menu->addAction( act );

    act = new QAction( QLatin1String( "Icedove Mail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::IcedoveFilter) );
    menu->addAction( act );
    act = new QAction( QLatin1String( "GMail filters" ), this );
    act->setData( QVariant::fromValue(MailCommon::FilterImporterExporter::GmailFilter) );
    menu->addAction( act );
    connect( menu, SIGNAL(triggered(QAction*)), SLOT(slotImportFilter(QAction*)) );

}

FilterImporterExporterGui::~FilterImporterExporterGui()
{

}

void FilterImporterExporterGui::slotImportFilter( QAction *act )
{
    if ( act ) {
        importFilters( act->data().value<MailCommon::FilterImporterExporter::FilterType>() );
    }
}

void FilterImporterExporterGui::importFilters( MailCommon::FilterImporterExporter::FilterType type )
{
    MailCommon::FilterImporterExporter importer( this );
    bool canceled = false;
    QList<MailCommon::MailFilter *> filters = importer.importFilters( canceled, type );
    if ( canceled ) {
        return;
    }
    //TODO
}


int main (int argc, char **argv)
{
    QApplication app(argc, argv);

    FilterImporterExporterGui *w = new FilterImporterExporterGui();
    w->resize(800, 200);
    w->show();
    app.exec();
    delete w;
    return 0;
}
