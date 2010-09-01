/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2005, The KNotes Developers

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

#include <tqdragobject.h>
#include <tqfont.h>

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kurldrag.h>
#include <kstdaction.h>
#include <kcolordialog.h>
#include <tqpopupmenu.h>
#include <kiconloader.h>
#include "knoteedit.h"
#include "knote.h"

static const short SEP = 5;
static const short ICON_SIZE = 10;


KNoteEdit::KNoteEdit( KActionCollection *actions, TQWidget *parent, const char *name )
    : KTextEdit( parent, name ), m_note( 0 )
{
    setAcceptDrops( true );
    setWordWrap( WidgetWidth );
    setWrapPolicy( AtWhiteSpace );
    setLinkUnderline( true );
    setCheckSpellingEnabled(false);
    // create the actions for the RMB menu
    undo = KStdAction::undo( this, TQT_SLOT(undo()), actions );
    redo = KStdAction::redo( this, TQT_SLOT(redo()), actions );
    undo->setEnabled( isUndoAvailable() );
    redo->setEnabled( isRedoAvailable() );

    m_cut = KStdAction::cut( this, TQT_SLOT(cut()), actions );
    m_copy = KStdAction::copy( this, TQT_SLOT(copy()), actions );
    m_paste = KStdAction::paste( this, TQT_SLOT(paste()), actions );

    m_cut->setEnabled( false );
    m_copy->setEnabled( false );
    m_paste->setEnabled( true );

    connect( this, TQT_SIGNAL(undoAvailable(bool)), this, TQT_SLOT(setEnabledUndo(bool)) );
    connect( this, TQT_SIGNAL(redoAvailable(bool)), this, TQT_SLOT(setEnabledRedo(bool)) );

    connect( this, TQT_SIGNAL(copyAvailable(bool)), this, TQT_SLOT( slotCutEnabled( bool ) ) );
    connect( this, TQT_SIGNAL(copyAvailable(bool)), m_copy, TQT_SLOT(setEnabled(bool)) );

    new KAction( KStdGuiItem::clear(), 0, this, TQT_SLOT(clear()), actions, "edit_clear" );
    KStdAction::selectAll( this, TQT_SLOT(selectAll()), actions );

    // create the actions modifying the text format
    m_textBold = new KToggleAction( i18n("Bold"), "text_bold", CTRL + Key_B, 0, 0,
                                    actions, "format_bold" );
    m_textItalic = new KToggleAction( i18n("Italic"), "text_italic", CTRL + Key_I, 0, 0,
                                      actions, "format_italic" );
    m_textUnderline = new KToggleAction( i18n("Underline"), "text_under", CTRL + Key_U, 0, 0,
                                         actions, "format_underline" );
    m_textStrikeOut = new KToggleAction( i18n("Strike Out"), "text_strike", CTRL + Key_S, 0, 0,
                                         actions, "format_strikeout" );

    connect( m_textBold, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setBold(bool)) );
    connect( m_textItalic, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setItalic(bool)) );
    connect( m_textUnderline, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setUnderline(bool)) );
    connect( m_textStrikeOut, TQT_SIGNAL(toggled(bool)), TQT_SLOT(textStrikeOut(bool)) );

    m_textAlignLeft = new KToggleAction( i18n("Align Left"), "text_left", ALT + Key_L,
                                 this, TQT_SLOT(textAlignLeft()),
                                 actions, "format_alignleft" );
    m_textAlignLeft->setChecked( true ); // just a dummy, will be updated later
    m_textAlignCenter = new KToggleAction( i18n("Align Center"), "text_center", ALT + Key_C,
                                 this, TQT_SLOT(textAlignCenter()),
                                 actions, "format_aligncenter" );
    m_textAlignRight = new KToggleAction( i18n("Align Right"), "text_right", ALT + Key_R,
                                 this, TQT_SLOT(textAlignRight()),
                                 actions, "format_alignright" );
    m_textAlignBlock = new KToggleAction( i18n("Align Block"), "text_block", ALT + Key_B,
                                 this, TQT_SLOT(textAlignBlock()),
                                 actions, "format_alignblock" );

    m_textAlignLeft->setExclusiveGroup( "align" );
    m_textAlignCenter->setExclusiveGroup( "align" );
    m_textAlignRight->setExclusiveGroup( "align" );
    m_textAlignBlock->setExclusiveGroup( "align" );

    m_textList = new KToggleAction( i18n("List"), "enum_list", 0,
                                    this, TQT_SLOT(textList()),
                                    actions, "format_list" );

    m_textList->setExclusiveGroup( "style" );

    m_textSuper = new KToggleAction( i18n("Superscript"), "text_super", 0,
                                     this, TQT_SLOT(textSuperScript()),
                                     actions, "format_super" );
    m_textSub = new KToggleAction( i18n("Subscript"), "text_sub", 0,
                                   this, TQT_SLOT(textSubScript()),
                                   actions, "format_sub" );

    m_textSuper->setExclusiveGroup( "valign" );
    m_textSub->setExclusiveGroup( "valign" );

