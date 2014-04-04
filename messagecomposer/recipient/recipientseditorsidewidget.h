/*
    Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
    Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    Refactored from earlier code by:
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef RECIPIENTSEDITORSIDEWIDGET_H
#define RECIPIENTSEDITORSIDEWIDGET_H

#include "recipientseditor.h"

class QLabel;
class QPushButton;

namespace MessageComposer {
class KWindowPositioner;

class RecipientsPicker;

class RecipientsEditorSideWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RecipientsEditorSideWidget( RecipientsEditor *editor, QWidget *parent );
    ~RecipientsEditorSideWidget();

    MessageComposer::RecipientsPicker* picker() const;

public slots:
    void setTotal( int recipients, int lines );
    void setFocus();
    void updateTotalToolTip();
    void pickRecipient();

signals:
    void pickedRecipient( const Recipient &, bool & );
    void saveDistributionList();

private:
    RecipientsEditor *mEditor;
    QLabel *mTotalLabel;
    QPushButton *mDistributionListButton;
    QPushButton *mSelectButton;
    /** The RecipientsPicker is lazy loaded, never access it directly,
      only through picker() */
    mutable MessageComposer::RecipientsPicker *mRecipientPicker;
    /** lazy loaded, don't access directly, unless you've called picker() */
    mutable MessageComposer::KWindowPositioner *mPickerPositioner;
};
}

#endif //RECIPIENTSEDITORSIDEWIDGET_H
