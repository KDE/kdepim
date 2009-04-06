/*
    This file is part of Akonadi.

    Copyright (c) 2009 Till Adam <adam@kde.org>

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

#include "jobtrackerwidget.h"

#include "jobtrackermodel.h"

#include <akonadi/control.h>

#include <KLocale>

#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>


using org::freedesktop::Akonadi::DebugInterface;

JobTrackerWidget::JobTrackerWidget( QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QTreeView *tv = new QTreeView( this );
  JobTrackerModel * model = new JobTrackerModel( this );
  tv->setModel( model );
  layout->addWidget( tv );

  Akonadi::Control::widgetNeedsAkonadi( this );
}


void JobTrackerWidget::jobStarted( const QString &identifier, const QString& )
{

}

#include "jobtrackerwidget.moc"
