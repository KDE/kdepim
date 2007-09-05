/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2006, The KNotes Developers

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

#include <QFont>
#include <QPixmap>

#include <kdebug.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kfontaction.h>
#include <kfontsizeaction.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <kcolordialog.h>
#include <ktoggleaction.h>
#include <kicon.h>
#include <kurl.h>

#include "knoteedit.h"

static const short SEP = 5;
static const short ICON_SIZE = 10;


KNoteEdit::KNoteEdit( KActionCollection *actions, QWidget *parent )
    : KTextEdit( parent ),
      m_editMenu( 0 )
{
    setAcceptDrops( true );
    setWordWrapMode( QTextOption::WordWrap );
    setLineWrapMode( WidgetWidth );
    setAutoFormatting( AutoAll );

    // create the actions for the RMB menu
    QAction * undo = KStandardAction::undo( this, SLOT(undo()), actions );
    QAction * redo = KStandardAction::redo( this, SLOT(redo()), actions );
    undo->setEnabled( document()->isUndoAvailable() );
    redo->setEnabled( document()->isRedoAvailable() );

    m_cut = KStandardAction::cut( this, SLOT(cut()), actions );
    m_copy = KStandardAction::copy( this, SLOT(copy()), actions );
    m_paste = KStandardAction::paste( this, SLOT(paste()), actions );

    m_cut->setEnabled( false );
    m_copy->setEnabled( false );
    m_paste->setEnabled( true );

    connect( this, SIGNAL(undoAvailable(bool)), undo, SLOT(setEnabled(bool)) );
    connect( this, SIGNAL(redoAvailable(bool)), redo, SLOT(setEnabled(bool)) );

    connect( this, SIGNAL(copyAvailable(bool)), m_cut, SLOT(setEnabled(bool)) );
    connect( this, SIGNAL(copyAvailable(bool)), m_copy, SLOT(setEnabled(bool)) );

    KStandardAction::clear( this, SLOT(clear()), actions );
    KStandardAction::selectAll( this, SLOT(selectAll()), actions );

    // create the actions modifying the text format
    m_textBold  = new KToggleAction(KIcon("format-text-bold"), i18n("Bold"), this);
    actions->addAction("format_bold", m_textBold );
    m_textBold->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    m_textItalic  = new KToggleAction(KIcon("format-text-italic"), i18n("Italic"), this);
    actions->addAction("format_italic", m_textItalic );
    m_textItalic->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    m_textUnderline  = new KToggleAction(KIcon("format-text-underline"), i18n("Underline"), this);
    actions->addAction("format_underline", m_textUnderline );
    m_textUnderline->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    m_textStrikeOut  = new KToggleAction(KIcon("format-text-strikethrough"), i18n("Strike Out"), this);
    actions->addAction("format_strikeout", m_textStrikeOut );
    m_textStrikeOut->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

    connect( m_textBold, SIGNAL(toggled(bool)), SLOT(textBold(bool)) );
    connect( m_textItalic, SIGNAL(toggled(bool)), SLOT(setFontItalic(bool)) );
    connect( m_textUnderline, SIGNAL(toggled(bool)), SLOT(setFontUnderline(bool)) );
    connect( m_textStrikeOut, SIGNAL(toggled(bool)), SLOT(textStrikeOut(bool)) );

    m_textAlignLeft  = new KToggleAction(KIcon("text_left"), i18n("Align Left"), this);
    actions->addAction("format_alignleft", m_textAlignLeft );
    connect(m_textAlignLeft, SIGNAL(triggered(bool) ), SLOT(textAlignLeft()));
    m_textAlignLeft->setShortcut(QKeySequence(Qt::ALT + Qt::Key_L));
    m_textAlignLeft->setChecked( true ); // just a dummy, will be updated later
    m_textAlignCenter  = new KToggleAction(KIcon("text_center"), i18n("Align Center"), this);
    actions->addAction("format_aligncenter", m_textAlignCenter );
    connect(m_textAlignCenter, SIGNAL(triggered(bool) ), SLOT(textAlignCenter()));
    m_textAlignCenter->setShortcut(QKeySequence(Qt::ALT + Qt::Key_C));
    m_textAlignRight  = new KToggleAction(KIcon("text_right"), i18n("Align Right"), this);
    actions->addAction("format_alignright", m_textAlignRight );
    connect(m_textAlignRight, SIGNAL(triggered(bool) ), SLOT(textAlignRight()));
    m_textAlignRight->setShortcut(QKeySequence(Qt::ALT + Qt::Key_R));
    m_textAlignBlock  = new KToggleAction(KIcon("format-justify-fill"), i18n("Align Block"), this);
    actions->addAction("format_alignblock", m_textAlignBlock );
    connect(m_textAlignBlock, SIGNAL(triggered(bool) ), SLOT(textAlignBlock()));
    m_textAlignBlock->setShortcut(QKeySequence(Qt::ALT + Qt::Key_B));

    QActionGroup *group = new QActionGroup( this );
    group->addAction( m_textAlignLeft );
    group->addAction( m_textAlignCenter );
    group->addAction( m_textAlignRight );
    group->addAction( m_textAlignBlock );

    m_textList  = new KToggleAction(KIcon("enum_list"), i18n("List"), this);
    actions->addAction("format_list", m_textList );
    connect(m_textList, SIGNAL(triggered(bool) ), SLOT(textList()));

    group = new QActionGroup( this );
    group->addAction( m_textList );

    m_textSuper  = new KToggleAction(KIcon("text_super"), i18n("Superscript"), this);
    actions->addAction("format_super", m_textSuper );
    connect(m_textSuper, SIGNAL(triggered(bool) ), SLOT(textSuperScript()));
    m_textSub  = new KToggleAction(KIcon("text_sub"), i18n("Subscript"), this);
    actions->addAction("format_sub", m_textSub );
    connect(m_textSub, SIGNAL(triggered(bool) ), SLOT(textSubScript()));

    group = new QActionGroup( this );
    group->addAction( m_textSuper );
    group->addAction( m_textSub );

    m_textIncreaseIndent = new KAction( i18n( "Increase Indent" ), this );
    actions->addAction( "format_increaseindent", m_textIncreaseIndent );
    m_textIncreaseIndent->setShortcut( QKeySequence( Qt::CTRL + Qt::ALT + Qt::Key_I ) );
    connect( m_textIncreaseIndent, SIGNAL( triggered( bool ) ), SLOT( textIncreaseIndent() ) );

    m_textDecreaseIndent = new KAction( i18n( "Decrease Indent" ), this );
    actions->addAction( "format_decreaseindent", m_textDecreaseIndent );
    m_textDecreaseIndent->setShortcut( QKeySequence( Qt::CTRL + Qt::ALT + Qt::Key_D ) );
    connect( m_textDecreaseIndent, SIGNAL( triggered( bool ) ), SLOT( textDecreaseIndent() ) );

    group = new QActionGroup( this );
    group->addAction( m_textIncreaseIndent );
    group->addAction( m_textDecreaseIndent );

    QPixmap pix( ICON_SIZE, ICON_SIZE );
    pix.fill( Qt::black );     // just a dummy, gets updated before widget is shown
    m_textColor  = new KAction(i18n("Text Color..."), this);
    actions->addAction("format_color", m_textColor );
    ((QAction*)m_textColor)->setIcon(pix);
    connect(m_textColor, SIGNAL(triggered(bool)), SLOT(slotTextColor()));

    m_textFont  = new KFontAction(i18n("Text Font"), this);
    actions->addAction("format_font", m_textFont );
    connect( m_textFont, SIGNAL(triggered( const QString & )),
             this, SLOT(setFontFamily( const QString & )) );

    m_textSize  = new KFontSizeAction(i18n("Text Size"), this);
    actions->addAction("format_size", m_textSize );
    connect( m_textSize, SIGNAL(fontSizeChanged( int )),
             this, SLOT(setFontWeight ( int )) );

    // QTextEdit connections
    connect( this, SIGNAL(currentCharFormatChanged( const QTextCharFormat & )),
             this, SLOT(slotCurrentCharFormatChanged( const QTextCharFormat & )) );

#ifdef __GNUC__
#warning moving the cursor through alignment changes does not update the button!
#endif
    slotCurrentCharFormatChanged( currentCharFormat() );
}

