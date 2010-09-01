// kmfawidgets.h - KMFilterAction parameter widgets
// Copyright: (c) 2001 Marc Mutz <mutz@kde.org>
// License: GNU Genaral Public License

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kmfawidgets.h"

#include <kabc/addresseedialog.h> // for the button in KMFilterActionWithAddress
#include <kiconloader.h>
#include <klocale.h>
#include <kaudioplayer.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>

#include <tqlayout.h>
#include <tqtooltip.h>

//=============================================================================
//
// class KMFilterActionWithAddressWidget
//
//=============================================================================

KMFilterActionWithAddressWidget::KMFilterActionWithAddressWidget( TQWidget* parent, const char* name )
  : TQWidget( parent, name )
{
  TQHBoxLayout *hbl = new TQHBoxLayout(this);
  hbl->setSpacing(4);
  mLineEdit = new KLineEdit(this);
  mLineEdit->setName( "addressEdit" );
  hbl->addWidget( mLineEdit, 1 /*stretch*/ );
  mBtn = new TQPushButton( TQString::null ,this );
  mBtn->setPixmap( BarIcon( "contents", KIcon::SizeSmall ) );
  mBtn->setFixedHeight( mLineEdit->sizeHint().height() );
  TQToolTip::add( mBtn, i18n( "Open Address Book" ) );
  hbl->addWidget( mBtn );

  connect( mBtn, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotAddrBook()) );
  connect( mLineEdit, TQT_SIGNAL( textChanged(const TQString&) ),
           this, TQT_SIGNAL( textChanged(const TQString&) ) );
}

void KMFilterActionWithAddressWidget::slotAddrBook()
{
  KABC::Addressee::List lst = KABC::AddresseeDialog::getAddressees( this );

  if ( lst.empty() )
    return;

  TQStringList addrList;

  for( KABC::Addressee::List::const_iterator it = lst.begin(); it != lst.end(); ++it )
    addrList << (*it).fullEmail();

  TQString txt = mLineEdit->text().stripWhiteSpace();

  if ( !txt.isEmpty() ) {
    if ( !txt.endsWith( "," ) )
      txt += ", ";
    else
      txt += ' ';
  }

  mLineEdit->setText( txt + addrList.join(",") );
}

KMSoundTestWidget::KMSoundTestWidget(TQWidget *parent, const char *name)
    : TQWidget( parent, name)
{
    TQHBoxLayout *lay1 = new TQHBoxLayout( this );
    m_playButton = new TQPushButton( this, "m_playButton" );
    m_playButton->setPixmap( SmallIcon( "1rightarrow" ) );
    connect( m_playButton, TQT_SIGNAL( clicked() ), TQT_SLOT( playSound() ));
    lay1->addWidget( m_playButton );

    m_urlRequester = new KURLRequester( this );
    lay1->addWidget( m_urlRequester );
    connect( m_urlRequester, TQT_SIGNAL( openFileDialog( KURLRequester * )),
             TQT_SLOT( openSoundDialog( KURLRequester * )));
    connect( m_urlRequester->lineEdit(), TQT_SIGNAL( textChanged ( const TQString & )), TQT_SLOT( slotUrlChanged(const TQString & )));
    slotUrlChanged(m_urlRequester->lineEdit()->text() );
}

KMSoundTestWidget::~KMSoundTestWidget()
{
}

void KMSoundTestWidget::slotUrlChanged(const TQString &_text )
{
    m_playButton->setEnabled( !_text.isEmpty());
}

void KMSoundTestWidget::openSoundDialog( KURLRequester * )
{
    static bool init = true;
    if ( !init )
        return;

    init = false;

    KFileDialog *fileDialog = m_urlRequester->fileDialog();
    fileDialog->setCaption( i18n("Select Sound File") );
    TQStringList filters;
    filters << "audio/x-wav" << "audio/x-mp3" << "application/x-ogg"
            << "audio/x-adpcm";
    fileDialog->setMimeFilter( filters );

   TQStringList soundDirs = KGlobal::dirs()->resourceDirs( "sound" );

    if ( !soundDirs.isEmpty() ) {
        KURL soundURL;
        TQDir dir;
        dir.setFilter( TQDir::Files | TQDir::Readable );
        TQStringList::ConstIterator it = soundDirs.begin();
        while ( it != soundDirs.end() ) {
            dir = *it;
            if ( dir.isReadable() && dir.count() > 2 ) {
                soundURL.setPath( *it );
                fileDialog->setURL( soundURL );
                break;
            }
            ++it;
        }
    }

}

void KMSoundTestWidget::playSound()
{
    TQString parameter= m_urlRequester->lineEdit()->text();
    if ( parameter.isEmpty() )
        return ;
    TQString play = parameter;
    TQString file = TQString::fromLatin1("file:");
    if (parameter.startsWith(file))
        play = parameter.mid(file.length());
    KAudioPlayer::play(TQFile::encodeName(play));
}


TQString KMSoundTestWidget::url() const
{
    return m_urlRequester->lineEdit()->text();
}

void KMSoundTestWidget::setUrl(const TQString & url)
{
    m_urlRequester->lineEdit()->setText(url);
}

void KMSoundTestWidget::clear()
{
    m_urlRequester->lineEdit()->clear();
}

//--------------------------------------------
#include "kmfawidgets.moc"
