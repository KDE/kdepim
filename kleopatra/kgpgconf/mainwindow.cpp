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

#include "configreader.h"
#include "configuration.h"
#include "configwriter.h"
#include "exception.h"

#include <KLocalizedString>

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QStringList>
#include <QTemporaryFile>
#include <QTimer>
#include <QTreeWidgetItem>

#include <cassert>

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags flags ) : QMainWindow( parent, flags ), m_config( 0 ), m_selectedEntry( 0 )
{
    QWidget* mainWidget = new QWidget( this );
    m_ui.setupUi( mainWidget );
    setCentralWidget( mainWidget );
    connect( m_ui.treeWidget, SIGNAL(itemSelectionChanged()),
             SLOT(treeWidgetItemSelectionChanged()) );
    connect( m_ui.treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
             SLOT(treeWidgetItemChanged(QTreeWidgetItem*,int)) );
    connect(m_ui.readOnlyBox, &QCheckBox::stateChanged, this, &MainWindow::readOnlyStateChanged);
    connect(m_ui.valueLE, &QLineEdit::textChanged, this, &MainWindow::optionValueChanged);
    connect(m_ui.useCustomRB, &QRadioButton::toggled, m_ui.valueLE, &QLineEdit::setEnabled);
    connect(m_ui.useDefaultRB, &QRadioButton::toggled, this, &MainWindow::useDefaultToggled);

    QMenu* const fileMenu = menuBar()->addMenu( i18n( "&File" ) );
    fileMenu->addAction( i18n( "Save As..." ), this, SLOT(saveAs()) );
    fileMenu->addSeparator();
    QAction* const quit = fileMenu->addAction( i18n( "Quit" ), qApp, SLOT(quit()) );
    quit->setShortcut( Qt::CTRL + Qt::Key_Q );
    resize( 640, 480 );
    QTimer::singleShot( 0, this, SLOT(delayedInit()) );
}

void MainWindow::delayedInit()
{
    ConfigReader reader;
    try
    {
        m_config = reader.readConfig();
        readConfiguration();
    }
    catch ( const GpgConfRunException& e )
    {
        QMessageBox::critical( this, i18n( "Setup Error" ), i18n( "KGpgConf could not execute gpgconf.exe.\n\nError: %1\nError Code: %2", e.message(), e.errorCode() ) );
        qApp->quit();
    }
    catch( const MalformedGpgConfOutputException& e )
    {
        QMessageBox::critical( this, i18n( "Parsing Error" ), i18n( "An error occurred while reading the current configuration.\n\nError: %1", e.message() ) );
        qApp->quit();
    }
    catch( const KGpgConfException& e )
    {
        QMessageBox::critical( this, i18n( "Error" ), i18n( "An error occurred while reading the current configuration.\n\nError: %1", e.message() ) );
        qApp->quit();
    }
}

MainWindow::~MainWindow()
{
    delete m_config;
}

void MainWindow::treeWidgetItemSelectionChanged()
{
    m_selectedEntry = 0;
    const QList<QTreeWidgetItem*> selected = m_ui.treeWidget->selectedItems();
    assert( selected.count() <= 1 );
    ConfigEntry* const entry = selected.isEmpty() ? 0 : m_itemToEntry[selected.first()];
    m_ui.componentLabel->setText( entry ? m_componentForEntry[entry]->name() : QString() );
    m_ui.optionLabel->setText( entry ? entry->name() : QString() );
    m_ui.descriptionLabel->setText( entry ? entry->description() : QString() );
    m_ui.typeLabel->setText( entry ? entry->typeDescription() : QString() );
    m_ui.valueLE->setText( entry ? entry->stringValue() : QString() );
    m_ui.useDefaultRB->setChecked( entry && entry->useBuiltInDefault() );
    m_ui.useCustomRB->setChecked( entry && !entry->useBuiltInDefault() );
    m_ui.readOnlyBox->setCheckState( ( entry && entry->mutability() == ConfigEntry::NoChange ) ? Qt::Checked : Qt::Unchecked );
    m_ui.readOnlyBox->setEnabled( entry );
    m_ui.useCustomRB->setEnabled( entry );
    m_ui.useDefaultRB->setEnabled( entry );
    m_ui.valueLE->setEnabled( entry );
    m_ui.optionLabelLabel->setEnabled( entry );
    m_ui.typeLabelLabel->setEnabled( entry );
    m_ui.componentLabelLabel->setEnabled( entry );
    m_ui.descriptionLabelLabel->setEnabled( entry );
    m_selectedEntry = entry;
}

