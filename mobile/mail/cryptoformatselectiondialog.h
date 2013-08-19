/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#ifndef CRYPTOFORMATSELECTIONDIALOG_H
#define CRYPTOFORMATSELECTIONDIALOG_H

#include <kdialog.h>

#include <kleo/enum.h>

class KComboBox;

/**
 * @short A dialog to select a crypto format.
 *
 * This dialog is used to select a crypto format
 * for the message viewer.
 */
class CryptoFormatSelectionDialog : public KDialog
{
  public:
    /**
     * Creates a new crypto format selection dialog.
     *
     * @param parent The parent widget.
     */
    explicit CryptoFormatSelectionDialog( QWidget *parent = 0 );

    /**
     * Destroys the crypto format selection dialog.
     */
    ~CryptoFormatSelectionDialog();

    /**
     * Sets the currently selected @p crypto format.
     */
    void setCryptoFormat( Kleo::CryptoMessageFormat format );

    /**
     * Returns the currently selected crypto format.
     */
     Kleo::CryptoMessageFormat cryptoFormat() const;

  private:
    KComboBox *mCryptoFormatCombo;
};

#endif
