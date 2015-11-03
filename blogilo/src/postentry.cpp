/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>
    Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "postentry.h"
#include "bilbomedia.h"
#include "backend.h"
#include "dbman.h"
#include "global.h"
#include "sendtoblogdialog.h"
#include "settings.h"
#include "bilboblog.h"
#include "syncuploader.h"

#include "composer/blogilocomposereditor.h"
#include "composer/blogilocomposerview.h"
#include "composer/bilbobrowser.h"
#include "composer/htmleditor.h"
#include "composer/blogilocomposerwidget.h"

#include <PimCommon/SpellCheckLineEdit>

#include "blogilo_debug.h"
#include <klocalizedstring.h>
#include <klineedit.h>
#include <KMessageBox>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kio/job.h>
#include <QTabWidget>

#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <QLayout>
#include <QHBoxLayout>

#define MINUTE 60000

class PostEntry::Private
{
public:
    QPointer<QProgressBar> progress;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *labelTitle;
    PimCommon::SpellCheckLineEdit *txtTitle;
    QTimer *mTimer;
    BilboPost mCurrentPost;
    int mCurrentPostBlogId;
    QMap <QString, BilboMedia *> mMediaList;

    int mNumOfFilesToBeUploaded;
    bool isUploadingMediaFilesFailed;
    bool isNewPost;
    bool isPostContentModified;

    QTabWidget *tabWidget;
    QWidget *tabVisual;
    QWidget *tabHtml;
    QWidget *tabPreview;

    BlogiloComposerWidget *wysiwygEditor;
    KTextEditor::View *htmlEditor;
    BilboBrowser *previewer;

    int prev_index;
};

PostEntry::PostEntry(QWidget *parent)
    : QFrame(parent), d(new Private)
{
    createUi();
    connect(d->wysiwygEditor->editor(), &ComposerEditorNG::ComposerEditor::textChanged, this, &PostEntry::textChanged);
    connect(d->htmlEditor->document(), &KTextEditor::Document::textChanged,
            this, &PostEntry::textChanged);
    layout()->addWidget(d->tabWidget);
    d->mTimer = new QTimer(this);
    d->mTimer->start(Settings::autosaveInterval() * MINUTE);
    connect(d->mTimer, &QTimer::timeout, this, &PostEntry::saveTemporary);
    d->progress = Q_NULLPTR;
    d->mCurrentPostBlogId = -1;
    d->mNumOfFilesToBeUploaded = 0;
    d->isPostContentModified = false;
    connect(this, &PostEntry::textChanged, this, &PostEntry::slotPostModified);
}

PostEntry::~PostEntry()
{
    delete d;
}

void PostEntry::settingsChanged()
{
    qCDebug(BLOGILO_LOG);
    d->mTimer->setInterval(Settings::autosaveInterval() * MINUTE);
    if (Settings::autosaveInterval()) {
        d->mTimer->start();
    } else {
        d->mTimer->stop();
    }
}

void PostEntry::createUi()
{
    d->tabWidget = new QTabWidget(this);
    d->tabVisual = new QWidget(d->tabWidget);
    d->tabHtml = new QWidget(d->tabWidget);
    d->tabPreview = new QWidget(d->tabWidget);
    d->tabWidget->addTab(d->tabVisual, i18nc("Software", "Visual Editor"));
    d->tabWidget->addTab(d->tabHtml, i18nc("Software", "HTML Editor"));
    d->tabWidget->addTab(d->tabPreview, i18nc("preview of the edited post", "Post Preview"));
    connect(d->tabWidget, &QTabWidget::currentChanged, this, &PostEntry::slotSyncEditors);
    d->prev_index = 0;

    /// WYSIWYG Editor:
    BlogiloComposerView *view = new BlogiloComposerView(this);
    d->wysiwygEditor = new BlogiloComposerWidget(view/*,d->tabVisual*/);
    QVBoxLayout *vLayout = new QVBoxLayout(d->tabVisual);
    vLayout->addWidget(d->wysiwygEditor);

    ///htmlEditor:
    d->htmlEditor = HtmlEditor::self()->createView(d->tabHtml);
    QGridLayout *hLayout = new QGridLayout(d->tabHtml);
    hLayout->addWidget(d->htmlEditor);

    ///previewer:
    d->previewer = new BilboBrowser(d->tabPreview);
    QGridLayout *gLayout = new QGridLayout(d->tabPreview);
    gLayout->addWidget(d->previewer);

    connect(d->previewer, &BilboBrowser::sigSetBlogStyle, this, &PostEntry::slotSetPostPreview);

    d->tabWidget->setCurrentIndex(0);

    d->gridLayout = new QGridLayout(this);

    d->horizontalLayout = new QHBoxLayout();
    d->horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);

    d->labelTitle = new QLabel(this);
    d->labelTitle->setText(i18nc("noun, the post title", "Title:"));
    d->horizontalLayout->addWidget(d->labelTitle);

    d->txtTitle = new PimCommon::SpellCheckLineEdit(this, QStringLiteral("blogilorc"));
    d->horizontalLayout->addWidget(d->txtTitle);
    d->labelTitle->setBuddy(d->txtTitle);
    connect(d->txtTitle, &PimCommon::SpellCheckLineEdit::textChanged, this, &PostEntry::slotTitleChanged);
    connect(d->txtTitle, &PimCommon::SpellCheckLineEdit::focusDown, this, &PostEntry::slotFocusEditor);

    d->gridLayout->addLayout(d->horizontalLayout, 0, 0, 1, 1);
}