// There is no easy possibility to implement text indenting with QTextEdit
//
//     m_textIncreaseIndent = new KAction( i18n("Increase Indent"), "format_increaseindent", 0,
//                                         this, TQT_SLOT(textIncreaseIndent()),
//                                         actions, "format_increaseindent" );
//
//     m_textDecreaseIndent = new KAction( i18n("Decrease Indent"), "format_decreaseindent", 0,
//                                         this, TQT_SLOT(textDecreaseIndent()),
//                                         actions, "format_decreaseindent" );

    TQPixmap pix( ICON_SIZE, ICON_SIZE );
    pix.fill( black );     // just a dummy, gets updated before widget is shown
    m_textColor = new KAction( i18n("Text Color..."), pix, 0, this,
                                  TQT_SLOT(textColor()), actions, "format_color" );

    m_textFont = new KFontAction( i18n("Text Font"), "text", KKey(),
                                  actions, "format_font" );
    connect( m_textFont, TQT_SIGNAL(activated( const TQString & )),
             this, TQT_SLOT(setFamily( const TQString & )) );

    m_textSize = new KFontSizeAction( i18n("Text Size"), KKey(),
                                      actions, "format_size" );
    connect( m_textSize, TQT_SIGNAL(fontSizeChanged( int )),
             this, TQT_SLOT(setPointSize( int )) );

    // TQTextEdit connections
    connect( this, TQT_SIGNAL(returnPressed()), TQT_SLOT(slotReturnPressed()) );
    connect( this, TQT_SIGNAL(currentFontChanged( const TQFont & )),
             this, TQT_SLOT(fontChanged( const TQFont & )) );
    connect( this, TQT_SIGNAL(currentColorChanged( const TQColor & )),
             this, TQT_SLOT(colorChanged( const TQColor & )) );
    connect( this, TQT_SIGNAL(currentAlignmentChanged( int )),
             this, TQT_SLOT(alignmentChanged( int )) );
    connect( this, TQT_SIGNAL(currentVerticalAlignmentChanged( VerticalAlignment )),
             this, TQT_SLOT(verticalAlignmentChanged( VerticalAlignment )) );
}

KNoteEdit::~KNoteEdit()
{
}

void KNoteEdit::setEnabledRedo( bool b )
{
    redo->setEnabled( b && !isReadOnly() );
}

void KNoteEdit::setEnabledUndo( bool b )
{
    undo->setEnabled( b && !isReadOnly() );
}

void KNoteEdit::slotCutEnabled( bool b )
{
    m_cut->setEnabled( b && !isReadOnly() );
}

void KNoteEdit::setText( const TQString& text )
{
    // to update the font and font size combo box - TQTextEdit stopped
    // emitting the currentFontChanged signal with the new optimizations
    KTextEdit::setText( text );
    fontChanged( currentFont() );
}

void KNoteEdit::setTextFont( const TQFont& font )
{
    if ( textFormat() == PlainText )
        setFont( font );
    else
        setCurrentFont( font );
}

void KNoteEdit::setTextColor( const TQColor& color )
{
    setColor( color );
    colorChanged( color );
}

void KNoteEdit::setTabStop( int tabs )
{
    TQFontMetrics fm( font() );
    setTabStopWidth( fm.width( 'x' ) * tabs );
}

void KNoteEdit::setAutoIndentMode( bool newmode )
{
    m_autoIndentMode = newmode;
}


/** public slots **/

void KNoteEdit::setTextFormat( TextFormat f )
{
    if ( f == textFormat() )
        return;

    if ( f == RichText )
    {
        TQString t = text();
        KTextEdit::setTextFormat( f );

        // if the note contains html/xml source try to display it, otherwise
        // get the modified text again and set it to preserve newlines
        if ( TQStyleSheet::mightBeRichText( t ) )
            setText( t );
        else
            setText( text() );

        enableRichTextActions();
    }
    else
    {
        KTextEdit::setTextFormat( f );
        TQString t = text();
        setText( t );

        disableRichTextActions();
    }
}

