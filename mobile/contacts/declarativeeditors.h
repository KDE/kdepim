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
#include "contactgroupeditorview.h"
#include "declarativewidgetbase.h"
#include "editorbusiness.h"
#include "editorcontactgroup.h"
#include "editorcrypto.h"
#include "editorgeneral.h"
#include "editorlocation.h"

class EditorDummy : public EditorBase
{
  Q_OBJECT

  public:
    void loadContact( const KABC::Addressee &contact ) { Q_UNUSED( contact ); }

    void saveContact( KABC::Addressee &contact ) { Q_UNUSED( contact ); }
};

class  EditorMore : public EditorDummy
{
  Q_OBJECT
};

class DeclarativeEditorGeneral
  : public DeclarativeWidgetBase<EditorGeneral, ContactEditorView, &ContactEditorView::setEditorGeneral>
{
  Q_OBJECT

  public:
    explicit DeclarativeEditorGeneral( QDeclarativeItem *parent = 0 );
};                         

class DeclarativeEditorBusiness
  : public DeclarativeWidgetBase<EditorBusiness, ContactEditorView, &ContactEditorView::setEditorBusiness>
{
  Q_OBJECT

  public:
    explicit DeclarativeEditorBusiness( QDeclarativeItem *parent = 0 );
};

class DeclarativeEditorLocation
  : public DeclarativeWidgetBase<EditorLocation, ContactEditorView, &ContactEditorView::setEditorLocation>
{
  Q_OBJECT

  public:
    explicit DeclarativeEditorLocation( QDeclarativeItem *parent = 0 );
};

class DeclarativeEditorCrypto
  : public DeclarativeWidgetBase<EditorCrypto, ContactEditorView, &ContactEditorView::setEditorCrypto>
{
  Q_OBJECT

  public:
    explicit DeclarativeEditorCrypto( QDeclarativeItem *parent = 0 );
};

class DeclarativeEditorMore
  : public DeclarativeWidgetBase<EditorMore, ContactEditorView, &ContactEditorView::setEditorMore>
{
  Q_OBJECT

  public:
    explicit DeclarativeEditorMore( QDeclarativeItem *parent = 0 );
};

class DeclarativeEditorContactGroup
  : public DeclarativeWidgetBase<EditorContactGroup, ContactGroupEditorView, &ContactGroupEditorView::setEditor>
{
  Q_OBJECT

  public:
    explicit DeclarativeEditorContactGroup( QDeclarativeItem *parent = 0 );
};

#endif
