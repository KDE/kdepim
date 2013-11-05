/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "kncomposer.h"

#include <KPIMUtils/Email>
#include <KPIMIdentities/Identity>
#include <KPIMIdentities/IdentityManager>
#include <QVBoxLayout>
#include <QLabel>
#include <QtDBus/QtDBus>
#include <qgroupbox.h>
#include "libkdepim/addressline/recentaddresses.h"
using KPIM::RecentAddresses;
#include <akonadi/contact/emailaddressselectiondialog.h>
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <ktemporaryfile.h>
#include <libkpgp/kpgpblock.h>
#include <kcompletionbox.h>
#include <kxmlguifactory.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kselectaction.h>
#include <ktoggleaction.h>
#include "kngroupselectdialog.h"
#include "utilities.h"
#include "knglobals.h"
#include "knmainwidget.h"
#include "knaccountmanager.h"
#include "knnntpaccount.h"
#include "settings.h"
#include "kncomposerview.h"
#include "utils/locale.h"

using namespace KNode::Utilities;
using namespace KNode::Composer;


KNLineEdit::KNLineEdit( View *parent, bool useCompletion )
  : KABC::AddressLineEdit( parent, useCompletion ),
    composerView( parent )
{
}

KNLineEdit::KNLineEdit( QWidget *parent, bool useCompletion )
  : KABC::AddressLineEdit( parent, useCompletion ), composerView(0)
{
}


void KNLineEdit::contextMenuEvent( QContextMenuEvent*e )
{
   QMenu *popup = KLineEdit::createStandardContextMenu();
   popup->addSeparator();
   popup->addAction( i18n( "Edit Recent Addresses..." ),
                   this, SLOT(editRecentAddresses()) );
   popup->exec( e->globalPos() );
   delete popup;
}

void KNLineEdit::editRecentAddresses()
{
  KPIM::RecentAddressDialog dlg( this );
  dlg.setAddresses( RecentAddresses::self( knGlobals.config() )->addresses() );
  if ( dlg.exec() ) {
    RecentAddresses::self( knGlobals.config() )->clear();
    QStringList addrList = dlg.addresses();
    QStringList::Iterator it;
    for ( it = addrList.begin(); it != addrList.end(); ++it )
      RecentAddresses::self( knGlobals.config() )->add( *it );

    loadAddresses();
  }
}

void KNLineEdit::loadAddresses()
{
    KABC::AddressLineEdit::loadAddresses();

    QStringList recent = RecentAddresses::self(knGlobals.config())->addresses();
    QStringList::Iterator it = recent.begin();
    for ( ; it != recent.end(); ++it )
        addAddress( *it );
}

void KNLineEdit::keyPressEvent(QKeyEvent *e)
{
    // ---sven's Return is same Tab and arrow key navigation start ---
  if ((e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) &&
        !completionBox()->isVisible())
    {
        composerView->focusNextPrevEdit( this, true );
      return;
    }
    if (e->key() == Qt::Key_Up)
    {
        composerView->focusNextPrevEdit( this, false ); // Go up
      return;
    }
    if (e->key() == Qt::Key_Down)
    {
        composerView->focusNextPrevEdit( this, true ); // Go down
      return;
    }
    // ---sven's Return is same Tab and arrow key navigation end ---
  KABC::AddressLineEdit::keyPressEvent(e);
}


KNLineEditSpell::KNLineEditSpell( View *parent, bool useCompletion )
  : KNLineEdit( parent, useCompletion )
{
}

KNLineEditSpell::KNLineEditSpell( QWidget *parent, bool useCompletion )
  : KNLineEdit( parent, useCompletion )
{
}

void KNLineEditSpell::highLightWord( unsigned int length, unsigned int pos )
{
    setSelection ( pos, length );
}

void KNLineEditSpell::spellCheckDone( const QString &s )
{
    if( s != text() )
        setText( s );
}

void KNLineEditSpell::spellCheckerMisspelling( const QString &_text, const QStringList &, unsigned int pos)
{
     highLightWord( _text.length(),pos );
}

void KNLineEditSpell::spellCheckerCorrected( const QString &old, const QString &corr, unsigned int pos)
{
    if( old!= corr )
    {
        setSelection ( pos, old.length() );
        insert( corr );
        setSelection ( pos, corr.length() );
    }
}