void PostEntry::slotFocusEditor()
{
    switch (d->tabWidget->currentIndex()) {
    case 0:
        d->wysiwygEditor->editor()->startEditing();
        break;
    case 1:
        d->htmlEditor->setFocus();
        break;
    }
}

void PostEntry::slotSyncEditors(int index)
{
    if (index == 0) {
        if (d->prev_index == 2) {
            d->previewer->stop();
            d->prev_index = index;
            return;
        }//An else clause can do the job of goto, No? -Mehrdad :D
        d->wysiwygEditor->editor()->setHtmlContent(d->htmlEditor->document()->text());
        d->wysiwygEditor->editor()->setFocus();
        d->wysiwygEditor->editor()->startEditing();
    } else if (index == 1) {
        if (d->prev_index == 2) {
            d->previewer->stop();
            d->prev_index = index;
            return;
        }
        d->htmlEditor->document()->setText(d->wysiwygEditor->editor()->htmlContent());
        d->htmlEditor->setFocus();
    } else {
        if (d->prev_index == 1) {
            d->wysiwygEditor->editor()->setHtmlContent(d->htmlEditor->document()->text());
        } else {
            d->htmlEditor->document()->setText(d->wysiwygEditor->editor()->htmlContent());
        }
        d->previewer->setHtml(d->txtTitle->toPlainText(), d->htmlEditor->document()->text());
    }
    d->prev_index = index;
}

void PostEntry::slotSetPostPreview()
{
    if (d->tabWidget->currentIndex() == 2) {
        d->previewer->setHtml(d->txtTitle->toPlainText(), d->htmlEditor->document()->text());
    }
}

QString PostEntry::htmlContent() const
{
    if (d->tabWidget->currentIndex() == 1) {
        d->wysiwygEditor->editor()->setHtmlContent(d->htmlEditor->document()->text());
    } else {
        d->htmlEditor->document()->setText(d->wysiwygEditor->editor()->htmlContent());
    }
    return d->htmlEditor->document()->text();
}

QString PostEntry::plainTextContent() const
{
    return d->wysiwygEditor->editor()->plainTextContent();
}

void PostEntry::setHtmlContent(const QString &content)
{
    d->wysiwygEditor->editor()->setHtmlContent(content);
    d->htmlEditor->document()->setText(content);
}

void PostEntry::slotTitleChanged()
{
    const QString titleText(d->txtTitle->toPlainText());
    d->mCurrentPost.setTitle(titleText);
    Q_EMIT postTitleChanged(titleText);
}

QString PostEntry::postTitle() const
{
    return d->mCurrentPost.title();
}

void PostEntry::setPostTitle(const QString &title)
{
    d->txtTitle->setPlainText(title);
    d->mCurrentPost.setTitle(title);
}

void PostEntry::setPostBody(const QString &content, const QString &additionalContent)
{
    QString body;
    if (additionalContent.isEmpty()) {
        body = content;
    } else {
        body = content + QLatin1String("<hr/><!--split-->") + additionalContent;
        d->mCurrentPost.setAdditionalContent(QString());
    }
    //     if (body.isEmpty()){
    //         body = "<p></p>";//This is because of Bug #387578
    //     }
    d->mCurrentPost.setContent(body);
    setHtmlContent(body);
    d->isPostContentModified = false;
    connect(this, &PostEntry::textChanged, this, &PostEntry::slotPostModified);
    //     connect( txtTitle, SIGNAL(textChanged(QString)), this, SLOT(slotPostModified()) );
}

