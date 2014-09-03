/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MAILCOMMON_FILTERACTIONWITHCOMMAND_H
#define MAILCOMMON_FILTERACTIONWITHCOMMAND_H

#include "filteractionwithurl.h"

class QTemporaryFile;

namespace MailCommon
{

class FilterActionWithCommand : public FilterActionWithUrl
{
    Q_OBJECT

public:
    /**
     * @copydoc FilterAction::FilterAction
     */
    FilterActionWithCommand(const QString &name, const QString &label, QObject *parent = 0);

    /**
     * @copydoc FilterAction::createParamWidget
     */
    virtual QWidget *createParamWidget(QWidget *parent) const;

    /**
     * @copydoc FilterAction::applyParamWidgetValue
     */
    virtual void applyParamWidgetValue(QWidget *paramWidget);

    /**
     * @copydoc FilterAction::setParamWidgetValue
     */
    virtual void setParamWidgetValue(QWidget *paramWidget) const;

    /**
     * @copydoc FilterAction::clearParamWidget
     */
    virtual void clearParamWidget(QWidget *paramWidget) const;

    /**
     * Substitutes various placeholders for data from the message
     * resp. for filenames containing that data. Currently, only %n is
     * supported, where n in an integer >= 0. %n gets substituted for
     * the name of a tempfile holding the n'th message part, with n=0
     * meaning the body of the message.
     */
    virtual QString substituteCommandLineArgsFor(const KMime::Message::Ptr &aMsg,
            QList<QTemporaryFile *> &aTempFileList) const;

    virtual ReturnCode genericProcess(ItemContext &context, bool filtering) const;
};

}

#endif
