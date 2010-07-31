/*  -*- c++ -*-
    signatureconfigurator.cpp

    KMail, the KDE mail client.
    Copyright (c) 2002 the KMail authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "signatureconfigurator.h"

#include <klocale.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kshellcompletion.h>
#include <krun.h>

#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtextedit.h>
#include <tqwhatsthis.h>
#include <tqwidgetstack.h>

#include <assert.h>

using namespace KMail;

namespace KMail {

  SignatureConfigurator::SignatureConfigurator( TQWidget * parent, const char * name )
    : TQWidget( parent, name )
  {
    // tmp. vars:
    TQLabel * label;
    TQWidget * page;
    TQHBoxLayout * hlay;
    TQVBoxLayout * vlay;
    TQVBoxLayout * page_vlay;

    vlay = new TQVBoxLayout( this, 0, KDialog::spacingHint(), "main layout" );

    // "enable signatue" checkbox:
    mEnableCheck = new TQCheckBox( i18n("&Enable signature"), this );
    TQWhatsThis::add(mEnableCheck,
        i18n("Check this box if you want KMail to append a signature to mails "
             "written with this identity."));
    vlay->addWidget( mEnableCheck );

    // "obtain signature text from" combo and label:
    hlay = new TQHBoxLayout( vlay ); // inherits spacing
    mSourceCombo = new TQComboBox( false, this );
    TQWhatsThis::add(mSourceCombo,
        i18n("Click on the widgets below to obtain help on the input methods."));
    mSourceCombo->setEnabled( false ); // since !mEnableCheck->isChecked()
    mSourceCombo->insertStringList( TQStringList()
		   << i18n("continuation of \"obtain signature text from\"",
			   "Input Field Below")
		   << i18n("continuation of \"obtain signature text from\"",
			   "File")
                   << i18n("continuation of \"obtain signature text from\"",
			   "Output of Command")
		   );
    label = new TQLabel( mSourceCombo,
			i18n("Obtain signature &text from:"), this );
    label->setEnabled( false ); // since !mEnableCheck->isChecked()
    hlay->addWidget( label );
    hlay->addWidget( mSourceCombo, 1 );

    // widget stack that is controlled by the source combo:
    TQWidgetStack * widgetStack = new TQWidgetStack( this );
    widgetStack->setEnabled( false ); // since !mEnableCheck->isChecked()
    vlay->addWidget( widgetStack, 1 );
    connect( mSourceCombo, TQT_SIGNAL(highlighted(int)),
	     widgetStack, TQT_SLOT(raiseWidget(int)) );
    // connects for the enabling of the widgets depending on
    // signatureEnabled:
    connect( mEnableCheck, TQT_SIGNAL(toggled(bool)),
	     mSourceCombo, TQT_SLOT(setEnabled(bool)) );
    connect( mEnableCheck, TQT_SIGNAL(toggled(bool)),
	     widgetStack, TQT_SLOT(setEnabled(bool)) );
    connect( mEnableCheck, TQT_SIGNAL(toggled(bool)),
	     label, TQT_SLOT(setEnabled(bool)) );
    // The focus might be still in the widget that is disabled
    connect( mEnableCheck, TQT_SIGNAL(clicked()),
	     mEnableCheck, TQT_SLOT(setFocus()) );

    int pageno = 0;
    // page 0: input field for direct entering:
    mTextEdit = new TQTextEdit( widgetStack );
    TQWhatsThis::add(mTextEdit,
        i18n("Use this field to enter an arbitrary static signature."));
    widgetStack->addWidget( mTextEdit, pageno );
    mTextEdit->setFont( KGlobalSettings::fixedFont() );
    mTextEdit->setWordWrap( TQTextEdit::NoWrap );
    mTextEdit->setTextFormat( Qt::PlainText );

    widgetStack->raiseWidget( 0 ); // since mSourceCombo->currentItem() == 0

    // page 1: "signature file" requester, label, "edit file" button:
    ++pageno;
	page = new TQWidget( widgetStack );
    widgetStack->addWidget( page, pageno ); // force sequential numbers (play safe)
    page_vlay = new TQVBoxLayout( page, 0, KDialog::spacingHint() );
    hlay = new TQHBoxLayout( page_vlay ); // inherits spacing
    mFileRequester = new KURLRequester( page );
    TQWhatsThis::add(mFileRequester,
        i18n("Use this requester to specify a text file that contains your "
             "signature. It will be read every time you create a new mail or "
             "append a new signature."));
    hlay->addWidget( new TQLabel( mFileRequester,
				 i18n("S&pecify file:"), page ) );
    hlay->addWidget( mFileRequester, 1 );
    mFileRequester->button()->setAutoDefault( false );
    connect( mFileRequester, TQT_SIGNAL(textChanged(const TQString &)),
	     this, TQT_SLOT(slotEnableEditButton(const TQString &)) );
    mEditButton = new TQPushButton( i18n("Edit &File"), page );
    TQWhatsThis::add(mEditButton, i18n("Opens the specified file in a text editor."));
    connect( mEditButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotEdit()) );
    mEditButton->setAutoDefault( false );
    mEditButton->setEnabled( false ); // initially nothing to edit
    hlay->addWidget( mEditButton );
    page_vlay->addStretch( 1 ); // spacer

    // page 2: "signature command" requester and label:
    ++pageno;
    page = new TQWidget( widgetStack );
    widgetStack->addWidget( page, pageno );
    page_vlay = new TQVBoxLayout( page, 0, KDialog::spacingHint() );
    hlay = new TQHBoxLayout( page_vlay ); // inherits spacing
    mCommandEdit = new KLineEdit( page );
    mCommandEdit->setCompletionObject( new KShellCompletion() );
    mCommandEdit->setAutoDeleteCompletionObject( true );
    TQWhatsThis::add(mCommandEdit,
        i18n("You can add an arbitrary command here, either with or without path "
             "depending on whether or not the command is in your Path. For every "
             "new mail, KMail will execute the command and use what it outputs (to "
             "standard output) as a signature. Usual commands for use with this "
             "mechanism are \"fortune\" or \"ksig -random\"."));
    hlay->addWidget( new TQLabel( mCommandEdit,
				 i18n("S&pecify command:"), page ) );
    hlay->addWidget( mCommandEdit, 1 );
    page_vlay->addStretch( 1 ); // spacer

  }

  SignatureConfigurator::~SignatureConfigurator() {

  }

  bool SignatureConfigurator::isSignatureEnabled() const {
    return mEnableCheck->isChecked();
  }

  void SignatureConfigurator::setSignatureEnabled( bool enable ) {
    mEnableCheck->setChecked( enable );
  }

  Signature::Type SignatureConfigurator::signatureType() const {
    if ( !isSignatureEnabled() ) return Signature::Disabled;

    switch ( mSourceCombo->currentItem() ) {
    case 0:  return Signature::Inlined;
    case 1:  return Signature::FromFile;
    case 2:  return Signature::FromCommand;
    default: return Signature::Disabled;
    }
  }

  void SignatureConfigurator::setSignatureType( Signature::Type type ) {
    setSignatureEnabled( type != Signature::Disabled );

    int idx = 0;
    switch( type ) {
    case Signature::Inlined:     idx = 0; break;
    case Signature::FromFile:	 idx = 1; break;
    case Signature::FromCommand: idx = 2; break;
    default:                     idx = 0; break;
    };

    mSourceCombo->setCurrentItem( idx );
  }

  TQString SignatureConfigurator::inlineText() const {
    return mTextEdit->text();
  }

  void SignatureConfigurator::setInlineText( const TQString & text ) {
    mTextEdit->setText( text );
  }

  TQString SignatureConfigurator::fileURL() const {
    TQString file = mFileRequester->url().stripWhiteSpace();

    // Force the filename to be relative to ~ instead of $PWD depending
    // on the rest of the code (KRun::run in Edit and KFileItem on save)
    if ( !file.isEmpty() && TQFileInfo( file ).isRelative() )
        file = TQDir::home().absPath() + TQDir::separator() + file;

    return file;
  }

  void SignatureConfigurator::setFileURL( const TQString & url ) {
    mFileRequester->setURL( url );
  }

  TQString SignatureConfigurator::commandURL() const {
    return mCommandEdit->text();
  }

  void SignatureConfigurator::setCommandURL( const TQString & url ) {
    mCommandEdit->setText( url );
  }


  Signature SignatureConfigurator::signature() const {
    Signature sig;
    sig.setType( signatureType() );
    sig.setText( inlineText() );
    if ( signatureType() == Signature::FromCommand )
      sig.setUrl( commandURL(), true );
    if ( signatureType() == Signature::FromFile )
      sig.setUrl( fileURL(), false );
    return sig;
  }

  void SignatureConfigurator::setSignature( const Signature & sig ) {
    setSignatureType( sig.type() );
    setInlineText( sig.text() );
    if ( sig.type() == Signature::FromFile )
      setFileURL( sig.url() );
    else
      setFileURL( TQString::null );
    if ( sig.type() == Signature::FromCommand )
      setCommandURL( sig.url() );
    else
      setCommandURL( TQString::null );
  }

  void SignatureConfigurator::slotEnableEditButton( const TQString & url ) {
    mEditButton->setDisabled( url.stripWhiteSpace().isEmpty() );
  }

  void SignatureConfigurator::slotEdit() {
    TQString url = fileURL();
    // slotEnableEditButton should prevent this assert from being hit:
    assert( !url.isEmpty() );

    (void)KRun::runURL( KURL( url ), TQString::fromLatin1("text/plain") );
  }

} // namespace KMail

#include "signatureconfigurator.moc"
