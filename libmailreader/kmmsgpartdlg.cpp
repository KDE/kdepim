// kmmsgpartdlg.cpp


// my includes:
#include "kmmsgpartdlg.h"

// other KMail includes:
#include "kcursorsaver.h"

// other kdenetwork includes: (none)

// other KDE includes:
#include <klineedit.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <kstringvalidator.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kascii.h>
#include <KCharsets>

#include <kmime/kmime_content.h>

// other Qt includes:
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QByteArray>
#include <QCheckBox>
#include <QTextCodec>

// other includes:
#include <assert.h>

static const struct {
  KMMsgPartDialog::Encoding encoding;
  const char * displayName;
} encodingTypes[] = {
  { KMMsgPartDialog::SevenBit, I18N_NOOP2("message encoding type", "None (7-bit text)") },
  { KMMsgPartDialog::EightBit, I18N_NOOP2("message encoding type", "None (8-bit text)") },
  { KMMsgPartDialog::QuotedPrintable, I18N_NOOP2("message encoding type", "Quoted Printable") },
  { KMMsgPartDialog::Base64, I18N_NOOP2("message encoding type", "Base 64") },
};
static const int numEncodingTypes =
  sizeof encodingTypes / sizeof *encodingTypes;

QByteArray autoDetectCharset(const QByteArray &_encoding, const QStringList &encodingList, const QString &text);
QByteArray encodeRFC2231String( const QString& _str, const QByteArray& charset );
const QTextCodec* codecForName(const QByteArray& _str);
QByteArray toUsAscii(const QString& _str, bool *ok = 0);

