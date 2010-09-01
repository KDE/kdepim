// kmmsgpartdlg.cpp


// my includes:
#include <config.h>
#include "kmmsgpartdlg.h"

// other KMail includes:
#include "kmmessage.h"
#include "kmmsgpart.h"
#include "kcursorsaver.h"

// other kdenetwork includes: (none)

// other KDE includes:
#include <kmimetype.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <kstringvalidator.h>
#include <kcombobox.h>
#include <kdebug.h>

// other Qt includes:
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <klineedit.h>
#include <tqcheckbox.h>

// other includes:
#include <assert.h>

static const struct {
  KMMsgPartDialog::Encoding encoding;
  const char * displayName;
} encodingTypes[] = {
  { KMMsgPartDialog::SevenBit, I18N_NOOP("None (7-bit text)") },
  { KMMsgPartDialog::EightBit, I18N_NOOP("None (8-bit text)") },
  { KMMsgPartDialog::QuotedPrintable, I18N_NOOP("Quoted Printable") },
  { KMMsgPartDialog::Base64, I18N_NOOP("Base 64") },
};
static const int numEncodingTypes =
  sizeof encodingTypes / sizeof *encodingTypes;

KMMsgPartDialog::KMMsgPartDialog( const TQString & caption,
				  TQWidget * parent, const char * name )
  : KDialogBase( Plain,
		 caption.isEmpty() ? i18n("Message Part Properties") : caption,
		 Ok|Cancel|Help, Ok, parent, name, true, true)
{
  // tmp vars:
  TQGridLayout * glay;
  TQLabel      * label;
  TQString       msg;

  setHelp( TQString::fromLatin1("attachments") );

  for ( int i = 0 ; i < numEncodingTypes ; ++i )
    mI18nizedEncodings << i18n( encodingTypes[i].displayName );

  glay = new TQGridLayout( plainPage(), 9 /*rows*/, 2 /*cols*/, spacingHint() );
  glay->setColStretch( 1, 1 );
  glay->setRowStretch( 8, 1 );

  // mimetype icon:
  mIcon = new TQLabel( plainPage() );
  mIcon->setPixmap( DesktopIcon("unknown") );
  glay->addMultiCellWidget( mIcon, 0, 1, 0, 0 );

  // row 0: Type combobox:
  mMimeType = new KComboBox( true, plainPage() );
  mMimeType->setInsertionPolicy( TQComboBox::NoInsertion );
  mMimeType->setValidator( new KMimeTypeValidator( mMimeType ) );
  mMimeType->insertStringList( TQStringList()
			       << TQString::fromLatin1("text/html")
			       << TQString::fromLatin1("text/plain")
			       << TQString::fromLatin1("image/gif")
			       << TQString::fromLatin1("image/jpeg")
			       << TQString::fromLatin1("image/png")
			       << TQString::fromLatin1("application/octet-stream")
			       << TQString::fromLatin1("application/x-gunzip")
			       << TQString::fromLatin1("application/zip") );
  connect( mMimeType, TQT_SIGNAL(textChanged(const TQString&)),
	   this, TQT_SLOT(slotMimeTypeChanged(const TQString&)) );
  glay->addWidget( mMimeType, 0, 1 );

  msg = i18n("<qt><p>The <em>MIME type</em> of the file:</p>"
	     "<p>normally, you do not need to touch this setting, since the "
	     "type of the file is automatically checked; but, sometimes, %1 "
	     "may not detect the type correctly -- here is where you can fix "
	     "that.</p></qt>").arg( kapp->aboutData()->programName() );
  TQWhatsThis::add( mMimeType, msg );

  // row 1: Size label:
  mSize = new TQLabel( plainPage() );
  setSize( KIO::filesize_t(0) );
  glay->addWidget( mSize, 1, 1 );

  msg = i18n("<qt><p>The size of the part:</p>"
	     "<p>sometimes, %1 will only give an estimated size here, "
	     "because calculating the exact size would take too much time; "
	     "when this is the case, it will be made visible by adding "
	     "\"(est.)\" to the size displayed.</p></qt>")
    .arg( kapp->aboutData()->programName() );
  TQWhatsThis::add( mSize, msg );

  // row 2: "Name" lineedit and label:
  mFileName = new KLineEdit( plainPage() );
  label = new TQLabel( mFileName, i18n("&Name:"), plainPage() );
  glay->addWidget( label, 2, 0 );
  glay->addWidget( mFileName, 2, 1 );

  msg = i18n("<qt><p>The file name of the part:</p>"
	     "<p>although this defaults to the name of the attached file, "
	     "it does not specify the file to be attached; rather, it "
	     "suggests a file name to be used by the recipient's mail agent "
	     "when saving the part to disk.</p></qt>");
  TQWhatsThis::add( label, msg );
  TQWhatsThis::add( mFileName, msg );

  // row 3: "Description" lineedit and label:
  mDescription = new KLineEdit( plainPage() );
  label = new TQLabel( mDescription, i18n("&Description:"), plainPage() );
  glay->addWidget( label, 3, 0 );
  glay->addWidget( mDescription, 3, 1 );

  msg = i18n("<qt><p>A description of the part:</p>"
	     "<p>this is just an informational description of the part, "
	     "much like the Subject is for the whole message; most "
	     "mail agents will show this information in their message "
	     "previews alongside the attachment's icon.</p></qt>");
  TQWhatsThis::add( label, msg );
  TQWhatsThis::add( mDescription, msg );

  // row 4: "Encoding" combobox and label:
  mEncoding = new TQComboBox( false, plainPage() );
  mEncoding->insertStringList( mI18nizedEncodings );
  label = new TQLabel( mEncoding, i18n("&Encoding:"), plainPage() );
  glay->addWidget( label, 4, 0 );
  glay->addWidget( mEncoding, 4, 1 );

  msg = i18n("<qt><p>The transport encoding of this part:</p>"
	     "<p>normally, you do not need to change this, since %1 will use "
	     "a decent default encoding, depending on the MIME type; yet, "
	     "sometimes, you can significantly reduce the size of the "
	     "resulting message, e.g. if a PostScript file does not contain "
	     "binary data, but consists of pure text -- in this case, choosing "
	     "\"quoted-printable\" over the default \"base64\" will save up "
	     "to 25% in resulting message size.</p></qt>")
    .arg( kapp->aboutData()->programName() );
  TQWhatsThis::add( label, msg );
  TQWhatsThis::add( mEncoding, msg );

  // row 5: "Suggest automatic display..." checkbox:
  mInline = new TQCheckBox( i18n("Suggest &automatic display"), plainPage() );
  glay->addMultiCellWidget( mInline, 5, 5, 0, 1 );

  msg = i18n("<qt><p>Check this option if you want to suggest to the "
	     "recipient the automatic (inline) display of this part in the "
	     "message preview, instead of the default icon view;</p>"
	     "<p>technically, this is carried out by setting this part's "
	     "<em>Content-Disposition</em> header field to \"inline\" "
	     "instead of the default \"attachment\".</p></qt>");
  TQWhatsThis::add( mInline, msg );

  // row 6: "Sign" checkbox:
  mSigned = new TQCheckBox( i18n("&Sign this part"), plainPage() );
  glay->addMultiCellWidget( mSigned, 6, 6, 0, 1 );

  msg = i18n("<qt><p>Check this option if you want this message part to be "
	     "signed;</p>"
	     "<p>the signature will be made with the key that you associated "
	     "with the currently-selected identity.</p></qt>");
  TQWhatsThis::add( mSigned, msg );

  // row 7: "Encrypt" checkbox:
  mEncrypted = new TQCheckBox( i18n("Encr&ypt this part"), plainPage() );
  glay->addMultiCellWidget( mEncrypted, 7, 7, 0, 1 );

  msg = i18n("<qt><p>Check this option if you want this message part to be "
	     "encrypted;</p>"
	     "<p>the part will be encrypted for the recipients of this "
	     "message</p></qt>");
  TQWhatsThis::add( mEncrypted, msg );
  // (row 8: spacer)
}


