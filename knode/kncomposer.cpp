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

#include <q3header.h>
#include <QTextCodec>
#include <QApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QEvent>
#include <QTextStream>
#include <QByteArray>
#include <QContextMenuEvent>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QCloseEvent>
#include <QLabel>
#include <QtDBus/QtDBus>
#include <qgroupbox.h>
#include <kdeversion.h>
#include "addressesdialog.h"
using KPIM::AddressesDialog;
#include "recentaddresses.h"
using KPIM::RecentAddresses;
#include <kcharsets.h>
#include <kmessagebox.h>
#include <kabc/addresseedialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <kmenu.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <ktemporaryfile.h>
#include <libkpgp/kpgp.h>
#include <libkpgp/kpgpblock.h>
#include <kpimutils/spellingfilter.h>
#include <kcompletionbox.h>
#include <kxmlguifactory.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kselectaction.h>
#include <KStandardGuiItem>
#include <ktoggleaction.h>
#include <kconfiggroup.h>
#include <kicon.h>
#include "kngroupselectdialog.h"
#include "utilities.h"
#include "knglobals.h"
#include "kncomposer.h"
#include "knmainwidget.h"
#include "knconfigmanager.h"
#include "knaccountmanager.h"
#include "knnntpaccount.h"
#include "knarticlefactory.h"
#include "settings.h"
#include "kncomposerview.h"
#include <kmeditor.h>
#include "kncomposereditor.h"


KNLineEdit::KNLineEdit( KNComposer::ComposerView *_composerView, bool useCompletion,
                        QWidget *parent )
    : KNLineEditInherited( parent,useCompletion ), composerView( _composerView )

{
}

void KNLineEdit::contextMenuEvent( QContextMenuEvent*e )
{
   QMenu *popup = KLineEdit::createStandardContextMenu();
   popup->addSeparator();
   popup->insertItem( i18n( "Edit Recent Addresses..." ),
		   this, SLOT( editRecentAddresses() ) );
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
    KNLineEditInherited::loadAddresses();

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
  KNLineEditInherited::keyPressEvent(e);
}

KNLineEditSpell::KNLineEditSpell( KNComposer::ComposerView *_composerView, bool useCompletion,QWidget * parent )
    :KNLineEdit( _composerView, useCompletion, parent )
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


