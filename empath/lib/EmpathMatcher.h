/*
    Empath - Mailer for KDE

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef EMPATHMATCHER_H
#define EMPATHMATCHER_H

#include "EmpathDefines.h"
#include "EmpathURL.h"

/**
 * @short Match expression, used by filters
 * @author Rikkus
 */
class EmpathMatcher
{
    public:

        enum MatchExprType {
            Size,
            BodyExpr,
            HeaderExpr,
            HasAttachments,
            AnyMessage
        };

        EmpathMatcher();
        virtual ~EmpathMatcher();

        /**
         * Called by containing filter to request config load.
         */
        void load(const QString & parentName, Q_UINT32 id);
        /**
         * Called by containing filter to request config save.
         */
        void save(const QString & parentName, Q_UINT32 id);

        /**
         * Attempt to match the given message.
         */
        bool match(const EmpathURL &);

        /**
         * Nice description of this matcher.
         */
        QString description() const;

        /**
         * Type of this expression.
         */
        MatchExprType type() const { return type_; }

        /**
         * Change the type of this expression.
         */
        void setType(MatchExprType t) { type_ = t; }

        /**
         * Maximum size (used when set to match on size).
         */
        Q_UINT32 size() { return size_; }

        /**
         * Header to match on (used when set to match on header).
         */
        const QString & matchHeader() { return matchHeader_; }

        /**
         * Match expression (used when set to match using an expression)
         */
        const QString & matchExpr() { return matchExpr_; }

        /**
         * Set the size of messages to match (> size).
         */
        void setSize(Q_UINT32 s) { size_ = s; }

        /**
         * Set the header name to match upon.
         */
        void setMatchHeader(const QString & s) { matchHeader_ = s; }

        /**
         * Set the match expression to use.
         */
        void setMatchExpr(const QString & s) { matchExpr_ = s; }

        /**
         * @internal
         */
        const char * className() const { return "EmpathMatcher"; }

    private:

        MatchExprType type_;
        Q_UINT32 size_;
        QString matchHeader_;
        QString matchExpr_;
};

#endif

// vim:ts=4:sw=4:tw=78