KNComposer::KNComposer( KNLocalArticle::Ptr a, const QString &text, const QString &unwraped, bool firstEdit, bool dislikesCopies, bool createCopy, bool allowMail )
    : KXmlGuiWindow(0), r_esult(CRsave), a_rticle(a),
      u_nwraped(unwraped),
      n_eeds8Bit(true), v_alidated(false), a_uthorDislikesMailCopies(dislikesCopies), e_xternalEdited(false), e_xternalEditor(0),
      e_ditorTempfile(0), a_ttChanged(false),
      mFirstEdit( firstEdit )
{
  setObjectName( "composerWindow" );

  if( knGlobals.componentData().isValid() )
    setComponentData( knGlobals.componentData() );

  // activate dnd of attachments...
  setAcceptDrops(true);

  //init v_iew
  v_iew = new View( this );
  setCentralWidget(v_iew);

  connect( v_iew, SIGNAL(closeExternalEditor()), this, SLOT(slotCancelEditor()) );

  //statusbar
  KStatusBar *sb=statusBar();
  sb->insertItem( QString(), 1, 1 );                 // type
  sb->setItemAlignment( 1, Qt::AlignLeft | Qt::AlignVCenter );
  sb->insertItem( QString(), 2, 1 );                 // charset
  sb->setItemAlignment( 2, Qt::AlignLeft | Qt::AlignVCenter );
  sb->insertItem( QString(), 3, 1 );                 // write mode
  sb->setItemAlignment(3, Qt::AlignCenter | Qt::AlignVCenter );
  sb->insertItem( QString(), 4, 1 );                 // column
  sb->setItemAlignment(4, Qt::AlignCenter | Qt::AlignVCenter );
  sb->insertItem( QString(), 5, 1 );                 // line
  sb->setItemAlignment( 5, Qt::AlignCenter | Qt::AlignVCenter );
  connect(v_iew->editor(), SIGNAL(cursorPositionChanged()), SLOT(slotUpdateCursorPos()));
  connect(v_iew->editor(), SIGNAL(insertModeChanged()), SLOT(slotUpdateStatusBar()));

  QDBusConnection::sessionBus().registerObject( "/Composer", this, QDBusConnection::ExportScriptableSlots );
  //------------------------------- <Actions> --------------------------------------

  //file menu
  KAction *action = actionCollection()->addAction("send_now");
  action->setIcon(KIcon("mail-send"));
  action->setText(i18n("&Send Now"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotSendNow()));
  action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));

  action = actionCollection()->addAction("send_later");
  action->setIcon(KIcon("mail-queue"));
  action->setText(i18n("Send &Later"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotSendLater()));

  action = actionCollection()->addAction("save_as_draft");
  action->setIcon(KIcon("document-save"));
  action->setText(i18n("Save as &Draft"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotSaveAsDraft()));

  action = actionCollection()->addAction("art_delete");
  action->setIcon(KIcon("edit-delete"));
  action->setText(i18n("D&elete"));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotArtDelete()));

  KStandardAction::close(this, SLOT(close()),actionCollection());

  //edit menu
  KStandardAction::undo(this, SLOT(slotUndo()), actionCollection());
  KStandardAction::redo(this, SLOT(slotRedo()), actionCollection());

  KStandardAction::cut(this, SLOT(slotCut()), actionCollection());


  KStandardAction::copy(this, SLOT(slotCopy()), actionCollection());

  KStandardAction::pasteText(this, SLOT(slotPaste()), actionCollection());

  action = actionCollection()->addAction("paste_quoted");
  action->setText(i18n("Paste as &Quotation"));
  connect(action, SIGNAL(triggered(bool)), v_iew->editor(), SLOT(slotPasteAsQuotation()));

  KStandardAction::selectAll(this, SLOT(slotSelectAll()), actionCollection());

  KStandardAction::find(v_iew->editor(), SLOT(slotFind()), actionCollection());
  KStandardAction::findNext(v_iew->editor(), SLOT(slotFindNext()), actionCollection());

  KStandardAction::replace(v_iew->editor(), SLOT(slotReplace()), actionCollection());

  //attach menu
  action = actionCollection()->addAction("append_signature");
  action->setText(i18n("Append &Signature"));
  connect( action, SIGNAL(triggered(bool)), v_iew, SLOT(appendSignature()) );

  action = actionCollection()->addAction("insert_file");
  action->setText(i18n("&Insert File..."));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotInsertFile()));

  action = actionCollection()->addAction("insert_file_boxed");
  action->setText(i18n("Insert File (in a &box)..."));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotInsertFileBoxed()));

  action = actionCollection()->addAction("attach_file");
  action->setIcon(KIcon("mail-attachment"));
  action->setText(i18n("Attach &File..."));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotAttachFile()));

  a_ctPGPsign = actionCollection()->add<KToggleAction>("sign_article");
  a_ctPGPsign->setText(i18n("Sign Article with &PGP"));
  a_ctPGPsign->setIcon(KIcon("document-sign"));

  a_ctRemoveAttachment = actionCollection()->addAction("remove_attachment");
  a_ctRemoveAttachment->setText(i18n("&Remove"));
  connect( a_ctRemoveAttachment, SIGNAL(triggered(bool)), v_iew, SLOT(removeCurrentAttachment()) );

  a_ctAttachmentProperties = actionCollection()->addAction("attachment_properties");
  a_ctAttachmentProperties->setText(i18n("&Properties"));
  connect( a_ctAttachmentProperties, SIGNAL(triggered(bool)), v_iew, SLOT(editCurrentAttachment()) );

  //options menu

  a_ctDoPost = actionCollection()->add<KToggleAction>("send_news");
  a_ctDoPost->setIcon(KIcon("document-new"));
  a_ctDoPost->setText(i18n("Send &News Article"));
  connect(a_ctDoPost, SIGNAL(triggered(bool)), SLOT(slotToggleDoPost()));

  a_ctDoMail = actionCollection()->add<KToggleAction>("send_mail");
  a_ctDoMail->setIcon(KIcon("mail-send"));
  a_ctDoMail->setText(i18n("Send E&mail"));
  a_ctDoMail->setEnabled(allowMail);
  connect(a_ctDoMail, SIGNAL(triggered(bool)), SLOT(slotToggleDoMail()));

  a_ctSetCharset = actionCollection()->add<KSelectAction>("set_charset");
  a_ctSetCharset->setText(i18n("Set &Charset"));
  a_ctSetCharset->setItems( Locale::encodings() );
  a_ctSetCharset->setShortcutConfigurable(false);
  connect(a_ctSetCharset, SIGNAL(triggered(QString)),
  this, SLOT(slotSetCharset(QString)));

  a_ctSetCharsetKeyb = actionCollection()->addAction("set_charset_keyboard");
  a_ctSetCharsetKeyb->setText(i18n("Set Charset"));
  connect(a_ctSetCharsetKeyb, SIGNAL(triggered(bool)), SLOT(slotSetCharsetKeyboard()));
  addAction( a_ctSetCharsetKeyb );


  a_ctWordWrap = actionCollection()->add<KToggleAction>("toggle_wordwrap");
  a_ctWordWrap->setText(i18n("&Word Wrap"));
  connect(a_ctWordWrap, SIGNAL(triggered(bool)), SLOT(slotToggleWordWrap()));

  a_ctAutoSpellChecking = new KToggleAction( KIcon( "tools-check-spelling" ), i18n("&Automatic Spellchecking"), this );
  actionCollection()->addAction( "options_auto_spellchecking", a_ctAutoSpellChecking );
  a_ctAutoSpellChecking->setChecked( knGlobals.settings()->autoSpellChecking() );
  slotUpdateCheckSpellChecking( knGlobals.settings()->autoSpellChecking() );
  slotAutoSpellCheckingToggled();
  connect(a_ctAutoSpellChecking, SIGNAL(triggered(bool)), SLOT(slotAutoSpellCheckingToggled()));
  connect( v_iew->editor(), SIGNAL(checkSpellingChanged(bool)), this, SLOT(slotUpdateCheckSpellChecking(bool)));


  //tools menu

  action = actionCollection()->addAction("tools_quote");
  action->setText(i18n("Add &Quote Characters"));
  connect(action, SIGNAL(triggered(bool)), v_iew->editor(), SLOT(slotAddQuotes()));

  action = actionCollection()->addAction("tools_unquote");
  action->setText(i18n("&Remove Quote Characters"));
  connect(action, SIGNAL(triggered(bool)), v_iew->editor(), SLOT(slotRemoveQuotes()));

  action = actionCollection()->addAction("tools_box");
  action->setText(i18n("Add &Box"));
  connect(action, SIGNAL(triggered(bool)), v_iew->editor(), SLOT(slotAddBox()));

  action = actionCollection()->addAction("tools_unbox");
  action->setText(i18n("Re&move Box"));
  connect(action, SIGNAL(triggered(bool)), v_iew->editor(), SLOT(slotRemoveBox()));

  QAction *undoRewrap = actionCollection()->addAction("tools_undoRewrap");
  undoRewrap->setText(i18n("Get &Original Text (not re-wrapped)"));
  connect(undoRewrap, SIGNAL(triggered(bool)), SLOT(slotUndoRewrap()));
  undoRewrap->setEnabled(!u_nwraped.isNull());

  QAction *rot13 = actionCollection()->addAction("tools_rot13");
  rot13->setIcon(KIcon("document-encrypt"));
  rot13->setText(i18n("S&cramble (Rot 13)"));
  connect(rot13, SIGNAL(triggered(bool)), v_iew->editor(), SLOT(slotRot13()));
  rot13->setEnabled(false);
  connect(v_iew->editor(), SIGNAL(copyAvailable(bool)), rot13, SLOT(setEnabled(bool)));

  a_ctExternalEditor = actionCollection()->addAction("external_editor");
  a_ctExternalEditor->setIcon(KIcon("system-run"));
  a_ctExternalEditor->setText(i18n("Start &External Editor"));
  connect(a_ctExternalEditor, SIGNAL(triggered(bool)), SLOT(slotExternalEditor()));

  a_ctSpellCheck = KStandardAction::spelling ( v_iew->editor(), SLOT(checkSpelling()), actionCollection());

  //settings menu
  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);

  KStandardAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());

  KStandardAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());

  KStandardAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());


  createGUI("kncomposerui.rc");

  //---------------------------------- </Actions> ----------------------------------------

  //init
  initData(text);

  //apply configuration
  setConfig(false);

  if (createCopy && (m_ode==news)) {
    a_ctDoMail->setChecked(true);
    slotToggleDoMail();
  }

  v_iew->completeSetup( firstEdit, m_ode );

  // restore window & toolbar configuration
  resize(535,450);    // default optimized for 800x600
  applyMainWindowSettings(knGlobals.config()->group("composerWindow_options"));

  // starting the external editor
  if ( knGlobals.settings()->useExternalEditor() )
    slotExternalEditor();
}