KNComposer::KNComposer(KNLocalArticle *a, const QString &text, const QString &sig, const QString &unwraped, bool firstEdit, bool dislikesCopies, bool createCopy)
    : KXmlGuiWindow(0), r_esult(CRsave), a_rticle(a), s_ignature(sig), u_nwraped(unwraped),
      n_eeds8Bit(true), v_alidated(false), a_uthorDislikesMailCopies(dislikesCopies), e_xternalEdited(false), e_xternalEditor(0),
      e_ditorTempfile(0), a_ttChanged(false),
      mFirstEdit( firstEdit )
{
  setObjectName( "composerWindow" );
    mSpellingFilter = 0;
    spellLineEdit = false;

  if( knGlobals.componentData().isValid() )
    setComponentData( knGlobals.componentData() );

  // activate dnd of attachments...
  setAcceptDrops(true);

  //init v_iew
  v_iew=new ComposerView(this);
  setCentralWidget(v_iew);

  connect(v_iew->c_ancelEditorBtn, SIGNAL(clicked()), SLOT(slotCancelEditor()));
  connect(v_iew->e_dit, SIGNAL(sigDragEnterEvent(QDragEnterEvent *)), SLOT(slotDragEnterEvent(QDragEnterEvent *)));
  connect(v_iew->e_dit, SIGNAL(sigDropEvent(QDropEvent *)), SLOT(slotDropEvent(QDropEvent *)));

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
  connect(v_iew->e_dit, SIGNAL(cursorPositionChanged()), SLOT(slotUpdateCursorPos()));
  connect(v_iew->e_dit, SIGNAL(overwriteModeText()), SLOT(slotUpdateStatusBar()));

  QDBusConnection::sessionBus().registerObject( "/Composer", this, QDBusConnection::ExportScriptableSlots );
  //------------------------------- <Actions> --------------------------------------

  //file menu
  QAction *action = actionCollection()->addAction("send_now");
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
  connect(action, SIGNAL(triggered(bool) ), v_iew->e_dit, SLOT(slotPasteAsQuotation()));

  KStandardAction::selectAll(this, SLOT(slotSelectAll()), actionCollection());

  KStandardAction::find(v_iew->e_dit, SLOT(slotFind()), actionCollection());
  KStandardAction::findNext(v_iew->e_dit, SLOT(slotFindNext()), actionCollection());

  KStandardAction::replace(v_iew->e_dit, SLOT(slotReplace()), actionCollection());

  //attach menu
  action = actionCollection()->addAction("append_signature");
  action->setText(i18n("Append &Signature"));
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotAppendSig()));

  action = actionCollection()->addAction("insert_file");
  action->setText(i18n("&Insert File..."));
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotInsertFile()));

  action = actionCollection()->addAction("insert_file_boxed");
  action->setText(i18n("Insert File (in a &box)..."));
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotInsertFileBoxed()));

  action = actionCollection()->addAction("attach_file");
  action->setIcon(KIcon("mail-attachment"));
  action->setText(i18n("Attach &File..."));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotAttachFile()));

  a_ctPGPsign = actionCollection()->add<KToggleAction>("sign_article");
  a_ctPGPsign->setText(i18n("Sign Article with &PGP"));
  a_ctPGPsign->setIcon(KIcon("document-sign"));

  a_ctRemoveAttachment = actionCollection()->addAction("remove_attachment");
  a_ctRemoveAttachment->setText(i18n("&Remove"));
  connect(a_ctRemoveAttachment, SIGNAL(triggered(bool) ), SLOT(slotRemoveAttachment()));

  a_ctAttachmentProperties = actionCollection()->addAction("attachment_properties");
  a_ctAttachmentProperties->setText(i18n("&Properties"));
  connect(a_ctAttachmentProperties, SIGNAL(triggered(bool) ), SLOT(slotAttachmentProperties()));

  //options menu

  a_ctDoPost = actionCollection()->add<KToggleAction>("send_news");
  a_ctDoPost->setIcon(KIcon("document-new"));
  a_ctDoPost->setText(i18n("Send &News Article"));
  connect(a_ctDoPost, SIGNAL(triggered(bool) ), SLOT(slotToggleDoPost()));

  a_ctDoMail = actionCollection()->add<KToggleAction>("send_mail");
  a_ctDoMail->setIcon(KIcon("mail-send"));
  a_ctDoMail->setText(i18n("Send E&mail"));
  connect(a_ctDoMail, SIGNAL(triggered(bool) ), SLOT(slotToggleDoMail()));

  a_ctSetCharset = actionCollection()->add<KSelectAction>("set_charset");
  a_ctSetCharset->setText(i18n("Set &Charset"));
  a_ctSetCharset->setItems( KGlobal::charsets()->availableEncodingNames() );
  a_ctSetCharset->setShortcutConfigurable(false);
  connect(a_ctSetCharset, SIGNAL(triggered(const QString&)),
  this, SLOT(slotSetCharset(const QString&)));

  a_ctSetCharsetKeyb = actionCollection()->addAction("set_charset_keyboard");
  a_ctSetCharsetKeyb->setText(i18n("Set Charset"));
  connect(a_ctSetCharsetKeyb, SIGNAL(triggered(bool) ), SLOT(slotSetCharsetKeyboard()));
  addAction( a_ctSetCharsetKeyb );


  a_ctWordWrap = actionCollection()->add<KToggleAction>("toggle_wordwrap");
  a_ctWordWrap->setText(i18n("&Word Wrap"));
  connect(a_ctWordWrap, SIGNAL(triggered(bool) ), SLOT(slotToggleWordWrap()));

  //tools menu

  action = actionCollection()->addAction("tools_quote");
  action->setText(i18n("Add &Quote Characters"));
  connect(action, SIGNAL(triggered(bool) ), v_iew->e_dit, SLOT(slotAddQuotes()));

  action = actionCollection()->addAction("tools_unquote");
  action->setText(i18n("&Remove Quote Characters"));
  connect(action, SIGNAL(triggered(bool) ), v_iew->e_dit, SLOT(slotRemoveQuotes()));

  action = actionCollection()->addAction("tools_box");
  action->setText(i18n("Add &Box"));
  connect(action, SIGNAL(triggered(bool) ), v_iew->e_dit, SLOT(slotAddBox()));

  action = actionCollection()->addAction("tools_unbox");
  action->setText(i18n("Re&move Box"));
  connect(action, SIGNAL(triggered(bool) ), v_iew->e_dit, SLOT(slotRemoveBox()));

  QAction *undoRewrap = actionCollection()->addAction("tools_undoRewrap");
  undoRewrap->setText(i18n("Get &Original Text (not re-wrapped)"));
  connect(undoRewrap, SIGNAL(triggered(bool) ), SLOT(slotUndoRewrap()));
  undoRewrap->setEnabled(!u_nwraped.isNull());

  QAction *rot13 = actionCollection()->addAction("tools_rot13");
  rot13->setIcon(KIcon("document-encrypt"));
  rot13->setText(i18n("S&cramble (Rot 13)"));
  connect(rot13, SIGNAL(triggered(bool)), v_iew->e_dit, SLOT(slotRot13()));
  rot13->setEnabled(false);
  connect(v_iew->e_dit, SIGNAL(copyAvailable(bool)), rot13, SLOT(setEnabled(bool)));

  a_ctExternalEditor = actionCollection()->addAction("external_editor");
  a_ctExternalEditor->setIcon(KIcon("system-run"));
  a_ctExternalEditor->setText(i18n("Start &External Editor"));
  connect(a_ctExternalEditor, SIGNAL(triggered(bool)), SLOT(slotExternalEditor()));

  a_ctSpellCheck = KStandardAction::spelling ( v_iew->e_dit, SLOT(checkSpelling()), actionCollection());

  //settings menu
  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);

  KStandardAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());

  KStandardAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());

  KStandardAction::preferences(knGlobals.top, SLOT(slotSettings()), actionCollection());


  createGUI("kncomposerui.rc");

  //---------------------------------- </Actions> ----------------------------------------


  //attachment popup
  a_ttPopup=static_cast<QMenu*> (factory()->container("attachment_popup", this));
  if(!a_ttPopup) a_ttPopup = new QMenu();
  slotAttachmentSelected(0);

  //init
  initData(text);

  //apply configuration
  setConfig(false);

  if (firstEdit) {   // now we place the cursor at the end of the quoted text / below the attribution line
    if ( knGlobals.settings()->cursorOnTop() ) {
      int numLines = knGlobals.settings()->intro().count( "%L" );
      //Laurent fix me
      //v_iew->e_dit->setCursorPosition(numLines+1,0);
    }
    else
    {
     //Laurent fixme
     //v_iew->e_dit->setCursorPosition(v_iew->e_dit->numLines()-1,0);
     }
  } else
  {
    //Laurent fixme
     //v_iew->e_dit->setCursorPosition(0,0);
  }
  v_iew->e_dit->setFocus();

  if (v_iew->s_ubject->text().length() == 0) {
    v_iew->s_ubject->setFocus();
  }

  if (v_iew->g_roups->text().length() == 0 && m_ode == news) {
    v_iew->g_roups->setFocus();
  }

  if (v_iew->t_o->text().length() == 0 && m_ode == mail) {
    v_iew->t_o->setFocus();
  }

  if( firstEdit && knGlobals.settings()->appendOwnSignature() )
    slotAppendSig();

  if (createCopy && (m_ode==news)) {
    a_ctDoMail->setChecked(true);
    slotToggleDoMail();
  }

  v_iew->e_dit->setModified(false);

  // restore window & toolbar configuration
  resize(535,450);    // default optimized for 800x600
  applyMainWindowSettings(knGlobals.config()->group("composerWindow_options"));

  // starting the external editor
  if ( knGlobals.settings()->useExternalEditor() )
    slotExternalEditor();
}