KMMsgPartDialog::~KMMsgPartDialog() {}


TQString KMMsgPartDialog::mimeType() const {
  return mMimeType->currentText();
}

void KMMsgPartDialog::setMimeType( const TQString & mimeType ) {
  int dummy = 0;
  TQString tmp = mimeType; // get rid of const'ness
  if ( mMimeType->validator() && mMimeType->validator()->validate( tmp, dummy ) )
    for ( int i = 0 ; i < mMimeType->count() ; ++i )
      if ( mMimeType->text( i ) == mimeType ) {
	mMimeType->setCurrentItem( i );
	return;
      }
  mMimeType->insertItem( mimeType, 0 );
  mMimeType->setCurrentItem( 0 );
  slotMimeTypeChanged( mimeType );
}

void KMMsgPartDialog::setMimeType( const TQString & type,
				   const TQString & subtype ) {
  setMimeType( TQString::fromLatin1("%1/%2").arg(type).arg(subtype) );
}

void KMMsgPartDialog::setMimeTypeList( const TQStringList & mimeTypes ) {
  mMimeType->insertStringList( mimeTypes );
}

void KMMsgPartDialog::setSize( KIO::filesize_t size, bool estimated ) {
  TQString text = KIO::convertSize( size );
  if ( estimated )
    text = i18n("%1: a filesize incl. unit (e.g. \"1.3 KB\")",
		"%1 (est.)").arg( text );
  mSize->setText( text );
}