KMMsgPartDialog::KMMsgPartDialog( const QString & caption,
                                  QWidget * parent )
  : KDialog( parent )
{
  setCaption( caption.isEmpty() ? i18n("Message Part Properties") : caption );
  setButtons( Ok|Cancel|Help );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( true );

  // tmp vars:
  QGridLayout * glay;
  QLabel      * label;
  QString       msg;

  setHelp( QString::fromLatin1("attachments") );

  for ( int i = 0 ; i < numEncodingTypes ; ++i )
    mI18nizedEncodings << i18nc( "message encoding type", encodingTypes[i].displayName );
  QFrame *frame = new QFrame( this );
  setMainWidget( frame );
  glay = new QGridLayout(frame );
  glay->setSpacing( spacingHint() );
  glay->setColumnStretch( 1, 1 );
  glay->setRowStretch( 8, 1 );

  // mimetype icon:
  mIcon = new QLabel( frame );
  mIcon->setPixmap( DesktopIcon("unknown") );
  glay->addWidget( mIcon, 0, 0, 2, 1);

  // row 0: Type combobox:
  mMimeType = new KComboBox( true, frame );
  mMimeType->setInsertPolicy( KComboBox::NoInsert );
  mMimeType->setValidator( new KMimeTypeValidator( mMimeType ) );
  mMimeType->addItems( QStringList()
                               << QString::fromLatin1("text/html")
                               << QString::fromLatin1("text/plain")
                               << QString::fromLatin1("image/gif")
                               << QString::fromLatin1("image/jpeg")
                               << QString::fromLatin1("image/png")
                               << QString::fromLatin1("application/octet-stream")
                               << QString::fromLatin1("application/x-gunzip")
                               << QString::fromLatin1("application/zip") );
  connect( mMimeType, SIGNAL(editTextChanged(const QString&)),
           this, SLOT(slotMimeTypeChanged(const QString&)) );
  glay->addWidget( mMimeType, 0, 1 );

  msg = i18n("<qt><p>The <em>MIME type</em> of the file:</p>"
             "<p>normally, you do not need to touch this setting, since the "
             "type of the file is automatically checked; but, sometimes, %1 "
             "may not detect the type correctly -- here is where you can fix "
             "that.</p></qt>", KGlobal::mainComponent().aboutData()->programName() );
  mMimeType->setWhatsThis( msg );

  // row 1: Size label:
  mSize = new QLabel( frame );
  setSize( KIO::filesize_t(0) );
  glay->addWidget( mSize, 1, 1 );

  msg = i18n("<qt><p>The size of the part:</p>"
             "<p>sometimes, %1 will only give an estimated size here, "
             "because calculating the exact size would take too much time; "
             "when this is the case, it will be made visible by adding "
             "\"(est.)\" to the size displayed.</p></qt>",
      KGlobal::mainComponent().aboutData()->programName() );
  mSize->setWhatsThis( msg );

  // row 2: "Name" lineedit and label:
  mFileName = new KLineEdit( frame );
  mFileName->setClearButtonShown( true );
  label = new QLabel( i18nc("file name of the attachment.", "&Name:"), frame );
  label->setBuddy( mFileName );
  glay->addWidget( label, 2, 0 );
  glay->addWidget( mFileName, 2, 1 );

  msg = i18n("<qt><p>The file name of the part:</p>"
             "<p>although this defaults to the name of the attached file, "
             "it does not specify the file to be attached; rather, it "
             "suggests a file name to be used by the recipient's mail agent "
             "when saving the part to disk.</p></qt>");
  label->setWhatsThis( msg );
  mFileName->setWhatsThis( msg );

  // row 3: "Description" lineedit and label:
  mDescription = new KLineEdit( frame );
  mDescription->setClearButtonShown( true );
  label = new QLabel( i18n("&Description:"), frame );
  label->setBuddy( mDescription );
  glay->addWidget( label, 3, 0 );
  glay->addWidget( mDescription, 3, 1 );

  msg = i18n("<qt><p>A description of the part:</p>"
             "<p>this is just an informational description of the part, "
             "much like the Subject is for the whole message; most "
             "mail agents will show this information in their message "
             "previews alongside the attachment's icon.</p></qt>");
  label->setWhatsThis( msg );
  mDescription->setWhatsThis( msg );

  // row 4: "Encoding" combobox and label:
  mEncoding = new KComboBox( frame );
  mEncoding->setEditable( false );
  mEncoding->addItems( mI18nizedEncodings );
  label = new QLabel( i18n("&Encoding:"), frame );
  label->setBuddy( mEncoding );
  glay->addWidget( label, 4, 0 );
  glay->addWidget( mEncoding, 4, 1 );

  msg = i18n("<qt><p>The transport encoding of this part:</p>"
             "<p>normally, you do not need to change this, since %1 will use "
             "a decent default encoding, depending on the MIME type; yet, "
             "sometimes, you can significantly reduce the size of the "
             "resulting message, e.g. if a PostScript file does not contain "
             "binary data, but consists of pure text -- in this case, choosing "
             "\"quoted-printable\" over the default \"base64\" will save up "
             "to 25% in resulting message size.</p></qt>",
      KGlobal::mainComponent().aboutData()->programName() );
  label->setWhatsThis( msg );
  mEncoding->setWhatsThis( msg );

  // row 5: "Suggest automatic display..." checkbox:
  mInline = new QCheckBox( i18n("Suggest &automatic display"), frame );
  glay->addWidget( mInline, 5, 0, 1, 2 );

  msg = i18n("<qt><p>Check this option if you want to suggest to the "
             "recipient the automatic (inline) display of this part in the "
             "message preview, instead of the default icon view;</p>"
             "<p>technically, this is carried out by setting this part's "
             "<em>Content-Disposition</em> header field to \"inline\" "
             "instead of the default \"attachment\".</p></qt>");
  mInline->setWhatsThis( msg );

  // row 6: "Sign" checkbox:
  mSigned = new QCheckBox( i18n("&Sign this part"), frame );
  glay->addWidget( mSigned, 6, 0, 1, 2 );

  msg = i18n("<qt><p>Check this option if you want this message part to be "
             "signed;</p>"
             "<p>the signature will be made with the key that you associated "
             "with the currently-selected identity.</p></qt>");
  mSigned->setWhatsThis( msg );

  // row 7: "Encrypt" checkbox:
  mEncrypted = new QCheckBox( i18n("Encr&ypt this part"), frame );
  glay->addWidget( mEncrypted, 7, 0, 1, 2 );

  msg = i18n("<qt><p>Check this option if you want this message part to be "
             "encrypted;</p>"
             "<p>the part will be encrypted for the recipients of this "
             "message</p></qt>");
  mEncrypted->setWhatsThis( msg );
  // (row 8: spacer)
}


