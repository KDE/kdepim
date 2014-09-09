//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//  Copyright (C) 2007-2008 Stephen Kelly <steveire@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

//Own Header
#include "kjotsedit.h"

#include <QMimeData>
#include <QTextCursor>
#include <QStackedWidget>
#include <QUrl>
#include <QMenu>
#include <QItemSelectionModel>
#include <QPointer>
#include <QClipboard>

#include <QAction>
#include <kactioncollection.h>
#include <krun.h>
#include <KApplication>

#include "kjotslinkdialog.h"

#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/item.h>

#include <KMime/Message>

#include <qdebug.h>
#include "kjotsmodel.h"
#include "noteshared/attributes/notelockattribute.h"
#include "noteshared/editor/noteeditorutils.h"

Q_DECLARE_METATYPE(QTextDocument *)
Q_DECLARE_METATYPE(QTextCursor)

using namespace Akonadi;

KJotsEdit::KJotsEdit(QItemSelectionModel *selectionModel, QWidget *parent)
    : KRichTextWidget(parent),
      actionCollection(0),
      allowAutoDecimal(false),
      m_selectionModel(selectionModel)
{
    setAcceptRichText(true);
    setWordWrapMode(QTextOption::WordWrap);
    setCheckSpellingEnabled(true);
    setRichTextSupport(FullTextFormattingSupport
                       | FullListSupport
                       | SupportAlignment
                       | SupportRuleLine
                       | SupportFormatPainting);

    setFocusPolicy(Qt::StrongFocus);

    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &KJotsEdit::selectionChanged);
    connect(m_selectionModel->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(tryDisableEditing()));
}

KJotsEdit::~KJotsEdit()
{
}

void KJotsEdit::mousePopupMenuImplementation(const QPoint &pos)
{
    QMenu *popup = mousePopupMenu();
    if (popup) {
        popup->addSeparator();
        QAction *act = actionCollection->action(QLatin1String("copyIntoTitle"));
        popup->addAction(act);
        act = actionCollection->action(QLatin1String("insert_checkmark"));
        act->setEnabled(!isReadOnly());
        popup->addAction(act);

        if (!KApplication::kApplication()->clipboard()->text().isEmpty()) {
            act = actionCollection->action(QLatin1String("paste_plain_text"));
            act->setEnabled(!isReadOnly());
            popup->addAction(act);
        }

        aboutToShowContextMenu(popup);
        popup->exec(pos);
        delete popup;
    }
}

void KJotsEdit::delayedInitialization(KActionCollection *collection)
{
    actionCollection = collection;

    connect(actionCollection->action(QLatin1String("auto_bullet")), SIGNAL(triggered()), SLOT(onAutoBullet()));
    connect(actionCollection->action(QLatin1String("auto_decimal")), SIGNAL(triggered()), SLOT(onAutoDecimal())); //auto decimal list
    connect(actionCollection->action(QLatin1String("manage_link")), SIGNAL(triggered()), SLOT(onLinkify()));
    connect(actionCollection->action(QLatin1String("insert_checkmark")), SIGNAL(triggered()), SLOT(addCheckmark()));
    connect(actionCollection->action(QLatin1String("manual_save")), SIGNAL(triggered()), SLOT(savePage()));
    connect(actionCollection->action(QLatin1String("insert_date")), SIGNAL(triggered()), SLOT(insertDate()));
}

void KJotsEdit::insertDate()
{
    NoteShared::NoteEditorUtils::insertDate(this);
}

void KJotsEdit::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    tryDisableEditing();
}

void KJotsEdit::tryDisableEditing()
{
    if (!m_selectionModel->hasSelection()) {
        return setReadOnly(true);
    }

    QModelIndexList list = m_selectionModel->selectedRows();
    if (list.size() != 1) {
        return setReadOnly(true);
    }

    Item item = list.first().data(EntityTreeModel::ItemRole).value<Item>();

    if (!item.isValid()) {
        return setReadOnly(true);
    }

    if (item.hasAttribute<NoteShared::NoteLockAttribute>()) {
        return setReadOnly(true);
    }

    setReadOnly(false);
}

