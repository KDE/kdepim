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

#include <qlabel.h>
#include <qsizegrip.h>
#include <qpalette.h>
#include <qcolor.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qbitmap.h>
#include <qpointarray.h>
#include <qpaintdevicemetrics.h>

#include <kaction.h>
#include <kstdaction.h>
#include <kxmlgui.h>
#include <kprinter.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <klineeditdlg.h>
#include <kpopupmenu.h>
#include <kmdcodec.h>
#include <kio/netaccess.h>

#include "knote.h"
#include "knotebutton.h"
#include "knoteedit.h"
#include "knoteconfigdlg.h"
#include "qrichtext_p.h"

#include <kwin.h>
#include <netwm.h>

// fscking X headers
#ifdef FocusIn
#undef FocusIn
#endif
#ifdef FocusOut
#undef FocusOut
#endif


// -------------------- Initialisation -------------------- //
KNote::KNote( KXMLGUIBuilder* builder, QDomDocument buildDoc, const QString& file,
              bool load, QWidget* parent, const char* name )
  : QFrame( parent, name, WStyle_Customize | WStyle_NoBorderEx | WDestructiveClose ),
      m_noteDir( KGlobal::dirs()->saveLocation( "appdata", "notes/" ) ),
      m_configFile( file )
{
    // create the menu items for the note - not the editor...
    // rename, mail, print, insert date, close, delete, new note
    new KAction( i18n("New"), "filenew", 0, this, SLOT(slotNewNote()), actionCollection(), "new_note" );
    new KAction( i18n("Rename"), "text", 0, this, SLOT(slotRename()), actionCollection(), "rename_note" );
    new KAction( i18n("Delete"), "knotesdelete", 0, this, SLOT(slotKill()), actionCollection(), "delete_note" );

    new KAction( i18n("Insert Date"), 0, this, SLOT(slotInsDate()), actionCollection(), "insert_date" );
    new KAction( i18n("Mail"), "mail_send", 0, this, SLOT(slotMail()), actionCollection(), "mail_note" );
    new KAction( i18n("Print"), "fileprint", 0, this, SLOT(slotPrint()), actionCollection(), "print_note" );
    new KAction( i18n("Note Preferences..."), "configure", 0, this, SLOT(slotPreferences()), actionCollection(), "configure_note" );

    m_alwaysOnTop = new KToggleAction( i18n("Always On Top"), "attach", 0, this, SLOT(slotToggleAlwaysOnTop()), actionCollection(), "always_on_top" );
    connect( m_alwaysOnTop, SIGNAL(toggled(bool)), m_alwaysOnTop, SLOT(setChecked(bool)) );
    m_toDesktop = new KListAction( i18n("To Desktop"), 0, this, SLOT(slotToDesktop(int)), actionCollection(), "to_desktop" );
    connect( m_toDesktop->popupMenu(), SIGNAL(aboutToShow()), this, SLOT(slotUpdateDesktopActions()) );

    // create the note header, button and label...
    m_button = new KNoteButton( this );
    m_button->setPixmap( BarIcon( "knotesclose" ) );
    connect( m_button, SIGNAL( clicked() ), this, SLOT( slotClose() ) );

    m_label = new QLabel( this );
    m_label->setAlignment( AlignHCenter );
    m_label->installEventFilter( this );  // recieve events (for dragging & action menu)

    // create the note editor
    m_editor = new KNoteEdit( this );
    m_editor->installEventFilter( this ); // recieve events (for modified)
    m_editor->viewport()->installEventFilter( this );

    setDOMDocument( buildDoc );
    factory = new KXMLGUIFactory( builder, this, "guifactory" );
    factory->addClient( this );

    m_menu = static_cast<KPopupMenu*>(factory->container( "note_context", this ));
    m_edit_menu = static_cast<KPopupMenu*>(factory->container( "note_edit", this ));

    setFocusProxy( m_editor );

    // create the resize handle
    m_editor->setCornerWidget( new QSizeGrip( this ) );
    int width = m_editor->cornerWidget()->width();
    int height = m_editor->cornerWidget()->height();
    QBitmap mask;
    mask.resize( width, height );
    mask.fill( color0 );
    QPointArray array;
    array.setPoints( 3, 0, height, width, height, width, 0 );
    QPainter p;
    p.begin( &mask );
    p.setBrush( color1 );
    p.drawPolygon( array );
    p.end();
    m_editor->cornerWidget()->setMask( mask );

    // set up the look&feel of the note
    setFrameStyle( WinPanel | Raised );
    setLineWidth( 1 );
    setMinimumSize( 20, 20 );
    m_editor->setMargin( 5 );

    // now create or load the data and configuration
    bool oldconfig = false;
    // WARNING: if the config file doesn't exist a new note will be created!
    if ( load && m_noteDir.exists( m_configFile ) )
    {
        KSimpleConfig* test = new KSimpleConfig( m_noteDir.absFilePath( m_configFile ), true );
        test->setGroup( "General" );
        oldconfig = ( test->readNumEntry( "version", 1 ) == 1 );
        delete test;
    }
    else
    {
        m_label->setText( m_configFile );

        // set the new configfile's name...
        for ( int i = 1; ; i++ )
        {
            m_configFile = QString( "KNote %1" ).arg(i);
            if ( !m_noteDir.exists( m_configFile ) )
                break;
        }

        // ...and "fill" it with the default config
        KIO::NetAccess::copy( KURL( KGlobal::dirs()->findResource( "config", "knotesrc" ) ),
                              KURL( m_noteDir.absFilePath( m_configFile ) ) );
    }

    if ( oldconfig ) {
        //read and convert the old configuration
        convertOldConfig();
    } else {
        // load the display configuration of the note
        KSimpleConfig config( m_noteDir.absFilePath( m_configFile ) );
        config.setGroup( "Display" );
        uint width  = config.readUnsignedNumEntry( "width", 200 );
        uint height = config.readUnsignedNumEntry( "height", 200 );
        resize( width, height );

        config.setGroup( "WindowDisplay" );
        int note_desktop = config.readNumEntry( "desktop", KWin::currentDesktop() );
        ulong note_state = config.readUnsignedLongNumEntry( "state", NET::SkipTaskbar );
        QPoint default_position = QPoint( -1, -1 );
        QPoint position  = config.readPointEntry( "position", &default_position );

        KWin::setState( winId(), note_state );
        if ( note_state & NET::StaysOnTop )
            m_alwaysOnTop->setChecked( true );

        if ( position != default_position )
            move( position );                    // do before calling show() to avoid flicker

        // read configuration settings...
        slotApplyConfig();

        // show the note if desired
        if ( note_desktop != 0 && !isVisible() )
        {
            // HACK HACK
            if( note_desktop != NETWinInfo::OnAllDesktops )
            {
                // to avoid flicker, call this before show()
                slotToDesktop( note_desktop );
                show();
            } else {
                show();
                // if this is called before show(),
                // it won't work for sticky notes!!!
                slotToDesktop( note_desktop );
            }
        }

        // load the saved text and put it in m_editor...
        QString datafile = "." + m_configFile + "_data";
        if ( m_noteDir.exists( datafile ) )
        {
            QString absfile = m_noteDir.absFilePath( datafile );
            m_editor->readFile( absfile );
        }
    }
}

