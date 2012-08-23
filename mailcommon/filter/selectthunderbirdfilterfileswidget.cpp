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

#include "selectthunderbirdfilterfileswidget.h"
#include "ui_selectthunderbirdfilterfileswidget.h"
#include "mailimporter/filter_thunderbird.h"
#include <QAbstractButton>
#include <QListWidgetItem>
#include <KUrlRequester>

SelectThunderbirdFilterFilesWidget::SelectThunderbirdFilterFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectThunderbirdFilterFilesWidget)
{
  ui->setupUi(this);
  connect(ui->buttonGroup,SIGNAL(buttonClicked (QAbstractButton*)),SLOT(slotButtonClicked(QAbstractButton*)));
  connect(ui->profiles,SIGNAL(currentIndexChanged(int)),SLOT(slotProfileChanged(int)));

  ui->listFiles->setSelectionMode(QAbstractItemView::MultiSelection);
  QString defaultProfile;
  QMap<QString,QString> listProfile = MailImporter::FilterThunderbird::listProfile(defaultProfile);
  QMap<QString, QString>::const_iterator i = listProfile.constBegin();
  while (i != listProfile.constEnd()) {
    QString name = i.key();
    if(i.value()==defaultProfile){
      name+=i18n(" (default)");
    }
    ui->profiles->addItem(name,i.value());
    ++i;
  }
  ui->fileUrl->setEnabled(true);
  ui->profiles->setEnabled(false);
  ui->listFiles->setEnabled(false);
  slotProfileChanged(0);
}

SelectThunderbirdFilterFilesWidget::~SelectThunderbirdFilterFilesWidget()
{
  delete ui;
}

void SelectThunderbirdFilterFilesWidget::slotButtonClicked(QAbstractButton*button)
{
  if(button == ui->selectFile) {
    ui->fileUrl->setEnabled(true);
    ui->profiles->setEnabled(false);
    ui->listFiles->setEnabled(false);
  } else {
    ui->fileUrl->setEnabled(false);
    ui->profiles->setEnabled(true);
    ui->listFiles->setEnabled(true);
  }
}

void SelectThunderbirdFilterFilesWidget::slotProfileChanged(int index)
{
  if(index >= ui->profiles->count()) {
    return;
  }

  QStringList listFilterFiles;
  const QString path(MailImporter::FilterThunderbird::defaultPath() + ui->profiles->itemData(index).toString());
  QDir dir(path);
  const QStringList subDir = dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot,QDir::Name);
  Q_FOREACH( const QString& mailPath, subDir ) {
    const QString subMailPath(path + QLatin1Char('/') + mailPath);
    QDir dirMail(subMailPath);
    const QStringList subDirMail = dirMail.entryList(QDir::AllDirs|QDir::NoDotAndDotDot,QDir::Name);
    Q_FOREACH( const QString& file, subDirMail ) {
      const QString filterFile(subMailPath +QLatin1Char('/')+ file + QLatin1String("/msgFilterRules.dat"));
      if(QFile(filterFile).exists()) {
          listFilterFiles<<filterFile;
      }
    }
  }
  ui->listFiles->clear();
  ui->listFiles->addItems(listFilterFiles);
}

QStringList SelectThunderbirdFilterFilesWidget::selectedFiles() const
{
  QStringList listFiles;
  if(ui->selectFile->isChecked()) {
    listFiles<<ui->fileUrl->url().path();
  } else {
     QList<QListWidgetItem *> list = ui->listFiles->selectedItems();
     Q_FOREACH(QListWidgetItem * item, list) {
         listFiles<<item->text();
     }
  }
  return listFiles;
}

#include "selectthunderbirdfilterfileswidget.moc"
