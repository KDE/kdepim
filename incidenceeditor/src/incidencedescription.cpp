/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include "incidencedescription.h"
#include "ui_dialogdesktop.h"
#include <KPIMTextEdit/RichTextComposer>

#include "incidenceeditor_debug.h"
#include <KActionCollection>
#include <KToolBar>
#include <KLocalizedString>

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG
{

class IncidenceDescriptionPrivate
{
public:
    IncidenceDescriptionPrivate() : mRichTextEnabled(false)
    {
    }

    QString mRealOriginalDescriptionEditContents;
    bool mRichTextEnabled;
};

}

IncidenceDescription::IncidenceDescription(Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(0), mUi(ui), d(new IncidenceDescriptionPrivate())
{
    setObjectName(QStringLiteral("IncidenceDescription"));
    mUi->mRichTextLabel->setContextMenuPolicy(Qt::NoContextMenu);
    setupToolBar();
    connect(mUi->mRichTextLabel, &QLabel::linkActivated, this, &IncidenceDescription::toggleRichTextDescription);
    connect(mUi->mDescriptionEdit->richTextComposer(), &KPIMTextEdit::RichTextComposer::textChanged, this, &IncidenceDescription::checkDirtyStatus);
}

IncidenceDescription::~IncidenceDescription()
{
    delete d;
}

void IncidenceDescription::load(const KCalCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;

    d->mRealOriginalDescriptionEditContents.clear();

    if (incidence) {
        enableRichTextDescription(incidence->descriptionIsRich());
        if (incidence->descriptionIsRich()) {
            mUi->mDescriptionEdit->richTextComposer()->setHtml(incidence->richDescription());
            d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->richTextComposer()->toHtml();
        } else {
            mUi->mDescriptionEdit->richTextComposer()->setPlainText(incidence->description());
            d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->richTextComposer()->toPlainText();
        }
    } else {
        enableRichTextDescription(false);
        mUi->mDescriptionEdit->richTextComposer()->clear();
    }

    mWasDirty = false;
}

void IncidenceDescription::save(const KCalCore::Incidence::Ptr &incidence)
{
    if (d->mRichTextEnabled) {
        incidence->setDescription(mUi->mDescriptionEdit->richTextComposer()->toHtml(), true);
    } else {
        incidence->setDescription(mUi->mDescriptionEdit->richTextComposer()->toPlainText(), false);
    }
}

bool IncidenceDescription::isDirty() const
{

    /* Sometimes, what you put in a KRichTextWidget isn't the same as what you get out.
       Line terminators (cr,lf) for example can be converted.

       So, to see if the user changed something, we can't compare the original incidence
       with the new editor content.

       Instead we compare the new editor content, with the original editor content, this way
       any tranformation regarding non-printable chars will be irrelevant.
    */
    if (d->mRichTextEnabled) {
        return !mLoadedIncidence->descriptionIsRich() ||
               d->mRealOriginalDescriptionEditContents != mUi->mDescriptionEdit->richTextComposer()->toHtml();
    } else {
        return mLoadedIncidence->descriptionIsRich() ||
               d->mRealOriginalDescriptionEditContents != mUi->mDescriptionEdit->richTextComposer()->toPlainText();
    }
}

void IncidenceDescription::enableRichTextDescription(bool enable)
{
    d->mRichTextEnabled = enable;

    QString rt(i18nc("@action Enable or disable rich text editting", "Enable rich text"));
    QString placeholder(QStringLiteral("<a href=\"show\"><font color='blue'>%1 &gt;&gt;</font></a>"));

    if (enable) {
        rt = i18nc("@action Enable or disable rich text editting", "Disable rich text");
        placeholder = QStringLiteral("<a href=\"show\"><font color='blue'>&lt;&lt; %1</font></a>");
        mUi->mDescriptionEdit->richTextComposer()->activateRichText();
        d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->richTextComposer()->toHtml();
    } else {
        mUi->mDescriptionEdit->richTextComposer()->switchToPlainText();
        d->mRealOriginalDescriptionEditContents = mUi->mDescriptionEdit->richTextComposer()->toPlainText();
    }

    placeholder = placeholder.arg(rt);
    mUi->mRichTextLabel->setText(placeholder);
    mUi->mDescriptionEdit->richTextComposer()->setEnableActions(enable);
    mUi->mEditToolBarPlaceHolder->setVisible(enable);
    checkDirtyStatus();
}

void IncidenceDescription::toggleRichTextDescription()
{
    enableRichTextDescription(!d->mRichTextEnabled);
}

void IncidenceDescription::setupToolBar()
{
#ifndef QT_NO_TOOLBAR
    KActionCollection *collection = new KActionCollection(this);
    mUi->mDescriptionEdit->richTextComposer()->createActions(collection);

    KToolBar *mEditToolBar = new KToolBar(mUi->mEditToolBarPlaceHolder);
    mEditToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mEditToolBar->addAction(collection->action(QStringLiteral("format_text_bold")));
    mEditToolBar->addAction(collection->action(QStringLiteral("format_text_italic")));
    mEditToolBar->addAction(collection->action(QStringLiteral("format_text_underline")));
    mEditToolBar->addAction(collection->action(QStringLiteral("format_text_strikeout")));
    mEditToolBar->addSeparator();

    mEditToolBar->addAction(collection->action(QStringLiteral("format_list_style")));
    mEditToolBar->addSeparator();

    mEditToolBar->addAction(collection->action(QStringLiteral("format_align_left")));
    mEditToolBar->addAction(collection->action(QStringLiteral("format_align_center")));
    mEditToolBar->addAction(collection->action(QStringLiteral("format_align_right")));
    mEditToolBar->addAction(collection->action(QStringLiteral("format_align_justify")));
    mEditToolBar->addSeparator();

    mEditToolBar->addAction(collection->action(QStringLiteral("format_painter")));
    mUi->mDescriptionEdit->richTextComposer()->setEnableActions(false);

    QGridLayout *layout = new QGridLayout(mUi->mEditToolBarPlaceHolder);
    layout->addWidget(mEditToolBar);
#endif

    // By default we don't show the rich text toolbar.
    mUi->mEditToolBarPlaceHolder->setVisible(false);
    d->mRichTextEnabled = false;
}

void IncidenceDescription::printDebugInfo() const
{
    // We're going to crash
    qCDebug(INCIDENCEEDITOR_LOG) << "RichText enabled " << d->mRichTextEnabled;

    if (mLoadedIncidence) {
        qCDebug(INCIDENCEEDITOR_LOG) << "Incidence description is rich " << mLoadedIncidence->descriptionIsRich();

        if (mLoadedIncidence->descriptionIsRich()) {
            qCDebug(INCIDENCEEDITOR_LOG) << "desc is rich, and it is <desc>" <<  mLoadedIncidence->richDescription()
                                         << "</desc>; "
                                         << "widget has <desc>" << mUi->mDescriptionEdit->richTextComposer()->toHtml()
                                         << "</desc>; "
                                         << "expr mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->toHtml() is "
                                         << (mLoadedIncidence->richDescription() != mUi->mDescriptionEdit->richTextComposer()->toHtml());
        } else {
            qCDebug(INCIDENCEEDITOR_LOG) << "desc is not rich, and it is <desc>" << mLoadedIncidence->description()
                                         << "</desc>; "
                                         << "widget has <desc>" << mUi->mDescriptionEdit->richTextComposer()->toPlainText()
                                         << "</desc>; "
                                         << "expr mLoadedIncidence->description() != mUi->mDescriptionEdit->toPlainText() is "
                                         << (mLoadedIncidence->description() != mUi->mDescriptionEdit->richTextComposer()->toPlainText());
        }

    } else {
        qCDebug(INCIDENCEEDITOR_LOG) << "Incidence is invalid";
    }
}

