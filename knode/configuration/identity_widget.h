/*
  Copyright 2009 Olivier Trichet <nive@nivalis.org>

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef KNODE_IDENTITY_WIDGET_H
#define KNODE_IDENTITY_WIDGET_H

#include "ui_identity_widget.h"

class KNGroup;
class KNNntpAccount;
namespace KNode {
  class SettingsContainerInterface;
}

namespace KNode {

/**
  Configuration widget for an identity.
*/
class KNODE_EXPORT IdentityWidget : public KCModule, private Ui::IdentityWidget
{
  Q_OBJECT

  public:
    /**
      Constructor for the selection widget of identity.
    */
    explicit IdentityWidget( SettingsContainerInterface *settingsContainer, const KComponentData &inst, QWidget *parent = 0 );

    ~IdentityWidget();

  public slots:
    /**
      Reimplemented from KCModule::load().
    */
    virtual void load();
    /**
      Reimplemented from KCModule::save();
    */
    virtual void save();

  private:
    /**
      Loads the @p identity into this widget.
    */
    void loadFromIdentity( const KPIMIdentities::Identity &identity );

    /**
      Set up this widget.
      @param identity original identity of the settings container passed
      to one of the constructor (may be null)
    */
    void setup( const KPIMIdentities::Identity &identity );

  private slots:
    /**
      Called when a new identity is selected.
      It loads the widget with its information.
    */
    void identitySelected( uint uoid );

    /**
      Called when the checkbox "Use a specific identity" is
      (un)checked.
    */
    void useSpecificIdentity( bool useSpecific );

    /**
      Called when the user click the "modify..." button.
    */
    void modifyIdentities();

  private:
    /**
      The global settings, account or group passed to the constructor.
    */
    SettingsContainerInterface *mConfigurationContainer;
};

} // namespace KNode


#endif // KNODE_IDENTITY_WIDGET_H