KNote::~KNote()
{
    emit sigKilled( m_label->text() );

    delete m_menu;
    delete m_edit_menu;
}


// -------------------- public member functions -------------------- //

void KNote::saveData() const
{
    QString datafile = m_noteDir.absFilePath( "." + m_configFile + "_data" );
    m_editor->dumpToFile( datafile );
    m_editor->setModified( false );
}

void KNote::saveConfig() const
{
    //all that needs to get saved here is the size and name
    //everything else would have been saved by the preferences dialog
    KSimpleConfig config( m_noteDir.absFilePath( m_configFile ) );

    //store config settings...
    //need to save the new size to KSimpleConfig object
    config.setGroup( "Display" );
    config.writeEntry( "width", width() );
    config.writeEntry( "height", height() );

    //save name....
    config.setGroup( "Data" );
    config.writeEntry( "name", m_label->text() );
}

void KNote::saveDisplayConfig() const
{
    KSimpleConfig config( m_noteDir.absFilePath( m_configFile ) );
    NETWinInfo wm_client( qt_xdisplay(), winId(), qt_xrootwin(), NET::WMDesktop | NET::WMState );

    config.setGroup( "WindowDisplay" );
    config.writeEntry( "desktop", wm_client.desktop() );
    config.writeEntry( "state", wm_client.state() );
    config.writeEntry( "position", pos() );
}