void KNoteEdit::textStrikeOut( bool s )
{
    // TQTextEdit does not support stroke out text (no saving,
    // no changing of more than one selected character)
    TQFont font;

    if ( !hasSelectedText() )
    {
        font = currentFont();
        font.setStrikeOut( s );
        setCurrentFont( font );
    }
    else
    {
        int pFrom, pTo, iFrom, iTo, iF, iT;
        int cp, ci;

        getSelection( &pFrom, &iFrom, &pTo, &iTo );
        getCursorPosition( &cp, &ci );

        for ( int p = pFrom; p <= pTo; p++ )
        {
            iF = 0;
            iT = paragraphLength( p );

            if ( p == pFrom )
                iF = iFrom;

            if ( p == pTo )
                iT = iTo;

            for ( int i = iF; i < iT; i++ )
            {
                setCursorPosition( p, i + 1 );
                setSelection( p, i, p, i + 1 );
                font = currentFont();
                font.setStrikeOut( s );
                setCurrentFont( font );
            }
        }

        setSelection( pFrom, iFrom, pTo, iTo );
        setCursorPosition( cp, ci );
    }
}

void KNoteEdit::textColor()
{
    if ( m_note )
        m_note->blockEmitDataChanged( true );
    TQColor c = color();
    int ret = KColorDialog::getColor( c, this );
    if ( ret == TQDialog::Accepted )
        setTextColor( c );
    if ( m_note )
        m_note->blockEmitDataChanged( false );
}

void KNoteEdit::textAlignLeft()
{
    setAlignment( AlignLeft );
    m_textAlignLeft->setChecked( true );
}

void KNoteEdit::textAlignCenter()
{
    setAlignment( AlignCenter );
    m_textAlignCenter->setChecked( true );
}

void KNoteEdit::textAlignRight()
{
    setAlignment( AlignRight );
    m_textAlignRight->setChecked( true );
}

void KNoteEdit::textAlignBlock()
{
    setAlignment( AlignJustify );
    m_textAlignBlock->setChecked( true );
}

void KNoteEdit::textList()
{
    if ( m_textList->isChecked() )
        setParagType( TQStyleSheetItem::DisplayListItem, TQStyleSheetItem::ListDisc );
    else
        setParagType( TQStyleSheetItem::DisplayBlock, TQStyleSheetItem::ListDisc );
}

void KNoteEdit::textSuperScript()
{
    if ( m_textSuper->isChecked() )
        setVerticalAlignment( AlignSuperScript );
    else
        setVerticalAlignment( AlignNormal );
}

void KNoteEdit::textSubScript()
{
    if ( m_textSub->isChecked() )
        setVerticalAlignment( AlignSubScript );
    else
        setVerticalAlignment( AlignNormal );
}

//void KNoteEdit::textIncreaseIndent()
//{
//}

//void KNoteEdit::textDecreaseIndent()
//{
//}


/** protected methods **/

void KNoteEdit::contentsDragEnterEvent( TQDragEnterEvent *e )
{
    if ( KURLDrag::canDecode( e ) )
        e->accept();
    else
        KTextEdit::contentsDragEnterEvent( e );
}

void KNoteEdit::contentsDropEvent( TQDropEvent *e )
{
    KURL::List list;

    if ( KURLDrag::decode( e, list ) )
    {
	KURL::List::ConstIterator begin = list.constBegin();
	KURL::List::ConstIterator end = list.constEnd();
        for ( KURL::List::ConstIterator it = begin; it != end; ++it )
        {
            if ( it != begin )
                insert( ", " );

            insert( (*it).prettyURL() );
        }
    }
    else
        KTextEdit::contentsDropEvent( e );
}

/** private slots **/

void KNoteEdit::slotReturnPressed()
{
    if ( m_autoIndentMode )
        autoIndent();
}

void KNoteEdit::fontChanged( const TQFont &f )
{
    m_textFont->setFont( f.family() );
    m_textSize->setFontSize( f.pointSize() );

    m_textBold->setChecked( f.bold() );
    m_textItalic->setChecked( f.italic() );
    m_textUnderline->setChecked( f.underline() );
    m_textStrikeOut->setChecked( f.strikeOut() );
}

void KNoteEdit::colorChanged( const TQColor &c )
{
    TQPixmap pix( ICON_SIZE, ICON_SIZE );
    pix.fill( c );
    m_textColor->setIconSet( pix );
}

void KNoteEdit::alignmentChanged( int a )
{
    // TODO: AlignAuto
    if ( ( a == AlignAuto ) || ( a & AlignLeft ) )
        m_textAlignLeft->setChecked( true );
    else if ( ( a & AlignHCenter ) )
        m_textAlignCenter->setChecked( true );
    else if ( ( a & AlignRight ) )
        m_textAlignRight->setChecked( true );
    else if ( ( a & AlignJustify ) )
        m_textAlignBlock->setChecked( true );
}

