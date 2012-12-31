/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "selectthunderbirdfilterfilesdialog.h"
#include "selectthunderbirdfilterfileswidget.h"
#include "mailkernel.h"
#include <KLocale>

#include <QHBoxLayout>

using namespace MailCommon;

SelectThunderbirdFilterFilesDialog::SelectThunderbirdFilterFilesDialog(QWidget *parent)
    :KDialog(parent)
{
  setCaption( i18n( "Select thunderbird filter files" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setModal( true );
  QWidget *mainWidget = new QWidget( this );
  QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );
  mainLayout->setSpacing( KDialog::spacingHint() );
  mainLayout->setMargin( KDialog::marginHint() );
  setMainWidget( mainWidget );

  mSelectFilterFilesWidget = new SelectThunderbirdFilterFilesWidget(mainWidget);
  mainLayout->addWidget(mSelectFilterFilesWidget);
  readConfig();
}

SelectThunderbirdFilterFilesDialog::~SelectThunderbirdFilterFilesDialog()
{
  writeConfig();
}

QStringList SelectThunderbirdFilterFilesDialog::selectedFiles() const
{
  return mSelectFilterFilesWidget->selectedFiles();
}

void SelectThunderbirdFilterFilesDialog::setStartDir(const KUrl& url)
{
  mSelectFilterFilesWidget->setStartDir(url);
}

void SelectThunderbirdFilterFilesDialog::readConfig()
{
  KConfigGroup group( KernelIf->config(), "SelectThunderbirdFilterFilesDialog" );

  const QSize size = group.readEntry( "Size", QSize() );
  if ( size.isValid() ) {
    resize( size );
  } else {
    resize( 500, 300 );
  }
}

void SelectThunderbirdFilterFilesDialog::writeConfig()
{
  KConfigGroup group( KernelIf->config(), "SelectThunderbirdFilterFilesDialog" );
  group.writeEntry( "Size", size() );
}