TQString KMMsgPartDialog::fileName() const {
  return mFileName->text();
}

void KMMsgPartDialog::setFileName( const TQString & fileName ) {
  mFileName->setText( fileName );
}

TQString KMMsgPartDialog::description() const {
  return mDescription->text();
}

void KMMsgPartDialog::setDescription( const TQString & description ) {
  mDescription->setText( description );
}

KMMsgPartDialog::Encoding KMMsgPartDialog::encoding() const {
  TQString s = mEncoding->currentText();
  for ( unsigned int i = 0 ; i < mI18nizedEncodings.count() ; ++i )
    if ( s == *mI18nizedEncodings.at(i) )
      return encodingTypes[i].encoding;
  kdFatal(5006) << "KMMsgPartDialog::encoding(): Unknown encoding encountered!"
		<< endl;
  return None; // keep compiler happy
}

void KMMsgPartDialog::setEncoding( Encoding encoding ) {
  for ( int i = 0 ; i < numEncodingTypes ; ++i )
    if ( encodingTypes[i].encoding == encoding ) {
      TQString text = *mI18nizedEncodings.at(i);
      for ( int j = 0 ; j < mEncoding->count() ; ++j )
	if ( mEncoding->text(j) == text ) {
	  mEncoding->setCurrentItem( j );
	  return;
	}
      mEncoding->insertItem( text, 0 );
      mEncoding->setCurrentItem( 0 );
    }
  kdFatal(5006) << "KMMsgPartDialog::setEncoding(): "
    "Unknown encoding encountered!" << endl;
}

void KMMsgPartDialog::setShownEncodings( int encodings ) {
  mEncoding->clear();
  for ( int i = 0 ; i < numEncodingTypes ; ++i )
    if ( encodingTypes[i].encoding & encodings )
      mEncoding->insertItem( *mI18nizedEncodings.at(i) );
}

bool KMMsgPartDialog::isInline() const {
  return mInline->isChecked();
}

void KMMsgPartDialog::setInline( bool inlined ) {
  mInline->setChecked( inlined );
}

bool KMMsgPartDialog::isEncrypted() const {
  return mEncrypted->isChecked();
}

void KMMsgPartDialog::setEncrypted( bool encrypted ) {
  mEncrypted->setChecked( encrypted );
}

void KMMsgPartDialog::setCanEncrypt( bool enable ) {
  mEncrypted->setEnabled( enable );
}

bool KMMsgPartDialog::isSigned() const {
  return mSigned->isChecked();
}

void KMMsgPartDialog::setSigned( bool sign ) {
  mSigned->setChecked( sign );
}

void KMMsgPartDialog::setCanSign( bool enable ) {
  mSigned->setEnabled( enable );
}

void KMMsgPartDialog::slotMimeTypeChanged( const TQString & mimeType ) {
  // message subparts MUST have 7bit or 8bit encoding...
#if 0
  // ...but until KMail can recode 8bit messages on attach, so that
  // they can be signed, we can't enforce this :-(
  if ( mimeType.startsWith("message/") ) {
    setEncoding( SevenBit );
    mEncoding->setEnabled( false );
  } else {
    mEncoding->setEnabled( !mReadOnly );
  }
#endif
  // find a mimetype icon:
  int dummy = 0;
  TQString tmp = mimeType; // get rid of const'ness
  if ( mMimeType->validator() && mMimeType->validator()->validate( tmp, dummy )
       == TQValidator::Acceptable )
    mIcon->setPixmap( KMimeType::mimeType( mimeType )->pixmap( KIcon::Desktop ) );
  else
    mIcon->setPixmap( DesktopIcon("unknown") );
}




