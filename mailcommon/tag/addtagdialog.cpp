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

#include <AkonadiCore/tagcreatejob.h>

#include <KLocalizedString>
#include <QLineEdit>
#include <KMessageBox>
#include <KJob>
#include <QDebug>

#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace MailCommon;

AddTagDialog::AddTagDialog(const QList<KActionCollection *>& actions, QWidget *parent)
    : QDialog(parent)
{
    setModal( true );
    setWindowTitle( i18n( "Add Tag" ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mOkButton->setDefault(true);

    connect(mTagWidget->tagNameLineEdit(), SIGNAL(textChanged(QString)), SLOT(slotTagNameChanged(QString)));
    mOkButton->setEnabled(false);
    mainLayout->addWidget(mTagWidget);
    mainLayout->addWidget(buttonBox);
}

AddTagDialog::~AddTagDialog()
{
}

void AddTagDialog::setTags(const QList<MailCommon::Tag::Ptr> &tags)
{
    mTags = tags;
}

void AddTagDialog::slotTagNameChanged(const QString &text)
{
    mOkButton->setEnabled(!text.trimmed().isEmpty());
}

void AddTagDialog::slotSave()
{
    const QString name(mTagWidget->tagNameLineEdit()->text());

    Q_FOREACH ( const MailCommon::Tag::Ptr &tag, mTags ) {
        if ( tag->name() == name ) {
            KMessageBox::error( this, i18n( "Tag %1 already exists", name ) );
            mTagWidget->tagNameLineEdit()->setFocus();
            mTagWidget->tagNameLineEdit()->selectAll();
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
}

void AddTagDialog::onTagCreated(KJob *job)
{
    if (job->error()) {
        qWarning() << "Failed to create tag: " << job->errorString();
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



