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

#include <tqwidget.h>

#include <ktextedit.h>
class KNote;
class TQFont;
class TQColor;
class TQPushButton;
class KAction;
class KActionCollection;
class KToggleAction;
class KFontAction;
class KFontSizeAction;


class KNoteEdit : public KTextEdit
{
    Q_OBJECT
public:
    KNoteEdit( KActionCollection *actions, TQWidget *parent=0, const char *name=0 );
    ~KNoteEdit();
    void setNote( KNote *_note ) {
        m_note = _note;
    }
    void setText( const TQString& text );
    void setTextFont( const TQFont& font );
    void setTextColor( const TQColor& color );
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
    void slotCutEnabled( bool );

protected:
    virtual void contentsDragEnterEvent( TQDragEnterEvent *e );
    virtual void contentsDropEvent( TQDropEvent *e );
    virtual TQPopupMenu *createPopupMenu( const TQPoint &pos );

private slots:
    void slotReturnPressed();

    void fontChanged( const TQFont &f );
    void colorChanged( const TQColor &c );
    void alignmentChanged( int a );
    void verticalAlignmentChanged( VerticalAlignment a );
    void slotAllowTab();

    void setEnabledRedo( bool b );
    void setEnabledUndo( bool b );

private:
    void autoIndent();

    virtual bool linksEnabled() const { return true; }
    virtual void emitLinkClicked( const TQString &s );

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
    KAction* undo;
    KAction* redo;
    bool m_autoIndentMode;
    KNote           *m_note;
};

#endif