KNComposer::~KNComposer()
{
  delete mSpellingFilter;
  delete e_xternalEditor;  // this also kills the editor process if it's still running

  if(e_ditorTempfile) {
    delete e_ditorTempfile;
  }

  for ( QList<KNAttachment*>::Iterator it = mDeletedAttachments.begin(); it != mDeletedAttachments.end(); ++it )
    delete (*it);

  saveMainWindowSettings(knGlobals.config()->group("composerWindow_options"));
  qDeleteAll( m_listAction );
}

int KNComposer::listOfResultOfCheckWord( const QStringList & lst , const QString & selectWord)
{
    createGUI("kncomposerui.rc");
    unplugActionList("spell_result" );
    qDeleteAll( m_listAction );
    m_listAction.clear();
    if ( !lst.contains( selectWord ) )
    {
        QStringList::ConstIterator it = lst.begin();
        for ( ; it != lst.end() ; ++it )
        {
            if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
            {
                KAction *act = new KAction( *it, 0 );
                connect(act, SIGNAL(triggered(bool) ), v_iew->e_dit, SLOT( slotCorrectWord() ));

                m_listAction.append( act );
            }
        }
    }
    if ( m_listAction.count()>0 )
        plugActionList("spell_result", m_listAction );
    return m_listAction.count();
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

  if (fw->inherits("KEdit"))
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
    v_iew->e_dit->wordWrapToggled( knGlobals.settings()->wordWrap());
    v_iew->e_dit->setWrapColumnOrWidth( knGlobals.settings()->maxLineLength() );
    a_ctWordWrap->setChecked( knGlobals.settings()->wordWrap() );

    Kpgp::Module *pgp = Kpgp::Module::getKpgp();
    a_ctPGPsign->setEnabled(pgp->usePGP());
  }

  QFont fnt = knGlobals.settings()->composerFont();
  v_iew->s_ubject->setFont(fnt);
  v_iew->t_o->setFont(fnt);
  v_iew->g_roups->setFont(fnt);
  v_iew->f_up2->setFont(fnt);
  v_iew->e_dit->setFont(fnt);

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
  QString s = v_iew->e_dit->document ()->begin()->text ();
  if (m_ode == news_mail) {
    if (!s.contains(i18n("<posted & mailed>"))) {
      QTextCursor cursor(v_iew->e_dit->document ()->begin());
      cursor.setPosition(0);
      cursor.insertText(i18n("<posted & mailed>\n\n"));
      v_iew->e_dit->setTextCursor(cursor);
      }
  } else {
    if (s == i18n("<posted & mailed>")) {
      v_iew->e_dit->removeLine(0);
      if (v_iew->e_dit->textLine(0).isEmpty())
        v_iew->e_dit->removeLine(0);
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

  if (v_iew->s_ubject->text().isEmpty()) {
    KMessageBox::sorry(this, i18n("Please enter a subject."));
    return false;
  }
  if (!n_eeds8Bit && !KMime::isUsAscii(v_iew->s_ubject->text()))
    n_eeds8Bit=true;

  if (m_ode != mail) {
    if (v_iew->g_roups->text().isEmpty()) {
      KMessageBox::sorry(this, i18n("Please enter a newsgroup."));
      return false;
    }

    int groupCount = v_iew->g_roups->text().split(',', QString::SkipEmptyParts).count();
    int fupCount = v_iew->f_up2->currentText().split(',', QString::SkipEmptyParts).count();
    bool followUp = !v_iew->f_up2->currentText().isEmpty();

    if (groupCount>12) {
      KMessageBox::sorry(this, i18n("You are crossposting to more than 12 newsgroups.\nPlease remove all newsgroups in which your article is off-topic."));
      return false;
    }

    if (groupCount>5)
      if ( KMessageBox::warningYesNo( this, i18n("You are crossposting to more than five newsgroups.\nPlease reconsider whether this is really useful\nand remove groups in which your article is off-topic.\nDo you want to re-edit the article or send it anyway?"),
            QString(), KGuiItem(i18n("&Send")), KGuiItem(i18nc("edit article","&Edit")) ) != KMessageBox::Yes )
        return false;

    if ( !followUp && groupCount > 2 ) {
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
    if (v_iew->t_o->text().isEmpty() ) {
      KMessageBox::sorry(this, i18n("Please enter the email address."));
      return false;
    }
    if (!n_eeds8Bit && !KMime::isUsAscii(v_iew->t_o->text()))
      n_eeds8Bit=true;
  }

  //GNKSA body checks
  bool firstLine = true;
  bool empty = true;
  bool longLine = false;
  bool hasAttributionLine = false;
  int sigLength = 0;
  int notQuoted = 0;
  int textLines = 0;
  //Laurent fixme
  QStringList text;// = v_iew->e_dit->processedText();

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

  if (n_eeds8Bit && (c_harset.toLower()=="us-ascii")) {
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
    QByteArray signingKey = knGlobals.configManager()->identity()->signingKey().toLatin1();
    KNNntpAccount *acc = knGlobals.accountManager()->account( a_rticle->serverId() );
    if ( acc ) {
      KMime::Headers::Newsgroups *grps = a_rticle->newsgroups();
      if ( !grps->isEmpty() ) {
        KNGroup *grp = knGlobals.groupManager()->group( grps->groups().first(), acc );
        if (grp && grp->identity())
          signingKey = grp->identity()->signingKey().toLatin1();
        else if (acc->identity())
          signingKey = acc->identity()->signingKey().toLatin1();
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
  KNAttachment *a=0;

  //Date
  a_rticle->date()->setDateTime( KDateTime::currentLocalDateTime() );    //set current date+time

  //Subject
  a_rticle->subject()->fromUnicodeString(v_iew->s_ubject->text(), c_harset);

  //Newsgroups
  if (m_ode != mail) {
    a_rticle->newsgroups()->fromUnicodeString(v_iew->g_roups->text().remove(QRegExp("\\s")), KMime::Headers::Latin1);
    a_rticle->setDoPost(true);
  } else
    a_rticle->setDoPost(false);

  //To
  if (m_ode != news) {
    a_rticle->to()->fromUnicodeString(v_iew->t_o->text(), c_harset);
    a_rticle->setDoMail(true);
  } else
    a_rticle->setDoMail(false);

  //Followup-To
  if( a_rticle->doPost() && !v_iew->f_up2->currentText().isEmpty())
    a_rticle->followUpTo()->fromUnicodeString(v_iew->f_up2->currentText(), KMime::Headers::Latin1);
  else
    a_rticle->removeHeader("Followup-To");

  if(a_ttChanged && (v_iew->a_ttView)) {

    Q3ListViewItemIterator it(v_iew->a_ttView);
    while(it.current()) {
      a=(static_cast<AttachmentViewItem*> (it.current()))->attachment;
      if(a->hasChanged()) {
        if(a->isAttached())
          a->updateContentInfo();
        else
          a->attach(a_rticle);
      }
      ++it;
    }
  }

  for ( QList<KNAttachment*>::Iterator it = mDeletedAttachments.begin(); it != mDeletedAttachments.end(); ++it )
    if ( (*it)->isAttached() )
      (*it)->detach( a_rticle );

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
      text->contentType()->setCharset(c_harset);
      if ( knGlobals.settings()->allow8BitBody() )
        text->contentTransferEncoding()->setEncoding(KMime::Headers::CE8Bit);
      else
        text->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
    } else {
      text->contentType()->setCharset("us-ascii");   // fall back to us-ascii
      text->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    }
  } else {             // save as draft
    text->contentType()->setCharset(c_harset);
    if (c_harset.toLower()=="us-ascii")
      text->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    else
      text->contentTransferEncoding()->setEncoding( knGlobals.settings()->allow8BitBody()
          ? KMime::Headers::CE8Bit : KMime::Headers::CEquPr );
  }

  //assemble the text line by line
  QString tmp;
  //Laurent : fixme
  QStringList textLines; //= v_iew->e_dit->processedText();
  for (QStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it)
    tmp += *it + '\n';

  // Sign article if needed
  if ( a_ctPGPsign->isChecked() ) {
      // first get the signing key
      QByteArray signingKey = knGlobals.configManager()->identity()->signingKey().toLatin1();
      KNNntpAccount *acc = knGlobals.accountManager()->account( a_rticle->serverId() );
      if ( acc ) {
          KMime::Headers::Newsgroups *grps = a_rticle->newsgroups();
          if ( !grps->isEmpty() ) {
            KNGroup *grp = knGlobals.groupManager()->group( grps->groups().first(), acc );
            if (grp && grp->identity())
              signingKey = grp->identity()->signingKey().toLatin1();
            else if (acc->identity())
              signingKey = acc->identity()->signingKey().toLatin1();
          }
      }
      // now try to sign the article
      if (!signingKey.isEmpty()) {
          QString tmpText = tmp;
          Kpgp::Block block;
          bool ok=true;
          QTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);
          if(!ok) // no suitable codec found => try local settings and hope the best ;-)
              codec=KGlobal::locale()->codecForEncoding();

          block.setText( codec->fromUnicode(tmpText) );
          kDebug(5003) <<"signing article from" << article()->from()->addresses();
          if( block.clearsign( signingKey, codec->name() ) == Kpgp::Ok ) {
              QByteArray result = block.text();
              tmp = codec->toUnicode(result.data(), result.length() );
          } else {
              return false;
          }
      }
  }

  text->fromUnicodeString(tmp);

  //text is set and all attached contents have been assembled => now set lines
  a_rticle->lines()->setNumberOfLines(a_rticle->lineCount());

  a_rticle->assemble();
  a_rticle->updateListItem();
  return true;
}


void KNComposer::closeEvent(QCloseEvent *e)
{
  if(!v_iew->e_dit->isModified() && !a_ttChanged) {  // nothing to save, don't show nag screen
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
  //Subject
  if(a_rticle->subject()->isEmpty())
    slotSubjectChanged( QString() );
  else
    v_iew->s_ubject->setText(a_rticle->subject()->asUnicodeString());

  //Newsgroups
  v_iew->g_roups->setText(a_rticle->newsgroups()->asUnicodeString());

  //To
  v_iew->t_o->setText(a_rticle->to()->asUnicodeString());

  //Followup-To
  KMime::Headers::FollowUpTo *fup2=a_rticle->followUpTo(false);
  if(fup2 && !fup2->isEmpty())
    v_iew->f_up2->lineEdit()->setText(fup2->asUnicodeString());

  KMime::Content *textContent=a_rticle->textContent();
  QString s;

  if(text.isEmpty()) {
    if(textContent)
      s = textContent->decodedText();
  } else
    s = text;

  v_iew->e_dit->setText(s);

  // initialize the charset select action
  if(textContent)
    c_harset=textContent->contentType()->charset();
  else
    c_harset = knGlobals.settings()->charset().toLatin1();

  a_ctSetCharset->setCurrentItem( a_ctSetCharset->items().indexOf( c_harset ) );

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
    KMime::Content::List attList = a_rticle->attachments();
    AttachmentViewItem *item=0;
    foreach ( KMime::Content *c, attList )
      item=new AttachmentViewItem(v_iew->a_ttView, new KNAttachment(c));
  }
}

// inserts at cursor position if clear is false, replaces content otherwise
// puts the file content into a box if box==true
// "file" is already open for reading
void KNComposer::insertFile( QFile *file, bool clear, bool box, const QString &boxTitle )
{
  QString temp;
  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);
  QTextStream ts(file);
  ts.setCodec(codec);

  if (box)
    temp = QString::fromLatin1(",----[ %1 ]\n").arg(boxTitle);
  //Laurent fixme
  if (box && (v_iew->e_dit->wordWrapMode()!=QTextOption::NoWrap)) {
    int wrapAt = v_iew->e_dit->wrapColumnOrWidth();
    QStringList lst;
    QString line;
    while(!file->atEnd()) {
      line=ts.readLine();
      if (!file->atEnd())
        line+='\n';
      lst.append(line);
      qDebug()<<" lst :"<<lst;
    }
    temp+=KNHelper::rewrapStringList(lst, wrapAt, '|', false, true);
  } else {
    while(!file->atEnd()) {
      if (box)
        temp+="| ";
      temp+=ts.readLine();
      if (!file->atEnd())
        temp += '\n';
    }
  }
  if (box)
    temp += QString::fromLatin1("`----");

  if(clear)
    v_iew->e_dit->setText(temp);
  else
    v_iew->e_dit->insert(temp);
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
      boxName = url.path();
    else
      boxName = url.prettyUrl();

    insertFile(file,clear,box,boxName);
  }
}