KNComposer::~KNComposer()
{
  // prevent slotEditorFinished from being called
  if (e_xternalEditor)
    e_xternalEditor->disconnect();
  delete e_xternalEditor;  // this also kills the editor process if it's still running

  delete e_ditorTempfile;

  saveMainWindowSettings(knGlobals.config()->group("composerWindow_options"));

  KNGlobals::self()->settings()->setAutoSpellChecking( a_ctAutoSpellChecking->isChecked() );
  KNGlobals::self()->settings()->writeConfig();
}

void KNComposer::slotUpdateCheckSpellChecking(bool _b)
{
  a_ctAutoSpellChecking->setChecked(_b);
}


void KNComposer::slotUndo()
{
    QWidget* fw = focusWidget();
    if (!fw) return;

    if (fw->inherits("KTextEdit"))
        ((KTextEdit*)fw)->undo();
    else if (fw->inherits("QLineEdit"))
        ((QLineEdit*)fw)->undo();
}

void KNComposer::slotRedo()
{
    QWidget* fw = focusWidget();
    if (!fw) return;

    if (fw->inherits("KTextEdit"))
        ((KTextEdit*)fw)->redo();
    else if (fw->inherits("QLineEdit"))
        ((QLineEdit*)fw)->redo();
}

void KNComposer::slotCut()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KTextEdit"))
    ((KTextEdit*)fw)->cut();
  else if (fw->inherits("QLineEdit"))
    ((QLineEdit*)fw)->cut();
  else kDebug(5003) <<"wrong focus widget";
}

void KNComposer::slotCopy()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KTextEdit"))
    ((KTextEdit*)fw)->copy();
  else if (fw->inherits("QLineEdit"))
    ((QLineEdit*)fw)->copy();
  else kDebug(5003) <<"wrong focus widget";

}


void KNComposer::slotPaste()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("KTextEdit"))
    ((KTextEdit*)fw)->paste();
  else if (fw->inherits("QLineEdit"))
    ((QLineEdit*)fw)->paste();
  else kDebug(5003) <<"wrong focus widget";
}

void KNComposer::slotSelectAll()
{
  QWidget* fw = focusWidget();
  if (!fw) return;

  if (fw->inherits("QLineEdit"))
      ((QLineEdit*)fw)->selectAll();
  else if (fw->inherits("KTextEdit"))
    ((KTextEdit*)fw)->selectAll();
}


void KNComposer::setConfig(bool onlyFonts)
{
  if (!onlyFonts) {
    a_ctWordWrap->setChecked( knGlobals.settings()->wordWrap() );
    slotToggleWordWrap();

    a_ctAutoSpellChecking->setChecked( knGlobals.settings()->autoSpellChecking() );
    Kpgp::Module *pgp = Kpgp::Module::getKpgp();
    a_ctPGPsign->setEnabled(pgp->usePGP());
  }

  v_iew->setComposingFont( knGlobals.settings()->composerFont() );

  slotUpdateStatusBar();
}


void KNComposer::setMessageMode(MessageMode mode)
{
  m_ode = mode;
  a_ctDoPost->setChecked(m_ode!=mail);
  a_ctDoMail->setChecked(m_ode!=news);
  v_iew->setMessageMode(m_ode);
  //Laurent fixme
#if 0
  QString s = v_iew->editor()->document ()->begin().text ();
  if (m_ode == news_mail) {
    if (!s.contains(i18n("<posted & mailed>"))) {
      QTextCursor cursor(v_iew->editor()->document ()->begin());
      cursor.setPosition(0);
      cursor.insertText(i18n("<posted & mailed>\n\n"));
      v_iew->editor()->setTextCursor(cursor);
      }
  } else {
    if (s == i18n("<posted & mailed>")) {
      v_iew->editor()->removeLine(0);
      if (v_iew->editor()->textLine(0).isEmpty())
        v_iew->editor()->removeLine(0);
    }
  }
#endif
  slotUpdateStatusBar();
}


