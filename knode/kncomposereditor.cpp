/**
 * kncomposereditor.cpp
 *
 * Copyright (C)  2008 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "kncomposereditor.h"
#include "kncomposereditor.moc"
#include "kemailquotinghighter.h"
#include "knglobals.h"
#include <KConfigGroup>
#include <QApplication>

KNComposerEditor::KNComposerEditor( QWidget *parent)
 :KMeditor(parent)
{
}

KNComposerEditor::~KNComposerEditor()
{
}

void KNComposerEditor::changeHighlighterColors(KPIM::KEMailQuotingHighlighter * highlighter )
{
   KConfigGroup config( knGlobals.config(), "VISUAL_APPEARANCE" );
  QColor defaultColor1( qApp->palette().color( QPalette::Text )); // defaults from kmreaderwin.cpp
  QColor defaultColor2( qApp->palette().color( QPalette::Text ) );
  QColor defaultColor3( qApp->palette().color( QPalette::Text ) );
  QColor defaultForeground( qApp->palette().color( QPalette::Text ) );
  QColor col1 = config.readEntry( "ForegroundColor", defaultForeground );
  QColor col2 = config.readEntry( "quote3Color", defaultColor3 );
  QColor col3 = config.readEntry( "quote2Color", defaultColor2 );
  QColor col4 = config.readEntry( "quote1Color", defaultColor1 );
  QColor c = QColor("red");
  highlighter->setQuoteColor(col1, col2, col3, col4);
}