//-------------------------------- <Actions> ------------------------------------


void KNComposer::addRecentAddress()
{
    if( !v_iew->t_o->isHidden() )
        RecentAddresses::self(knGlobals.config())->add( v_iew->t_o->text() );
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


void KNComposer::slotAppendSig()
{
  v_iew->e_dit->insertSignature(s_ignature);
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
   if (!v_iew->v_iewOpen) {
      KNHelper::saveWindowSize("composer", size());
      v_iew->showAttachmentView();
    }
    (void) new AttachmentViewItem(v_iew->a_ttView, new KNAttachment(helper));
    a_ttChanged=true;
  } else {
    delete helper;
  }
}


void KNComposer::slotRemoveAttachment()
{
  if(!v_iew->v_iewOpen) return;

  if(v_iew->a_ttView->currentItem()) {
    AttachmentViewItem *it=static_cast<AttachmentViewItem*>(v_iew->a_ttView->currentItem());
    if(it->attachment->isAttached()) {
      mDeletedAttachments.append( it->attachment );
      it->attachment=0;
    }
    delete it;

    if(v_iew->a_ttView->childCount()==0) {
      KNHelper::saveWindowSize("composerAtt", size());
      v_iew->hideAttachmentView();
    }

    a_ttChanged=true;
  }
}

