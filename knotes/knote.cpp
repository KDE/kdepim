/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2007, The KNotes Developers

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

#include <QBitmap>
#include <QCheckBox>
#include <QCursor>
#include <QDesktopWidget>
#include <QFile>
#include <QLabel>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QScrollBar>
#include <QSize>
#include <QSizeGrip>
#include <QTextStream>

#include <k3colordrag.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcodecs.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfind.h>
#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kselectaction.h>
#include <ksocketfactory.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kxmlguibuilder.h>
#include <kxmlguifactory.h>

#include <kwindowsystem.h>
#include <netwm.h>

#ifdef Q_WS_X11
#include <fixx11h.h>
#include <QX11Info>
#endif

#include <kcal/journal.h>
#include <kicon.h>

#include "knote.h"
#include "knotealarmdlg.h"
#include "knotebutton.h"
#include "knoteconfig.h"
#include "knoteconfigdlg.h"
#include "knoteedit.h"
#include "knotehostdlg.h"
#include "knoteprinter.h"
#include "knotesglobalconfig.h"
#include "knotesnetsend.h"
#include "version.h"

using namespace KCal;


KNote::KNote( QDomDocument buildDoc, Journal *j, QWidget *parent )
  : QFrame( parent, Qt::FramelessWindowHint ), m_label( 0 ), m_grip( 0 ),
    m_button( 0 ), m_tool( 0 ), m_editor( 0 ), m_config( 0 ), m_journal( j ),
    m_find( 0 ), m_kwinConf( KSharedConfig::openConfig( "kwinrc" ) )
{
  setAttribute( Qt::WA_DeleteOnClose );
  setAcceptDrops( true );
  setObjectName( m_journal->uid() );
  
  actionCollection()->setAssociatedWidget( this );
  
  setDOMDocument( buildDoc );
  
  // just set the name of the file to save the actions to, do NOT reparse it
  setXMLFile( componentData().componentName() + "ui.rc", false, false );
  
  // if there is no title yet, use the start date if valid
  // (KOrganizer's journals don't have titles but a valid start date)
  if ( m_journal->summary().isNull() && m_journal->dtStart().isValid() ) {
    QString s = KGlobal::locale()->formatDateTime( m_journal->dtStart() );
    m_journal->setSummary( s );
  }
  
  // create the menu items for the note - not the editor...
  // rename, mail, print, save as, insert date, alarm, close, delete, new note
  KAction *action  = new KAction( KIcon( "document-new" ), i18n( "New" ),
                                  this );
  actionCollection()->addAction( "new_note", action );
  connect( action, SIGNAL( triggered( bool ) ), SIGNAL( sigRequestNewNote() ) );
  action  = new KAction( KIcon( "text" ), i18n( "Rename..." ), this );
  actionCollection()->addAction( "rename_note", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotRename() ) );
  m_readOnly  = new KToggleAction( KIcon( "system-lock-screen" ),
                                   i18n( "Lock" ), this );
  actionCollection()->addAction( "lock_note", m_readOnly );
  connect( m_readOnly, SIGNAL( triggered( bool ) ),
          SLOT( slotUpdateReadOnly() ) );
  m_readOnly->setCheckedState( KGuiItem( i18n( "Unlock" ), "unlock" ) );
  action  = new KAction( KIcon( "window-close" ), i18n( "Hide" ), this );
  actionCollection()->addAction( "hide_note", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotClose() ) );
  action->setShortcut( QKeySequence( Qt::Key_Escape ) );
  action  = new KAction( KIcon( "knotes_delete" ), i18n( "Delete" ), this );
  actionCollection()->addAction( "delete_note", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotKill() ) );
  
  action  = new KAction( KIcon( "knotes_date" ), i18n( "Insert Date" ), this );
  actionCollection()->addAction( "insert_date", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotInsDate() ) );
  action  = new KAction( KIcon( "knotes_alarm" ), i18n( "Set Alarm..." ),
                         this );
  actionCollection()->addAction( "set_alarm", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotSetAlarm() ) );
  
  action  = new KAction( KIcon( "network-wired" ), i18n( "Send..." ), this );
  actionCollection()->addAction( "send_note", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotSend() ) );
  action  = new KAction( KIcon( "mail-send" ), i18n( "Mail..." ), this );
  actionCollection()->addAction( "mail_note", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotMail() ) );
  action  = new KAction( KIcon( "document-save-as" ), i18n( "Save As..." ),
                                this );
  actionCollection()->addAction( "save_note", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotSaveAs() ) );
  actionCollection()->addAction( KStandardAction::Print,  "print_note", this,
                                 SLOT( slotPrint() ) );
  action  = new KAction( KIcon( "configure" ), i18n( "Preferences..." ), this );
  actionCollection()->addAction( "configure_note", action );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( slotPreferences() ) );
  
  QActionGroup *kab = new QActionGroup( this );
  kab->setExclusive( true );
  
  m_keepAbove  = new KToggleAction( KIcon( "go-up" ),
                                    i18n( "Keep Above Others" ), this );
  actionCollection()->addAction( "keep_above", m_keepAbove );
  connect( m_keepAbove, SIGNAL( triggered( bool ) ),
           SLOT( slotUpdateKeepAboveBelow() ) );
  kab->addAction( m_keepAbove );
  
  m_keepBelow  = new KToggleAction( KIcon( "go-down" ),
                                    i18n( "Keep Below Others" ), this );
  actionCollection()->addAction( "keep_below", m_keepBelow );
  connect( m_keepBelow, SIGNAL( triggered( bool ) ),
           SLOT( slotUpdateKeepAboveBelow() ) );
  kab->addAction( m_keepBelow );
  
  m_toDesktop  = new KSelectAction( i18n( "To Desktop" ), this );
  actionCollection()->addAction( "to_desktop", m_toDesktop );
  connect( m_toDesktop, SIGNAL( triggered( int ) ),
           SLOT( slotPopupActionToDesktop( int ) ) );
  connect( m_toDesktop->menu(), SIGNAL( aboutToShow() ),
           SLOT( slotUpdateDesktopActions() ) );
  
  // invisible action to walk through the notes to make this configurable
  //FIXME: this backtab thing doesn't work anymore!
  action  = new KAction( i18n( "Walk Through Notes" ), this );
  actionCollection()->addAction( "walk_notes", action );
  connect( action, SIGNAL( triggered( bool ) ), SIGNAL( sigShowNextNote() ) );
  action->setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_Backtab ) );
  
  // create the note header, button and label...
  m_label = new QLabel( this );
  m_label->setFrameStyle( NoFrame );
  m_label->setBackgroundRole( QPalette::Base );
  m_label->setLineWidth( 0 );
  m_label->setAutoFillBackground( true );
  m_label->installEventFilter( this );  // receive events ( for dragging &
                                        // action menu )
  setName( m_journal->summary() );      // don't worry, no signals are
                                        // connected at this stage yet
  
  m_button = new KNoteButton( "knotes_close", this );
  connect( m_button, SIGNAL( clicked() ), this, SLOT( slotClose() ) );
  
  // create the note editor
  m_editor = new KNoteEdit( actionCollection(), this );
  m_editor->installEventFilter( this ); // receive focus events for modified
  
  // now that we have created all actions build the gui
  KXMLGUIBuilder builder( this );
  KXMLGUIFactory factory( &builder, this );
  factory.addClient( this );
  
  m_menu = dynamic_cast<KMenu*>( factory.container( "note_context", this ) );
  m_editor->setContextMenu( dynamic_cast<KMenu *>(
                              factory.container( "note_edit", this ) ) );
  
  m_tool = dynamic_cast<KToolBar*>( factory.container( "note_tool", this ) );
  
  if ( m_tool ) {
    m_tool->setIconSize( QSize( 10, 10 ) );
    m_tool->setFixedHeight( 24 );
    m_tool->setToolButtonStyle( Qt::ToolButtonIconOnly );
  }
  
  // if there was just a way of making KComboBox adhere the toolbar height...
  foreach ( KComboBox *combo, m_tool->findChildren<KComboBox *>() ) {
    QFont font = combo->font();
    font.setPointSize( 7 );
    combo->setFont( font );
    combo->setFixedHeight( 14 );
  }
  if ( m_tool ) {
    m_tool->hide();
  }
  
  setFocusProxy( m_editor );
  
  // create the resize handle
  m_grip = new QSizeGrip( m_editor );
  uint width = m_editor->verticalScrollBar()->sizeHint().width();
  uint height = m_editor->horizontalScrollBar()->sizeHint().height();
  QBitmap mask( width, height );
  mask.fill( Qt::color0 );
  QPolygon array;
  array.setPoints( 3, 0, height, width, height, width, 0 );
  QPainter p;
  p.begin( &mask );
  p.setBrush( Qt::color1 );
  p.drawPolygon( array );
  p.end();
  m_grip->setMask( mask );
  m_grip->setBackgroundRole( QPalette::Base );
  m_editor->setCornerWidget( m_grip );
  
  // the config file location
  QString configFile = KGlobal::dirs()->saveLocation( "appdata", "notes/" );
  configFile += m_journal->uid();
  
  // no config file yet? -> use the default display config if available
  // we want to write to configFile, so use "false"
  bool newNote = !KIO::NetAccess::exists( KUrl( configFile ),
                                          KIO::NetAccess::DestinationSide, 0 );
  
  m_config = new KNoteConfig( KSharedConfig::openConfig( configFile,
                                                         KConfig::NoGlobals ) );
  m_config->readConfig();
  m_config->setVersion( KNOTES_VERSION );
  
  if ( newNote ) {
    // until kdelibs provides copying of KConfigSkeletons (KDE 3.4)
    KNotesGlobalConfig *globalConfig = KNotesGlobalConfig::self();
    m_config->setBgColor( globalConfig->bgColor() );
    m_config->setFgColor( globalConfig->fgColor() );
    m_config->setWidth( globalConfig->width() );
    m_config->setHeight( globalConfig->height() );
    
    m_config->setFont( globalConfig->font() );
    m_config->setTitleFont( globalConfig->titleFont() );
    m_config->setAutoIndent( globalConfig->autoIndent() );
    m_config->setRichText( globalConfig->richText() );
    m_config->setTabSize( globalConfig->tabSize() );
    m_config->setReadOnly( globalConfig->readOnly() );
    
    m_config->setDesktop( globalConfig->desktop() );
    m_config->setHideNote( globalConfig->hideNote() );
    m_config->setPosition( globalConfig->position() );
    m_config->setShowInTaskbar( globalConfig->showInTaskbar() );
    m_config->setRememberDesktop( globalConfig->rememberDesktop() );
    m_config->setKeepAbove( globalConfig->keepAbove() );
    m_config->setKeepBelow( globalConfig->keepBelow() );
    
    m_config->writeConfig();
  }
  
  // set up the look&feel of the note
  setFrameStyle( Panel | Raised );
  setMinimumSize( 20, 20 );
  setBackgroundRole( QPalette::Base );
  
  m_editor->setContentsMargins( 0, 0, 0, 0 );
  m_editor->setBackgroundRole( QPalette::Base );
  m_editor->setFrameStyle( NoFrame );
  m_editor->setText( m_journal->description() );
  m_editor->document()->setModified( false );
  
  // can be done here since this doesn't pick up changes while KNotes is
  // running anyway
  bool closeLeft = false;
  KConfigGroup styleGroup( m_kwinConf, "Style" );
  if ( styleGroup.readEntry( "CustomButtonPositions", false ) ) {
    closeLeft = styleGroup.readEntry( "ButtonsOnLeft" ).contains( 'X' );
  }
  
  // load the display configuration of the note
  width = m_config->width();
  height = m_config->height();
  resize( width, height );
  
  // let KWin do the placement if the position is illegal--at least 10 pixels
  // of a note need to be visible
  const QPoint& position = m_config->position();
  QRect desk = kapp->desktop()->rect();
  desk.adjust( 10, 10, -10, -10 );
  if ( desk.intersects( QRect( position, QSize( width, height ) ) ) ) {
    move( position );           // do before calling show() to avoid flicker
  }
  
  // config items in the journal have priority
  QString property = m_journal->customProperty( "KNotes", "FgColor" );
  if ( !property.isNull() ) {
    m_config->setFgColor( QColor( property ) );
  } else {
    m_journal->setCustomProperty( "KNotes", "FgColor",
                                  m_config->fgColor().name() );
  }
  
  property = m_journal->customProperty( "KNotes", "BgColor" );
  if ( !property.isNull() ) {
    m_config->setBgColor( QColor( property ) );
  } else {
    m_journal->setCustomProperty( "KNotes", "BgColor",
                                  m_config->bgColor().name() );
  }
  property = m_journal->customProperty( "KNotes", "RichText" );
  if ( !property.isNull() ) {
    m_config->setRichText( property == "true" ? true : false );
  } else {
    m_journal->setCustomProperty( "KNotes", "RichText",
                                  m_config->richText() ? "true" : "false" );
  }
  // read configuration settings...
  slotApplyConfig();
  
  // if this is a new note put on current desktop - we can't use defaults
  // in KConfig XT since only _changes_ will be stored in the config file
  int desktop = m_config->desktop();
  