int KNote::noteId() const
{
    return m_configFile.mid( 6 ).toInt();
}

QString KNote::name() const
{
    return m_label->text();
}

QString KNote::text() const
{
    return m_editor->text();
}

void KNote::setName( const QString& name )
{
    m_label->setText( name );

    saveConfig();
}

void KNote::setText( const QString& text )
{
    m_editor->setText( text );
}

void KNote::sync( const QString& app )
{
    QByteArray sep( 1 );
    sep[0] = '\0';

    KMD5 hash;
    HASHHEX result;

    hash.update( m_label->text().utf8() );
    hash.update( sep );
    hash.update( m_editor->text().utf8() );
    hash.finalize();
    hash.hexDigest( result );

    if ( !hash.hasErrored() )
    {
        KSimpleConfig config( m_noteDir.absFilePath( m_configFile ) );

        config.setGroup( "Synchronisation" );
        config.writeEntry( app, result );
    }
    else
        kdWarning() << "Couldn't calculate digest because of an error!" << endl;

    hash.reset();
}

bool KNote::isNew( const QString& app ) const
{
    KSimpleConfig config( m_noteDir.absFilePath( m_configFile ) );

    config.setGroup( "Synchronisation" );
    QString hash = config.readEntry( app );
    return hash.isEmpty();
}

bool KNote::isModified( const QString& app ) const
{
    QByteArray sep( 1 );
    sep[0] = '\0';

    KMD5 hash;
    hash.update( m_label->text().utf8() );
    hash.update( sep );
    hash.update( m_editor->text().utf8() );
    hash.finalize();
    hash.hexDigest();

    KSimpleConfig config( m_noteDir.absFilePath( m_configFile ) );
    config.setGroup( "Synchronisation" );
    QString orig = config.readEntry( app );

    if ( hash.verify( orig.utf8() ) )   // returns false on error!
        return false;
    else
        return true;
}


// -------------------- public slots -------------------- //

void KNote::slotNewNote()
{
    emit sigNewNote();
}

void KNote::slotRename()
{
    //pop up dialog to get the new name
    bool ok;
    QString newname = KLineEditDlg::getText( i18n("Please enter the new name"),
                                             m_label->text(), &ok, this );
    if ( !ok ) // handle cancel
        return;

    if ( newname.isEmpty() ) {
        KMessageBox::sorry( this, i18n("A name must have at least one character") );
        return;
    }

    emit sigRenamed( m_label->text(), newname );
}

void KNote::slotClose()
{
    m_editor->clearFocus();
    hide(); //just hide the note so it's still available from the dock window
}

void KNote::slotKill()
{
    if ( !m_noteDir.remove( m_configFile ) )
        kdWarning() << "could not remove conf file for note " << m_label->text() << endl;

    if ( !m_noteDir.remove( "." + m_configFile + "_data" ) )
        kdWarning() << "could not remove data file for note " << m_label->text() << endl;

    delete this;
}

void KNote::slotInsDate()
{
    m_editor->insert( KGlobal::locale()->formatDateTime(QDateTime::currentDateTime()) );
}

void KNote::slotPreferences()
{
    saveConfig();

    //launch preferences dialog...
    KNoteConfigDlg configDlg( m_noteDir.absFilePath( m_configFile ),
                              i18n("Local Settings"), false );
    connect( &configDlg, SIGNAL( updateConfig() ), this, SLOT( slotApplyConfig() ) );

    configDlg.show();
}

