/*
    This file is part of KJots.

    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#ifndef KJOTSBOOKSHELFENTRYVALIDATOR
#define KJOTSBOOKSHELFENTRYVALIDATOR

#include <QValidator>
#include <QAbstractItemModel>

/**
  This class is a validator intended to be used with an editable QComboBox.

  \a validate returns the state of whether the text in the lineedit matches one of the items in the
  model. All matching is attempted on the first column of the model.

*/
class KJotsBookshelfEntryValidator : public QValidator
{
    Q_OBJECT
public:
    /**
      Create a new Validator for Entries in the KJots bookshelf.
      @param parent The parent object.
      @param model The model to use to validate the input.
    */
    explicit KJotsBookshelfEntryValidator(QAbstractItemModel *model, QObject *parent = 0);

    /**
      Destructor.
    */
    ~KJotsBookshelfEntryValidator();

    /**
      Reimplemented. Returns the state of whether \p input matches one of the items in the model.

      If the text does not match any item in the model, the state is Invalid.
      If the text is empty, the state is Intermediate.
      If the text matches the start of one or more items in the model, the state is Intermediate.
      If the text matches one of the items in the model exactly, the state is Acceptable.

      \return The validation state.
    */
    virtual QValidator::State validate(QString &input, int &pos) const;

private:
    QAbstractItemModel *m_model;

};

#endif