KNoteEdit::~KNoteEdit()
{
}

void KNoteEdit::setText( const QString& text )
{
    if ( acceptRichText() && Qt::mightBeRichText( text ) )
        setHtml( text );
    else
        setPlainText( text );
}

QString KNoteEdit::text() const
{
    if ( acceptRichText() )
        return toHtml();
    else
        return toPlainText();
}

void KNoteEdit::setTextFont( const QFont& font )
{
    QTextCharFormat f;
    f.setFont( font );
    setTextFormat( f );
}

void KNoteEdit::setTextColor( const QColor& color )
{
    QTextCharFormat f;
    f.setForeground( QBrush( color ) );
    setTextFormat( f );
}

void KNoteEdit::setTabStop( int tabs )
{
    QFontMetrics fm( font() );
    setTabStopWidth( fm.width( 'x' ) * tabs );
}

void KNoteEdit::setAutoIndentMode( bool newmode )
{
    m_autoIndentMode = newmode;
}


/** public slots **/

void KNoteEdit::setRichText( bool f )
{
    if ( f == acceptRichText() )
        return;

    setAcceptRichText( f );

    QString t = toPlainText();
    if ( f )
    {
        // if the note contains html source try to render it
        if ( Qt::mightBeRichText( t ) )
            setHtml( t );
        else
            setPlainText( t );

        enableRichTextActions();
    }
    else
    {
        setPlainText( t );
        disableRichTextActions();
    }
}