void KNote::slotToggleAlwaysOnTop()
{
    if ( KWin::info(winId()).state & NET::StaysOnTop )
        KWin::clearState( winId(), NET::StaysOnTop );
    else
        KWin::setState( winId(), KWin::info(winId()).state | NET::StaysOnTop );
}

void KNote::slotToDesktop( int id )
{
    if ( id == 0 || id == NETWinInfo::OnAllDesktops )
        KWin::setOnAllDesktops( winId(), true );
    else
        KWin::setOnDesktop( winId(), id );
}

void KNote::slotUpdateDesktopActions()
{
    NETRootInfo wm_root( qt_xdisplay(), NET::NumberOfDesktops | NET::DesktopNames );
    NETWinInfo wm_client( qt_xdisplay(), winId(), qt_xrootwin(), NET::WMDesktop );

    QStringList desktops;
    desktops.append( i18n("&All desktops") );
    desktops.append( QString::null );           // Separator

    int count = wm_root.numberOfDesktops();
    for ( int n = 1; n <= count; n++ )
        desktops.append( QString("&%1 %2").arg( n ).arg( QString::fromUtf8(wm_root.desktopName( n )) ) );

    m_toDesktop->setItems( desktops );

    if ( wm_client.desktop() == NETWinInfo::OnAllDesktops )
        m_toDesktop->setCurrentItem( 0 );
    else
        m_toDesktop->setCurrentItem( wm_client.desktop() );
}

void KNote::slotMail() const
{
    saveData();
    KSimpleConfig config( m_noteDir.absFilePath( m_configFile ), true );

    //sync up the data on note and the data file
    QString msg_body = m_noteDir.absFilePath( "." + m_configFile + "_data" );

    //get the mail action command
    config.setGroup( "Actions" );
    QString mail_cmd = config.readEntry( "mail", "kmail --msg %f" );
    QStringList cmd_list = QStringList::split( QChar(' '), mail_cmd );

    KProcess mail;
    for ( QStringList::Iterator it = cmd_list.begin();
        it != cmd_list.end(); ++it )
    {
        if ( *it == "%f" )
            mail << msg_body;
        else if ( *it == "%t" )
            mail << m_label->text();
        else
            mail << *it;
    }

    if ( !mail.start( KProcess::DontCare ) )
    {
        // TODO: use KMessageBox!
        kdDebug() << "could not start process" << endl;
    }
}

void KNote::slotPrint() const
{
    saveData();

    KPrinter printer;
    printer.setFullPage( true );

    if ( printer.setup() )
    {
        KSimpleConfig config( m_noteDir.absFilePath( m_configFile ), true );

        // TODO
        const int margin = 40;  // pt
        QFont font( "helvetica" );
        font = config.readFontEntry( "font", &font );

        QPainter painter;
        painter.begin( &printer );

        QPaintDeviceMetrics metrics( painter.device() );
        int marginX = margin * metrics.logicalDpiX() / 72;
        int marginY = margin * metrics.logicalDpiY() / 72;

        QRect body( marginX, marginY,
                    metrics.width() - marginX * 2,
                    metrics.height() - marginY * 2 );

        QTextDocument* textDoc = new QTextDocument( 0 );
        textDoc->setFormatter( new QTextFormatterBreakWords );
        textDoc->setDefaultFont( font );        // only needed for the pointsize
        textDoc->setUnderlineLinks( true );
        textDoc->setStyleSheet( m_editor->styleSheet() );
        textDoc->setMimeSourceFactory( m_editor->mimeSourceFactory() );
        textDoc->flow()->setPageSize( body.height() );
        textDoc->setPageBreakEnabled( true );
        textDoc->setText( m_editor->text(), m_editor->context() );

        textDoc->doLayout( &painter, body.width() );

        QRect view( body );

        int page = 1;
        int x = body.left();
        int y = body.top();

        for (;;) {
            painter.translate( x, y );
            view.moveBy( 0, -y );
            textDoc->draw( &painter, view, colorGroup() );
            view.moveBy( 0, y );
            painter.translate( -x, -y );

            view.moveBy( 0, body.height() );
            painter.translate( 0 , -body.height() );

            // page numbers
            painter.setFont( font );
            painter.drawText(
                view.right() - painter.fontMetrics().width( QString::number( page ) ),
                view.bottom() + painter.fontMetrics().ascent() + 5, QString::number( page )
            );

            if ( view.top()  >= textDoc->height() )
                break;

            printer.newPage();
            page++;
        }

        painter.end();
    }
}


