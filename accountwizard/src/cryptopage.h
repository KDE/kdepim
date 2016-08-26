/*
    Copyright (c) 2016 Klarälvdalens Datakonsult AB

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

#ifndef CRYPTOPAGE_H
#define CRYPTOPAGE_H

#include "page.h"
#include "ui_cryptopage.h"

class Dialog;
class SetupManager;

namespace Kleo
{
class Job;
}

class CryptoPage : public Page
{
    Q_OBJECT

public:
    explicit CryptoPage(Dialog *parent);

    void enterPageNext() Q_DECL_OVERRIDE;
    void leavePageNext() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void customItemSelected(const QVariant &data);

private:
    enum Action {
        NoKey = 1,
        GenerateKey,
        ImportKey
    };

    void generateKeyPair();
    void importKey();

    Ui::CryptoPage ui;
    SetupManager *mSetupManager;
};

#endif // CRYPTOPAGE_H