KMMsgPartDialog::~KMMsgPartDialog() {}


QString KMMsgPartDialog::mimeType() const {
  return mMimeType->currentText();
}

void KMMsgPartDialog::setMimeType( const QString & mimeType ) {
  for ( int i = 0 ; i < mMimeType->count() ; ++i )
    if ( mMimeType->itemText( i ) == mimeType ) {
      mMimeType->setCurrentIndex( i );
      slotMimeTypeChanged( mimeType );
      return;
    }
  mMimeType->insertItem( 0, mimeType );
  mMimeType->setCurrentIndex( 0 );
  slotMimeTypeChanged( mimeType );
}

void KMMsgPartDialog::setMimeType( const QString & type,
                                   const QString & subtype ) {
  setMimeType( QString::fromLatin1("%1/%2").arg(type).arg(subtype) );
}

void KMMsgPartDialog::setMimeTypeList( const QStringList & mimeTypes ) {
  mMimeType->addItems( mimeTypes );
}

void KMMsgPartDialog::setSize( KIO::filesize_t size, bool estimated ) {
  QString text = KIO::convertSize( size );
  if ( estimated )
    text = i18nc("%1: a filesize incl. unit (e.g. \"1.3 KB\")",
                "%1 (est.)", text );
  mSize->setText( text );
}

QString KMMsgPartDialog::fileName() const {
  return mFileName->text();
}

void KMMsgPartDialog::setFileName( const QString & fileName ) {
  mFileName->setText( fileName );
}

QString KMMsgPartDialog::description() const {
  return mDescription->text();
}

void KMMsgPartDialog::setDescription( const QString & description ) {
  mDescription->setText( description );
}

KMMsgPartDialog::Encoding KMMsgPartDialog::encoding() const {
  QString s = mEncoding->currentText();
  for ( int i = 0 ; i < mI18nizedEncodings.count() ; ++i )
    if ( s == mI18nizedEncodings.at(i) )
      return encodingTypes[i].encoding;
  kFatal(5006) <<"KMMsgPartDialog::encoding(): Unknown encoding encountered!";
  return None; // keep compiler happy
}

void KMMsgPartDialog::setEncoding( Encoding encoding ) {
  for ( int i = 0 ; i < numEncodingTypes ; ++i )
    if ( encodingTypes[i].encoding == encoding ) {
      QString text = mI18nizedEncodings.at(i);
      for ( int j = 0 ; j < mEncoding->count() ; ++j )
        if ( mEncoding->itemText(j) == text ) {
          mEncoding->setCurrentIndex( j );
          return;
        }
      mEncoding->insertItem( 0, text );
      mEncoding->setCurrentIndex( 0 );
    }
  kFatal(5006) <<"KMMsgPartDialog::setEncoding():"
    "Unknown encoding encountered!";
}

void KMMsgPartDialog::setShownEncodings( int encodings ) {
  mEncoding->clear();
  for ( int i = 0 ; i < numEncodingTypes ; ++i )
    if ( encodingTypes[i].encoding & encodings )
      mEncoding->addItem( mI18nizedEncodings.at(i) );
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

void KMMsgPartDialog::slotMimeTypeChanged( const QString & mimeType ) {
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
  KMimeType::Ptr mt = KMimeType::mimeType( mimeType, KMimeType::ResolveAliases );
  if ( !mt.isNull() )
    mIcon->setPixmap( KIconLoader::global()->loadMimeTypeIcon( mt->iconName(),
                      KIconLoader::Desktop ) );
  else
    mIcon->setPixmap( DesktopIcon("unknown") );
}




KMMsgPartDialogCompat::KMMsgPartDialogCompat( const char *, bool readOnly)
  : KMMsgPartDialog(), mMsgPart( 0 )
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
  connect(this,SIGNAL(okClicked()),SLOT(slotOk()));
}

