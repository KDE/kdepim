/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  Based on KMail code by various authors (kmmsgpartdlg).

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "attachmentpropertiesdialog.h"

#include "attachmentfrommimecontentjob.h"
#include "ui_attachmentpropertiesdialog.h"

#include <KAboutData>
#include <KComponentData>
#include <KDebug>
#include <KGlobal>
#include <KMimeType>

#include <kmime/kmime_content.h>
#include <kmime/kmime_headers.h>
#include <kmime/kmime_util.h>

#include <boost/shared_ptr.hpp>

using namespace MessageCore;

class MessageCore::AttachmentPropertiesDialog::Private
{
  public:
    Private( AttachmentPropertiesDialog *qq )
      : q( qq ),
        mReadOnly( false )
    {
    }

    void init( const AttachmentPart::Ptr &part, bool readOnly );
    void polishUi();
    void mimeTypeChanged( const QString &type ); // slot
    void populateEncodings();
    void populateMimeTypes();
    void populateWhatsThis();
    void loadFromPart();
    void saveToPart();

    AttachmentPropertiesDialog *const q;
    bool mReadOnly;
    AttachmentPart::Ptr mPart;
    Ui::AttachmentPropertiesDialog ui;
};

void AttachmentPropertiesDialog::Private::init( const AttachmentPart::Ptr &part, bool readOnly )
{
  mReadOnly = readOnly;
  mPart = part;

  QWidget *widget = new QWidget( q );
  q->setMainWidget( widget );
  ui.setupUi( widget );
  polishUi();
  q->setModal( true );
  q->showButtonSeparator( true );
  q->setHelp( QString::fromLatin1( "attachments" ) );

  loadFromPart();
}

void AttachmentPropertiesDialog::Private::polishUi()
{
  // Update the icon when the selected mime type changes.
  connect( ui.mimeType, SIGNAL(currentIndexChanged(QString)),
           q, SLOT(mimeTypeChanged(QString)) );

  // Tweak the dialog, depending on whether it is read-only or not.
  if ( mReadOnly ) {
    ui.mimeType->setEditable( false );
    ui.name->setReadOnly( true );
    ui.description->setReadOnly( true );
    ui.encoding->setEnabled( false );
    ui.sign->hide();
    ui.autoDisplay->hide();
    ui.encrypt->hide();
    q->setButtons( Close | Help );
  } else {
    populateMimeTypes();

    q->setButtons( Ok | Cancel | Help );
  }
  populateEncodings();
  q->setDefaultButton( Ok );
  populateWhatsThis();
}

void AttachmentPropertiesDialog::Private::mimeTypeChanged( const QString &type )
{
  const KMimeType::Ptr mimeType = KMimeType::mimeType( type, KMimeType::ResolveAliases );
  if ( !mimeType.isNull() ) {
    ui.mimeIcon->setPixmap( KIconLoader::global()->loadMimeTypeIcon( mimeType->iconName(),
                            KIconLoader::Desktop ) );
  } else {
    ui.mimeIcon->setPixmap( DesktopIcon( "unknown" ) );
  }
}

void AttachmentPropertiesDialog::Private::populateWhatsThis()
{
  // FIXME These are such a mess... Make them straightforward and pretty.

  QString msg = i18n( "<p>The <em>MIME type</em> of the file:</p>"
              "<p>Normally, you do not need to touch this setting, since the "
              "type of the file is automatically checked; but, sometimes, %1 "
              "may not detect the type correctly -- here is where you can fix "
              "that.</p>", KGlobal::mainComponent().aboutData()->programName() );
  ui.mimeType->setWhatsThis( msg );

  msg = i18n( "<p>The estimated size of the attachment:</p>"
              "<p>Note that, in an email message, a binary file encoded with "
              "base64 will take up four thirds the actual size of the file.</p>" );
  ui.size->setWhatsThis( msg );

  msg = i18n( "<p>The file name of the part:</p>"
              "<p>Although this defaults to the name of the attached file, "
              "it does not specify the file to be attached; rather, it "
              "suggests a file name to be used by the recipient's mail agent "
              "when saving the part to disk.</p>" );
  ui.name->setWhatsThis( msg );

  msg = i18n( "<p>A description of the part:</p>"
              "<p>This is just an informational description of the part, "
              "much like the Subject is for the whole message; most "
              "mail agents will show this information in their message "
              "previews alongside the attachment's icon.</p>" );
  ui.description->setWhatsThis( msg );

  msg = i18n( "<p>The transport encoding of this part:</p>"
              "<p>Normally, you do not need to change this, since %1 will use "
              "a decent default encoding, depending on the MIME type; yet, "
              "sometimes, you can significantly reduce the size of the "
              "resulting message, e.g. if a PostScript file does not contain "
              "binary data, but consists of pure text -- in this case, choosing "
              "\"quoted-printable\" over the default \"base64\" will save up "
              "to 25% in resulting message size.</p>",
              KGlobal::mainComponent().aboutData()->programName() );
  ui.encoding->setWhatsThis( msg );

  msg = i18n( "<p>Check this option if you want to suggest to the "
              "recipient the automatic (inline) display of this part in the "
              "message preview, instead of the default icon view;</p>"
              "<p>Technically, this is carried out by setting this part's "
              "<em>Content-Disposition</em> header field to \"inline\" "
              "instead of the default \"attachment\".</p>" );
  ui.autoDisplay->setWhatsThis( msg );

  msg = i18n( "<p>Check this option if you want this message part to be "
              "signed.</p>"
              "<p>The signature will be made with the key that you associated "
              "with the currently-selected identity.</p>" );
  ui.sign->setWhatsThis( msg );

  msg = i18n( "<p>Check this option if you want this message part to be "
              "encrypted.</p>"
              "<p>The part will be encrypted for the recipients of this "
              "message.</p>" );
  ui.encrypt->setWhatsThis( msg );
}

