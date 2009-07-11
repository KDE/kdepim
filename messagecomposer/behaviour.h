/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MESSAGECOMPOSER_BEHAVIOUR_H
#define MESSAGECOMPOSER_BEHAVIOUR_H

#include "messagecomposer_export.h"

#include <QtCore/QSharedDataPointer>

namespace MessageComposer {

/**
*/
class MESSAGECOMPOSER_EXPORT Behaviour
{
  public:
    enum Action
    {
      UseGui,
      UseCrypto,
      UseWrapping,
      UseFallbackCharset,
      WarnBadCharset,
      WarnZeroRecipients,
      CustomHeaders,
      LastAction // TODO should this be made =100 for further expansion?
    };

    Behaviour();
    virtual ~Behaviour();

    bool isActionEnabled( Action action ) const;
    void enableAction( Action action, bool enable = true );
    void disableAction( Action action );

    static Behaviour behaviourForSending();
    static Behaviour behaviourForPrinting();
    static Behaviour behaviourForAutosaving();
    static Behaviour behaviourForSavingLocally();

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

/*
  FIXME
  I can't figure out how else to make this work.  If I put it in the cpp or in another
  file like behaviour_p.h, I get 'incomplete type' errors.  If I put it in behavior_p.h
  and include it in the bottom, I need to install the _p anyway.
  The problem seems to be that Behaviour needs Behaviour::Private (for QSharedDataPointer),
  but Behaviour::Private needs Behaviour back (for Behaviour:LastAction).
  -> I removed the latter dependency and the problem persists :-/
*/
/**
  @internal
*/
class Behaviour::Private : public QSharedData
{
  public:
    Private()
    {
    }

    Private( const Private &other )
      : QSharedData( other )
    {
      for( int i = 0; i < Behaviour::LastAction; i++ ) {
        actions[i] = other.actions[i];
      }
    }

    bool actions[ Behaviour::LastAction ];
};

}

#endif
