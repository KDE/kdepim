/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "notificationmonitor.h"
#include "notificationmodel.h"

#include <AkonadiCore/Control>

#include <KLocalizedString>
#include <KFileDialog>
#include <QUrl>

#include <QHeaderView>
#include <QCheckBox>
#include <QMenu>
#include <QTreeView>
#include <QVBoxLayout>
#include <QPushButton>

NotificationMonitor::NotificationMonitor(QWidget* parent) :
  QWidget( parent )
{
  m_model = new NotificationModel( this );
  m_model->setEnabled( false ); // since it can be slow, default to off

  QVBoxLayout *layout = new QVBoxLayout( this );

  QCheckBox* enableCB = new QCheckBox( this );
  enableCB->setText(i18n("Enable notification monitor"));
  enableCB->setChecked(m_model->isEnabled());
  connect(enableCB, &QCheckBox::toggled, m_model, &NotificationModel::setEnabled);
  layout->addWidget(enableCB);

  QTreeView *tv = new QTreeView( this );
  tv->setModel( m_model );
  tv->expandAll();
  tv->setAlternatingRowColors( true );
  tv->setContextMenuPolicy( Qt::CustomContextMenu );
  tv->header()->setResizeMode( QHeaderView::ResizeToContents );
  connect(tv, &QTreeView::customContextMenuRequested, this, &NotificationMonitor::contextMenu);
  layout->addWidget( tv );

  QHBoxLayout *layout2 = new QHBoxLayout( this );
  QPushButton *button = new QPushButton( QLatin1String( "Save to file..." ), this );
  connect( button, SIGNAL(clicked(bool)),
           this, SLOT(slotSaveToFile()) );
  layout2->addWidget( button );
  layout2->addStretch( 1 );
  layout->addLayout( layout2 );

  Akonadi::Control::widgetNeedsAkonadi( this );
}

void NotificationMonitor::contextMenu(const QPoint& pos)
{
  QMenu menu;
  menu.addAction( i18n( "Clear View" ), m_model, SLOT(clear()) );
  menu.exec( mapToGlobal( pos ) );
}

void NotificationMonitor::slotSaveToFile()
{
  const QString fileName = KFileDialog::getSaveFileName( QUrl(), QString(), 0, QString(), KFileDialog::ConfirmOverwrite );
  if ( fileName.isEmpty() ) {
    return;
  }

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    return;
  }

  file.write("Operation/ID\tType/RID\tSession/REV\tResource/MimeType\tDestination Resource\tParent\tDestination\tParts\tAdded Flags\tRemoved Flags\n");

  writeRows( QModelIndex(), file, 0 );

  file.close();
}

void NotificationMonitor::writeRows( const QModelIndex &parent, QFile &file, int indentLevel )
{
  for ( int row = 0; row < m_model->rowCount( parent ); ++row ) {
    QByteArray data;
    for ( int tabs = 0; tabs < indentLevel; ++tabs ) {
      data += '\t';
    }
    const int columnCount = m_model->columnCount( parent );
    for ( int column = 0; column < columnCount; ++column ) {
      const QModelIndex index = m_model->index( row, column, parent );
      data += index.data().toByteArray();
      if ( column < columnCount - 1 ) {
        data += '\t';
      }
    }
    data += '\n';
    file.write( data );

    const QModelIndex index = m_model->index( row, 0, parent );
    writeRows( index, file, indentLevel + 1 );
  }
}