int PostEntry::currentPostBlogId() const
{
    return d->mCurrentPostBlogId;
}

void PostEntry::setCurrentPostBlogId(int blog_id)
{
    d->mCurrentPostBlogId = blog_id;
    if (blog_id != -1 && DBMan::self()->blogList().contains(blog_id)) {
        setDefaultLayoutDirection(DBMan::self()->blogList().value(blog_id)->direction());
    }
}

void PostEntry::setCurrentPostFromEditor()
{
    if (d->isPostContentModified) {
        qCDebug(BLOGILO_LOG);
        const QString &str = htmlContent();
        d->mCurrentPost.setContent(str);
        d->isPostContentModified = false;
        connect(this, &PostEntry::textChanged, this, &PostEntry::slotPostModified);
    }
}

BilboPost *PostEntry::currentPost()
{
    setCurrentPostFromEditor();
    return &d->mCurrentPost;
}

void PostEntry::setCurrentPost(const BilboPost &post)
{
    d->mCurrentPost = post;
    qCDebug(BLOGILO_LOG) << "local_id: " << d->mCurrentPost.localId();
    this->setPostBody(d->mCurrentPost.content(), d->mCurrentPost.additionalContent());
    this->setPostTitle(d->mCurrentPost.title());
}

Qt::LayoutDirection PostEntry::defaultLayoutDirection() const
{
    return d->txtTitle->layoutDirection();
}

void PostEntry::setDefaultLayoutDirection(Qt::LayoutDirection direction)
{
    qCDebug(BLOGILO_LOG);
    d->tabWidget->setLayoutDirection(direction);
    d->txtTitle->setLayoutDirection(direction);
}

QList< BilboMedia * > PostEntry::localImages() const
{
    return d->wysiwygEditor->editor()->getLocalImages();
}

void PostEntry::replaceImageSrc(const QString &src, const QString &dest)
{
    d->wysiwygEditor->editor()->replaceImageSrc(src, dest);
}

bool PostEntry::uploadMediaFiles(Backend *backend)
{
    bool localBackend = false;
    bool result = true;
    if (!backend) {
        localBackend = true;
        backend = new Backend(d->mCurrentPostBlogId, this);
    }
    QList<BilboMedia *> lImages = localImages();
    if (!lImages.isEmpty()) {
        showProgressBar();
        QList<BilboMedia *>::iterator it = lImages.begin();
        QList<BilboMedia *>::iterator endIt = lImages.end();
        for (; it != endIt; ++it) {
            BilboMedia *media = (*it);
            SyncUploader *uploader = new SyncUploader(this);
            if (uploader->uploadMedia(backend, media)) {
                replaceImageSrc(media->localUrl().url(),
                                media->remoteUrl().url());
            } else {
                const QString err = i18n("Uploading the media file %1 failed.\n%2",
                                         media->name(), uploader->errorMessage());
                Q_EMIT postPublishingDone(true, err);
                result = false;
                break;
            }
            uploader->deleteLater();
        }
        d->mCurrentPost.setContent(htmlContent());
    }
    if (localBackend) {
        backend->deleteLater();
    }
    return result;
}

void PostEntry::slotError(const QString &errMsg)
{
    const QString err = i18n("An error occurred in the last transaction.\n%1", errMsg);
    Q_EMIT postPublishingDone(true, err);
    deleteProgressBar();
    sender()->deleteLater();
}

