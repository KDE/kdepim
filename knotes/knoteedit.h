/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2001, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*******************************************************************/

#ifndef KNOTEEDIT_H
#define KNOTEEDIT_H

#include <qwidget.h>
#include <qtextedit.h>

class QFont;
class QColor;
class KAction;
class KToggleAction;


class KNoteEdit : public QTextEdit
{
    Q_OBJECT
public:
    KNoteEdit( QWidget *parent=0, const char *name=0 );
    ~KNoteEdit();

    void readFile( QString& filename );
    void dumpToFile( QString& filename ) const;
    void setTextFont( QFont& font );
    void setTextColor( QColor& color );

    void setTabStop( int tabs );
    void setAutoIndentMode( bool newmode );

protected:
    void dragMoveEvent( QDragMoveEvent* event );
    void dragEnterEvent( QDragEnterEvent* event );
    void dropEvent( QDropEvent* event );

public slots:
//    void textStyleSelected( int );
//    void textSizeSelected( int );
//    void textFontSelected( const QString & );

//    void textColor();

    void textAlignLeft();
    void textAlignCenter();
    void textAlignRight();
    void textAlignBlock();

    void textList();

    void textSuperScript();
    void textSubScript();

    void textIncreaseIndent();
    void textDecreaseIndent();

protected slots:
    void slotReturnPressed();
    void slotSelectionChanged();

signals:
    void gotUrlDrop( const QString& url );

private:
    void autoIndent();

    KAction* m_cut;
    KAction* m_copy;
    KAction* m_paste;

    KToggleAction* m_textBold;
    KToggleAction* m_textItalic;
    KToggleAction* m_textUnderline;

    KToggleAction* m_textAlignLeft;
    KToggleAction* m_textAlignCenter;
    KToggleAction* m_textAlignRight;
    KToggleAction* m_textAlignBlock;

    KToggleAction* m_textList;
    KToggleAction* m_textSuper;
    KToggleAction* m_textSub;

    KAction* m_textIncreaseIndent;
    KAction* m_textDecreaseIndent;

    bool m_autoIndentMode;
};

#endif