KMMsgPartDialogCompat::~KMMsgPartDialogCompat() {}

void KMMsgPartDialogCompat::setMsgPart( KMime::Content * aMsgPart )
{
  mMsgPart = aMsgPart;
  assert( mMsgPart );

  QByteArray enc = mMsgPart->contentTransferEncoding()->as7BitString();
  if ( enc == "7bit" )
    setEncoding( SevenBit );
  else if ( enc == "8bit" )
    setEncoding( EightBit );
  else if ( enc == "quoted-printable" )
    setEncoding( QuotedPrintable );
  else
    setEncoding( Base64 );

  setDescription( mMsgPart->contentDescription()->asUnicodeString() );
  setFileName( mMsgPart->contentDisposition()->filename() );
  setMimeType( mMsgPart->contentType()->mediaType(), mMsgPart->contentType()->subType() );
  setSize( mMsgPart->decodedContent().size() );
  QString cd(mMsgPart->contentDisposition()->asUnicodeString());
  setInline( cd.indexOf( QRegExp("^\\s*inline", Qt::CaseInsensitive) ) >= 0 );
}


void KMMsgPartDialogCompat::applyChanges()
{
  if (!mMsgPart) return;

  KCursorSaver busy(KBusyPtr::busy());

  // apply Content-Disposition:
  QByteArray cDisp;
  if ( isInline() )
    cDisp = "inline;";
  else
    cDisp = "attachment;";

  QString name = fileName();
  if ( !name.isEmpty() || !mMsgPart->contentType()->name().isEmpty()) {
    mMsgPart->contentType()->setName( name, mMsgPart->contentType()->charset()); //FIXME(Andras) check if the second arg is ok
    QByteArray encoding = autoDetectCharset( mMsgPart->contentType()->charset(), QStringList()
      /*KMMessage::preferredCharsets()*/, name ); //FIXME(Andras) read the pref charset
    if ( encoding.isEmpty() ) encoding = "utf-8";
    QByteArray encName = encodeRFC2231String( name, encoding );

    cDisp += "\n\tfilename";
    if ( name != QString( encName ) )
      cDisp += "*=" + encName;
    else
      cDisp += "=\"" + encName.replace( '\\', "\\\\" ).replace( '"', "\\\"" ) + '"';
    mMsgPart->contentDisposition()->from7BitString( cDisp );
  }

  // apply Content-Description"
  QString desc = description();
  if ( !desc.isEmpty() || !mMsgPart->contentDescription()->asUnicodeString().isEmpty() )
    mMsgPart->contentDescription()->fromUnicodeString( desc, mMsgPart->contentDescription()->defaultCharset() );

  // apply Content-Type:
  QByteArray type = mimeType().toLatin1();
  QByteArray subtype;
  int idx = type.indexOf('/');
  if ( idx < 0 )
    subtype = "";
  else {
    subtype = type.mid( idx+1 );
    type = type.left( idx );
  }
  mMsgPart->contentType()->setMimeType( type + "/" + subtype );

  // apply Content-Transfer-Encoding:
  QByteArray cte;
  if (subtype == "rfc822" && type == "message")
    kWarning( encoding() != SevenBit && encoding() != EightBit, 5006 )
      << "encoding on rfc822/message must be \"7bit\" or \"8bit\"";
  switch ( encoding() ) {
  case SevenBit:        cte = "7bit";             break;
  case EightBit:        cte = "8bit";             break;
  case QuotedPrintable: cte = "quoted-printable"; break;
  case Base64: default: cte = "base64";           break;
  }
  if ( cte != mMsgPart->contentTransferEncoding()->as7BitString().toLower() ) {
    QByteArray body = mMsgPart->decodedContent();
    mMsgPart->contentTransferEncoding()->from7BitString( cte );
    mMsgPart->setBody( body );
  }
}