#ifdef Q_WS_X11
  if ( ( desktop < 0 && desktop != NETWinInfo::OnAllDesktops ) ||
       !m_config->rememberDesktop() )
    desktop = KWindowSystem::currentDesktop();
#endif
  
  // show the note if desired
  if ( desktop != 0 && !m_config->hideNote() ) {
    // to avoid flicker, call this before show()
    toDesktop( desktop );
    show();
    
    // because KWin forgets about that for hidden windows
#ifdef Q_WS_X11
    if ( desktop == NETWinInfo::OnAllDesktops ) {
      toDesktop( desktop );
    }
#endif
  }
  
  m_readOnly->setChecked( m_config->readOnly() );
  slotUpdateReadOnly();

  if ( m_config->keepAbove() ) {
    m_keepAbove->setChecked( true );
  } else if ( m_config->keepBelow() ) {
    m_keepBelow->setChecked( true );
  } else {
    m_keepAbove->setChecked( false );
    m_keepBelow->setChecked( false );
  }
  slotUpdateKeepAboveBelow();
  
  // HACK: update the icon color - again after showing the note, to make kicker
  // aware of the new colors
  KIconEffect effect;
  QPixmap icon = effect.apply( qApp->windowIcon().pixmap(
                                 IconSize( KIconLoader::Desktop ),
                                 IconSize( KIconLoader::Desktop ) ),
                               KIconEffect::Colorize,
                               1, m_config->bgColor(), false );
  QPixmap miniIcon = effect.apply( qApp->windowIcon().pixmap(
                                     IconSize( KIconLoader::Small ),
                                     IconSize( KIconLoader::Small ) ),
                                   KIconEffect::Colorize,
                                   1, m_config->bgColor(), false );