void KNComposer::slotAttachmentProperties()
{
  if(!v_iew->v_iewOpen) return;

  if(v_iew->a_ttView->currentItem()) {
    AttachmentViewItem *it=static_cast<AttachmentViewItem*>(v_iew->a_ttView->currentItem());
    AttachmentPropertiesDlg *d=new AttachmentPropertiesDlg(it->attachment, this);
    if(d->exec()) {
      d->apply();
      it->setText(1, it->attachment->mimeType());
      it->setText(3, it->attachment->description());
      it->setText(4, it->attachment->encoding());
    }
    delete d;
    a_ttChanged=true;
  }
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
//Laurent fix me
#if 0
    if ( knGlobals.settings()->useExternalMailer() ) {
      QString s = v_iew->e_dit->textLine(0);
      if (!s.contains(i18n("<posted & mailed>")))
        v_iew->e_dit->insertAt(i18n("<posted & mailed>\n\n"),0,0);
      QString tmp;
      QStringList textLines = v_iew->e_dit->processedText();
      for (QStringList::Iterator it = textLines.begin(); it != textLines.end(); ++it) {
        if (*it == "-- ")   // try to be smart, don't include the signature,
          break;            // kmail will append one, too.
        tmp+=*it+'\n';
      }
      knGlobals.artFactory->sendMailExternal(v_iew->t_o->text(), v_iew->s_ubject->text(), tmp);
      a_ctDoMail->setChecked(false); //revert
      return;
    } else {
      if (a_ctDoPost->isChecked())
        m_ode=news_mail;
      else
        m_ode=mail;
    }
  } else {
    if (a_ctDoPost->isChecked())
      m_ode=news;
    else {     // invalid
      a_ctDoMail->setChecked(true); //revert
      return;
    }
#endif
  }
  setMessageMode(m_ode);
}


void KNComposer::slotSetCharset(const QString &s)
{
  if(s.isEmpty())
    return;

  c_harset = s.toLatin1();
  setConfig(true); //adjust fonts
}


void KNComposer::slotSetCharsetKeyboard()
{
  int newCS = KNHelper::selectDialog(this, i18n("Select Charset"), a_ctSetCharset->items(), a_ctSetCharset->currentItem());
  if (newCS != -1) {
    a_ctSetCharset->setCurrentItem(newCS);
    slotSetCharset( a_ctSetCharset->items()[newCS] );
  }
}


void KNComposer::slotToggleWordWrap()
{
  v_iew->e_dit->wordWrapToggled(a_ctWordWrap->isChecked());
}


void KNComposer::slotUndoRewrap()
{
  if (KMessageBox::warningContinueCancel( this, i18n("This will replace all text you have written.")) == KMessageBox::Continue) {
    v_iew->e_dit->setText(u_nwraped);
    slotAppendSig();
  }
}

void KNComposer::slotExternalEditor()
{
  if(e_xternalEditor)   // in progress...
    return;

  QString editorCommand = knGlobals.settings()->externalEditor();

  if(editorCommand.isEmpty())
    KMessageBox::sorry(this, i18n("No editor configured.\nPlease do this in the settings dialog."));

  if(e_ditorTempfile) {       // shouldn't happen...
    delete e_ditorTempfile;
    e_ditorTempfile=0;
  }

  e_ditorTempfile=new KTemporaryFile();

  if(!e_ditorTempfile->open()) {
    KNHelper::displayTempFileError(this);
    delete e_ditorTempfile;
    e_ditorTempfile=0;
    return;
  }

  bool ok=true;
  QTextCodec *codec=KGlobal::charsets()->codecForName(c_harset, ok);

  QString tmp;
// Laurent fixme

  QStringList textLines;// = v_iew->e_dit->processedText();
  for (QStringList::Iterator it = textLines.begin(); it != textLines.end();) {
    tmp += *it;
    ++it;
    if (it != textLines.end())
      tmp+='\n';
  }

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

  connect(e_xternalEditor, SIGNAL( finished ( int, QProcess::ExitStatus)),this, SLOT(slotEditorFinished( finished ( int, QProcess::ExitStatus))));
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
  if (v_iew->e_dit->overwriteMode ())
    overwriteDesc = i18n(" OVR ");
  else
    overwriteDesc = i18n(" INS ");

  statusBar()->changeItem(i18n(" Type: %1 ", typeDesc), 1);
  statusBar()->changeItem(i18n(" Charset: %1 ", QString( c_harset ) ), 2);
  statusBar()->changeItem(overwriteDesc, 3);
  statusBar()->changeItem(i18n(" Column: %1 ", v_iew->e_dit->columnNumber () + 1), 4);
  statusBar()->changeItem(i18n(" Line: %1 ", v_iew->e_dit->linePosition() + 1), 5);
}