bool KNComposer::hasValidData()
{
  v_alidated=false;
  n_eeds8Bit=false;

  // header checks
  if ( KPIMUtils::isValidAddress( v_iew->from() ) != KPIMUtils::AddressOk ) {
    KMessageBox::sorry( this, i18n( "Your email address does not appears to be valid. Please modify it." ) );
    return false;
  }

  if ( v_iew->subject().isEmpty() ) {
    KMessageBox::sorry(this, i18n("Please enter a subject."));
    return false;
  }
  if ( !n_eeds8Bit && !KMime::isUsAscii( v_iew->subject() ) ) {
    n_eeds8Bit=true;
  }

  if (m_ode != mail) {
    int groupCount = v_iew->groups().count();
    if ( groupCount == 0 ) {
      KMessageBox::sorry(this, i18n("Please enter a newsgroup."));
      return false;
    }

    if (groupCount>12) {
      KMessageBox::sorry(this, i18n("You are crossposting to more than 12 newsgroups.\nPlease remove all newsgroups in which your article is off-topic."));
      return false;
    }

    if (groupCount>5)
      if ( KMessageBox::warningYesNo( this, i18n("You are crossposting to more than five newsgroups.\nPlease reconsider whether this is really useful\nand remove groups in which your article is off-topic.\nDo you want to re-edit the article or send it anyway?"),
            QString(), KGuiItem(i18n("&Send")), KGuiItem(i18nc("edit article","&Edit")) ) != KMessageBox::Yes )
        return false;

    int fupCount = v_iew->followupTo().count();
    if ( fupCount == 0 && groupCount > 2 ) {
      if ( KMessageBox::warningYesNo( this,
           i18n("You are crossposting to more than two newsgroups.\n"
                "Please use the \"Followup-To\" header to direct the replies "
                "to your article into one group.\n"
                "Do you want to re-edit the article or send it anyway?"),
           QString(), KGuiItem(i18n("&Send")), KGuiItem(i18nc("edit article","&Edit")), "missingFollowUpTo" )
           != KMessageBox::Yes )
        return false;
    }

    if (fupCount>12) {
      KMessageBox::sorry(this, i18n("You are directing replies to more than 12 newsgroups.\nPlease remove some newsgroups from the \"Followup-To\" header."));
      return false;
    }

    if (fupCount>5)
      if ( KMessageBox::warningYesNo( this, i18n("You are directing replies to more than five newsgroups.\nPlease reconsider whether this is really useful.\nDo you want to re-edit the article or send it anyway?"),
            QString(), KGuiItem(i18n("&Send")),KGuiItem(i18nc("edit article","&Edit")) ) != KMessageBox::Yes )
        return false;
  }

  if (m_ode != news) {
    if ( v_iew->emailRecipient().isEmpty() ) {
      KMessageBox::sorry(this, i18n("Please enter the email address."));
      return false;
    }
    if ( !n_eeds8Bit && !KMime::isUsAscii( v_iew->emailRecipient() ) ) {
      n_eeds8Bit=true;
    }
  }

  //GNKSA body checks
  bool firstLine = true;
  bool empty = true;
  bool longLine = false;
  bool hasAttributionLine = false;
  int sigLength = 0;
  int notQuoted = 0;
  int textLines = 0;
  QStringList text = v_iew->editor()->toWrappedPlainText().split('\n');

  for (QStringList::Iterator it = text.begin(); it != text.end(); ++it) {

    if (!n_eeds8Bit && !KMime::isUsAscii(*it))
      n_eeds8Bit=true;

    if (*it == "-- ") {   // signature text
      for (++it; it != text.end(); ++it) {

        if (!n_eeds8Bit && !KMime::isUsAscii(*it))
          n_eeds8Bit=true;

        sigLength++;
        if((*it).length()>80) {
          longLine = true;
        }
      }
      break;
    }

    if(!(*it).isEmpty()) {
      empty = false;
      textLines++;
      if ((*it)[0]!='>') {
        notQuoted++;
        if (firstLine) hasAttributionLine = true;
      }
    }
    if((*it).length()>80) {
      longLine = true;
    }

    firstLine = false;
  }

  if (n_eeds8Bit && ( mCharset.toLower()=="us-ascii" )) {
    KMessageBox::sorry(this, i18n("Your message contains characters which are not included\nin the \"us-ascii\" character set; please choose\na suitable character set from the \"Options\" menu."));
    return false;
  }

  if (empty) {
    KMessageBox::sorry(this, i18n("You cannot post an empty message."));
    return false;
  }

  if ((textLines>1)&&(notQuoted==1)) {
    if (hasAttributionLine)
      if ( KMessageBox::warningYesNo( this, i18n("Your article seems to consist entirely of quoted text;\ndo you want to re-edit the article or send it anyway?"),
           QString(), KGuiItem(i18n("&Send")), KGuiItem(i18nc("edit article","&Edit")) ) != KMessageBox::Yes )
        return false;
  } else {
    if (notQuoted==0) {
      KMessageBox::sorry(this, i18n("You cannot post an article consisting\n"
                              "entirely of quoted text."));
      return false;
    }
  }

  if (longLine)
    if ( KMessageBox::warningYesNo( this,
          i18n("Your article contains lines longer than 80 characters.\n"
               "Do you want to re-edit the article or send it anyway?"),
          QString(), KGuiItem(i18n("&Send")),
          KGuiItem(i18nc("edit article","&Edit")) ) != KMessageBox::Yes )
      return false;

  if (sigLength>8) {
    if ( KMessageBox::warningYesNo( this, i18n("Your signature is more than 8 lines long.\nYou should shorten it to match the widely accepted limit of 4 lines.\nDo you want to re-edit the article or send it anyway?"),
         QString(), KGuiItem(i18n("&Send")), KGuiItem(i18nc("edit article","&Edit")) ) != KMessageBox::Yes )
      return false;
  } else
    if (sigLength>4)
       KMessageBox::information( this, i18n("Your signature exceeds the widely-accepted limit of 4 lines:\nplease consider shortening your signature;\notherwise, you will probably annoy your readers."),
                                QString(), "longSignatureWarning" );

  // check if article can be signed
  if ( a_ctPGPsign->isChecked() ) {
    // try to get the signing key
    QByteArray signingKey = KNGlobals::self()->settings()->identity().pgpSigningKey();
    KNNntpAccount::Ptr acc = knGlobals.accountManager()->account( a_rticle->serverId() );
    if ( acc ) {
      KMime::Headers::Newsgroups *grps = a_rticle->newsgroups();
      if ( !grps->isEmpty() ) {
        KNGroup::Ptr grp = knGlobals.groupManager()->group( grps->groups().first(), acc );
        if ( grp && !grp->identity().isNull() ) {
          signingKey = grp->identity().pgpSigningKey();
        } else if ( !acc->identity().isNull() ) {
          signingKey = acc->identity().pgpSigningKey();
        }
      }
    }

    // the article can only be signed if we have a key
    if (signingKey.isEmpty()) {
          if ( KMessageBox::warningContinueCancel( this,
                   i18n("You have not configured your preferred "
                        "signing key yet;\n"
                        "please specify it in the global "
                        "identity configuration,\n"
                        "in the account properties or in the "
                        "group properties.\n"
                        "The article will be sent unsigned." ),
                   QString(), KGuiItem(i18n( "Send Unsigned" )),
                   KStandardGuiItem::cancel(), "sendUnsignedDialog" )
               == KMessageBox::Cancel )
             return false;
    }
  }

  v_alidated=true;
  return true;
}