#ifdef Q_WS_X11
  KWindowSystem::setIcons( winId(), icon, miniIcon );
#endif
}

KNote::~KNote()
{
  delete m_config;
}


// -------------------- public slots -------------------- //

void KNote::slotKill( bool force )
{
  if ( !force &&
       ( KMessageBox::warningContinueCancel( this,
         i18n( "<qt>Do you really want to delete note <b>%1</b>?</qt>",
               m_label->text() ),
         i18n( "Confirm Delete" ),
         KGuiItem( i18n( "&Delete" ), "edit-delete" ),
         KStandardGuiItem::cancel(),
         "ConfirmDeleteNote" ) != KMessageBox::Continue ) ) {
    return;
  }
  
  // delete the configuration first, then the corresponding file
  delete m_config;
  m_config = 0;
  
  QString configFile = KGlobal::dirs()->saveLocation( "appdata", "notes/" );
  configFile += m_journal->uid();
  
  if ( !KIO::NetAccess::del( KUrl( configFile ), this ) ) {
    kError( 5500 ) <<"Can't remove the note config:" << configFile;
  }
  
  emit sigKillNote( m_journal );
}


// -------------------- public member functions -------------------- //

void KNote::saveData()
{
  m_journal->setSummary( m_label->text() );
  m_journal->setDescription( m_editor->text() );
  m_journal->setCustomProperty( "KNotes", "FgColor",
                                m_config->fgColor().name() );
  m_journal->setCustomProperty( "KNotes", "BgColor",
                                m_config->bgColor().name() );
  m_journal->setCustomProperty( "KNotes", "RichText",
                                m_config->richText() ? "true" : "false" );
  
  emit sigDataChanged();
  m_editor->document()->setModified( false );
}