void KNoteEdit::textBold( bool b )
{
    QTextCharFormat f;
    f.setFontWeight( b ? QFont::Bold : QFont::Normal );
    mergeCurrentCharFormat( f );
}

void KNoteEdit::textStrikeOut( bool s )
{
    QTextCharFormat f;
    f.setFontStrikeOut( s );
    mergeCurrentCharFormat( f );
}

void KNoteEdit::slotTextColor()
{
    QColor c = textColor();
    int ret = KColorDialog::getColor( c, this );
    if ( ret == QDialog::Accepted )
        setTextColor( c );
}

void KNoteEdit::textAlignLeft()
{
    setAlignment( Qt::AlignLeft );
    m_textAlignLeft->setChecked( true );
}

void KNoteEdit::textAlignCenter()
{
    setAlignment( Qt::AlignCenter );
    m_textAlignCenter->setChecked( true );
}

void KNoteEdit::textAlignRight()
{
    setAlignment( Qt::AlignRight );
    m_textAlignRight->setChecked( true );
}

void KNoteEdit::textAlignBlock()
{
    setAlignment( Qt::AlignJustify );
    m_textAlignBlock->setChecked( true );
}

void KNoteEdit::textList()
{
    QTextCursor c = textCursor();
    c.beginEditBlock();

    if ( m_textList->isChecked() )
    {
        QTextListFormat lf;
        QTextBlockFormat bf = c.blockFormat();

        lf.setIndent( bf.indent() + 1 );
        bf.setIndent( 0 );

        lf.setStyle( QTextListFormat::ListDisc );

        c.setBlockFormat( bf );
        c.createList( lf );
    }
    else
    {
        QTextBlockFormat bf;
        bf.setObjectIndex( -1 );
        c.mergeBlockFormat( bf );
    }

    c.endEditBlock();
}

void KNoteEdit::textSuperScript()
{
    QTextCharFormat f;

    if ( m_textSuper->isChecked() )
        f.setVerticalAlignment( QTextCharFormat::AlignSuperScript );
    else
        f.setVerticalAlignment( QTextCharFormat::AlignNormal );

    mergeCurrentCharFormat( f );
}

void KNoteEdit::textSubScript()
{
    QTextCharFormat f;

    if ( m_textSub->isChecked() )
        f.setVerticalAlignment( QTextCharFormat::AlignSubScript );
    else
        f.setVerticalAlignment( QTextCharFormat::AlignNormal );

    mergeCurrentCharFormat( f );
}

void KNoteEdit::textIncreaseIndent()
{
    QTextBlockFormat f = textCursor().blockFormat();
    f.setIndent( f.indent() + 1 );
    textCursor().setBlockFormat( f );
}

void KNoteEdit::textDecreaseIndent()
{
    QTextBlockFormat f = textCursor().blockFormat();
    short int curIndent = f.indent();

    if ( curIndent > 0 )
        f.setIndent( curIndent - 1 );

    textCursor().setBlockFormat( f );
}


/** protected methods **/