void MainWindow::treeWidgetItemChanged( QTreeWidgetItem* item, int column )
{
    if ( column != ReadOnlyColumn )
        return;
    ConfigEntry* entry = m_itemToEntry[item];
    assert( entry );
    entry->setMutability( item->checkState( ReadOnlyColumn ) == Qt::Checked ? ConfigEntry::NoChange : ConfigEntry::UnspecifiedMutability );
    if ( entry == m_selectedEntry )
    {
        m_ui.readOnlyBox->setCheckState( entry->mutability() == ConfigEntry::NoChange ? Qt::Checked : Qt::Unchecked );
    }
}

void MainWindow::useDefaultToggled( bool useDefault )
{
    if ( !m_selectedEntry )
        return;
    m_selectedEntry->setUseBuiltInDefault( useDefault );
}

void MainWindow::readOnlyStateChanged( int state )
{
    if ( !m_selectedEntry )
        return;
    assert( state != Qt::PartiallyChecked );
    m_selectedEntry->setMutability( state == Qt::Checked ? ConfigEntry::NoChange: ConfigEntry::UnspecifiedMutability );
    QTreeWidgetItem* const item = m_entryToItem[m_selectedEntry];
    assert( item );
    item->setCheckState( ReadOnlyColumn, static_cast<Qt::CheckState>( state ) );
}

void MainWindow::optionValueChanged()
{
    if ( !m_selectedEntry )
        return;
    m_selectedEntry->setValueFromUiString( m_ui.valueLE->text() );
}

void MainWindow::readConfiguration()
{
    QStringList components = m_config->componentList();
    qSort( components );
    Q_FOREACH ( const QString& i, components )
    {
        ConfigComponent* const component = m_config->component( i );
        assert( component );
        QTreeWidgetItem* const componentItem = new QTreeWidgetItem;
        componentItem->setText( NameColumn, component->name() );
        m_ui.treeWidget->addTopLevelItem( componentItem );
        Q_FOREACH ( const QString& j, component->groupList() )
        {
            ConfigGroup* const group = component->group( j );
            assert( group );
            QTreeWidgetItem* const groupItem = new QTreeWidgetItem;
            groupItem->setText( NameColumn, group->name() );
            componentItem->addChild( groupItem );
            Q_FOREACH( const QString& k, group->entryList() )
            {
                ConfigEntry* const entry = group->entry( k );
                assert( entry );
                QTreeWidgetItem* const entryItem = new QTreeWidgetItem;
                entryItem->setData( NameColumn, IsOptionRole, true );
                entryItem->setText( NameColumn, entry->name() );
//                entryItem->setText( ValueColumn, QString() );
                entryItem->setCheckState( ReadOnlyColumn, entry->mutability() == ConfigEntry::NoChange ? Qt::Checked : Qt::Unchecked );
                groupItem->addChild( entryItem );
                m_componentForEntry[entry] = component;
                m_itemToEntry[entryItem] = entry;
                m_entryToItem[entry] = entryItem;
            }
        }
    }
}

void MainWindow::saveToFile( const QString& fileName )
{
    if ( fileName.isEmpty() )
        return;

    QTemporaryFile tmp( fileName );
    if ( !tmp.open() )
    {
        QMessageBox::warning( this, i18n( "Write Error" ), i18n( "Could not open file %1 for writing. You might not have the permission to write to that file.", fileName ) );
        return;
    }
    tmp.setTextModeEnabled( true );
    ConfigWriter writer( &tmp );
    if ( writer.writeConfig( m_config ) )
    {
        const QString tmpFileName = tmp.fileName(); // close() clears fileName()
        tmp.close();
        tmp.setAutoRemove( false );
        for ( int i = 0; i < 10; ++i )
        {
            if ( QFile::rename( tmpFileName, fileName ) )
                return;
            else
                QFile::remove( fileName );
        }
        tmp.setAutoRemove( true );
    }
    QMessageBox::critical( this, i18n( "Write Error" ), i18n( "Error while writing to file %1.", fileName ) );

}

void MainWindow::saveAs()
{
    saveToFile( QFileDialog::getSaveFileName( this, i18n( "Save As"), QString(), QLatin1String("*.conf") ) );
}