void KNote::saveConfig() const
{
  m_config->setWidth( width() );
  if ( m_tool ) {
    m_config->setHeight(
        height() - ( m_tool->isHidden() ? 0 : m_tool->height() ) );
  } else {
    m_config->setHeight( 0 );
  }
  m_config->setPosition( pos() );
  
#ifdef Q_WS_X11
  NETWinInfo wm_client( QX11Info::display(), winId(),
                        QX11Info::appRootWindow(), NET::WMDesktop );
  if ( ( wm_client.desktop() == NETWinInfo::OnAllDesktops ) ||
     ( wm_client.desktop() > 0 ) ) {
    m_config->setDesktop( wm_client.desktop() );
  }
#endif
  
  // actually store the config on disk
  m_config->writeConfig();
}

QString KNote::noteId() const
{
  return m_journal->uid();
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
  updateLabelAlignment();
  
  if ( m_editor ) {    // not called from CTOR?
    saveData();
  }
#ifdef Q_WS_X11
  // set the window's name for the taskbar entry to be more helpful (#58338)
  NETWinInfo note_win( QX11Info::display(), winId(), QX11Info::appRootWindow(),
                       NET::WMDesktop );
  note_win.setName( name.toUtf8() );
#endif
  
  emit sigNameChanged();
}

void KNote::setText( const QString& text )
{
  m_editor->setText( text );
  
  saveData();
}

void KNote::find( const QString& pattern, long options )
{
  delete m_find;
  m_find = new KFind( pattern, options, this );
  
  connect( m_find, SIGNAL( highlight( const QString &, int, int ) ),
           this, SLOT( slotHighlight( const QString &, int, int ) ) );
  connect( m_find, SIGNAL( findNext() ), this, SLOT( slotFindNext() ) );
  
  m_find->setData( m_editor->toPlainText() );
  slotFindNext();
}

void KNote::slotFindNext()
{
  // TODO: honor FindBackwards
  // TODO: dialogClosed() -> delete m_find

  // Let KFind inspect the text fragment, and display a dialog if a match is
  // found
  KFind::Result res = m_find->find();
  
  if ( res == KFind::NoMatch ) { // i.e. at end-pos
    // use a different text cursor!
    m_editor->textCursor().clearSelection();
    emit sigFindFinished();
    delete m_find;
    m_find = 0;
  } else {
    show();
#ifdef Q_WS_X11
    KWindowSystem::setCurrentDesktop( KWindowSystem::windowInfo( winId(),
                                      NET::WMDesktop ).desktop() );
#endif
  }
}

void KNote::slotHighlight( const QString& /*str*/, int idx, int len )
{
  QTextCursor c = m_editor->textCursor();
  c.setPosition( idx );
  c.setPosition( idx + len, QTextCursor::KeepAnchor );
  // TODO: modify the selection color, use a different QTextCursor?
}