void AttachmentPropertiesDialog::Private::populateEncodings()
{
  using namespace KMime;
  using namespace KMime::Headers;

  ui.encoding->clear();
  ui.encoding->addItem( nameForEncoding( CE7Bit ), int( CE7Bit ) );
  ui.encoding->addItem( nameForEncoding( CE8Bit ), int( CE8Bit ) );
  ui.encoding->addItem( nameForEncoding( CEquPr ), int( CEquPr ) );
  ui.encoding->addItem( nameForEncoding( CEbase64 ), int( CEbase64 ) );

  // TODO 8bit should be disabled if it is disabled in Settings.
  // Also, if it's a message/* part, base64 and qp should be disabled.
  // But since this is a dialog for power users anyway, let them shoot
  // themselves in the foot.  (The AttachmentJob will fail when they
  // try to compose the message.)
}

void AttachmentPropertiesDialog::Private::populateMimeTypes()
{
  QStringList list;
  list << QString::fromLatin1( "text/html" )
       << QString::fromLatin1( "text/plain" )
       << QString::fromLatin1( "image/gif" )
       << QString::fromLatin1( "image/jpeg" )
       << QString::fromLatin1( "image/png" )
       << QString::fromLatin1( "application/octet-stream" )
       << QString::fromLatin1( "application/x-gunzip" )
       << QString::fromLatin1( "application/zip" );

  ui.mimeType->addItems( list );
}

void AttachmentPropertiesDialog::Private::loadFromPart()
{
  Q_ASSERT( mPart );

  ui.mimeType->setCurrentItem( mPart->mimeType(), true );
  ui.size->setText( KGlobal::locale()->formatByteSize( mPart->size() ) );
  ui.name->setText( mPart->name().isEmpty() ? mPart->fileName() : mPart->name()  );
  ui.description->setText( mPart->description() );
  ui.encoding->setCurrentIndex( int( mPart->encoding() ) );
  ui.autoDisplay->setChecked( mPart->isInline() );
  ui.encrypt->setChecked( mPart->isEncrypted() );
  ui.sign->setChecked( mPart->isSigned() );
}

static QString removeNewlines( const QString &input )
{
  QString ret( input );
  ret.replace( '\n', ' ' );
  return ret;
}

void AttachmentPropertiesDialog::Private::saveToPart()
{
  Q_ASSERT( mPart );
  Q_ASSERT( !mReadOnly );

  mPart->setMimeType( ui.mimeType->currentText().toLatin1() );
  mPart->setName( removeNewlines( ui.name->text() ) );
  // TODO what about fileName? Extra field??
  mPart->setDescription( removeNewlines( ui.description->text() ) );
  mPart->setInline( ui.autoDisplay->isChecked() );
  mPart->setSigned( ui.sign->isChecked() );
  mPart->setEncrypted( ui.encrypt->isChecked() );
  mPart->setInline( ui.autoDisplay->isChecked() );

  if ( ui.mimeType->currentText().startsWith( QLatin1String( "message" ) ) &&
       ui.encoding->itemData( ui.encoding->currentIndex() ) != KMime::Headers::CE7Bit &&
       ui.encoding->itemData( ui.encoding->currentIndex() ) != KMime::Headers::CE8Bit ) {
    kWarning() << "Encoding on message/rfc822 must be \"7bit\" or \"8bit\".";
  }

  mPart->setEncoding( KMime::Headers::contentEncoding(
                      ui.encoding->itemData( ui.encoding->currentIndex() ).toInt() ) );
}

AttachmentPropertiesDialog::AttachmentPropertiesDialog( const AttachmentPart::Ptr &part,
                                                        bool readOnly, QWidget *parent )
  : KDialog( parent ),
    d( new Private( this ) )
{
  d->init( part, readOnly );
  setWindowTitle(i18n("Attachment Properties"));
}

AttachmentPropertiesDialog::AttachmentPropertiesDialog( const KMime::Content *content,
                                                        QWidget *parent )
  : KDialog( parent ),
    d( new Private( this ) )
{
  AttachmentFromMimeContentJob *job = new AttachmentFromMimeContentJob( content, this );
  job->exec();
  if ( job->error() ) {
    kError() << "AttachmentFromMimeContentJob failed."<<job->errorString();
  }

  const AttachmentPart::Ptr part = job->attachmentPart();
  d->init( part, true );
  setWindowTitle(i18n("Attachment Properties"));
}

AttachmentPropertiesDialog::~AttachmentPropertiesDialog()
{
  delete d;
}

AttachmentPart::Ptr AttachmentPropertiesDialog::attachmentPart() const
{
  return d->mPart;
}

bool AttachmentPropertiesDialog::isEncryptEnabled() const
{
  return d->ui.encrypt->isEnabled();
}

void AttachmentPropertiesDialog::setEncryptEnabled( bool enabled )
{
  d->ui.encrypt->setEnabled( enabled );
}

bool AttachmentPropertiesDialog::isSignEnabled() const
{
  return d->ui.sign->isEnabled();
}

void AttachmentPropertiesDialog::setSignEnabled( bool enabled )
{
  d->ui.sign->setEnabled( enabled );
}

void AttachmentPropertiesDialog::accept()
{
  if ( !d->mReadOnly ) {
    d->saveToPart();
  }

  KDialog::accept();
}

#include "attachmentpropertiesdialog.moc"
