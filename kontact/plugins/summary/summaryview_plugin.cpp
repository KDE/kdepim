/*
*   Copyright 2010 Ryan Rix <ry@n.rix.si>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "summaryview_plugin.h"

#include <kontactinterface/core.h>
#include <kontactinterface/plugin.h>

#include <kactioncollection.h>
#include <kgenericfactory.h>
#include <kicon.h>
#include <kparts/componentfactory.h>
#include <kdialog.h>
#include <KDebug>

EXPORT_KONTACT_PLUGIN( SummaryPlugin, summary )

SummaryPlugin::SummaryPlugin( KontactInterface::Core *core, const QVariantList & )
    : KontactInterface::Plugin( core, core, "Summary" )
{
    setComponentData( KontactPluginFactory::componentData() );

    setXMLFile("kontactsummary.rc");

    KAction* action = new KAction("&Configure", actionCollection());
    connect(action, SIGNAL(triggered()), this, SLOT(optionsPreferences()));
    actionCollection()->addAction("summaryview_configure", action);
}

SummaryPlugin::~SummaryPlugin()
{
}

KParts::ReadOnlyPart *SummaryPlugin::createPart()
{
    m_part = core()->createPart( "plasma-kpart" );

    connect( m_part, SIGNAL(showPart()), this, SLOT(showPart()) );

    return m_part;
}

void SummaryPlugin::readProperties( const KConfigGroup &config )
{
}

void SummaryPlugin::saveProperties( KConfigGroup &config )
{
}

void SummaryPlugin::showPart()
{
    core()->selectPlugin( this );
}

void SummaryPlugin::optionsPreferences()
{
    if( !m_dialog )
    {
        QWidget* widget = 0;
        m_dialog = new KDialog(qobject_cast<QWidget*>(this));
        widget = new QWidget(m_dialog);


        m_dialog->setMainWidget( widget );
        createConfigurationInterface(widget);

        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Default );
        m_dialog->show();
    }
    else
    {
        m_dialog->show();
    }
}

QWidget* SummaryPlugin::createConfigurationInterface(QWidget* parent)
{
    connect(this,SIGNAL(sigCreateConfigurationInterface(QWidget*)), m_part, SLOT(createConfigurationInterface(QWidget*)));

    emit sigCreateConfigurationInterface(parent);

    disconnect(m_part,SLOT(createConfigurationInterface(QWidget*)));
}

#include "summaryview_plugin.moc"