bool KNote::isModified() const
{
  return m_editor->document()->isModified();
}

// ------------------ private slots (menu actions) ------------------ //

void KNote::slotRename()
{
  // pop up dialog to get the new name
  bool ok;
  QString newName = KInputDialog::getText( QString::null,
    //krazy:exclude=nullstrassign for old broken gcc
    i18n( "Please enter the new name:" ), m_label->text(), &ok, this );
  
  if ( !ok ) { // handle cancel
    return;
  }
  
  setName( newName );
}

void KNote::slotUpdateReadOnly()
{
  const bool readOnly = m_readOnly->isChecked();
  
  m_editor->setReadOnly( readOnly );
  m_config->setReadOnly( readOnly );
  
  // enable/disable actions accordingly
  actionCollection()->action( "configure_note" )->setEnabled( !readOnly );
  actionCollection()->action( "insert_date" )->setEnabled( !readOnly );
  actionCollection()->action( "delete_note" )->setEnabled( !readOnly );
  
  actionCollection()->action( "edit_undo" )->setEnabled( !readOnly &&
                              m_editor->document()->isUndoAvailable() );
  actionCollection()->action( "edit_redo" )->setEnabled( !readOnly &&
                              m_editor->document()->isRedoAvailable() );
  actionCollection()->action( "edit_cut" )->setEnabled( !readOnly &&
                              m_editor->textCursor().hasSelection() );
  actionCollection()->action( "edit_paste" )->setEnabled( !readOnly );
  actionCollection()->action( "edit_clear" )->setEnabled( !readOnly );
  
  updateFocus();
}

void KNote::slotClose()
{
#ifdef Q_WS_X11
  NETWinInfo wm_client( QX11Info::display(), winId(),
                        QX11Info::appRootWindow(), NET::WMDesktop );
  if ( ( wm_client.desktop() == NETWinInfo::OnAllDesktops ) ||
       ( wm_client.desktop() > 0 ) ) {
    m_config->setDesktop( wm_client.desktop() );
  }
#endif
  
  m_editor->clearFocus();
  m_config->setHideNote( true );
  m_config->setPosition( pos() );
  
  // just hide the note so it's still available from the dock window
  hide();
}

void KNote::slotInsDate()
{
  m_editor->insertPlainText(
    KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ) );
}

void KNote::slotSetAlarm()
{
  KNoteAlarmDlg dlg( name(), this );
  dlg.setIncidence( m_journal );
  
  if ( dlg.exec() == QDialog::Accepted ) {
    emit sigDataChanged();
  }
}

void KNote::slotPreferences()
{
  // reuse if possible
  if ( KNoteConfigDlg::showDialog( noteId() ) ) {
    return;
  }
  
  // create a new preferences dialog...
  KNoteConfigDlg *dialog = new KNoteConfigDlg( m_config, name(), this, noteId() );
  connect( dialog, SIGNAL( settingsChanged( const QString & ) ) , this,
           SLOT( slotApplyConfig() ) );
  connect( this, SIGNAL( sigNameChanged() ), dialog,
           SLOT( slotUpdateCaption() ) );
           dialog->show();
}

void KNote::slotSend()
{
  // pop up dialog to get the IP
  KNoteHostDlg hostDlg( i18n( "Send \"%1\"", name() ), this );
  bool ok = ( hostDlg.exec() == QDialog::Accepted );
  QString host = hostDlg.host();
  
  if ( !ok ) { // handle cancel
    return;
  }
  
  if ( host.isEmpty() ) {
    KMessageBox::sorry( this, i18n( "The host cannot be empty." ) );
    return;
  }
  
  // Send the note
  
  KNotesNetworkSender *sender = new KNotesNetworkSender(
    KSocketFactory::connectToHost( "knotes", host,
                                   KNotesGlobalConfig::port() ) );
  sender->setSenderId( KNotesGlobalConfig::senderID() );
  sender->setNote( name(), text() ); // FIXME: plainText ??
}

void KNote::slotMail()
{
  // get the mail action command
  QStringList cmd_list = KNotesGlobalConfig::mailAction().split( QChar(' '),
      QString::SkipEmptyParts );
  
  KProcess mail;
  foreach ( QString cmd, cmd_list ) {
    if ( cmd == "%f" ) {
      mail << m_editor->toPlainText();
    } else if ( cmd == "%t" ) {
      mail << m_label->text();
    } else {
      mail << cmd;
    }
  }
  
  if ( !mail.startDetached() ) {
    KMessageBox::sorry( this, i18n( "Unable to start the mail process." ) );
  }
}

void KNote::slotPrint()
{
  saveData();
  QString content;
  if ( !Qt::mightBeRichText( m_editor->text() ) ) {
    content = Qt::convertFromPlainText( m_editor->text() );
  } else {
    content = m_editor->text();
  }
  KNotePrinter printer;
  printer.setDefaultFont( m_config->font() );
  printer.printNote( name(), content );
}

