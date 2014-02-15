/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "addtagdialog.h"
#include "mailcommon/tag/tagwidget.h"

#include <akonadi/tagcreatejob.h>

#include <KLocale>
#include <KLineEdit>
#include <KMessageBox>
#include <KJob>

#include <QVBoxLayout>

using namespace MailCommon;

AddTagDialog::AddTagDialog(const QList<KActionCollection *>& actions, QWidget *parent)
  : KDialog(parent)
{
  setModal( true );
  setCaption( i18n( "Add Tag" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
  mTagWidget = new MailCommon::TagWidget(actions,this);
  lay->addWidget(mTagWidget);
  connect(mTagWidget->tagNameLineEdit(), SIGNAL(textChanged(QString)), SLOT(slotTagNameChanged(QString)));
  enableButtonOk(false);
}

AddTagDialog::~AddTagDialog()
{
}

void AddTagDialog::setTags(const QList<MailCommon::Tag::Ptr>& tags)
{
    mTags = tags;
}

void AddTagDialog::slotTagNameChanged(const QString& text)
{
  enableButtonOk(!text.isEmpty());
}

void AddTagDialog::slotButtonClicked(int button)
{
  if (button == KDialog::Ok) {
    const QString name(mTagWidget->tagNameLineEdit()->text());

    Q_FOREACH ( const MailCommon::Tag::Ptr &tag, mTags ) {
      if ( tag->name() == name ) {
        KMessageBox::error( this, i18n( "Tag %1 already exists", name ) );
        return;
      }
    }

    MailCommon::Tag::Ptr tag( Tag::createDefaultTag( name ) );
    mTagWidget->recordTagSettings(tag);
    MailCommon::Tag::SaveFlags saveFlags = mTagWidget->saveFlags();
    const Akonadi::Tag akonadiTag = tag->saveToAkonadi( saveFlags );
    Akonadi::TagCreateJob *createJob = new Akonadi::TagCreateJob(akonadiTag, this);
    connect(createJob, SIGNAL(result(KJob*)), this, SLOT(onTagCreated(KJob*)));

    mLabel = name;
  } else {
    reject();
  }
}

void AddTagDialog::onTagCreated(KJob *job)
{
  if (job->error()) {
    kWarning() << "Failed to create tag: " << job->errorString();
    reject();
    return;
  }
  Akonadi::TagCreateJob *createJob = static_cast<Akonadi::TagCreateJob*>(job);
  mTag = createJob->tag();
  accept();
}

QString AddTagDialog::label() const
{
  return mLabel;
}

Akonadi::Tag AddTagDialog::tag() const
{
  return mTag;
}