bool KNComposer::applyChanges()
{
  KMime::Content *text=0;
  bool result = true; // no error occurs ?


  // Identity (for later edition)
  const KPIMIdentities::Identity identity = KNGlobals::self()->identityManager()->identityForUoid( v_iew->selectedIdentity() );
  if ( !identity.isNull() ) {
    KMime::Headers::Generic *xKnodeIdentity = new KMime::Headers::Generic( "X-KNode-Identity",
                                                                          a_rticle.get(),
                                                                          QByteArray::number( identity.uoid() ) );
    a_rticle->setHeader( xKnodeIdentity );
  }

  //From
  if ( KPIMUtils::isValidAddress( v_iew->from() ) != KPIMUtils::AddressOk ) {
    result = false;
  }
  // FIXME: if v_iew->from() is not valid, the following call is a
  // no-op: the content of the from keeps its previous value! (thanks KMime)
  a_rticle->from()->fromUnicodeString( v_iew->from(), mCharset.toLatin1() );

  //Reply-To
  if ( KPIMUtils::isValidAddress( identity.replyToAddr() ) == KPIMUtils::AddressOk ) {
    a_rticle->replyTo()->fromUnicodeString( identity.replyToAddr(), mCharset.toLatin1() );
  } else {
    a_rticle->removeHeader( "Reply-To" );
  }

  //Mail-Copies-To
  if ( !identity.property( "Mail-Copies-To" ).toString().trimmed().isEmpty() ) {
    a_rticle->mailCopiesTo()->fromUnicodeString( identity.property( "Mail-Copies-To" ).toString(), mCharset.toLatin1() );
  } else {
    a_rticle->removeHeader( "Mail-Copies-To" );
  }

  //Organization
  if ( !identity.organization().trimmed().isEmpty() ) {
    a_rticle->organization()->fromUnicodeString( identity.organization(), mCharset.toLatin1() );
  } else {
    a_rticle->removeHeader( "Organization" );
  }

  //Date
  a_rticle->date()->setDateTime( KDateTime::currentLocalDateTime() );    //set current date+time

  //Subject
  a_rticle->subject()->fromUnicodeString( v_iew->subject(), mCharset.toLatin1() );

  //Newsgroups
  if (m_ode != mail) {
    a_rticle->newsgroups()->fromUnicodeString( v_iew->groups().join( QString( ',' ) ), KMime::Headers::Latin1 );
    a_rticle->setDoPost(true);
  } else {
    a_rticle->setDoPost(false);
    a_rticle->removeHeader( "Newsgroups" );
  }

  //To
  if (m_ode != news) {
    a_rticle->to()->fromUnicodeString( v_iew->emailRecipient(), mCharset.toLatin1() );
    a_rticle->setDoMail(true);
  } else {
    a_rticle->setDoMail(false);
    a_rticle->removeHeader( "To" );
    a_rticle->removeHeader( "Cc" );
  }

  //Followup-To
  if ( a_rticle->doPost() && !v_iew->followupTo().isEmpty() ) {
    a_rticle->followUpTo()->fromUnicodeString( v_iew->followupTo().join( QString( ',' ) ), KMime::Headers::Latin1 );
  } else {
    a_rticle->removeHeader("Followup-To");
  }


  // Attachments
  if ( a_ttChanged ) {
    const QList<KNAttachment::Ptr> l = v_iew->attachments();
    foreach ( const KNAttachment::Ptr a, l ) {
      if(a->hasChanged()) {
        if(a->isAttached())
          a->updateContentInfo();
        else
          a->attach( a_rticle.get() );
      }
    }
  }

  for ( QList<KNAttachment::Ptr>::Iterator it = mDeletedAttachments.begin(); it != mDeletedAttachments.end(); ++it )
    if ( (*it)->isAttached() )
      (*it)->detach( a_rticle.get() );

  text=a_rticle->textContent();

  if(!text) {
    text=new KMime::Content();
    KMime::Headers::ContentType *type=text->contentType();
    KMime::Headers::ContentTransferEncoding *enc=text->contentTransferEncoding();
    type->setMimeType("text/plain");
    enc->setDecoded(true);
    text->assemble();
    a_rticle->addContent(text, true);
  }

  //set text
  if (v_alidated) {
    if (n_eeds8Bit) {
      text->contentType()->setCharset( mCharset.toLatin1() );
      if ( knGlobals.settings()->allow8BitBody() )
        text->contentTransferEncoding()->setEncoding(KMime::Headers::CE8Bit);
      else
        text->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
    } else {
      setCharset( "us-ascii" ); // fall back to us-ascii
      text->contentType()->setCharset( mCharset.toLatin1() );
      text->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    }
  } else {             // save as draft
    text->contentType()->setCharset( mCharset.toLatin1() );
    if ( mCharset.toLower()=="us-ascii" ) {
      text->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    } else {
      text->contentTransferEncoding()->setEncoding( knGlobals.settings()->allow8BitBody()
          ? KMime::Headers::CE8Bit : KMime::Headers::CEquPr );
    }
  }

  QString tmp = v_iew->editor()->toWrappedPlainText();

  // Sign article if needed
  if ( a_ctPGPsign->isChecked() ) {
      QByteArray signingKey = identity.pgpSigningKey();
      if (!signingKey.isEmpty()) {
          QString tmpText = tmp;
          Kpgp::Block block;
          bool ok=true;
          QTextCodec *codec=KGlobal::charsets()->codecForName( mCharset, ok);
          if(!ok) // no suitable codec found => try local settings and hope the best ;-)
              codec=KGlobal::locale()->codecForEncoding();

          block.setText( codec->fromUnicode(tmpText) );
          kDebug(5003) <<"signing article from" << article()->from()->addresses();
          if( block.clearsign( signingKey, codec->name() ) == Kpgp::Ok ) {
              QByteArray result = block.text();
              tmp = codec->toUnicode(result.data(), result.length() );
          } else {
              result = false;
          }
      }
  }

  text->fromUnicodeString(tmp);

  //text is set and all attached contents have been assembled => now set lines
  a_rticle->lines()->setNumberOfLines(a_rticle->lineCount());

  a_rticle->assemble();
  a_rticle->updateListItem();

  return result;
}

void KNComposer::setCharset( const QString &charset )
{
  mCharset = Locale::toMimeCharset( charset );
  slotUpdateStatusBar();
}


void KNComposer::closeEvent(QCloseEvent *e)
{
  if(!v_iew->editor()->document()->isModified() && !a_ttChanged) {  // nothing to save, don't show nag screen
    if(a_rticle->id()==-1)
      r_esult=CRdel;
    else
      r_esult=CRcancel;
  }
  else {
    switch ( KMessageBox::warningYesNoCancel( this, i18n("Do you want to save this article in the draft folder?"),
                                              QString(), KStandardGuiItem::save(), KStandardGuiItem::discard() ) ) {
      case KMessageBox::Yes :
        r_esult=CRsave;
        break;
      case KMessageBox::No :
        if (a_rticle->id()==-1) r_esult=CRdel;
          else r_esult=CRcancel;
        break;
      default:            // cancel
        e->ignore();
        return;
    }
  }

    e->accept();
  emit composerDone(this);
  // we're dead at this point, don't access members!
}


void KNComposer::initData(const QString &text)
{
  // Identity
  KPIMIdentities::IdentityManager *idManager = KNGlobals::self()->identityManager();
  KPIMIdentities::Identity identity = idManager->defaultIdentity();
  KMime::Headers::Base* xKnodeIdentity = a_rticle->headerByType( "X-KNode-Identity" );
  if ( xKnodeIdentity && !xKnodeIdentity->isEmpty() ) {
    uint uoid = xKnodeIdentity->asUnicodeString().toUInt();
    // Note: this ensure the identity exists even if it was removed
    identity = idManager->identityForUoidOrDefault( uoid );
  }
  v_iew->setIdentity( identity.uoid() );

  //From
  KMime::Headers::From *from = a_rticle->from( false );
  if ( from  ) {
    v_iew->setFrom( from->asUnicodeString() );
  } else {
    v_iew->setFrom( identity.fullEmailAddr() );
  }

  //Subject
  if(a_rticle->subject()->isEmpty())
    slotSubjectChanged( QString() );
  else
    v_iew->setSubject( a_rticle->subject()->asUnicodeString() );

  //Newsgroups
  KMime::Headers::Newsgroups *hNewsgroup = a_rticle->newsgroups( false );
  if ( hNewsgroup && !hNewsgroup->isEmpty() ) {
    v_iew->setGroups( hNewsgroup->asUnicodeString() );
  }

  //To
  KMime::Headers::To *hTo = a_rticle->to( false );
  if ( hTo && !hTo->isEmpty() ) {
    v_iew->setEmailRecipient( hTo->asUnicodeString() );
  }

  //Followup-To
  KMime::Headers::FollowUpTo *fup2=a_rticle->followUpTo(false);
  if( fup2 && !fup2->isEmpty() ) {
    v_iew->setFollowupTo( fup2->asUnicodeString() );
  }

  KMime::Content *textContent=a_rticle->textContent();
  QString s;

  if(text.isEmpty()) {
    if(textContent)
      s = textContent->decodedText();
  } else
    s = text;

  v_iew->editor()->setText(s);

  // initialize the charset select action
  if(textContent) {
    setCharset( textContent->contentType()->charset() );
  } else {
    setCharset( knGlobals.settings()->charset() );
  }

  QString charsetDesc = KGlobal::charsets()->descriptionForEncoding( mCharset );
  a_ctSetCharset->setCurrentItem( a_ctSetCharset->items().indexOf( charsetDesc ) );

  // initialize the message type select action
  if (a_rticle->doPost() && a_rticle->doMail())
    m_ode = news_mail;
  else
    if (a_rticle->doPost())
      m_ode = news;
    else
      m_ode = mail;
  setMessageMode(m_ode);

  if(a_rticle->contentType()->isMultipart()) {
    v_iew->showAttachmentView();
    const KMime::Content::List attList = a_rticle->attachments();
    foreach ( KMime::Content *c, attList ) {
      v_iew->addAttachment( KNAttachment::Ptr( new KNAttachment( c ) ) );
    }
  }
}