void KNote::slotSaveAs()
{
  // TODO: where to put pdf file support? In the printer??!??!
  
  QCheckBox *convert = 0;
  
  if ( m_editor->acceptRichText() ) {
    convert = new QCheckBox( 0 );
    convert->setText( i18n( "Save note as plain text" ) );
  }
  
  KUrl url;
  KFileDialog dlg( url, QString(), this, convert );
  dlg.setOperationMode( KFileDialog::Saving );
  dlg.setCaption( i18n( "Save As" ) );
  dlg.exec();
  
  QString fileName = dlg.selectedFile();
  if ( fileName.isEmpty() ) {
    return;
  }
  
  QFile file( fileName );
  
  if ( file.exists() &&
       KMessageBox::warningContinueCancel( this,
          i18n( "<qt>A file named <b>%1</b> already exists.<br />"
                "Are you sure you want to overwrite it?</qt>",
                QFileInfo( file ).fileName() ) ) != KMessageBox::Continue ) {
    return;
  }
  
  if ( file.open( QIODevice::WriteOnly ) ) {
    QTextStream stream( &file );
    if ( convert && !convert->isChecked() ) {
      stream << m_editor->toHtml();
    } else {
      stream << m_editor->toPlainText();
    }
  }
}

void KNote::slotPopupActionToDesktop( int id )
{
  toDesktop( id - 1 ); // compensate for the menu separator, -1 == all desktops
}


// ------------------ private slots (configuration) ------------------ //

void KNote::slotApplyConfig()
{
  m_label->setFont( m_config->titleFont() );
  m_editor->setRichText( m_config->richText() );
  m_editor->setTextFont( m_config->font() );
  m_editor->setTabStop( m_config->tabSize() );
  m_editor->setAutoIndentMode( m_config->autoIndent() );
  
  // if called as a slot, save the text, we might have changed the
  // text format - otherwise the journal will not be updated
  if ( sender() ) {
    saveData();
  }
  
  setColor( m_config->fgColor(), m_config->bgColor() );
  
  updateLayout();
  slotUpdateShowInTaskbar();
}

void KNote::slotUpdateKeepAboveBelow()
{
#ifdef Q_WS_X11
  KWindowInfo info( KWindowSystem::windowInfo( winId(), NET::WMState ) );
#endif
  if ( m_keepAbove->isChecked() ) {
    m_config->setKeepAbove( true );
    m_config->setKeepBelow( false );
#ifdef Q_WS_X11
    KWindowSystem::setState( winId(), info.state() | NET::KeepAbove );
#endif
  } else if ( m_keepBelow->isChecked() ) {
    m_config->setKeepAbove( false );
    m_config->setKeepBelow( true );
#ifdef Q_WS_X11
    KWindowSystem::setState( winId(), info.state() | NET::KeepBelow );
#endif
  } else {
    m_config->setKeepAbove( false );
#ifdef Q_WS_X11
    KWindowSystem::clearState( winId(), NET::KeepAbove );
#endif
    m_config->setKeepBelow( false );
#ifdef Q_WS_X11
    KWindowSystem::clearState( winId(), NET::KeepBelow );
#endif
  }
}

void KNote::slotUpdateShowInTaskbar()
{
#ifdef Q_WS_X11
  if ( !m_config->showInTaskbar() ) {
    KWindowSystem::setState( winId(), KWindowSystem::windowInfo( winId(),
                             NET::WMState ).state() | NET::SkipTaskbar );
  } else {
    KWindowSystem::clearState( winId(), NET::SkipTaskbar );
  }
#endif
}

void KNote::slotUpdateDesktopActions()
{
#ifdef Q_WS_X11
  NETRootInfo wm_root( QX11Info::display(), NET::NumberOfDesktops |
                       NET::DesktopNames );
  NETWinInfo wm_client( QX11Info::display(), winId(),
                        QX11Info::appRootWindow(), NET::WMDesktop );
  
  QStringList desktops;
  desktops.append( i18n( "&All Desktops" ) );
  desktops.append( QString::null );           // Separator
                                              // krazy:exclude=nullstrassign
                                              // for old broken gcc
  
  int count = wm_root.numberOfDesktops();
  for ( int n = 1; n <= count; n++ ) {
    desktops.append( QString( "&%1 %2" ).arg( n ).arg(
      QString::fromUtf8( wm_root.desktopName( n ) ) ) );
  }
  m_toDesktop->setItems( desktops );
  
  if ( wm_client.desktop() == NETWinInfo::OnAllDesktops ) {
    m_toDesktop->setCurrentItem( 0 );
  } else {
    m_toDesktop->setCurrentItem( wm_client.desktop() + 1 ); // compensate for
                                                            // separator (+1)
  }
#endif
}


// -------------------- private methods -------------------- //

void KNote::toDesktop( int desktop )
{
  if ( desktop == 0 ) {
    return;
  }
  
#ifdef Q_WS_X11
  if ( desktop == NETWinInfo::OnAllDesktops ) {
    KWindowSystem::setOnAllDesktops( winId(), true );
  } else {
    KWindowSystem::setOnDesktop( winId(), desktop );
  }
#endif
}

