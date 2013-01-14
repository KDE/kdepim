/* Copyright (C) 2011, 2012, 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sieveeditor.h"
#include "sievefindbar.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <KTextEdit>
#include <errno.h>

#include <QSplitter>
#include <QTextStream>
#include <QPointer>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QShortcut>

using namespace KSieveUi;

SieveEditor::SieveEditor( QWidget * parent )
  : KDialog( parent )
{
  setCaption( i18n( "Edit Sieve Script" ) );
  setButtons( Ok|Cancel|User1|User2|User3 );
  setButtonText( User1, i18n( "Check Syntax" ) );
  setButtonGuiItem( User2, KStandardGuiItem::saveAs() );
  setButtonText( User3, i18n( "Import..." ) );
  setDefaultButton( Ok );
  setModal( true );

  QWidget *mainWidget = new QWidget;
  QVBoxLayout *lay = new QVBoxLayout;
  mainWidget->setLayout( lay );
  QHBoxLayout *nameLayout = new QHBoxLayout;
  QLabel * label = new QLabel( i18n( "Script name:" ) );
  nameLayout->addWidget( label );
  mScriptName = new QLineEdit;
  mScriptName->setReadOnly( true );
  nameLayout->addWidget( mScriptName );
  lay->addLayout( nameLayout );
  
  QSplitter *splitter = new QSplitter;
  splitter->setOrientation( Qt::Vertical );
  lay->addWidget( splitter );
  QList<int> size;
  size << 400 << 100;

  mTextEdit = new SieveTextEdit( splitter );
  mFindBar = new SieveFindBar( mTextEdit, splitter );

  QWidget *widget = new QWidget( splitter );
  QVBoxLayout *layTextEdit = new QVBoxLayout;
  layTextEdit->addWidget( mTextEdit );
  layTextEdit->addWidget( mFindBar );
  widget->setLayout( layTextEdit );

  QShortcut *shortcut = new QShortcut( this );
  shortcut->setKey( Qt::Key_F+Qt::CTRL );
  connect( shortcut, SIGNAL(activated()), SLOT(slotFind()) );
  connect( mTextEdit, SIGNAL(findText()), SLOT(slotFind()) );
   
  mDebugTextEdit = new KTextEdit;
  mDebugTextEdit->setReadOnly( true );
  splitter->addWidget( widget );
  splitter->addWidget( mDebugTextEdit );
  splitter->setSizes( size );
  connect( mTextEdit, SIGNAL(textChanged()), SLOT(slotTextChanged()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(slotSaveAs()) );
  connect( this, SIGNAL(user3Clicked()), SLOT(slotImport()) );

  setMainWidget( mainWidget );
  resize( 800,600);
  mTextEdit->setFocus();
}

SieveEditor::~SieveEditor()
{
}

void SieveEditor::slotFind()
{
  if ( mTextEdit->textCursor().hasSelection() )
    mFindBar->setText( mTextEdit->textCursor().selectedText() );
  mTextEdit->moveCursor(QTextCursor::Start);
  mFindBar->show();
  mFindBar->focusAndSetCursor();  
}

void SieveEditor::slotSaveAs()
{
  KUrl url;
  const QString filter = i18n( "*.siv|sieve files (*.siv)\n*|all files (*)" );
  QPointer<KFileDialog> fdlg( new KFileDialog( url, filter, this) );

  fdlg->setMode( KFile::File );
  fdlg->setOperationMode( KFileDialog::Saving );
  if ( fdlg->exec() == QDialog::Accepted && fdlg )
  {
    const QString fileName = fdlg->selectedFile();
    if ( !saveToFile( fileName ) )
    {
      KMessageBox::error( this,
                          i18n( "Could not write the file %1:\n"
                                "\"%2\" is the detailed error description.",
                                fileName,
                                QString::fromLocal8Bit( strerror( errno ) ) ),
                          i18n( "Sieve Editor Error" ) );
    }
  }
  delete fdlg;
 
}

bool SieveEditor::saveToFile( const QString&filename )
{
  QFile file( filename );
  if ( !file.open( QIODevice::WriteOnly|QIODevice::Text ) )
    return false;
  QTextStream out(&file);
  out << mTextEdit->toPlainText();
  return true;
}

void SieveEditor::slotImport()
{
  if ( !mTextEdit->toPlainText().isEmpty() )
  {
    if ( KMessageBox::warningYesNo(this, i18n( "You will overwrite script. Do you want to continue?" ), i18n( "Import Script" ) ) == KMessageBox::No )
      return;
  }
  KUrl url;
  const QString filter = i18n( "*.siv|sieve files (*.siv)\n*|all files (*)" );
  QPointer<KFileDialog> fdlg( new KFileDialog( url, filter, this) );

  fdlg->setMode( KFile::File );
  fdlg->setOperationMode( KFileDialog::Opening );
  if ( fdlg->exec() == QDialog::Accepted && fdlg )
  {
    const QString fileName = fdlg->selectedFile();
    if ( !loadFromFile( fileName ) )
    {
      KMessageBox::error( this,
                          i18n( "Could not load the file %1:\n"
                                "\"%2\" is the detailed error description.",
                                fileName,
                                QString::fromLocal8Bit( strerror( errno ) ) ),
                          i18n( "Sieve Editor Error" ) );
    }
  }
  delete fdlg;
}

bool SieveEditor::loadFromFile( const QString& filename )
{
  QFile file( filename );
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QTextStream in(&file);
  QString line = in.readLine();
  QString scriptText;
  while (!line.isNull()) {
    if ( scriptText.isEmpty() )
      scriptText = line;
    else
      scriptText += QLatin1String( "\n" ) + line;
    line = in.readLine();
  }
  mTextEdit->setPlainText( scriptText );
  return true;
}

void SieveEditor::slotTextChanged()
{
  const bool enabled = !script().isEmpty();
  enableButton( User1, enabled );
  enableButtonOk( enabled );
}

QString SieveEditor::script() const
{
  return mTextEdit->toPlainText();
}

void SieveEditor::setScript( const QString & script )
{
  mTextEdit->setPlainText( script );
}

void SieveEditor::setDebugColor( const QColor& col )
{
  mDebugTextEdit->setTextColor( col );
}

void SieveEditor::setDebugScript( const QString& debug )
{
  mDebugTextEdit->setText( debug );
}

void SieveEditor::setScriptName( const QString&name )
{
  mScriptName->setText( name );
}  

#include "sieveeditor.moc"