// inserts at cursor position if clear is false, replaces content otherwise
// puts the file content into a box if box==true
// "file" is already open for reading
void KNComposer::insertFile( QFile *file, bool clear, bool box, const QString &boxTitle )
{
  QString temp;
  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName( mCharset, ok);
  QTextStream ts(file);
  ts.setCodec(codec);

  if (box)
    temp = QString::fromLatin1(",----[ %1 ]\n").arg(boxTitle);
  //Laurent fixme
  if (box && (v_iew->editor()->wordWrapMode()!=QTextOption::NoWrap)) {
    int wrapAt = v_iew->editor()->lineWrapColumnOrWidth();
    QStringList lst;
    QString line;
    while(!ts.atEnd()) {
      line=ts.readLine();
      if (!ts.atEnd())
        line+='\n';
      lst.append(line);
    }
    temp+=KNHelper::rewrapStringList(lst, wrapAt, '|', false, true);
  } else {
    while(!ts.atEnd()) {
      if (box)
        temp+="| ";
      temp+=ts.readLine();
      if (!ts.atEnd())
        temp += '\n';
    }
  }
  if (box)
    temp += QString::fromLatin1("`----");

  if(clear)
    v_iew->editor()->setText(temp);
  else
    v_iew->editor()->insertPlainText(temp);
}


// ask for a filename, handle network urls
void KNComposer::insertFile(bool clear, bool box)
{
  KNLoadHelper helper(this);
  QFile *file = helper.getFile(i18n("Insert File"));
  KUrl url;
  QString boxName;

  if (file) {
    url = helper.getURL();

    if (url.isLocalFile())
      boxName = url.toLocalFile();
    else
      boxName = url.prettyUrl();

    insertFile(file,clear,box,boxName);
  }
}


//-------------------------------- <Actions> ------------------------------------


void KNComposer::addRecentAddress()
{
  if ( m_ode == mail || m_ode == news_mail ) {
    RecentAddresses::self( knGlobals.config() )->add( v_iew->emailRecipient() );
  }
}

void KNComposer::slotSendNow()
{
  r_esult=CRsendNow;
  addRecentAddress();
  emit composerDone(this);
}


void KNComposer::slotSendLater()
{
  r_esult=CRsendLater;
  addRecentAddress();
  emit composerDone(this);
}


void KNComposer::slotSaveAsDraft()
{
  r_esult=CRsave;
  addRecentAddress();
  emit composerDone(this);
}


void KNComposer::slotArtDelete()
{
  r_esult=CRdelAsk;
  emit composerDone(this);
}


void KNComposer::slotInsertFile()
{
  insertFile(false,false);
}


void KNComposer::slotInsertFileBoxed()
{
  insertFile(false,true);
}

void KNComposer::slotAttachFile()
{
  KNLoadHelper *helper = new KNLoadHelper(this);

  if (helper->getFile(i18n("Attach File"))) {
   if ( !v_iew->isAttachmentViewVisible() ) {
      KNHelper::saveWindowSize("composer", size());
      v_iew->showAttachmentView();
    }
    v_iew->addAttachment( KNAttachment::Ptr( new KNAttachment( helper ) ) );
    a_ttChanged=true;
  } else {
    delete helper;
  }
}


void KNComposer::slotAttachmentRemoved( KNAttachment::Ptr attachment, bool last )
{
   if( !attachment ) {
    return;
  }

  if ( attachment->isAttached() ) {
    mDeletedAttachments.append( attachment );
  }

  if ( last ) {
    KNHelper::saveWindowSize( "composerAtt", size() );
    v_iew->hideAttachmentView();
  }

  a_ttChanged = true;
}

void KNComposer::slotAttachmentChanged()
{
  a_ttChanged = true;
}



void KNComposer::slotToggleDoPost()
{
  if (a_ctDoPost->isChecked()) {
    if (a_ctDoMail->isChecked())
      m_ode=news_mail;
    else
      m_ode=news;
  } else {
    if (a_ctDoMail->isChecked())
      m_ode=mail;
    else {     // invalid
      a_ctDoPost->setChecked(true); //revert
      return;
    }
  }
  setMessageMode(m_ode);
}