//-----------------------------------------------------------------------------
void KMMsgPartDialogCompat::slotOk()
{
  applyChanges();
}

QByteArray autoDetectCharset(const QByteArray &_encoding, const QStringList &encodingList, const QString &text)
{
    QStringList charsets = encodingList;
    if (!_encoding.isEmpty())
    {
       QString currentCharset = QString::fromLatin1(_encoding);
       charsets.removeAll(currentCharset);
       charsets.prepend(currentCharset);
    }

    QStringList::ConstIterator it = charsets.constBegin();
    for (; it != charsets.constEnd(); ++it)
    {
       QByteArray encoding = (*it).toLatin1();
       if (encoding == "locale")
       {
       /*FIXME(Andras) port it
         encoding = kmkernel->networkCodec()->name();
         kAsciiToLower(encoding.data());
         */
       }
       if (text.isEmpty())
         return encoding;
       if (encoding == "us-ascii") {
         bool ok;
         (void) toUsAscii(text, &ok);
         if (ok)
            return encoding;
       }
       else
       {
         const QTextCodec *codec = codecForName(encoding);
         if (!codec) {
           kDebug() <<"Auto-Charset: Something is wrong and I can not get a codec. [" << encoding <<"]";
         } else {
           if (codec->canEncode(text))
              return encoding;
         }
       }
    }
    return 0;
}

QByteArray encodeRFC2231String( const QString& _str,
                                         const QByteArray& charset )
{
  static const QByteArray especials = "()<>@,;:\"/[]?.= \033";

  if ( _str.isEmpty() )
    return QByteArray();

  QByteArray cset;
  if ( charset.isEmpty() )
  {
  /*FIXME(Andras) port it
    cset = kmkernel->networkCodec()->name();
    kAsciiToLower( cset.data() );
    */
  }
  else
    cset = charset;
  const QTextCodec *codec = codecForName( cset );
  QByteArray latin;
  if ( charset == "us-ascii" )
    latin = toUsAscii( _str );
  else if ( codec )
    latin = codec->fromUnicode( _str );
  else
    latin = _str.toLocal8Bit();

  char *l;
  for ( l = latin.data(); *l; ++l ) {
    if ( ( ( *l & 0xE0 ) == 0 ) || ( *l & 0x80 ) )
      // *l is control character or 8-bit char
      break;
  }
  if ( !*l )
    return latin;

  QByteArray result = cset + "''";
  for ( l = latin.data(); *l; ++l ) {
    bool needsQuoting = ( *l & 0x80 ) || ( *l == '%' );
    if( !needsQuoting ) {
      int len = especials.length();
      for ( int i = 0; i < len; i++ )
        if ( *l == especials[i] ) {
          needsQuoting = true;
          break;
        }
    }
    if ( needsQuoting ) {
      result += '%';
      unsigned char hexcode;
      hexcode = ( ( *l & 0xF0 ) >> 4 ) + 48;
      if ( hexcode >= 58 )
        hexcode += 7;
      result += hexcode;
      hexcode = ( *l & 0x0F ) + 48;
      if ( hexcode >= 58 )
        hexcode += 7;
      result += hexcode;
    } else {
      result += *l;
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
const QTextCodec* codecForName(const QByteArray& _str)
{
  if (_str.isEmpty())
    return 0;
  QByteArray codec = _str;
  kAsciiToLower(codec.data());
  return KGlobal::charsets()->codecForName(codec);
}

QByteArray toUsAscii(const QString& _str, bool *ok)
{
  bool all_ok =true;
  QString result = _str;
  int len = result.length();
  for (int i = 0; i < len; i++)
    if (result.at(i).unicode() >= 128) {
      result[i] = '?';
      all_ok = false;
    }
  if (ok)
    *ok = all_ok;
  return result.toLatin1();
}



//-----------------------------------------------------------------------------
#include "kmmsgpartdlg.moc"