// -------------------- private slots -------------------- //

void KNote::slotApplyConfig()
{
    KSimpleConfig config( m_noteDir.absFilePath( m_configFile ) );

    //do the Editor group: tabsize, autoindent, font, fontsize, fontstyle
    config.setGroup( "Editor" );

    QFont def( "helvetica" );
    def = config.readFontEntry( "font", &def );
    m_editor->setTextFont( def );

    def = QFont( "helvetica" );
    def = config.readFontEntry( "titlefont", &def );
    m_label->setFont( def );

    uint tab_size = config.readUnsignedNumEntry( "tabsize", 4 );
    m_editor->setTabStop( tab_size );

    bool indent = true;
    indent = config.readBoolEntry( "autoindent", &indent );
    m_editor->setAutoIndentMode( indent );

    //do the Data Group- name, data
    config.setGroup( "Data" );

    // TODO
    if ( m_label->text().isEmpty() )
    {
        QString notename = config.readEntry( "name", m_configFile );
        m_label->setText( notename );
    }

    // do Display group - bgcolor, fgcolor, transparent
    config.setGroup( "Display" );

    // create a pallete...
    QColor bg = config.readColorEntry( "bgcolor", &(Qt::yellow) );
    QColor fg = config.readColorEntry( "fgcolor", &(Qt::black) );

    QPalette newpalette = palette();
    newpalette.setColor( QColorGroup::Background, bg );
    newpalette.setColor( QColorGroup::Foreground, fg );
    newpalette.setColor( QColorGroup::Base,       bg ); // text background
    newpalette.setColor( QColorGroup::Text,       fg ); // text color

    // the shadow
    newpalette.setColor( QColorGroup::Midlight, bg.light(110) );
    newpalette.setColor( QColorGroup::Shadow, bg.dark(116) );
    newpalette.setColor( QColorGroup::Light, bg.light(180) );
    newpalette.setColor( QColorGroup::Dark, bg.dark(108) );
    setPalette( newpalette );

    // set the text color
    m_editor->setTextColor( fg );

    // set darker values for the label and button...
    m_button->setBackgroundColor( bg.dark(116) );
    if ( hasFocus() )
    {
        m_label->setBackgroundColor( bg.dark(116) );
        m_button->show();
        m_editor->cornerWidget()->show();
    }
    else
    {
        m_label->setBackgroundColor( bg );
        m_button->hide();
        m_editor->cornerWidget()->hide();
    }
}


// -------------------- private methods -------------------- //