void KNComposer::slotToggleDoMail()
{
  if (a_ctDoMail->isChecked()) {
    if (a_uthorDislikesMailCopies) {
      if ( KMessageBox::warningContinueCancel( this, i18n("The poster does not want a mail copy of your reply (Mail-Copies-To: nobody);\nplease respect their request."),
                                               QString(), KGuiItem(i18n("&Send Copy")) ) != KMessageBox::Continue ) {
        a_ctDoMail->setChecked(false); //revert
        return;
      }
    }
  }
//Laurent fix me
#if 0
    if ( knGlobals.settings()->useExternalMailer() ) {
      QString s = v_iew->editor()->textLine(0);
      if (!s.contains(i18n("<posted & mailed>")))
        v_iew->editor()->insertAt(i18n("<posted & mailed>\n\n"),0,0);
      QString tmp;
      QStringList textLines = v_iew->editor()->processedText();
      for (QStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it) {
        if (*it == "-- ")   // try to be smart, don't include the signature,
          break;            // kmail will append one, too.
        tmp+=*it+'\n';
      }
      knGlobals.artFactory->sendMailExternal( v_iew->t_o->text(), v_iew->subject(), tmp );
      a_ctDoMail->setChecked(false); //revert
      return;
    } else {
#endif
  if (a_ctDoMail->isChecked()) {
    if (a_ctDoPost->isChecked())
      m_ode=news_mail;
    else
      m_ode=mail;
  } else {
    if (a_ctDoPost->isChecked())
      m_ode=news;
    else {     // invalid
      a_ctDoMail->setChecked(true); //revert
      return;
    }
  }
  setMessageMode(m_ode);
}


void KNComposer::slotSetCharset(const QString &s)
{
  if(s.isEmpty())
    return;

  QString charset = KGlobal::charsets()->encodingForName( s );
  setCharset( charset );
}


void KNComposer::slotSetCharsetKeyboard()
{
  int newCS = KNHelper::selectDialog(this, i18n("Select Charset"), a_ctSetCharset->items(), a_ctSetCharset->currentItem());
  if (newCS != -1) {
    a_ctSetCharset->setCurrentItem(newCS);
    QString charset = KGlobal::charsets()->encodingForName( a_ctSetCharset->items()[ newCS ] );
    setCharset( charset );
  }
}


void KNComposer::slotToggleWordWrap()
{
  if ( a_ctWordWrap->isChecked() )
    v_iew->editor()->enableWordWrap( knGlobals.settings()->maxLineLength() );
  else
    v_iew->editor()->disableWordWrap();
}

void KNComposer::slotAutoSpellCheckingToggled()
{
  v_iew->editor()->setCheckSpellingEnabled( a_ctAutoSpellChecking->isChecked() );
}


void KNComposer::slotUndoRewrap()
{
  if (KMessageBox::warningContinueCancel( this, i18n("This will replace all text you have written.")) == KMessageBox::Continue) {
    v_iew->editor()->setText(u_nwraped);
    v_iew->appendSignature();
  }
}

void KNComposer::slotExternalEditor()
{
  if(e_xternalEditor)   // in progress...
    return;

  QString editorCommand = knGlobals.settings()->externalEditor();

  if(editorCommand.isEmpty())
    KMessageBox::sorry(this, i18n("No editor configured.\nPlease do this in the settings dialog."));

  delete e_ditorTempfile;
  e_ditorTempfile=new KTemporaryFile();

  if(!e_ditorTempfile->open()) {
    KNHelper::displayTempFileError(this);
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName( mCharset, ok );

  QString tmp = v_iew->editor()->toWrappedPlainText();

  QByteArray local = codec->fromUnicode(tmp);
  e_ditorTempfile->write(local.data(),local.length());
  e_ditorTempfile->flush();

  if(!e_ditorTempfile->open()) {
    KNHelper::displayTempFileError(this);
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  e_xternalEditor=new KProcess();

  // construct command line...
  QStringList command = editorCommand.split(' ', QString::SkipEmptyParts);
  bool filenameAdded=false;
  for ( QStringList::Iterator it = command.begin(); it != command.end(); ++it ) {
    if ((*it).contains("%f")) {
      (*it).replace(QRegExp("%f"),e_ditorTempfile->fileName());
      filenameAdded=true;
    }
    (*e_xternalEditor) << (*it);
  }
  if(!filenameAdded)    // no %f in the editor command
    (*e_xternalEditor) << e_ditorTempfile->fileName();

  connect(e_xternalEditor, SIGNAL(finished(int,QProcess::ExitStatus)),this, SLOT(slotEditorFinished(int,QProcess::ExitStatus)));
  e_xternalEditor->start();
  if(!e_xternalEditor->waitForStarted()) {
    KMessageBox::error(this, i18n("Unable to start external editor.\nPlease check your configuration in the settings dialog."));
    delete e_xternalEditor;
    e_xternalEditor=0;
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  a_ctExternalEditor->setEnabled(false);   // block other edit action while the editor is running...
  a_ctSpellCheck->setEnabled(false);
  v_iew->showExternalNotification();
}

void KNComposer::slotUpdateStatusBar()
{
  QString typeDesc;
  switch (m_ode) {
    case news:  typeDesc = i18n("News Article");
                break;
    case mail:  typeDesc = i18n("Email");
                break;
    default  :  typeDesc = i18n("News Article & Email");
  }
  QString overwriteDesc;
  if (v_iew->editor()->overwriteMode ())
    overwriteDesc = i18n(" OVR ");
  else
    overwriteDesc = i18n(" INS ");

  statusBar()->changeItem(i18n(" Type: %1 ", typeDesc), 1);
  statusBar()->changeItem(i18n(" Charset: %1 ", mCharset ), 2);
  statusBar()->changeItem(overwriteDesc, 3);
  statusBar()->changeItem(i18n(" Column: %1 ", v_iew->editor()->columnNumber () + 1), 4);
  statusBar()->changeItem(i18n(" Line: %1 ", v_iew->editor()->linePosition() + 1), 5);
}


void KNComposer::slotUpdateCursorPos()
{
  statusBar()->changeItem(i18n(" Column: %1 ", v_iew->editor()->columnNumber () + 1), 4);
  statusBar()->changeItem(i18n(" Line: %1 ", v_iew->editor()->linePosition() + 1), 5);
}


void KNComposer::slotConfKeys()
{
  KShortcutsDialog::configure(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this, true);
}


void KNComposer::slotConfToolbar()
{
  saveMainWindowSettings(knGlobals.config()->group( "composerWindow_options") );
  KEditToolBar dlg(guiFactory(),this);
  connect(&dlg,SIGNAL(newToolBarConfig()), this, SLOT(slotNewToolbarConfig()));
  dlg.exec();
}

void KNComposer::slotNewToolbarConfig()
{
  createGUI("kncomposerui.rc");

  applyMainWindowSettings(knGlobals.config()->group("composerWindow_options"));
}

//-------------------------------- </Actions> -----------------------------------


void KNComposer::slotSubjectChanged(const QString &t)
{
  // replace newlines
  QString subject = t;
  subject.replace( '\n', ' ' );
  subject.replace( '\r', ' ' );
  if ( subject != t ) // setText() sets the cursor to the end
    v_iew->setSubject( subject );
  // update caption
  if( !subject.isEmpty() )
    setCaption( subject );
  else
    setCaption( i18n("No Subject") );
}


void KNComposer::slotToBtnClicked()
{
  Akonadi::EmailAddressSelectionDialog dlg( this );
  dlg.view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );

  if ( !dlg.exec() )
    return;

  QStringList addresses;
  foreach ( const Akonadi::EmailAddressSelection &selection, dlg.selectedAddresses() )
    addresses << selection.quotedEmail();

  QString to = v_iew->emailRecipient();
  if ( !to.isEmpty() )
      to += ", ";
  to += addresses.join( ", " );

  v_iew->setEmailRecipient( to );
}


void KNComposer::slotGroupsBtnClicked()
{
  int id=a_rticle->serverId();
  KNNntpAccount::Ptr nntp;

  if(id!=-1)
    nntp=knGlobals.accountManager()->account(id);

  if(!nntp)
    nntp=knGlobals.accountManager()->first();

  if(!nntp) {
    KMessageBox::error(this, i18n("You have no valid news accounts configured."));
    v_iew->setGroups( QString() );
    return;
  }

  if(id==-1)
    a_rticle->setServerId(nntp->id());

  KNGroupSelectDialog *dlg = new KNGroupSelectDialog( this, nntp, v_iew->groups() );

  connect( dlg, SIGNAL(loadList(KNNntpAccount::Ptr)),
           knGlobals.groupManager(), SLOT(slotLoadGroupList(KNNntpAccount::Ptr)) );
  connect( KNGlobals::self()->groupManager(), SIGNAL(newListReady(KNGroupListData::Ptr)),
           dlg, SLOT(slotReceiveList(KNGroupListData::Ptr)) );

  if(dlg->exec())
    v_iew->setGroups( dlg->selectedGroups() );

  delete dlg;
}

void KNComposer::slotEditorFinished(int, QProcess::ExitStatus exitStatus)
{
  if(exitStatus == QProcess::NormalExit) {
    e_ditorTempfile->flush();
    e_ditorTempfile->seek(0);
    insertFile(e_ditorTempfile, true);
    e_xternalEdited=true;
  }

  slotCancelEditor();   // cleanup...
}


void KNComposer::slotCancelEditor()
{
  if (e_xternalEditor)
    e_xternalEditor->deleteLater();  // this also kills the editor process if it's still running
  e_xternalEditor=0;
  delete e_ditorTempfile;
  e_ditorTempfile=0;

  a_ctExternalEditor->setEnabled(true);
  a_ctSpellCheck->setEnabled(true);
  v_iew->hideExternalNotification();
}


void KNComposer::slotAttachmentPopup( const QPoint &point )
{
  QMenu *menu = static_cast<QMenu*>( factory()->container( "attachment_popup", this ) );
  if ( menu ) {
    menu->popup( point );
  }
}

void KNComposer::dragEnterEvent( QDragEnterEvent *event )
{
  if ( KUrl::List::canDecode( event->mimeData() ) ) {
    event->setDropAction( Qt::CopyAction );
    event->accept();
  }
}

void KNComposer::dropEvent( QDropEvent *event )
{
  KUrl::List urls = KUrl::List::fromMimeData( event->mimeData() );

  foreach ( const KUrl &url, urls ) {
    KNLoadHelper *helper = new KNLoadHelper(this);

    if (helper->setURL(url)) {
      if ( !v_iew->isAttachmentViewVisible() ) {
        KNHelper::saveWindowSize("composer", size());
        v_iew->showAttachmentView();
      }
      v_iew->addAttachment( KNAttachment::Ptr( new KNAttachment( helper ) ) );
      a_ttChanged=true;
    } else {
      delete helper;
    }
  }
}


//=====================================================================================


KNComposer::AttachmentPropertiesDlg::AttachmentPropertiesDlg( KNAttachment::Ptr a, QWidget *parent ) :
  KDialog( parent ), a_ttachment(a),
  n_onTextAsText(false)
{
  //init GUI
  setCaption( i18n("Attachment Properties") );
  setButtons( Help | Ok | Cancel );
  QWidget *page=new QWidget(this);
  setMainWidget(page);
  QVBoxLayout *topL=new QVBoxLayout(page);

  //file info
  QGroupBox *fileGB = new QGroupBox( i18n("File"), page );
  QGridLayout *fileL=new QGridLayout(fileGB);
  fileL->setSpacing(5);
  fileL->setMargin(15);

  fileL->addItem( new QSpacerItem( 0, fontMetrics().lineSpacing()-9), 0, 0 );
  fileL->addWidget(new QLabel(i18n("Name:"), fileGB) ,1,0);
  fileL->addWidget(new QLabel(QString("<b>%1</b>").arg(a->name()), fileGB), 1,1, Qt::AlignLeft);
  fileL->addWidget(new QLabel(i18n("Size:"), fileGB), 2,0);
  fileL->addWidget(new QLabel(a->contentSize(), fileGB), 2,1, Qt::AlignLeft);

  fileL->setColumnStretch(1,1);
  topL->addWidget(fileGB);

  //mime info
  QGroupBox *mimeGB = new QGroupBox( i18n("Mime"), page );
  QGridLayout *mimeL=new QGridLayout(mimeGB);
  mimeL->setSpacing(5);
  mimeL->setMargin(15);

  mimeL->addItem( new QSpacerItem( 0, fontMetrics().lineSpacing()-9), 0, 0 );
  m_imeType=new KLineEdit(mimeGB);
  m_imeType->setText(a->mimeType());
  mimeL->addWidget(m_imeType, 1,1);
  QLabel *label = new QLabel(i18n("&Mime-Type:"), mimeGB);
  label->setBuddy(m_imeType);
  mimeL->addWidget(label, 1,0);

  d_escription=new KLineEdit(mimeGB);
  d_escription->setText(a->description());
  mimeL->addWidget(d_escription, 2,1);
  label=new QLabel(i18n("&Description:"), mimeGB);
  label->setBuddy(d_escription);
  mimeL->addWidget(label, 2,0);

  e_ncoding=new QComboBox(mimeGB);
  e_ncoding->setEditable(false);
  e_ncoding->addItem("7Bit");
  e_ncoding->addItem("8Bit");
  e_ncoding->addItem("quoted-printable");
  e_ncoding->addItem("base64");
  if(a->isFixedBase64()) {
    e_ncoding->setCurrentIndex(3);
    e_ncoding->setEnabled(false);
  }
  else
    e_ncoding->setCurrentIndex(a->cte());
  mimeL->addWidget(e_ncoding, 3,1);
  label=new QLabel(i18n("&Encoding:"), mimeGB);
  label->setBuddy(e_ncoding);
  mimeL->addWidget(label, 3,0);

  mimeL->setColumnStretch(1,1);
  topL->addWidget(mimeGB);

  //connections
  connect(m_imeType, SIGNAL(textChanged(QString)),
    this, SLOT(slotMimeTypeTextChanged(QString)));

  //finish GUI
  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("attProperties", this, QSize(300,250));
  setHelp("anc-knode-editor-advanced");
}


KNComposer::AttachmentPropertiesDlg::~AttachmentPropertiesDlg()
{
  KNHelper::saveWindowSize("attProperties", this->size());
}


void KNComposer::AttachmentPropertiesDlg::apply()
{
  a_ttachment->setDescription(d_escription->text());
  a_ttachment->setMimeType(m_imeType->text());
  a_ttachment->setCte(e_ncoding->currentIndex());
}


void KNComposer::AttachmentPropertiesDlg::accept()
{
  if ( m_imeType->text().indexOf('/') == -1 ) {
    KMessageBox::sorry(this, i18n("You have set an invalid mime-type.\nPlease change it."));
    return;
  }
  else if ( n_onTextAsText && m_imeType->text().indexOf( "text/", 0, Qt::CaseInsensitive ) != -1 &&
       KMessageBox::warningContinueCancel(this,
       i18n("You have changed the mime-type of this non-textual attachment\nto text. This might cause an error while loading or encoding the file.\nProceed?")
       ) == KMessageBox::Cancel) return;

  // update the attachment
  apply();

  KDialog::accept();
}


void KNComposer::AttachmentPropertiesDlg::slotMimeTypeTextChanged(const QString &text)
{
    enableButtonOk( !text.isEmpty() );
  if(text.left(5)!="text/") {
    n_onTextAsText=a_ttachment->isFixedBase64();
    e_ncoding->setCurrentIndex(3);
    e_ncoding->setEnabled(false);
  }
  else {
    e_ncoding->setCurrentIndex(a_ttachment->cte());
    e_ncoding->setEnabled(true);
  }
}


//--------------------------------


// kate: space-indent on; indent-width 2;
