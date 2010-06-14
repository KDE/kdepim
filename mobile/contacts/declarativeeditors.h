/*
    Copyright (c) 2010 Kevin Krammer <kevin.krammer@gmx.at>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef DECLARATIVEEDITORS_H
#define DECLARATIVEEDITORS_H

#include "contacteditorview.h"
#include "declarativewidgetbase.h"

#include <Akonadi/Contact/ContactEditor>
#include <Akonadi/CollectionComboBox>

class DeclarativeCollectionSelector
  : public DeclarativeWidgetBase<Akonadi::CollectionComboBox,
                                 ContactEditorView,
                                 &ContactEditorView::setCollectionSelector>
{
  Q_OBJECT
 
  public:
    explicit DeclarativeCollectionSelector( QDeclarativeItem *parent = 0 );
};

class ContactCreateWidget : public Akonadi::ContactEditor
{
  Q_OBJECT

  public:
    explicit ContactCreateWidget( QWidget *parent = 0 );
};

class ContactEditWidget : public Akonadi::ContactEditor
{
  Q_OBJECT

  public:
    explicit ContactEditWidget( QWidget *parent = 0 );
};

class DeclarativeContactCreator
  : public DeclarativeWidgetBase<ContactCreateWidget,
                                 ContactEditorView,
                                 &ContactEditorView::setCreatorWidget>
{
  Q_OBJECT

  public:
    explicit DeclarativeContactCreator( QDeclarativeItem *parent = 0 );
};

class DeclarativeContactEditor
  : public DeclarativeWidgetBase<ContactEditWidget,
                                 ContactEditorView,
                                 &ContactEditorView::setEditorWidget>
{
  Q_OBJECT

  public:
    explicit DeclarativeContactEditor( QDeclarativeItem *parent = 0 );
};

#endif