void KJotsEdit::onBookshelfSelection(void)
{
    // TODO: PORT. Review and remove. Possibly keep the bug workaround.
#if 0
    QList<QTreeWidgetItem *> selection = bookshelf->selectedItems();
    int selectionSize = selection.size();

    if (selectionSize !=  1) {
        disableEditing();
    } else {
        KJotsPage *newPage = dynamic_cast<KJotsPage *>(selection[0]);
        if (!newPage) {
            disableEditing();
        } else {
            setEnabled(newPage->isEditable());
            setReadOnly(!newPage->isEditable());
            if (currentPage != newPage) {
                if (currentPage) {
                    currentPage->setCursor(textCursor());
                }
                currentPage = newPage;

                setDocument(currentPage->body());
                if (!currentPage->getCursor().isNull()) {
                    setTextCursor(currentPage->getCursor());
                }

                QStackedWidget *stack = static_cast<QStackedWidget *>(parent());
                stack->setCurrentWidget(this);
                setFocus();

                if (textCursor().atStart()) {
                    // Reflect formatting when switching pages and the first word is formatted
                    // Work-around for qrextedit bug. The format does not seem to exist
                    // before the first character. Submitted to qt-bugs, id 192886.
                    moveCursor(QTextCursor::Right);
                    moveCursor(QTextCursor::Left);
                }

            }
        }
    }
#endif
}

void KJotsEdit::onAutoBullet(void)
{
    KTextEdit::AutoFormatting currentFormatting = autoFormatting();

    //TODO: set line spacing properly.

    if (currentFormatting == KTextEdit::AutoBulletList) {
        setAutoFormatting(KTextEdit::AutoNone);
        actionCollection->action(QLatin1String("auto_bullet"))->setChecked(false);
    } else {
        setAutoFormatting(KTextEdit::AutoBulletList);
        actionCollection->action(QLatin1String("auto_bullet"))->setChecked(true);
    }
}

