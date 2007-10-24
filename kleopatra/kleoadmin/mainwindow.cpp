/* -*- mode: c++; c-basic-offset:4 -*-
    mainwindow.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "mainwindow.h"

#include <qgpgmecryptoconfig.h>

#include <QStringList>
#include <QTreeWidgetItem>

#include <cassert>

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow( parent, flags ), m_config( new QGpgMECryptoConfig )
{
    QWidget* mainWidget = new QWidget( this );
    m_ui.setupUi( mainWidget );
    setCentralWidget( mainWidget );
    connect( m_ui.treeWidget, SIGNAL( itemSelectionChanged() ), 
             SLOT( treeWidgetItemSelectionChanged() ) );
    readConfiguration();
}

MainWindow::~MainWindow()
{
    delete m_config;
}

void MainWindow::treeWidgetItemSelectionChanged()
{
    const QList<QTreeWidgetItem*> selected = m_ui.treeWidget->selectedItems();
    assert( selected.count() <= 1 );
    Kleo::CryptoConfigEntry* const entry = selected.isEmpty() ? 0 : m_itemToEntry[selected.first()];
    m_ui.componentLabel->setText( i18n( "Component: %1", entry ? m_componentForEntry[entry]->name() : QString() ) );
    m_ui.optionLabel->setText( i18n( "Option: %1", entry ? entry->name() : QString() ) );
    m_ui.descriptionLabel->setText( i18n( "Description: %1", entry ? entry->description() : QString() ) );
    m_ui.valueLE->setText( entry ? entry->stringValue() : QString() );
    m_ui.readOnlyBox->setCheckState( ( entry && entry->isReadOnly() ) ? Qt::Checked : Qt::Unchecked );
}

void MainWindow::readConfiguration()
{
    QStringList components = m_config->componentList();
    qSort( components );
    Q_FOREACH ( const QString i, components )
    {
        Kleo::CryptoConfigComponent* const component = m_config->component( i );
        assert( component );
        QTreeWidgetItem* const componentItem = new QTreeWidgetItem;
        componentItem->setText( NameColumn, component->name() );
        m_ui.treeWidget->addTopLevelItem( componentItem );
        Q_FOREACH ( const QString j, component->groupList() )
        {
            Kleo::CryptoConfigGroup* const group = component->group( j );
            assert( group );
            QTreeWidgetItem* const groupItem = new QTreeWidgetItem;
            groupItem->setText( NameColumn, group->name() );
            componentItem->addChild( groupItem );
            Q_FOREACH( const QString k, group->entryList() )
            {
                Kleo::CryptoConfigEntry* const entry = group->entry( k );
                assert( entry );
                QTreeWidgetItem* const entryItem = new QTreeWidgetItem;
                entryItem->setData( NameColumn, IsOptionRole, true );
                entryItem->setText( NameColumn, entry->name() );
                entryItem->setText( ValueColumn, entry->stringValue() );
                entryItem->setCheckState( ReadOnlyColumn, entry->isReadOnly() ? Qt::Checked : Qt::Unchecked ); 
                groupItem->addChild( entryItem );
                m_componentForEntry[entry] = component;
                m_itemToEntry[entryItem] = entry;
            }
        }
    }
}