void KNote::convertOldConfig()
{
    QFile infile( m_noteDir.absFilePath( m_configFile ) );

    if ( infile.open( IO_ReadOnly ) )
    {
        QTextStream input( &infile );

        // get the name
        m_label->setText( input.readLine() );

        // get the geometry
        QString geo = input.readLine();

        int pos, data[13];
        int n = 0;

        while ( (pos = geo.find('+')) != -1 )
        {
            if( n < 13 )
                data[n++] = geo.left(pos).toInt();
            geo.remove( 0, pos + 1 );
        }
        if ( n < 13 )
            data[n++] = geo.toInt();

        int note_desktop = data[0];
        if ( data[11] == 1 )
            note_desktop = NETWinInfo::OnAllDesktops;

        resize( data[3], data[4] );
        if ( data[1] >= 0 && data[2] >= 0 )   // just to be sure...
            move( data[1], data[2] );

        if ( data[12] & 2048 )
        {
            KWin::setState( winId(), NET::StaysOnTop | NET::SkipTaskbar );
            m_alwaysOnTop->setChecked( true );
        }
        else
            KWin::setState( winId(), NET::SkipTaskbar );

        // get the foreground color
        uint red = input.readLine().toUInt();
        uint green = input.readLine().toUInt();
        uint blue = input.readLine().toUInt();
        QColor bg = QColor( red, green, blue );

        // get the background color
        red = input.readLine().toUInt();
        green = input.readLine().toUInt();
        blue = input.readLine().toUInt();
        QColor fg = QColor( red, green, blue );

        QPalette newpalette = palette();
        newpalette.setColor( QColorGroup::Background, bg );
        newpalette.setColor( QColorGroup::Foreground, fg );
        newpalette.setColor( QColorGroup::Base,       bg ); // text background
        newpalette.setColor( QColorGroup::Text,       fg ); // text color

        // the shadow
        newpalette.setColor( QColorGroup::Midlight, bg.light(110) );
        newpalette.setColor( QColorGroup::Shadow, bg.dark(116) );
        newpalette.setColor( QColorGroup::Light, bg.light(180) );
        newpalette.setColor( QColorGroup::Dark, bg.dark(108) );
        setPalette( newpalette );

        m_editor->setTextColor( fg );

        // set darker values for the label and button...
        m_button->setBackgroundColor( bg.dark(116) );
        if ( hasFocus() )
        {
            m_label->setBackgroundColor( bg.dark(116) );
            m_button->show();
            m_editor->cornerWidget()->show();
        }
        else
        {
            m_label->setBackgroundColor( bg );
            m_button->hide();
            m_editor->cornerWidget()->hide();
        }

        // get the font
        QString fontfamily = input.readLine();
        if ( fontfamily.isEmpty() )
            fontfamily = QString( "helvetica" );
        uint size = input.readLine().toUInt();
        size = QMAX( size, 4 );
        uint weight = input.readLine().toUInt();
        bool italic = ( input.readLine().toUInt() == 1 );

        QFont font( fontfamily, size, weight, italic );
        m_label->setFont( font );
        m_editor->setTextFont( font );

        // 3d frame? Not supported yet!
        input.readLine();

        // autoindent
        bool indent = ( input.readLine().toUInt() == 1 );
        m_editor->setAutoIndentMode( indent );
        m_editor->setTabStop( 4 );

        // hidden
        bool hidden = ( input.readLine().toUInt() == 1 );

        // show the note
        if ( !hidden && !isVisible() )
        {
            // HACK HACK
            if ( note_desktop != NETWinInfo::OnAllDesktops )
            {
                // to avoid flicker, call this before show()
                slotToDesktop( note_desktop );
                show();
            } else {
                show();
                // if this is called before show(), it won't work for sticky notes!!!
                slotToDesktop( note_desktop );
            }
        }

        // get the text
        while ( !input.atEnd() )
            m_editor->insertParagraph( input.readLine(), -1 );

        infile.close();
        infile.remove();     // TODO: success?

        // set the new configfile's name...
        for ( int i = 1; ; i++ )
        {
            m_configFile = QString( "KNote %1" ).arg(i);
            if ( !m_noteDir.exists( m_configFile ) )
                break;
        }

        // write the new configuration
        KIO::NetAccess::copy(
            KURL( KGlobal::dirs()->findResource( "config", "knotesrc" ) ),
            KURL( m_noteDir.absFilePath( m_configFile ) )
        );

        saveData();
        saveConfig();
        saveDisplayConfig();

        // TODO: Needed? What about KConfig?
        KSimpleConfig config( m_noteDir.absFilePath( m_configFile ) );
        config.setGroup( "General" );
        config.writeEntry( "version", 2 );

        config.setGroup( "Display" );
        config.writeEntry( "fgcolor", fg );
        config.writeEntry( "bgcolor", bg );

        config.setGroup( "Actions" );      // use the new default for this group
        config.writeEntry( "mail", "kmail --msg %f" );
        config.writeEntry( "print", "a2ps -P %p -1 --center-title=%t --underlay=KDE %f" );

        config.setGroup( "Editor" );
        config.writeEntry( "autoindent", indent );
        config.writeEntry( "titlefont", font );
        config.writeEntry( "font", font );
        config.writeEntry( "tabsize", 4 );
        config.sync();
    } else
        kdDebug() << "could not open input file" << endl;
}