void PostEntry::submitPost(int blogId, const BilboPost &postData)
{
    setCurrentPostFromEditor();
    if (d->mCurrentPost.content().isEmpty() || d->mCurrentPost.title().isEmpty()) {
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("Your post title or body is empty.\nAre you sure you want to submit this post?")
                                              ) == KMessageBox::Cancel) {
            return;
        }
    }
    bool isNew = false;
    if (d->mCurrentPost.status() == BilboPost::New) {
        isNew = true;
    }
    QPointer<SendToBlogDialog> dia = new SendToBlogDialog(isNew, d->mCurrentPost.isPrivate(), this);
    dia->setAttribute(Qt::WA_DeleteOnClose, false);
    if (dia->exec() == QDialog::Accepted) {
        this->setCursor(Qt::BusyCursor);
        d->mCurrentPost.setProperties(postData);
        d->mCurrentPostBlogId = blogId;

        QString msgType;
        if (dia->isPrivate()) {
            msgType =  i18nc("Post status, e.g Draft or Published Post", "draft");
            d->mCurrentPost.setPrivate(true);
        } else {
            msgType =  i18nc("Post status, e.g Draft or Published Post", "post");
            d->mCurrentPost.setPrivate(false);
        }

        QString statusMsg;
        if (dia->isNew()) {
            statusMsg = i18n("Submitting new %1...", msgType);
            d->isNewPost = true;
        } else {
            statusMsg = i18n("Modifying %1...", msgType);
            d->isNewPost = false;
        }

        Q_EMIT showStatusMessage(statusMsg, true);
        Backend *b = new Backend(d->mCurrentPostBlogId, this);
        connect(b, &Backend::sigError, this, &PostEntry::slotError);
        if (uploadMediaFiles(b)) {
            qCDebug(BLOGILO_LOG) << "Uploading";
            showProgressBar();
            connect(b, &Backend::sigPostPublished, this, &PostEntry::slotPostPublished);
            if (d->isNewPost) {
                b->publishPost(&d->mCurrentPost);
            } else {
                b->modifyPost(&d->mCurrentPost);
            }
        } else {
            deleteProgressBar();
        }
    }
    delete dia;
}

void PostEntry::slotPostPublished(int blog_id, BilboPost *post)
{
    qCDebug(BLOGILO_LOG) << "BlogId: " << blog_id << "Post Id on server: " << post->postId();
    DBMan::self()->removeTempEntry(d->mCurrentPost);
    QString msg;
    setCurrentPost(*post);
    if (d->mCurrentPost.isPrivate()) {
        msg = i18n("Draft with title \"%1\" saved successfully.", post->title());
    } else if (d->mCurrentPost.status() == BilboPost::Modified) {
        msg = i18n("Post with title \"%1\" modified successfully.", post->title());
    } else {
        msg = i18n("Post with title \"%1\" published successfully.", post->title());
    }
    //     KMessageBox::information( this, msg, "Successful" );
    deleteProgressBar();
    this->unsetCursor();
    Q_EMIT postPublishingDone(false, msg);
    sender()->deleteLater(); //FIXME Check if this command needed or NOT -Mehrdad
}

void PostEntry::showProgressBar()
{
    if (!d->progress) {
        d->progress = new QProgressBar(this);
        layout()->addWidget(d->progress);
        d->progress->setRange(0, 0);
    }
}

void PostEntry::deleteProgressBar()
{
    if (d->progress) {
        this->layout()->removeWidget(d->progress);
        d->progress->deleteLater();
        d->progress = Q_NULLPTR;
    }
}

void PostEntry::saveLocally()
{
    if (currentPost()->content().isEmpty()) {
        if (KMessageBox::warningYesNo(this, i18n("The current post content is empty, are you sure you want to save an empty post?")) == KMessageBox::No) {
            return;
        }
    }
    const int resId = DBMan::self()->saveLocalEntry(*currentPost(), d->mCurrentPostBlogId);
    if (resId == -1) {
        KMessageBox::detailedSorry(this, i18n("Saving post locally failed."), DBMan::self()->lastErrorText());
        return;
    }
    d->mCurrentPost.setLocalId(resId);
    Q_EMIT postSavedLocally();
    Q_EMIT showStatusMessage(i18n("Post saved locally."), false);
    qCDebug(BLOGILO_LOG) << "Locally saved";
}

void PostEntry::saveTemporary()
{
    if (d->isPostContentModified) {
        const int res = DBMan::self()->saveTempEntry(*currentPost(), d->mCurrentPostBlogId);
        if (res != -1) {
            d->mCurrentPost.setLocalId(res);
            Q_EMIT postSavedTemporary();
            qCDebug(BLOGILO_LOG) << "Temporary saved";
        } else {
            qCDebug(BLOGILO_LOG) << "Saving temporary failed: " << DBMan::self()->lastErrorText();
        }
    }
}

void PostEntry::slotPostModified()
{
    qCDebug(BLOGILO_LOG);
    disconnect(this, &PostEntry::textChanged, this, &PostEntry::slotPostModified);
    //         disconnect( txtTitle, SIGNAL(textChanged(QString)), this, SLOT(slotPostModified()) );
    //     Q_EMIT postModified();
    d->isPostContentModified = true;
}