void KNoteEdit::contextMenuEvent( QContextMenuEvent *e )
{
    if ( m_editMenu )
        m_editMenu->popup( e->globalPos() );
}

void KNoteEdit::dragEnterEvent( QDragEnterEvent *e )
{
    const QMimeData *md = e->mimeData();
    if ( KUrl::List::canDecode( md ) )
        e->accept();
    else
        KTextEdit::dragEnterEvent( e );
}

void KNoteEdit::dropEvent( QDropEvent *e )
{
    const QMimeData *md = e->mimeData();
    if ( KUrl::List::canDecode( md ) ) {
        KUrl::List list = KUrl::List::fromMimeData( md );
        for ( KUrl::List::Iterator it = list.begin(); it != list.end(); ++it )
        {
            if ( it != list.begin() )
                insertPlainText( ", " );

            insertPlainText( (*it).prettyUrl() );
        }
    } else
        KTextEdit::dropEvent( e );
}

void KNoteEdit::keyPressEvent( QKeyEvent *e )
{
    KTextEdit::keyPressEvent( e );

    if ( m_autoIndentMode && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) )
        autoIndent();
}

void KNoteEdit::focusInEvent( QFocusEvent *e )
{
    KTextEdit::focusInEvent( e );

    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
}

void KNoteEdit::focusOutEvent( QFocusEvent *e )
{
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    KTextEdit::focusOutEvent( e );
}

/** private slots **/

void KNoteEdit::slotCurrentCharFormatChanged( const QTextCharFormat& f )
{
    // font changes
    m_textFont->setFont( f.fontFamily() );
    m_textSize->setFontSize( (int) f.fontPointSize() );

    m_textBold->setChecked( f.font().bold() );
    m_textItalic->setChecked( f.fontItalic() );
    m_textUnderline->setChecked( f.fontUnderline() );
    m_textStrikeOut->setChecked( f.fontStrikeOut() );

    // color changes
    QPixmap pix( ICON_SIZE, ICON_SIZE );
    pix.fill( f.foreground().color() );
    m_textColor->QAction::setIcon( pix );

    // alignment changes
    Qt::Alignment a = alignment();
    if ( a & Qt::AlignLeft )
        m_textAlignLeft->setChecked( true );
    else if ( a & Qt::AlignHCenter )
        m_textAlignCenter->setChecked( true );
    else if ( a & Qt::AlignRight )
        m_textAlignRight->setChecked( true );
    else if ( a & Qt::AlignJustify )
        m_textAlignBlock->setChecked( true );

    // vertical alignment changes
    QTextCharFormat::VerticalAlignment va = f.verticalAlignment();
    if ( va == QTextCharFormat::AlignNormal )
    {
        m_textSuper->setChecked( false );
        m_textSub->setChecked( false );
    }
    else if ( va == QTextCharFormat::AlignSuperScript )
        m_textSuper->setChecked( true );
    else if ( va == QTextCharFormat::AlignSubScript )
        m_textSub->setChecked( true );
}


/** private methods **/

void KNoteEdit::autoIndent()
{
    QTextCursor c = textCursor();
    QTextBlock b = c.block();

    QString string;
    while ( b.previous().length() > 0 && string.trimmed().isEmpty() )
    {
        b = b.previous();
        string = b.text();
    }

    if ( string.trimmed().isEmpty() )
        return;

    // This routine returns the whitespace before the first non white space
    // character in string.
    // It is assumed that string contains at least one non whitespace character
    // ie \n \r \t \v \f and space
    QString indentString;

    int len = string.length();
    int i = 0;
    while ( i < len && string.at(i).isSpace() )
        indentString += string.at( i++ );

    if ( !indentString.isEmpty() )
        c.insertText( indentString );
}

void KNoteEdit::setTextFormat( const QTextCharFormat& f )
{
    if ( acceptRichText() )
        textCursor().mergeCharFormat( f );
    else
    {
        QTextCursor c( document() );
        c.movePosition( QTextCursor::Start );
        c.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );
        c.mergeCharFormat( f );
    }
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

    m_textIncreaseIndent->setEnabled( true );
    m_textDecreaseIndent->setEnabled( true );
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

    m_textIncreaseIndent->setEnabled( false );
    m_textDecreaseIndent->setEnabled( false );
}

#include "knoteedit.moc"