void KNote::setColor( const QColor &fg, const QColor &bg )
{
  QPalette p = palette();
  
  // better: from light(150) to light(100) to light(75)
  // QLinearGradient g( width()/2, 0, width()/2, height() );
  // g.setColorAt( 0, bg );
  // g.setColorAt( 1, bg.dark(150) );
  
  p.setColor( QPalette::Window,     bg );
  // p.setBrush( QPalette::Window,     g );
  p.setColor( QPalette::Base,       bg );
  // p.setBrush( QPalette::Base,       g );
  
  p.setColor( QPalette::WindowText, fg );
  p.setColor( QPalette::Text,       fg );
  
  p.setColor( QPalette::Button,     bg.dark( 116 ) );
  p.setColor( QPalette::ButtonText, fg );
  
  //p.setColor( QPalette::Highlight,  bg );
  //p.setColor( QPalette::HighlightedText, fg );
  
  // order: Light, Midlight, Button, Mid, Dark, Shadow
  
  // the shadow
  p.setColor( QPalette::Light, bg.light( 180 ) );
  p.setColor( QPalette::Midlight, bg.light( 150 ) );
  p.setColor( QPalette::Mid, bg.light( 150 ) );
  p.setColor( QPalette::Dark, bg.dark( 108 ) );
  p.setColor( QPalette::Shadow, bg.dark( 116 ) );
  
  setPalette( p );
  
  // darker values for the active label
  p.setColor( QPalette::Active, QPalette::Base, bg.dark( 116 ) );
  
  m_label->setPalette( p );
  
  // set the text color
  m_editor->setTextColor( fg );
  
  // update the icon color
  KIconEffect effect;
  QPixmap icon = effect.apply( qApp->windowIcon().pixmap(
                                 IconSize( KIconLoader::Desktop ),
                                 IconSize( KIconLoader::Desktop ) ),
                               KIconEffect::Colorize, 1, bg, false );
  QPixmap miniIcon = effect.apply( qApp->windowIcon().pixmap(
                                     IconSize( KIconLoader::Small ),
                                     IconSize( KIconLoader::Small ) ),
                                   KIconEffect::Colorize, 1, bg, false );
#ifdef Q_WS_X11
  KWindowSystem::setIcons( winId(), icon, miniIcon );
#endif
  // update the color of the title
  updateFocus();
  emit sigColorChanged();
}

void KNote::updateLabelAlignment()
{
  // if the name is too long to fit, left-align it, otherwise center it (#59028)
  QString labelText = m_label->text();
  if ( m_label->fontMetrics().boundingRect( labelText ).width() >
       m_label->width() ) {
    m_label->setAlignment( Qt::AlignLeft );
  } else {
    m_label->setAlignment( Qt::AlignHCenter );
  }
}

void KNote::updateFocus()
{
  if ( hasFocus() ) {
    m_button->show();
    m_grip->show();
    
    if ( !m_editor->isReadOnly() ) {
      if ( m_tool && m_tool->isHidden() && m_editor->acceptRichText() ) {
        m_tool->show();
        setGeometry( x(), y(), width(), height() + m_tool->height() );
      }
    } else if ( m_tool && !m_tool->isHidden() ) {
      m_tool->hide();
      setGeometry( x(), y(), width(), height() - m_tool->height() );
      updateLayout();     // to update the minimum height
    }
  } else {
    m_button->hide();
    m_grip->hide();
    
    if ( m_tool && !m_tool->isHidden() ) {
      m_tool->hide();
      setGeometry( x(), y(), width(), height() - m_tool->height() );
      updateLayout();     // to update the minimum height
    }
  }
}

void KNote::updateLayout()
{
  const int headerHeight = m_label->sizeHint().height();
  bool closeLeft = false;
  int marginLeft, marginTop, marginRight, marginBottom;
  
  m_editor->getContentsMargins( &marginLeft, &marginTop, &marginRight,
                                &marginBottom );
  
  KConfigGroup styleGroup( m_kwinConf, "Style" );
  if ( styleGroup.readEntry( "CustomButtonPositions",false ) ) {
    closeLeft = styleGroup.readEntry( "ButtonsOnLeft" ).contains( 'X' );
  }
  
  m_button->setGeometry(
    closeLeft ? contentsRect().x() : contentsRect().width() - headerHeight + 1,
    contentsRect().y() + 1,
    headerHeight - 2,
    headerHeight - 2
  );
  
  m_label->setGeometry(
    contentsRect().x(), contentsRect().y(),
    contentsRect().width(), headerHeight
  );
  
  m_editor->setGeometry( QRect(
    QPoint( contentsRect().x(),
            contentsRect().y() + headerHeight ),
    QPoint( contentsRect().right(),
            contentsRect().bottom() -
              ( m_tool ? ( m_tool->isHidden() ? 0 : m_tool->height() ) : 0 ) ) )
    );
    
  if( m_tool ) {
    m_tool->setGeometry(
      contentsRect().x(),
      contentsRect().bottom() - m_tool->height() + 1,
      contentsRect().width(),
      m_tool->height()
    );
  }
  
  //  setMinimumSize(
  //  m_editor->cornerWidget()->width() + marginLeft + marginRight,
  //  headerHeight + (
  //    m_tool ? ( m_tool->isHidden() ? 0 : m_tool->height() ) : 0 ) +
  //  m_editor->cornerWidget()->height() + marginTop + marginBottom );

  setMinimumSize(
    marginLeft + marginRight,
    headerHeight + (
      m_tool ? ( m_tool->isHidden() ? 0 : m_tool->height() ) : 0 ) +
     marginTop + marginBottom );
  
  updateLabelAlignment();
}