KMMsgPartDialogCompat::KMMsgPartDialogCompat( TQWidget * parent, const char *, bool readOnly)
  : KMMsgPartDialog(TQString::null, parent ), mMsgPart( 0 )
{
  setShownEncodings( SevenBit|EightBit|QuotedPrintable|Base64 );
  if (readOnly)
  {
    mMimeType->setEditable(false);
    mMimeType->setEnabled(false);
    mFileName->setReadOnly(true);
    mDescription->setReadOnly(true);
    mEncoding->setEnabled(false);
    mInline->setEnabled(false);
    mEncrypted->setEnabled(false);
    mSigned->setEnabled(false);
  }
}

KMMsgPartDialogCompat::~KMMsgPartDialogCompat() {}

void KMMsgPartDialogCompat::setMsgPart( KMMessagePart * aMsgPart )
{
  mMsgPart = aMsgPart;
  assert( mMsgPart );

  TQCString enc = mMsgPart->cteStr();
  if ( enc == "7bit" )
    setEncoding( SevenBit );
  else if ( enc == "8bit" )
    setEncoding( EightBit );
  else if ( enc == "quoted-printable" )
    setEncoding( QuotedPrintable );
  else
    setEncoding( Base64 );

  setDescription( mMsgPart->contentDescription() );
  setFileName( mMsgPart->fileName() );
  setMimeType( mMsgPart->typeStr(), mMsgPart->subtypeStr() );
  setSize( mMsgPart->decodedSize() );
  setInline( mMsgPart->contentDisposition()
	     .find( TQRegExp("^\\s*inline", false) ) >= 0 );
}


void KMMsgPartDialogCompat::applyChanges()
{
  if (!mMsgPart) return;

  KCursorSaver busy(KBusyPtr::busy());

  // apply Content-Disposition:
  TQCString cDisp;
  if ( isInline() )
    cDisp = "inline;";
  else
    cDisp = "attachment;";

  TQString name = fileName();
  if ( !name.isEmpty() || !mMsgPart->name().isEmpty()) {
    mMsgPart->setName( name );
    TQCString encName = KMMsgBase::encodeRFC2231StringAutoDetectCharset( name, mMsgPart->charset() );

    cDisp += "\n\tfilename";
    if ( name != TQString( encName ) )
      cDisp += "*=" + encName;
    else
      cDisp += "=\"" + encName.replace( '\\', "\\\\" ).replace( '"', "\\\"" ) + '"';
    mMsgPart->setContentDisposition( cDisp );
  }

  // apply Content-Description"
  TQString desc = description();
  if ( !desc.isEmpty() || !mMsgPart->contentDescription().isEmpty() )
    mMsgPart->setContentDescription( desc );

  // apply Content-Type:
  TQCString type = mimeType().latin1();
  TQCString subtype;
  int idx = type.find('/');
  if ( idx < 0 )
    subtype = "";
  else {
    subtype = type.mid( idx+1 );
    type = type.left( idx );
  }
  mMsgPart->setTypeStr(type);
  mMsgPart->setSubtypeStr(subtype);

  // apply Content-Transfer-Encoding:
  TQCString cte;
  if (subtype == "rfc822" && type == "message")
    kdWarning( encoding() != SevenBit && encoding() != EightBit, 5006 )
      << "encoding on rfc822/message must be \"7bit\" or \"8bit\"" << endl;
  switch ( encoding() ) {
  case SevenBit:        cte = "7bit";             break;
  case EightBit:        cte = "8bit";             break;
  case QuotedPrintable: cte = "quoted-printable"; break;
  case Base64: default: cte = "base64";           break;
  }
  if ( cte != mMsgPart->cteStr().lower() ) {
    TQByteArray body = mMsgPart->bodyDecodedBinary();
    mMsgPart->setCteStr( cte );
    mMsgPart->setBodyEncodedBinary( body );
  }
}


//-----------------------------------------------------------------------------
void KMMsgPartDialogCompat::slotOk()
{
  applyChanges();
  KMMsgPartDialog::slotOk();
}


//-----------------------------------------------------------------------------
#include "kmmsgpartdlg.moc"
