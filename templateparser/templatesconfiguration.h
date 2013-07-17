/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TEMPLATEPARSER_TEMPLATESCONFIGURATION_H
#define TEMPLATEPARSER_TEMPLATESCONFIGURATION_H

#include "templateparser_export.h"
#include "ui_templatesconfiguration_base.h"
#include "templatesinsertcommand.h"

namespace TemplateParser {

class TEMPLATEPARSER_EXPORT TemplatesConfiguration : public QWidget, Ui::TemplatesConfigurationBase
{
  Q_OBJECT

  public:

    explicit TemplatesConfiguration( QWidget *parent = 0, const QString &name = QString() );

    void loadFromGlobal();
    void saveToGlobal();
    void loadFromIdentity( uint id );
    void saveToIdentity( uint id );
    void loadFromFolder( const QString &id, uint identity = 0 );
    void saveToFolder( const QString &id );
    void resetToDefault();

    QLabel *helpLabel() const { return mHelp; }

    /**
     * Returns the template configuration identifier string for a given identity.
     */
    static QString configIdString( uint id );

  public Q_SLOTS:
    void slotInsertCommand( const QString &cmd, int adjustCursor = 0 );
    void slotTextChanged();

  signals:
    void changed();

  protected:
    QString strOrBlank( const QString &str );
    QString mHelpString;

  private:
    KTextEdit *currentTextEdit() const;

  private Q_SLOTS:
    void slotHelpLinkClicked( const QString & );
};

}

#endif