// -------------------- protected methods -------------------- //

void KNote::contextMenuEvent( QContextMenuEvent *e )
{
  if ( m_menu ) {
    m_menu->popup( e->globalPos() );
  }
}

void KNote::showEvent( QShowEvent * )
{
  if ( m_config->hideNote() ) {
    // KWin does not preserve these properties for hidden windows
    slotUpdateKeepAboveBelow();
    slotUpdateShowInTaskbar();
    toDesktop( m_config->desktop() );
    move( m_config->position() );
    m_config->setHideNote( false );
  }
}

void KNote::resizeEvent( QResizeEvent *qre )
{
  QFrame::resizeEvent( qre );
  updateLayout();
}

void KNote::closeEvent( QCloseEvent * )
{
  slotClose();
}

void KNote::dragEnterEvent( QDragEnterEvent *e )
{
  if ( !m_config->readOnly() ) {
    e->setAccepted( e->mimeData()->hasColor() );
  }
}

void KNote::dropEvent( QDropEvent *e )
{
  if ( m_config->readOnly() ) {
    return;
  }
  
  const QMimeData *md = e->mimeData();
  if ( md->hasColor() ) {
       QColor bg =  qvariant_cast<QColor>( md->colorData() );
       setColor( palette().color( foregroundRole() ), bg );
       m_journal->setCustomProperty( "KNotes", "BgColor", bg.name() );
       m_config->setBgColor( bg );
  }
}

bool KNote::event( QEvent *ev )
{
  if ( ev->type() == QEvent::LayoutRequest ) {
    updateLayout();
    return true;
  } else {
    return QFrame::event( ev );
  }
}

bool KNote::eventFilter( QObject *o, QEvent *ev )
{
  if ( ev->type() == QEvent::DragEnter &&
    static_cast<QDragEnterEvent*>( ev )->mimeData()->hasColor() ) {
    dragEnterEvent( static_cast<QDragEnterEvent *>( ev ) );
    return true;
  }
  
  if ( ev->type() == QEvent::Drop &&
       static_cast<QDropEvent *>( ev )->mimeData()->hasColor() ) {
    dropEvent( static_cast<QDropEvent *>( ev ) );
    return true;
  }
  
  if ( o == m_label ) {
    QMouseEvent *e = ( QMouseEvent * )ev;
    
    if ( ev->type() == QEvent::MouseButtonDblClick ) {
      slotRename();
    }
    
    if ( ev->type() == QEvent::MouseButtonPress &&
        ( e->button() == Qt::LeftButton || e->button() == Qt::MidButton ) ) {
#ifdef Q_WS_X11
      e->button() == Qt::LeftButton ? KWindowSystem::raiseWindow( winId() )
                                    : KWindowSystem::lowerWindow( winId() );
      
      XUngrabPointer( QX11Info::display(), QX11Info::appTime() );
      NETRootInfo wm_root( QX11Info::display(), NET::WMMoveResize );
      wm_root.moveResizeRequest( winId(), e->globalX(), e->globalY(),
                                 NET::Move );
#endif
      return true;
    }
    
    if ( ev->type() == QEvent::MouseButtonRelease ) {
#ifdef Q_WS_X11
        NETRootInfo wm_root( QX11Info::display(), NET::WMMoveResize );
        wm_root.moveResizeRequest( winId(), e->globalX(), e->globalY(),
                                   NET::MoveResizeCancel );
#endif
        return false;
    }
    
    return false;
  }
  
  if ( o == m_editor ) {
    if ( ev->type() == QEvent::FocusOut ) {
          QFocusEvent *fe = static_cast<QFocusEvent *>( ev );
          if ( fe->reason() != Qt::PopupFocusReason &&
               fe->reason() != Qt::MouseFocusReason ) {
            updateFocus();
            if ( isModified() ) {
              saveData();
            }
          }
    } else if ( ev->type() == QEvent::FocusIn ) {
      updateFocus();
    }
    
    return false;
  }
  
  return false;
}


#include "knote.moc"
#include "knotebutton.moc"
