/*   -*- mode: C++; c-file-style: "gnu" -*-
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2006 Dmitry Morozhnikov
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <config.h>

#ifndef TEMPLATESCONFIGURATION_H
#define TEMPLATESCONFIGURATION_H

#include "templatesconfiguration_base.h"
#include "templatesinsertcommand.h"

class TemplatesConfiguration : public TemplatesConfigurationBase
{
  Q_OBJECT

  public:

    TemplatesConfiguration( TQWidget *parent = 0, const char *name = 0 );

    void loadFromGlobal();
    void saveToGlobal();
    void loadFromIdentity( uint id );
    void saveToIdentity( uint id );
    void loadFromFolder( TQString id, uint identity = 0 );
    void saveToFolder( TQString id );

    /** Do import settings from 'Phrases' configuration. */
    void loadFromPhrases();

    /** Do import 'Phrases' settings and store them as global templates. */
    static void importFromPhrases();

    /** Convert 'Phrases'-like placeholders into 'Templates' compatible. */
    static TQString convertPhrases( TQString &str );

    static TQString defaultNewMessage();
    static TQString defaultReply();
    static TQString defaultReplyAll();
    static TQString defaultForward();
    static TQString defaultQuoteString();

  public slots:

    void slotInsertCommand( TQString cmd, int adjustCursor = 0 );

    void slotTextChanged();

  signals:

    void changed();

  protected:

    TQString strOrBlank( TQString str );

};

#endif // TEMPLATESCONFIGURATION_H
