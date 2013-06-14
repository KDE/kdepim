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
#include "knglobals.h"
#include "utilities.h"
#include <KPIMTextEdit/EMailQuoteHighlighter>
#include <KConfigGroup>
#include <QApplication>
#include <messagecomposer/utils/util.h>

KNComposerEditor::KNComposerEditor( QWidget *parent)
 :KMeditor(parent)
{
  setSpellCheckingConfigFileName( KNGlobals::self()->config()->name() );
}

KNComposerEditor::~KNComposerEditor()
{
}

void KNComposerEditor::changeHighlighterColors(KPIMTextEdit::EMailQuoteHighlighter * highlighter )
{
  KConfigGroup config( knGlobals.config(), "VISUAL_APPEARANCE" );
  QColor defaultColor1( 0x00, 0x80, 0x00 ); // defaults from kmreaderwin.cpp
  QColor defaultColor2( 0x00, 0x70, 0x00 );
  QColor defaultColor3( 0x00, 0x60, 0x00 );
  QColor defaultForeground( qApp->palette().color( QPalette::Text ) );
  QColor col1 = config.readEntry( "ForegroundColor", defaultForeground );
  QColor col2 = config.readEntry( "quote3Color", defaultColor3 );
  QColor col3 = config.readEntry( "quote2Color", defaultColor2 );
  QColor col4 = config.readEntry( "quote1Color", defaultColor1 );

  highlighter->setQuoteColor(col1, col2, col3, col4);
}

void KNComposerEditor::slotRot13()
{
  QTextCursor cursor = textCursor();
  if ( cursor.hasSelection() )
    insertPlainText( MessageComposer::Util::rot13( cursor.selectedText() ) );
  //FIXME: breaks HTML formatting
}


void KNComposerEditor::slotRemoveBox()
{
  //Laurent: fix me
#if 0
    if (hasMarkedText()) {
    QString s = QString::fromLatin1("\n") + markedText() + QString::fromLatin1("\n");
    s.replace(QRegExp("\n,----[^\n]*\n"),"\n");
    s.replace(QRegExp("\n| "),"\n");
    s.replace(QRegExp("\n`----[^\n]*\n"),"\n");
    s.remove(0,1);
    s.truncate(s.length()-1);
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();

    QString s = textLine(l);   // test if we are in a box
    if (!((s.left(2) == "| ")||(s.left(5)==",----")||(s.left(5)=="`----")))
      return;

    setAutoUpdate(false);

    // find & remove box begin
    int x = l;
    while ((x>=0)&&(textLine(x).left(5)!=",----"))
      x--;
    if ((x>=0)&&(textLine(x).left(5)==",----")) {
      removeLine(x);
      l--;
      for (int i=x;i<=l;++i) {     // remove quotation
        s = textLine(i);
        if (s.left(2) == "| ") {
          s.remove(0,2);
          insertLine(s,i);
          removeLine(i+1);
        }
      }
    }

    // find & remove box end
    x = l;
    while ((x<numLines())&&(textLine(x).left(5)!="`----"))
      x++;
    if ((x<numLines())&&(textLine(x).left(5)=="`----")) {
      removeLine(x);
      for (int i=l+1;i<x;++i) {     // remove quotation
        s = textLine(i);
        if (s.left(2) == "| ") {
          s.remove(0,2);
          insertLine(s,i);
          removeLine(i+1);
        }
      }
    }

    setCursorPosition(l,c-2);

    setAutoUpdate(true);
    repaint();
  }
#endif
}

void KNComposerEditor::slotAddBox()
{
  MessageComposer::Util::addTextBox(this);
}
