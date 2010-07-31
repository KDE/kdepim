/*
    This file is part of KMail.

    Copyright (c) 2004 Jakob Schr�er <js@camaya.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xfaceconfigurator.h"

#include <kactivelabel.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kimageio.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kio/netaccess.h>
using namespace KIO;
#include <kxface.h>
using namespace KPIM;
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>
using namespace KABC;

#include <tqbitmap.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqimage.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>
#include <tqwidgetstack.h>


// #include <assert.h>

using namespace KMail;
using namespace KPIM;

namespace KMail {

  XFaceConfigurator::XFaceConfigurator( TQWidget * parent, const char * name )
    : TQWidget( parent, name )
  {
    // tmp. vars:
    TQLabel * label;
    TQLabel * label1;
    KActiveLabel * label2;
    TQWidget * page;
    TQVBoxLayout * vlay;
    TQHBoxLayout * hlay;
    TQVBoxLayout * page_vlay;
    TQPushButton * mFromFileBtn;
    TQPushButton * mFromAddrbkBtn;

    vlay = new TQVBoxLayout( this, 0, KDialog::spacingHint(), "main layout" );
    hlay = new TQHBoxLayout( vlay );

    // "enable X-Face" checkbox:
    mEnableCheck = new TQCheckBox( i18n("&Send picture with every message"), this );
    TQWhatsThis::add( mEnableCheck,
        i18n( "Check this box if you want KMail to add a so-called X-Face header to messages "
            "written with this identity. An X-Face is a small (48x48 pixels) black and "
            "white image that some mail clients are able to display." ) );
    hlay->addWidget( mEnableCheck, Qt::AlignLeft | Qt::AlignVCenter );

    mXFaceLabel = new TQLabel( this );
    TQWhatsThis::add(mXFaceLabel,
                    i18n( "This is a preview of the picture selected/entered below." ) );
    mXFaceLabel->setFixedSize(48, 48);
    mXFaceLabel->setFrameShape( TQFrame::Box );
    hlay->addWidget( mXFaceLabel );

//     label1 = new TQLabel( "X-Face:", this );
//     vlay->addWidget( label1 );

    // "obtain X-Face from" combo and label:
    hlay = new TQHBoxLayout( vlay ); // inherits spacing
    mSourceCombo = new TQComboBox( false, this );
    TQWhatsThis::add(mSourceCombo,
                    i18n("Click on the widgets below to obtain help on the input methods."));
    mSourceCombo->setEnabled( false ); // since !mEnableCheck->isChecked()
    mSourceCombo->insertStringList( TQStringList()
        << i18n( "continuation of \"obtain picture from\"",
                 "External Source" )
        << i18n( "continuation of \"obtain picture from\"",
                 "Input Field Below" ) );
    label = new TQLabel( mSourceCombo,
                        i18n("Obtain pic&ture from:"), this );
    label->setEnabled( false ); // since !mEnableCheck->isChecked()
    hlay->addWidget( label );
    hlay->addWidget( mSourceCombo, 1 );

    // widget stack that is controlled by the source combo:
    TQWidgetStack * widgetStack = new TQWidgetStack( this );
    widgetStack->setEnabled( false ); // since !mEnableCheck->isChecked()
    vlay->addWidget( widgetStack, 1 );
    connect( mSourceCombo, TQT_SIGNAL(highlighted(int)),
             widgetStack, TQT_SLOT(raiseWidget(int)) );
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
    // page 0: create X-Face from image file or address book entry
    page = new TQWidget( widgetStack );
    widgetStack->addWidget( page, pageno ); // force sequential numbers (play safe)
    page_vlay = new TQVBoxLayout( page, 0, KDialog::spacingHint() );
    hlay = new TQHBoxLayout( page_vlay ); // inherits spacing
    mFromFileBtn = new TQPushButton( i18n("Select File..."), page );
    TQWhatsThis::add( mFromFileBtn,
                     i18n("Use this to select an image file to create the picture from. "
                         "The image should be of high contrast and nearly quadratic shape. "
                         "A light background helps improve the result." ) );
    mFromFileBtn->setAutoDefault( false );
    page_vlay->addWidget( mFromFileBtn, 1 );
    connect( mFromFileBtn, TQT_SIGNAL(released()),
             this, TQT_SLOT(slotSelectFile()) );
    mFromAddrbkBtn = new TQPushButton( i18n("Set From Address Book"), page );
    TQWhatsThis::add( mFromAddrbkBtn,
                     i18n( "You can use a scaled-down version of the picture "
                         "you have set in your address book entry." ) );
    mFromAddrbkBtn->setAutoDefault( false );
    page_vlay->addWidget( mFromAddrbkBtn, 1 );
    connect( mFromAddrbkBtn, TQT_SIGNAL(released()),
             this, TQT_SLOT(slotSelectFromAddressbook()) );
    label1 = new TQLabel( i18n("<qt>KMail can send a small (48x48 pixels), low-quality, "
        "monochrome picture with every message. "
        "For example, this could be a picture of you or a glyph. "
        "It is shown in the recipient's mail client (if supported)." ), page );
    label1->setAlignment( TQLabel::WordBreak | TQLabel::AlignVCenter );
    page_vlay->addWidget( label1 );

    widgetStack->raiseWidget( 0 ); // since mSourceCombo->currentItem() == 0

    // page 1: input field for direct entering
    ++pageno;
    page = new TQWidget( widgetStack );
    widgetStack->addWidget( page, pageno );
    page_vlay = new TQVBoxLayout( page, 0, KDialog::spacingHint() );
    mTextEdit = new TQTextEdit( page );
    page_vlay->addWidget( mTextEdit );
    TQWhatsThis::add( mTextEdit, i18n( "Use this field to enter an arbitrary X-Face string." ) );
    mTextEdit->setFont( KGlobalSettings::fixedFont() );
    mTextEdit->setWrapPolicy( TQTextEdit::Anywhere );
    mTextEdit->setTextFormat( Qt::PlainText );
    label2 = new KActiveLabel( i18n("Examples are available at <a href=\"http://www.xs4all.nl/~ace/X-Faces/\">http://www.xs4all.nl/~ace/X-Faces/</a>."), page );
    page_vlay->addWidget( label2 );


    connect(mTextEdit, TQT_SIGNAL(textChanged()), this, TQT_SLOT(slotUpdateXFace()));
  }

  XFaceConfigurator::~XFaceConfigurator() {

  }

  bool XFaceConfigurator::isXFaceEnabled() const {
    return mEnableCheck->isChecked();
  }

  void XFaceConfigurator::setXFaceEnabled( bool enable ) {
    mEnableCheck->setChecked( enable );
  }

  TQString XFaceConfigurator::xface() const {
    return mTextEdit->text();
  }

  void XFaceConfigurator::setXFace( const TQString & text ) {
    mTextEdit->setText( text );
  }

  void XFaceConfigurator::setXfaceFromFile( const KURL &url )
  {
    TQString tmpFile;
    if( KIO::NetAccess::download( url, tmpFile, this ) )
    {
      KXFace xf;
      mTextEdit->setText( xf.fromImage( TQImage( tmpFile ) ) );
      KIO::NetAccess::removeTempFile( tmpFile );
    } else {
      KMessageBox::error(this, KIO::NetAccess::lastErrorString() );
    }
  }

  void XFaceConfigurator::slotSelectFile()
  {
    TQStringList mimeTypes = KImageIO::mimeTypes (KImageIO::Reading);
    TQString filter = mimeTypes.join (" ");
    KURL url = KFileDialog::getOpenURL( TQString::null, filter, this, TQString::null );
    if ( !url.isEmpty() )
      setXfaceFromFile( url );
  }

  void XFaceConfigurator::slotSelectFromAddressbook()
  {
    StdAddressBook *ab = StdAddressBook::self( true );
    Addressee me = ab->whoAmI();
    if ( !me.isEmpty() )
    {
      if ( me.photo().isIntern() )
      {
        TQImage photo = me.photo().data();
        if ( !photo.isNull() )
        {
          KXFace xf;
          mTextEdit->setText( xf.fromImage( photo ) );
        }
        else
          KMessageBox::information( this, i18n("No picture set for your address book entry."), i18n("No Picture") );

      }
      else
      {
        KURL url = me.photo().url();
        if( !url.isEmpty() )
          setXfaceFromFile( url );
        else
          KMessageBox::information( this, i18n("No picture set for your address book entry."), i18n("No Picture") );
      }
    }
    else
      KMessageBox::information( this, i18n("You do not have your own contact defined in the address book."), i18n("No Picture") );
  }

  void XFaceConfigurator::slotUpdateXFace()
  {
    TQString str = mTextEdit->text();
    if ( !str.isEmpty() )
    {
      if ( str.startsWith("x-face:", false) )
      {
        str = str.remove("x-face:", false);
        mTextEdit->setText(str);
      }
      KXFace xf;
      TQPixmap p( 48, 48, true );
      p.convertFromImage( xf.toImage(str) );
      mXFaceLabel->setPixmap( p );
    }
    else
      mXFaceLabel->setPixmap( 0L );
  }

} // namespace KMail

#include "xfaceconfigurator.moc"