void KJotsEdit::createAutoDecimalList(void)
{
    //this is an adaptation of Qt's createAutoBulletList() function for creating a bulleted list, except in this case I use it to create a decimal list.
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    QTextListFormat listFmt;
    listFmt.setStyle(QTextListFormat::ListDecimal);
    listFmt.setIndent(blockFmt.indent() + 1);

    blockFmt.setIndent(0);
    cursor.setBlockFormat(blockFmt);

    cursor.createList(listFmt);

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void KJotsEdit::DecimalList(void)
{
    QTextCursor cursor = textCursor();

    if (cursor.currentList()) {
        return;
    }

    QString blockText = cursor.block().text();

    if (blockText.length() == 2 && blockText == QLatin1String("1.")) {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        createAutoDecimalList();
    }
}

void KJotsEdit::onAutoDecimal(void)
{
    if (allowAutoDecimal == true) {
        allowAutoDecimal = false;
        disconnect(this, &KJotsEdit::textChanged, this, &KJotsEdit::DecimalList);
        actionCollection->action(QLatin1String("auto_decimal"))->setChecked(false);
    } else {
        allowAutoDecimal = true;
        connect(this, &KJotsEdit::textChanged, this, &KJotsEdit::DecimalList);
        actionCollection->action(QLatin1String("auto_decimal"))->setChecked(true);
    }
}

void KJotsEdit::onLinkify(void)
{
    selectLinkText();
    QPointer<KJotsLinkDialog> linkDialog = new KJotsLinkDialog(const_cast<QAbstractItemModel *>(m_selectionModel->model()), this);
    linkDialog->setLinkText(currentLinkText());
    linkDialog->setLinkUrl(currentLinkUrl());

    if (linkDialog->exec()) {
        updateLink(linkDialog->linkUrl(), linkDialog->linkText());
    }

    delete linkDialog;
}

void KJotsEdit::addCheckmark(void)
{
    QTextCursor cursor = textCursor();
    NoteShared::NoteEditorUtils::addCheckmark(cursor);
}

bool KJotsEdit::canInsertFromMimeData(const QMimeData *source) const
{
    if (source->formats().contains(QLatin1String("kjots/internal_link"))) {
        return true;
    } else if (source->hasUrls()) {
        return true;
    } else {
        return KTextEdit::canInsertFromMimeData(source);
    }
}

void KJotsEdit::insertFromMimeData(const QMimeData *source)
{
    if (source->formats().contains(QLatin1String("kjots/internal_link"))) {
        insertHtml(QLatin1String(source->data(QLatin1String("kjots/internal_link"))));
    } else if (source->hasUrls()) {
        foreach (const QUrl &url, source->urls()) {
            if (url.isValid()) {
                QString html = QString::fromLatin1("<a href='%1'>%2</a> ")
                               .arg(QString::fromUtf8(url.toEncoded()))
                               .arg(url.toString(QUrl::RemovePassword));
                insertHtml(html);
            }
        }
    } else if (source->hasHtml()) {
        // Don't have an action to set top and bottom margins on paragraphs yet.
        // Remove the margins for all inserted html.
//         qDebug() << source->html();
        QString str = source->html();
        int styleBegin = 0;
        while ((styleBegin = str.indexOf(QLatin1String("style=\""), styleBegin, Qt::CaseInsensitive) + 7) != (-1 + 7)) {
            int styleEnd = str.indexOf(QLatin1Char('"'), styleBegin);
            int styleFragmentStart = styleBegin;
            int styleFragmentEnd = styleBegin;
            while ((styleFragmentEnd = str.indexOf(QLatin1String(";"), styleFragmentEnd) + 1) != (-1 + 1)) {
                if (styleFragmentEnd > styleEnd) {
                    break;
                }
                int fragmentLength = styleFragmentEnd - styleFragmentStart;
                if (str.mid(styleFragmentStart, fragmentLength).contains(QLatin1String("margin"), Qt::CaseInsensitive)) {
                    str.remove(styleFragmentStart, fragmentLength);
                    styleEnd -= fragmentLength;
                    styleFragmentEnd = styleFragmentStart;

                    if (styleBegin == styleEnd) {
                        str.remove(styleBegin - 7, 7 + 1); // remove the now empty style attribute.
                    }
                } else {
                    styleFragmentStart = styleFragmentEnd;
                }
            }
            styleBegin = styleEnd;
        }
//         qDebug() << str;
        insertHtml(str);
    } else {
        KTextEdit::insertFromMimeData(source);
    }
}

void KJotsEdit::mouseReleaseEvent(QMouseEvent *event)
{
    // TODO: PORT
#if 0
    if ((event->modifiers() & Qt::ControlModifier) && (event->button() & Qt::LeftButton)
            && !anchorAt(event->pos()).isEmpty()) {
        QUrl anchor(anchorAt(event->pos()));
        if (anchor.scheme() == "kjots") {
            quint64 target = anchor.path().mid(1).toULongLong();
            bookshelf->jumpToId(target);
        } else {
            new KRun(anchor, this);
        }
    }
#endif
    KTextEdit::mouseReleaseEvent(event);
}

void KJotsEdit::pastePlainText()
{
    QString text = KApplication::kApplication()->clipboard()->text();
    if (!text.isEmpty()) {
        insertPlainText(text);
    }
}

bool KJotsEdit::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate) {
        savePage();
    }
    return KRichTextWidget::event(event);
}

void KJotsEdit::focusOutEvent(QFocusEvent *event)
{
    savePage();
    KRichTextWidget::focusOutEvent(event);
}

void KJotsEdit::savePage()
{
    if (!document()->isModified()) {
        return;
    }

    QModelIndexList rows = m_selectionModel->selectedRows();

    if (rows.size() != 1) {
        return;
    }

    QModelIndex index = rows.at(0);

    Item item = index.data(EntityTreeModel::ItemRole).value<Item>();

    if (!item.isValid()) {
        return;
    }

    if (!item.hasPayload<KMime::Message::Ptr>()) {
        return;
    }

    QAbstractItemModel *model = const_cast<QAbstractItemModel *>(m_selectionModel->model());

    document()->setModified(false);
    document()->setProperty("textCursor", QVariant::fromValue(textCursor()));
    model->setData(index, QVariant::fromValue(document()), KJotsModel::DocumentRole);
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
