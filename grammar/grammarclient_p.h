/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef GRAMMARCLIENT_H
#define GRAMMARCLIENT_H

#include "grammar_export.h"

#include <QObject>
#include <QStringList>
#include <QString>


namespace Grammar {
class GrammarPlugin;
class GRAMMAR_EXPORT GrammarClient : public QObject
{
    Q_OBJECT
public:
    explicit GrammarClient(QObject *parent = 0);

    virtual int reliability() const = 0;

    virtual QStringList languages() const = 0;

    virtual QString name() const = 0;

    virtual Grammar::GrammarPlugin *createGrammarChecker(const QString &language) = 0;
};

}

#endif // GRAMMARCLIENT_H
