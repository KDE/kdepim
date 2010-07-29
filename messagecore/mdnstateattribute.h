/******************************************************************************
 * 
 *  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>
 *  Copyright (c) 2010 KDAB
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This library is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA, 02110-1301, USA.
 *
 *****************************************************************************/

#ifndef MESSAGECORE_MDNSTATE_ATTRIBUTE_H
#define MESSAGECORE_MDNSTATE_ATTRIBUTE_H

#include "messagecore_export.h"
#include <akonadi/attribute.h>

namespace Akonadi {
  
  /**
   * @short An Attribute that keeps track of the MDN state of a mail message.
   *
   * Once a mail that contains a Message Disposition Notification is processed,
   *  the outcome of the user action will be stored in this attribute.
   *
   * @endcode
   *
   * @author Leo Franchi <lfranchi@kde.org>
   * @see Akonadi::Attribute
   * @since 4.6
   */
  class MESSAGECORE_EXPORT MDNStateAttribute : public Attribute
  {
  public:
    /** Flags for the "MDN sent" state. */
    typedef enum
    {
      MDNStateUnknown,
      MDNNone,
      MDNIgnore,
      MDNDisplayed,
      MDNDeleted,
      MDNDispatched,
      MDNProcessed,
      MDNDenied,
      MDNFailed
    } MDNSentState;
    
    /**
     * Creates a new MDN state attribute.
     */
    explicit MDNStateAttribute( const MDNSentState& state = MDNStateUnknown );
    explicit MDNStateAttribute( const QByteArray& stateData );
    
    /**
     * Destroys the MDN state attribute.
     */
    ~MDNStateAttribute();
    
    /**
     * Reimplemented from Attribute
     */
    QByteArray type() const;
    
    /**
     * Reimplemented from Attribute
     */
    MDNStateAttribute* clone() const;
    
    /**
     * Reimplemented from Attribute
     */
    QByteArray serialized() const;
    
    /**
     * Reimplemented from Attribute
     */
    void deserialize( const QByteArray &data );

    /**
     * Set the MDN state.
     */
    void setMDNState( const MDNSentState& state );

    /**
     * Get the sent MDN state.
     */
    MDNStateAttribute::MDNSentState mdnState() const;
    
  private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
  };
  
}

#endif
