/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2004, The KNotes Developers

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************/

#ifndef KNOTEEDIT_H
#define KNOTEEDIT_H

#include <qwidget.h>

#include <ktextedit.h>

class QFont;
class QColor;
class QPushButton;
class KAction;
class KActionCollection;
class KToggleAction;
class KFontAction;
class KFontSizeAction;


class KNoteEdit : public KTextEdit
{
    Q_OBJECT
public:
    KNoteEdit( KActionCollection *actions, QWidget *parent=0, const char *name=0 );
    ~KNoteEdit();

    void setText( const QString& text );
    void setTextFont( const QFont& font );
    void setTextColor( const QColor& color );
    void setTabStop( int tabs );
    void setAutoIndentMode( bool newmode );

public slots:
    virtual void setTextFormat( TextFormat f );

    void textStrikeOut( bool );

    void textColor();

    void textAlignLeft();
    void textAlignCenter();
    void textAlignRight();
    void textAlignBlock();

    void textList();

    void textSuperScript();
    void textSubScript();

    //void textIncreaseIndent();
    //void textDecreaseIndent();

protected:
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDropEvent( QDropEvent *e );

private slots:
    void slotReturnPressed();

    void fontChanged( const QFont &f );
    void colorChanged( const QColor &c );
    void alignmentChanged( int a );
    void verticalAlignmentChanged( VerticalAlignment a );

private:
    void autoIndent();

    virtual bool linksEnabled() const { return true; }
    virtual void emitLinkClicked( const QString &s );

    void enableRichTextActions();
    void disableRichTextActions();

private:
    KAction *m_cut;
    KAction *m_copy;
    KAction *m_paste;

    KToggleAction *m_textBold;
    KToggleAction *m_textItalic;
    KToggleAction *m_textUnderline;
    KToggleAction *m_textStrikeOut;

    KToggleAction *m_textAlignLeft;
    KToggleAction *m_textAlignCenter;
    KToggleAction *m_textAlignRight;
    KToggleAction *m_textAlignBlock;

    KToggleAction *m_textList;
    KToggleAction *m_textSuper;
    KToggleAction *m_textSub;

    //KAction       *m_textIncreaseIndent;
    //KAction       *m_textDecreaseIndent;

    KAction         *m_textColor;
    KFontAction     *m_textFont;
    KFontSizeAction *m_textSize;

    bool m_autoIndentMode;
};

#endif
