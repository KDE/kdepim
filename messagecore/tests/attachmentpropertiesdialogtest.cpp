/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "attachmentpropertiesdialogtest.h"
#include "qtest_messagecore.h"

#include <boost/shared_ptr.hpp>

#include <QCheckBox>

#include <QDebug>
#include <KComboBox>
#include <QLineEdit>
#include <qtest.h>

#include <kmime/kmime_content.h>
using namespace KMime;

#include <messagecore/attachment/attachmentpart.h>
#include <messagecore/attachment/attachmentpropertiesdialog.h>
using namespace MessageCore;

QTEST_MAIN( AttachmentPropertiesDialogTest )

void AttachmentPropertiesDialogTest::testAttachmentPartReadWrite()
{
  // Sample data.
  const QString name = QString::fromLatin1( "old name" );
  const QString newName = QString::fromLatin1( "new name" );
  const QString description = QString::fromLatin1( "old description" );
  const QString newDescription = QString::fromLatin1( "new description" );
  const QByteArray data( "12345" );
  const QByteArray mimeType( "text/plain" );
  const QByteArray newMimeType( "x-weird/x-type" );
  const Headers::contentEncoding encoding = Headers::CEquPr;
  const Headers::contentEncoding newEncoding = Headers::CE7Bit;
  const bool autoDisplay = true;
  const bool encrypt = true;
  const bool sign = true;

  // Create the part.
  AttachmentPart::Ptr part = AttachmentPart::Ptr( new AttachmentPart );
  part->setName( name );
  part->setDescription( description );
  part->setData( data );
  part->setMimeType( mimeType );
  part->setEncoding( encoding );
  part->setInline( autoDisplay );
  part->setEncrypted( encrypt );
  part->setSigned( sign );

  // Show the dialog and verify that it is accurate.
  AttachmentPropertiesDialog *dialog = new AttachmentPropertiesDialog( part );
  dialog->show();
  QLineEdit *nameEdit = dialog->findChild<QLineEdit*>( QLatin1String( "name" ) );
  Q_ASSERT( nameEdit );
  QCOMPARE( nameEdit->text(), name );
  QLineEdit *descriptionEdit = dialog->findChild<QLineEdit*>( QLatin1String( "description" ) );
  Q_ASSERT( descriptionEdit );
  QCOMPARE( descriptionEdit->text(), description );
  KComboBox *mimeTypeCombo = dialog->findChild<KComboBox*>( QLatin1String( "mimeType" ) );
  Q_ASSERT( mimeTypeCombo );
  QCOMPARE( mimeTypeCombo->currentText().toLatin1(), mimeType );
  KComboBox *encodingCombo = dialog->findChild<KComboBox*>( QLatin1String( "encoding" ) );
  Q_ASSERT( encodingCombo );
  QCOMPARE( encodingCombo->currentIndex(), int( encoding ) );
  QCheckBox *autoDisplayCheck = dialog->findChild<QCheckBox*>( QLatin1String( "autoDisplay" ) );
  Q_ASSERT( autoDisplayCheck );
  QCOMPARE( autoDisplayCheck->isChecked(), autoDisplay );
  QCheckBox *encryptCheck = dialog->findChild<QCheckBox*>( QLatin1String( "encrypt" ) );
  Q_ASSERT( encryptCheck );
  QCOMPARE( encryptCheck->isChecked(), encrypt );
  QCheckBox *signCheck = dialog->findChild<QCheckBox*>( QLatin1String( "sign" ) );
  Q_ASSERT( signCheck );
  QCOMPARE( signCheck->isChecked(), sign );
  //QTest::qWait( 5000 );

  // Make some changes in the dialog.
  nameEdit->setText( newName );
  descriptionEdit->setText( newDescription );
  mimeTypeCombo->setCurrentItem( QString::fromLatin1(newMimeType), true );
  encodingCombo->setCurrentIndex( int( newEncoding ) );
  autoDisplayCheck->setChecked( !autoDisplay );
  encryptCheck->setChecked( !encrypt );
  signCheck->setChecked( !sign );

  // Click on 'OK' and verify changes.
  dialog->accept();
  delete dialog;
  QCOMPARE( part->name(), newName );
  QCOMPARE( part->description(), newDescription );
  QCOMPARE( part->data(), data ); // Unchanged.
  QCOMPARE( part->mimeType(), newMimeType );
  QCOMPARE( int( part->encoding() ), int( newEncoding ) );
  QCOMPARE( part->isInline(), !autoDisplay );
  QCOMPARE( part->isEncrypted(), !encrypt );
  QCOMPARE( part->isSigned(), !sign );
}

void AttachmentPropertiesDialogTest::testAttachmentPartReadOnly()
{
  // Sample data.
  const QString name = QString::fromLatin1( "old name" );
  const QString newName = QString::fromLatin1( "new name" );

  // Create the part.
  AttachmentPart::Ptr part = AttachmentPart::Ptr( new AttachmentPart );
  part->setName( name );

  // Show the (read-only) dialog and do some changes.
  AttachmentPropertiesDialog *dialog = new AttachmentPropertiesDialog( part, true, 0 );
  dialog->show();
  // Click on 'OK'.  No changes should have been made.
  dialog->accept();
  delete dialog;
  QCOMPARE( part->name(), name ); // No change.
}

void AttachmentPropertiesDialogTest::testAttachmentPartCancel()
{
  // Sample data.
  const QString name = QString::fromLatin1( "old name" );
  const QString newName = QString::fromLatin1( "new name" );

  // Create the part.
  AttachmentPart::Ptr part = AttachmentPart::Ptr( new AttachmentPart );
  part->setName( name );

  // Show the (read-write) dialog and do some changes.
  AttachmentPropertiesDialog *dialog = new AttachmentPropertiesDialog( part );
  dialog->show();
  QLineEdit *nameEdit = dialog->findChild<QLineEdit*>( QLatin1String( "name" ) );
  Q_ASSERT( nameEdit );
  nameEdit->setText( newName );

  // Click on 'Cancel'.  No changes should have been made.
  dialog->reject();
  delete dialog;
  QCOMPARE( part->name(), name ); // No change.
}

void AttachmentPropertiesDialogTest::testMimeContentReadOnly()
{
  // Sample data.
  const QString name = QString::fromLatin1( "old name" );
  const QString newName = QString::fromLatin1( "new name" );
  const QByteArray charset( "us-ascii" );

  // Create the MIME Content.
  Content *content = new Content;
  content->contentType()->setName( name, charset );
  const Content *constContent = content;

  // Show the (read-only) dialog and do some changes.
  AttachmentPropertiesDialog *dialog = new AttachmentPropertiesDialog( constContent );
  dialog->show();

  // Click on 'OK'.  The MIME Content should be untouched.
  dialog->accept();
  delete dialog;
  QCOMPARE( content->contentType()->name(), name ); // No change.
}