void KNComposer::slotUpdateCursorPos()
{
  statusBar()->changeItem(i18n(" Column: %1 ", v_iew->e_dit->columnNumber () + 1), 4);
  statusBar()->changeItem(i18n(" Line: %1 ", v_iew->e_dit->linePosition() + 1), 5);
}


void KNComposer::slotConfKeys()
{
  KShortcutsDialog::configure(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this, true);
}


void KNComposer::slotConfToolbar()
{
  saveMainWindowSettings(knGlobals.config()->group( "composerWindow_options") );
  KEditToolBar dlg(guiFactory(),this);
  connect(&dlg,SIGNAL( newToolbarConfig() ), this, SLOT( slotNewToolbarConfig() ));
  dlg.exec();
}

void KNComposer::slotNewToolbarConfig()
{
  createGUI("kncomposerui.rc");

  a_ttPopup=static_cast<QMenu*> (factory()->container("attachment_popup", this));
  if(!a_ttPopup) a_ttPopup = new QMenu();

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
    v_iew->s_ubject->setText( subject );
  // update caption
  if( !subject.isEmpty() )
    setCaption( subject );
  else
    setCaption( i18n("No Subject") );
}


void KNComposer::slotGroupsChanged(const QString &t)
{
  QString currText=v_iew->f_up2->currentText();

  v_iew->f_up2->clear();

  QStringList groups = t.split(',');
  foreach ( QString s, groups ) {
    v_iew->f_up2->addItem( s );
  }
  v_iew->f_up2->addItem("");

  if ( !currText.isEmpty() || !mFirstEdit ) // user might have cleared fup2 intentionally during last edit
    v_iew->f_up2->lineEdit()->setText(currText);
}


void KNComposer::slotToBtnClicked()
{
  AddressesDialog dlg( this );
  QString txt;
  QString to = v_iew->t_o->text();
  dlg.setShowBCC(false);
  dlg.setShowCC(false);
#if 0
  QStringList lst;


  txt = mEdtTo->text().trimmed();
  if ( !txt.isEmpty() ) {
      lst = KMMessage::splitEmailAddrList( txt );
      dlg.setSelectedTo( lst );
  }
#endif
  dlg.setRecentAddresses( RecentAddresses::self(knGlobals.config())->kabcAddresses() );
  if (dlg.exec()==QDialog::Rejected) return;

  if(!to.isEmpty())
      to+=", ";
  to+=dlg.to().join(", ");

  v_iew->t_o->setText(to);

}


void KNComposer::slotGroupsBtnClicked()
{
  int id=a_rticle->serverId();
  KNNntpAccount *nntp=0;

  if(id!=-1)
    nntp=knGlobals.accountManager()->account(id);

  if(!nntp)
    nntp=knGlobals.accountManager()->first();

  if(!nntp) {
    KMessageBox::error(this, i18n("You have no valid news accounts configured."));
    v_iew->g_roups->clear();
    return;
  }

  if(id==-1)
    a_rticle->setServerId(nntp->id());

  KNGroupSelectDialog *dlg=new KNGroupSelectDialog(this, nntp, v_iew->g_roups->text().remove(QRegExp("\\s")));

  connect(dlg, SIGNAL(loadList(KNNntpAccount*)),
    knGlobals.groupManager(), SLOT(slotLoadGroupList(KNNntpAccount*)));
  connect(knGlobals.groupManager(), SIGNAL(newListReady(KNGroupListData*)),
    dlg, SLOT(slotReceiveList(KNGroupListData*)));

  if(dlg->exec())
    v_iew->g_roups->setText(dlg->selectedGroups());

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
  delete e_xternalEditor;  // this also kills the editor process if it's still running
  e_xternalEditor=0;
  delete e_ditorTempfile;
  e_ditorTempfile=0;

  a_ctExternalEditor->setEnabled(true);
  a_ctSpellCheck->setEnabled(true);
  v_iew->hideExternalNotification();
}


void KNComposer::slotAttachmentPopup(K3ListView*, Q3ListViewItem *it, const QPoint &p)
{
  if(it)
    a_ttPopup->popup(p);
}


void KNComposer::slotAttachmentSelected(Q3ListViewItem *it)
{
  if(v_iew->a_ttWidget) {
    v_iew->a_ttRemoveBtn->setEnabled((it!=0));
    v_iew->a_ttEditBtn->setEnabled((it!=0));
  }
}


void KNComposer::slotAttachmentEdit(Q3ListViewItem *)
{
  slotAttachmentProperties();
}


void KNComposer::slotAttachmentRemove(Q3ListViewItem *)
{
  slotRemoveAttachment();
}


//==============================================================================
// spellchecking code copied form kedit (Bernd Johannes Wuebben)
//==============================================================================
#if 0

void KNComposer::slotSpellStarted( K3Spell *)
{
    if( !spellLineEdit )
    {
        v_iew->e_dit->spellcheck_start();
        s_pellChecker->setProgressResolution(2);

        // read the quote indicator from the preferences
        KConfigGroup config( knGlobals.config(), "READNEWS" );
        QString quotePrefix;
        quotePrefix = config.readEntry("quoteCharacters",">");
//todo fixme
//quotePrefix = mComposer->msg()->formatString(quotePrefix);

        kDebug(5003) <<"spelling: new SpellingFilter with prefix=\"" << quotePrefix <<"\"";
        mSpellingFilter = new SpellingFilter(v_iew->e_dit->text(), quotePrefix, SpellingFilter::FilterUrls,
                                             SpellingFilter::FilterEmailAddresses);

        s_pellChecker->check(mSpellingFilter->filteredText());
    }
    else
        s_pellChecker->check( v_iew->s_ubject->text());
}

void KNComposer::slotSpellDone(const QString &newtext)
{
    a_ctExternalEditor->setEnabled(true);
    a_ctSpellCheck->setEnabled(true);
    if ( !spellLineEdit )
        v_iew->e_dit->spellcheck_stop();

    int dlgResult = s_pellChecker->dlgResult();
    if ( dlgResult == KS_CANCEL )
    {
        if( spellLineEdit)
        {
            //stop spell check
            spellLineEdit = false;
            QString tmpText( newtext);
            tmpText =  tmpText.remove('\n');

            if( tmpText != v_iew->s_ubject->text() )
                v_iew->s_ubject->setText( tmpText );
        }
        else
        {
            kDebug(5003) <<"spelling: canceled - restoring text from SpellingFilter";
            kDebug(5003)<<" mSpellingFilter->originalText() :"<<mSpellingFilter->originalText();
            v_iew->e_dit->setText(mSpellingFilter->originalText());

            //v_iew->e_dit->setModified(mWasModifiedBeforeSpellCheck);
        }
    }
    s_pellChecker->cleanUp();
    K3DictSpellingHighlighter::dictionaryChanged();
}