void KNote::updateLayout()
{
    int headerHeight = m_label->sizeHint().height();

    m_button->setGeometry( frameRect().width() - headerHeight - 2,
                           frameRect().y() + 2, headerHeight, headerHeight );

    m_label->setGeometry( frameRect().x() + 2,
              frameRect().y() + 2,
              frameRect().width() - (m_button->isHidden() ? 0 : headerHeight) - 4,
              headerHeight );

    m_editor->setGeometry( contentsRect().x(), contentsRect().y() + headerHeight + 2,
                contentsRect().width(), contentsRect().height() - headerHeight - 4 );
}

// -------------------- protected methods -------------------- //

void KNote::resizeEvent( QResizeEvent* qre )
{
    QFrame::resizeEvent( qre );
    updateLayout();
}

void KNote::closeEvent( QCloseEvent* e )
{
    saveConfig();
    saveDisplayConfig();

    QFrame::closeEvent( e );
}

void KNote::keyPressEvent( QKeyEvent* e )
{
    if ( e->key() == Key_Escape )
        slotClose();
    else
        e->ignore();
}

bool KNote::event( QEvent* ev )
{
    if ( ev->type() == QEvent::LayoutHint )
    {
        updateLayout();
        return true;
    }
    else
        return QFrame::event( ev );
}

bool KNote::eventFilter( QObject* o, QEvent* ev )
{
    if ( o == m_label )
    {
        QMouseEvent* e = (QMouseEvent*)ev;

        if ( ev->type() == QEvent::MouseButtonRelease )
        {
            if ( e->button() == LeftButton )
            {
                m_dragging = false;
                m_label->releaseMouse();
                raise();
            }
            if ( e->button() == MidButton )
                lower();
            return true;
        }

        if ( ev->type() == QEvent::MouseButtonPress && e->button() == LeftButton )
        {
            m_pointerOffset = e->pos();
            m_label->grabMouse( sizeAllCursor );
            return true;
        }
        if ( ev->type() == QEvent::MouseMove && m_label == mouseGrabber())
        {
            if ( m_dragging )
                move( QCursor::pos() - m_pointerOffset );
            else
            {
                m_dragging = (
                    (e->pos().x() - m_pointerOffset.x())
                    *
                    (e->pos().x() - m_pointerOffset.x())
                    +
                    (e->pos().y() - m_pointerOffset.y())
                    *
                    (e->pos().y() - m_pointerOffset.y())
                    >= 9 );
            }
            return true;
        }

        if ( m_menu && ( ev->type() == QEvent::MouseButtonPress )
            && ( e->button() == RightButton ) )
        {
            m_menu->popup( QCursor::pos() );
            return true;
        }

        return m_label->eventFilter( o, ev );
    }
    else if ( o == m_editor )
    {
        if ( ev->type() == QEvent::FocusOut )
        {
            m_label->setBackgroundColor( palette().active().background() );
            m_button->hide();
            m_editor->cornerWidget()->hide();

            if ( m_editor->isModified() )
                saveData();
        }
        else if ( ev->type() == QEvent::FocusIn )
        {
            m_label->setBackgroundColor( palette().active().shadow() );
            m_button->show();
            m_editor->cornerWidget()->show();
        }
        return m_editor->eventFilter( o, ev );
    }
    else if ( o == m_editor->viewport() )
    {
        if ( ev->type() == QEvent::MouseButtonPress )
            if ( m_edit_menu && ((QMouseEvent*)ev)->button() == RightButton )
                m_edit_menu->popup( QCursor::pos() );

        return m_editor->viewport()->eventFilter( o, ev );
    }
    return QFrame::eventFilter( o, ev );
}

#include "knote.moc"
#include "knotebutton.moc"
