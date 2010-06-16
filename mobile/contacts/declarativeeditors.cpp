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

#include "declarativeeditors.h"

DeclarativeEditorGeneral::DeclarativeEditorGeneral( QDeclarativeItem *parent )
  : DeclarativeWidgetBase<EditorGeneral, ContactEditorView, &ContactEditorView::setEditorGeneral>( parent )
{
}

DeclarativeEditorBusiness::DeclarativeEditorBusiness( QDeclarativeItem *parent )
  : DeclarativeWidgetBase<EditorBusiness, ContactEditorView, &ContactEditorView::setEditorBusiness>( parent )
{
}

DeclarativeEditorLocation::DeclarativeEditorLocation( QDeclarativeItem *parent )
  : DeclarativeWidgetBase<EditorLocation, ContactEditorView, &ContactEditorView::setEditorLocation>( parent )
{
}

DeclarativeEditorCrypto::DeclarativeEditorCrypto( QDeclarativeItem *parent )
  : DeclarativeWidgetBase<EditorCrypto, ContactEditorView, &ContactEditorView::setEditorCrypto>( parent )
{
}

DeclarativeEditorMore::DeclarativeEditorMore( QDeclarativeItem *parent )
  : DeclarativeWidgetBase<EditorMore, ContactEditorView, &ContactEditorView::setEditorMore>( parent )
{
}

DeclarativeEditorContactGroup::DeclarativeEditorContactGroup( QDeclarativeItem *parent )
  : DeclarativeWidgetBase<EditorContactGroup, ContactGroupEditorView, &ContactGroupEditorView::setEditor>( parent )
{
}

#include "declarativeeditors.moc"