void KNComposer::slotSpellFinished()
{
  a_ctExternalEditor->setEnabled(true);
  a_ctSpellCheck->setEnabled(true);
  K3Spell::spellStatus status=s_pellChecker->status();
  delete s_pellChecker;
  s_pellChecker=0;

  kDebug(5003) <<"spelling: delete SpellingFilter";
  delete mSpellingFilter;
  mSpellingFilter = 0;

  if(status==K3Spell::Error) {
    KMessageBox::error(this, i18n("ISpell could not be started.\n"
    "Please make sure you have ISpell properly configured and in your PATH."));
  }
  else if(status==K3Spell::Crashed) {
    v_iew->e_dit->spellcheck_stop();
    KMessageBox::error(this, i18n("ISpell seems to have crashed."));
  }
  else
  {
      if( spellLineEdit )
          slotSpellcheck();
      else if( status == K3Spell::FinishedNoMisspellingsEncountered )
          KMessageBox::information( this, i18n("No misspellings encountered."));
  }
}
#endif

void KNComposer::slotDragEnterEvent(QDragEnterEvent *ev)
{
  QStringList files;
  ev->setAccepted( KUrl::List::canDecode( ev->mimeData() ) );
}


void KNComposer::slotDropEvent(QDropEvent *ev)
{
  KUrl::List urls = KUrl::List::fromMimeData( ev->mimeData() );

  if ( urls.isEmpty() )
    return;

  for (KUrl::List::ConstIterator it = urls.begin(); it != urls.end(); ++it) {
    const KUrl &url = *it;
    KNLoadHelper *helper = new KNLoadHelper(this);

    if (helper->setURL(url)) {
      if (!v_iew->v_iewOpen) {
        KNHelper::saveWindowSize("composer", size());
        v_iew->showAttachmentView();
      }
      (void) new AttachmentViewItem(v_iew->a_ttView, new KNAttachment(helper));
      a_ttChanged=true;
    } else {
      delete helper;
    }
  }
}


void KNComposer::dragEnterEvent(QDragEnterEvent *ev)
{
  slotDragEnterEvent(ev);
}


void KNComposer::dropEvent(QDropEvent *ev)
{
  slotDropEvent(ev);
}

QMenu * KNComposer::popupMenu( const QString& name )
{
    Q_ASSERT(factory());
    if ( factory() )
        return ((QMenu*)factory()->container( name, this ));
    return 0L;
}




//=====================================================================================
#if 0
#include <kcursor.h>
KNComposer::Editor::Editor( KNComposer::ComposerView *_composerView, KNComposer *_composer, QWidget *parent )
    : KEdit( parent ), m_composer( _composer ), m_composerView( _composerView )
{
  setOverwriteEnabled(true);
  spell = 0L;
  installEventFilter(this);
  KCursor::setAutoHideCursor( this, true, true );
  m_bound = QRegExp( QString::fromLatin1("[\\s\\W]") );
}


KNComposer::Editor::~Editor()
{
    removeEventFilter(this);
    delete spell;
}

//-----------------------------------------------------------------------------
bool KNComposer::Editor::eventFilter(QObject*o, QEvent* e)
{
  if (o == this)
    KCursor::autoHideEventFilter(o, e);

  if (e->type() == QEvent::KeyPress)
  {
    QKeyEvent *k = (QKeyEvent*)e;
    // ---sven's Arrow key navigation start ---
    // Key Up in first line takes you to Subject line.
    if (k->key() == Qt::Key_Up && k->modifiers() != Qt::ShiftModifier && currentLine() == 0
      && lineOfChar(0, currentColumn()) == 0)
    {
      deselect();
      m_composerView->focusNextPrevEdit(0, false); //take me up
      return true;
    }
    // ---sven's Arrow key navigation end ---

    if (k->key() == Qt::Key_Backtab && k->modifiers() == Qt::ShiftModifier)
    {
      deselect();
      m_composerView->focusNextPrevEdit(0, false);
      return true;
    }
  } else if ( e->type() == QEvent::ContextMenu ) {
    QContextMenuEvent *event = (QContextMenuEvent*) e;

    int para = 1, charPos, firstSpace, lastSpace;

    //Get the character at the position of the click
    charPos = charAt( viewportToContents(event->pos() ), &para );
    QString paraText = text( para );

    if( !paraText.at(charPos).isSpace() )
    {
      //Get word right clicked on
      firstSpace = paraText.lastIndexOf( m_bound, charPos ) + 1;
      lastSpace = paraText.indexOf( m_bound, charPos );
      if( lastSpace == -1 )
        lastSpace = paraText.length();
      QString word = paraText.mid( firstSpace, lastSpace - firstSpace );
      //Continue if this word was misspelled
      if( !word.isEmpty() && m_replacements.contains( word ) )
      {
        KMenu p;
        p.addTitle( i18n("Suggestions") );

        //Add the suggestions to the popup menu
        QStringList reps = m_replacements[word];
        if( reps.count() > 0 )
        {
          int listPos = 0;
          for ( QStringList::Iterator it = reps.begin(); it != reps.end(); ++it ) {
            p.insertItem( *it, listPos );
            listPos++;
          }
        }
        else
        {
          p.insertItem( QString::fromLatin1("No Suggestions"), -2 );
        }

        //Execute the popup inline
#ifdef __GNUC__
#warning Port me!
#endif
        int id = -1;//p.exec( mapToGlobal( event->pos() ) );

        if( id > -1 )
        {
          //Save the cursor position
          int parIdx = 1, txtIdx = 1;
          getCursorPosition(&parIdx, &txtIdx);
          setSelection(para, firstSpace, para, lastSpace);
          insert(m_replacements[word][id]);
          // Restore the cursor position; if the cursor was behind the
          // misspelled word then adjust the cursor position
          if ( para == parIdx && txtIdx >= lastSpace )
            txtIdx += m_replacements[word][id].length() - word.length();
          setCursorPosition(parIdx, txtIdx);
        }
        //Cancel original event
        return true;
      }
    }
  }

  return KEdit::eventFilter(o, e);
}