void KNoteEdit::verticalAlignmentChanged( VerticalAlignment a )
{
    if ( a == AlignNormal )
    {
        m_textSuper->setChecked( false );
        m_textSub->setChecked( false );
    }
    else if ( a == AlignSuperScript )
        m_textSuper->setChecked( true );
    else if ( a == AlignSubScript )
        m_textSub->setChecked( true );
}


/** private methods **/

void KNoteEdit::autoIndent()
{
    int para, index;
    TQString string;
    getCursorPosition( &para, &index );
    while ( para > 0 && string.stripWhiteSpace().isEmpty() )
        string = text( --para );

    if ( string.stripWhiteSpace().isEmpty() )
        return;

    // This routine returns the whitespace before the first non white space
    // character in string.
    // It is assumed that string contains at least one non whitespace character
    // ie \n \r \t \v \f and space
    TQString indentString;

    int len = string.length();
    int i = 0;
    while ( i < len && string.at(i).isSpace() )
        indentString += string.at( i++ );

    if ( !indentString.isEmpty() )
        insert( indentString );
}

void KNoteEdit::emitLinkClicked( const TQString &s )
{
    kdDebug(5500) << k_funcinfo << s << endl;
}

void KNoteEdit::enableRichTextActions()
{
    m_textColor->setEnabled( true );
    m_textFont->setEnabled( true );
    m_textSize->setEnabled( true );

    m_textBold->setEnabled( true );
    m_textItalic->setEnabled( true );
    m_textUnderline->setEnabled( true );
    m_textStrikeOut->setEnabled( true );

    m_textAlignLeft->setEnabled( true );
    m_textAlignCenter->setEnabled( true );
    m_textAlignRight->setEnabled( true );
    m_textAlignBlock->setEnabled( true );

    m_textList->setEnabled( true );
    m_textSuper->setEnabled( true );
    m_textSub->setEnabled( true );

//    m_textIncreaseIndent->setEnabled( true );
//    m_textDecreaseIndent->setEnabled( true );
}

void KNoteEdit::disableRichTextActions()
{
    m_textColor->setEnabled( false );
    m_textFont->setEnabled( false );
    m_textSize->setEnabled( false );

    m_textBold->setEnabled( false );
    m_textItalic->setEnabled( false );
    m_textUnderline->setEnabled( false );
    m_textStrikeOut->setEnabled( false );

    m_textAlignLeft->setEnabled( false );
    m_textAlignCenter->setEnabled( false );
    m_textAlignRight->setEnabled( false );
    m_textAlignBlock->setEnabled( false );

    m_textList->setEnabled( false );
    m_textSuper->setEnabled( false );
    m_textSub->setEnabled( false );

//    m_textIncreaseIndent->setEnabled( false );
//    m_textDecreaseIndent->setEnabled( false );
}

void KNoteEdit::slotAllowTab()
{
    setTabChangesFocus(!tabChangesFocus());
}

TQPopupMenu *KNoteEdit::createPopupMenu( const TQPoint &pos )
{
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    TQPopupMenu *menu = TQTextEdit::createPopupMenu( pos );

    if ( isReadOnly() )
      menu->changeItem( menu->idAt(0), SmallIconSet("editcopy"), menu->text( menu->idAt(0) ) );
    else {
      int id = menu->idAt(0);
      menu->changeItem( id - IdUndo, SmallIconSet("undo"), menu->text( id - IdUndo) );
      menu->changeItem( id - IdRedo, SmallIconSet("redo"), menu->text( id - IdRedo) );
      menu->changeItem( id - IdCut, SmallIconSet("editcut"), menu->text( id - IdCut) );
      menu->changeItem( id - IdCopy, SmallIconSet("editcopy"), menu->text( id - IdCopy) );
      menu->changeItem( id - IdPaste, SmallIconSet("editpaste"), menu->text( id - IdPaste) );
      menu->changeItem( id - IdClear, SmallIconSet("editclear"), menu->text( id - IdClear) );

        menu->insertSeparator();
        id = menu->insertItem( SmallIconSet( "spellcheck" ), i18n( "Check Spelling..." ),
                                   this, TQT_SLOT( checkSpelling() ) );

        if( text().isEmpty() )
            menu->setItemEnabled( id, false );

	menu->insertSeparator();
	id=menu->insertItem(i18n("Allow Tabulations"),this,TQT_SLOT(slotAllowTab()));
	menu->setItemChecked(id, !tabChangesFocus());
    }

    return menu;
}

#include "knoteedit.moc"
