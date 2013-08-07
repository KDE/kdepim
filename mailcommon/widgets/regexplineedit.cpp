/*  -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2004 Ingo Kloecker <kloecker@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "regexplineedit.h"

#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <kregexpeditorinterface.h> //krazy:exclude=camelcase TODO wait for kdelibs4.10
#include <KServiceTypeTrader>

#include <QHBoxLayout>
#include <QString>
#include <QPushButton>

namespace MailCommon {

RegExpLineEdit::RegExpLineEdit( QWidget *parent )
  : QWidget( parent ),
    mLineEdit( 0 ),
    mRegExpEditButton( 0 ),
    mRegExpEditDialog( 0 )
{
  initWidget();
}

RegExpLineEdit::RegExpLineEdit( const QString &str, QWidget *parent )
  : QWidget( parent ),
    mLineEdit( 0 ),
    mRegExpEditButton( 0 ),
    mRegExpEditDialog( 0 )
{
  initWidget( str );
}

void RegExpLineEdit::initWidget( const QString &str )
{
  QHBoxLayout * hlay = new QHBoxLayout( this );
  hlay->setSpacing( KDialog::spacingHint() );
  hlay->setMargin( 0 );

  mLineEdit = new KLineEdit( str, this );
  mLineEdit->setClearButtonShown( true );
  mLineEdit->setTrapReturnKey(true);
  setFocusProxy( mLineEdit );
  hlay->addWidget( mLineEdit );

  connect( mLineEdit, SIGNAL(textChanged(QString)),
           this, SIGNAL(textChanged(QString)) );
  connect( mLineEdit, SIGNAL(returnPressed()),
           this, SIGNAL(returnPressed()) );

  if ( !KServiceTypeTrader::self()->query( QLatin1String("KRegExpEditor/KRegExpEditor") ).isEmpty() ) {
    mRegExpEditButton = new QPushButton( i18n( "Edit..." ), this );
    mRegExpEditButton->setObjectName( QLatin1String("mRegExpEditButton") );
    mRegExpEditButton->setSizePolicy( QSizePolicy::Minimum,
                                      QSizePolicy::Fixed );
    hlay->addWidget( mRegExpEditButton );

    connect( mRegExpEditButton, SIGNAL(clicked()),
             this, SLOT(slotEditRegExp()) );
  }
}

void RegExpLineEdit::clear()
{
  mLineEdit->clear();
}

QString RegExpLineEdit::text() const
{
  return mLineEdit->text();
}

void RegExpLineEdit::setText( const QString & str )
{
  mLineEdit->setText( str );
}

void RegExpLineEdit::showEditButton( bool show )
{
  if ( !mRegExpEditButton ) {
    return;
  }

  if ( show ) {
    mRegExpEditButton->show();
  } else {
    mRegExpEditButton->hide();
  }
}

void RegExpLineEdit::slotEditRegExp()
{
  if ( !mRegExpEditDialog ) {
    mRegExpEditDialog =
      KServiceTypeTrader::createInstanceFromQuery<KDialog>(
        QLatin1String("KRegExpEditor/KRegExpEditor"), QString(), this );
  }

  KRegExpEditorInterface *iface = qobject_cast<KRegExpEditorInterface *>( mRegExpEditDialog );

  if ( iface ) {
    iface->setRegExp( mLineEdit->text() );
    if ( mRegExpEditDialog->exec() == KDialog::Accepted ) {
      mLineEdit->setText( iface->regExp() );
    }
  }
}

} // namespace MailCommon

#include "regexplineedit.moc"
