/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef RICHTEXTCOMPOSERACTIONS_H
#define RICHTEXTCOMPOSERACTIONS_H

#include <QObject>
#include "messagecomposer_export.h"
#include "messagecomposer/richtextcomposer.h"
class KActionCollection;
class QTextCharFormat;
class QAction;
namespace MessageComposer
{
class RichTextComposerControler;
class MESSAGECOMPOSER_EXPORT RichTextComposerActions : public QObject
{
    Q_OBJECT
public:
    explicit RichTextComposerActions(MessageComposer::RichTextComposerControler *controler, QObject *parent = Q_NULLPTR);
    ~RichTextComposerActions();

    void createActions(KActionCollection *ac);
    int numberOfActions() const;

    QList<QAction *> richTextActionList() const;

    void uncheckActionFormatPainter();
    void updateActionStates();
    void textModeChanged(MessageComposer::RichTextComposer::Mode mode);

public Q_SLOTS:
    void setActionsEnabled(bool enabled);

private Q_SLOTS:
    void slotUpdateCharFormatActions(const QTextCharFormat &format);
    void slotUpdateMiscActions();

    void setListStyle(int _styleindex);
private:
    class RichTextComposerActionsPrivate;
    RichTextComposerActionsPrivate *const d;
};
}

#endif // RICHTEXTCOMPOSERACTIONS_H