// expand tabs to avoid the "tab-damage",
// auto-wraped paragraphs have to split (code taken from KEdit::saveText)
QStringList KNComposer::Editor::processedText()
{
  QStringList ret;
  int lines = numLines()-1;
  if (lines < 0)
    return ret;

  if (wordWrap() == NoWrap) {
    for (int i = 0; i <= lines; i++)
      ret.append(textLine(i));
  } else {
    for (int i = 0; i <= lines; i++) {
      int lines_in_parag = linesOfParagraph(i);

      if (lines_in_parag == 1) {
        ret.append(textLine(i));
      } else {
        QString parag_text = textLine(i);
        int pos = 0;
        int last_pos = 0;
        int current_line = 0;
        while (current_line+1 < lines_in_parag) {
          while (lineOfChar(i, pos) == current_line) pos++;
          ret.append(parag_text.mid(last_pos, pos - last_pos - 1));
          current_line++;
          last_pos = pos;
        }
        // add last line
        ret.append(parag_text.mid(pos));
      }
    }
  }

  QString replacement;
  int tabPos;
  for (QStringList::Iterator it = ret.begin(); it != ret.end(); ++it ) {
    while ( ( tabPos = (*it).indexOf('\t') ) != -1 ) {
      replacement.fill(QChar(' '), 8-(tabPos%8));
      (*it).replace(tabPos, 1, replacement);
    }
  }

  return ret;
}


void KNComposer::Editor::slotAddQuotes()
{
  if (hasMarkedText()) {
    QString s = markedText();
    s.prepend("> ");
    s.replace(QRegExp("\n"),"\n> ");
    insert(s);
  } else {
    int l = currentLine();
    int c = currentColumn();
    QString s = textLine(l);
    s.prepend("> ");
    insertLine(s,l);
    removeLine(l+1);
    setCursorPosition(l,c+2);
  }
}

void KNComposer::Editor::slotRemoveBox()
{
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
      for (int i=x;i<=l;i++) {     // remove quotation
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
      for (int i=l+1;i<x;i++) {     // remove quotation
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
}



void KNComposer::Editor::keyPressEvent ( QKeyEvent *e)
{
    if( e->key() == Qt::Key_Return ) {
        int line, col;
        getCursorPosition( &line, &col );
        QString lineText = text( line );
        // returns line with additional trailing space (bug in Qt?), cut it off
        lineText.truncate( lineText.length() - 1 );
        // special treatment of quoted lines only if the cursor is neither at
        // the begin nor at the end of the line
        if( ( col > 0 ) && ( col < int( lineText.length() ) ) ) {
            bool isQuotedLine = false;
            int bot = 0; // bot = begin of text after quote indicators
            while( bot < lineText.length() ) {
                if( ( lineText[bot] == '>' ) || ( lineText[bot] == '|' ) ) {
                    isQuotedLine = true;
                    ++bot;
                }
                else if( lineText[bot].isSpace() ) {
                    ++bot;
                }
                else {
                    break;
                }
            }

            KEdit::keyPressEvent( e );

            // duplicate quote indicators of the previous line before the new
            // line if the line actually contained text (apart from the quote
            // indicators) and the cursor is behind the quote indicators
            if( isQuotedLine
                && ( bot != lineText.length() )
                && ( col >= int( bot ) ) ) {
                QString newLine = text( line + 1 );
                // remove leading white space from the new line and instead
                // add the quote indicators of the previous line
                int leadingWhiteSpaceCount = 0;
                while( ( leadingWhiteSpaceCount < newLine.length() )
                       && newLine[leadingWhiteSpaceCount].isSpace() ) {
                    ++leadingWhiteSpaceCount;
                }
                newLine = newLine.replace( 0, leadingWhiteSpaceCount,
                                           lineText.left( bot ) );
                removeParagraph( line + 1 );
                insertParagraph( newLine, line + 1 );
                // place the cursor at the begin of the new line since
                // we assume that the user split the quoted line in order
                // to add a comment to the first part of the quoted line
                setCursorPosition( line + 1 , 0 );
            }
        }
        else
            KEdit::keyPressEvent( e );
    }
    else
        KEdit::keyPressEvent( e );
}



#endif

//=====================================================================================


KNComposer::AttachmentView::AttachmentView( QWidget *parent )
 : K3ListView( parent )
{
  setFrameStyle(QFrame::WinPanel | QFrame::Sunken);  // match the QMultiLineEdit style
  addColumn(i18n("File"), 115);
  addColumn(i18n("Type"), 91);
  addColumn(i18n("Size"), 55);
  addColumn(i18n("Description"), 110);
  addColumn(i18n("Encoding"), 60);
  header()->setClickEnabled(false);
  setAllColumnsShowFocus(true);
}


KNComposer::AttachmentView::~AttachmentView()
{
}


void KNComposer::AttachmentView::keyPressEvent(QKeyEvent *e)
{
  if(!e)
    return; // subclass bug

  if( (e->key()==Qt::Key_Delete) && (currentItem()) )
    emit(delPressed(currentItem()));
  else
    K3ListView::keyPressEvent(e);
}


//=====================================================================================


KNComposer::AttachmentViewItem::AttachmentViewItem(K3ListView *v, KNAttachment *a) :
  K3ListViewItem(v), attachment(a)
{
  setText(0, a->name());
  setText(1, a->mimeType());
  setText(2, a->contentSize());
  setText(3, a->description());
  setText(4, a->encoding());
}



KNComposer::AttachmentViewItem::~AttachmentViewItem()
{
  delete attachment;
}


//=====================================================================================


KNComposer::AttachmentPropertiesDlg::AttachmentPropertiesDlg( KNAttachment *a, QWidget *parent ) :
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
  connect(m_imeType, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotMimeTypeTextChanged(const QString&)));

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

#include "kncomposer.moc"

// kate: space-indent on; indent-width 2;
