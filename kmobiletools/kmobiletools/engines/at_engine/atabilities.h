/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>,
   Alexander Rensmann <zerraxys@gmx.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef ATABILITIES_H
#define ATABILITIES_H

class QStringList;
#include <qstring.h>
#include <qstringlist.h>
#include <libkmobiletools/devicesconfig.h>


/** \brief Container for all abilities of the phone.
*
* This object is a collection of all checked abilities of the mobile phone, generated in
* the job TestPhoneFeatures.
*/
class ATAbilities
{
    public:
        ATAbilities();
        /** Returns the available phonebook slots. See sl_pbSlots for more information.
        */
        QStringList getPBSlots() const { return sl_pbSlots; };
        int filesystem() { return i_filesystem; }
        /** Returns true if the phone can select phonebooks and there are slots available.
        */
        bool canSelectPBSlots() { return sl_pbSlots.count()!=0; };
        /** Returns all available characters sets.
        */
        QStringList getCharacterSets() const { return sl_CharacterSets; };
        /** Returns true if the phone can change the character set.
        */
        bool canSelectCharacterSets() { return sl_CharacterSets.count()!=0; };
        /** Returns all available slots for short messages.
        */
        QStringList getSMSSlots() const { return sl_SMSSlots; };
        /** Returns true if the phone can change slots for short message readout.
        */
        bool canSelectSMSSlots() { return sl_SMSSlots.count()!=0; };
        /** Returns true if the phone is set to process sms in pdu mode.
        */
        bool isPDU() { return smsPDUMode; };
        /** Return true if the mobile phone is a Siemens phone.
         */
        bool isSiemens() { return manufacturer.contains( "Siemens", Qt::CaseInsensitive ); };
         /** Return true if the mobile phone is a Motorola  phone.
          */
        bool isMotorola() { return manufacturer.contains( "Motorola", Qt::CaseInsensitive ); };
          /** Return true if the mobile phone is a Sony Ericsson phone.
           */
        bool isSonyEricsson() { return manufacturer.contains( "Ericsson", Qt::CaseInsensitive ); };
        /** This is Siemens specific. Returns true if the mobile understands the
        * ^SMGL command for listing sms. This command behaves exactly like CMGL
        * but does not change the status from unread to read
        */
        bool canSMGL() { return b_canSMGL; };
        /** This is Siemens specific. Returns true if the mobilecan use the
        * ^SBNR command to read the internal phonebook. This command returns
        * the addressbook in hexadecimal compacted VCF.
         */
        bool canSiemensVCF() { return b_canSiemensVCF; };
        /** This is Siemens specific. Returns true if the mobilecan use the
         * ^SDBR command to read the internal phonebook. This command returns
         * the addressbook (only phonenumbers). If available the VCF should be
         * preferred.
         */
        bool canSDBR() { return b_canSDBR; };
        /** Returns true if short messages can be stored in the mobile phone.
         * See StoreSMS on how to store messages.
         */
        bool canStoreSMS() { return b_canStoreSMS; };
        /** Returns true if short messages can be deleted from the mobile phone.
         * See RemoveSMS on how to remove messages.
         */
        bool canDeleteSMS() { return b_canDeleteSMS; };
        /** Returns true if short messages can be send via the mobile phone.
         * See SendSMS on how to remove messages.
         */
        bool canSendSMS() { return b_canSendSMS; };

        QString printCapabilities() {
                QString out="Phonebook Slots: %1";
                out += "\nCan select phonebook slots: %2";
                out += "\nCharset: %4";;
                out += "\nCan select charset:%5";
                out += "\nSMS Slots: %6" ;
                out += "\nCan select SMS slots: %7";;
                out += "\nPDU Mode: %8";;
                out=out.arg(getPBSlots().join(",")).arg(canSelectPBSlots()).arg(getCharacterSets().join(",")).arg(canSelectCharacterSets())
                        .arg(getSMSSlots().join(",")).arg(canSelectSMSSlots()).arg(isPDU());
                out += "\nSiemens phone: %1" ;
                out += "\nSMGL siemens command: %2";
                out += "\nSDBR siemens command: %3";
                out += "\nSiemens VCF: %4";
                out += "\nCan store SMS: %5";
                out += "\nCan delete SMS: %6";
                out += "\nCan send SMS: %7";
                out += "\nManufacturer: %8";
                out=out.arg(isSiemens()).arg(canSMGL()).arg(canSDBR()).arg(canSiemensVCF()).arg(canStoreSMS()).arg(canDeleteSMS()).arg(canSendSMS()).arg(manufacturer);
                out += "\nFileSystem abilities: %1";
                switch( filesystem() ){
                    case KMT_FILESYSTEM_NONE:
                        out=out.arg("none");
                        break;
                    case KMT_FILESYSTEM_Pk2:
                        out=out.arg("P2K");
                        break;
                }
                return out.append("\n");
        }
    protected:
        void setPBSlots( const QStringList &pbslots ) { sl_pbSlots = pbslots; };
        void setSMSSlots( const QStringList &smsslots ) { sl_SMSSlots = smsslots; };
        void setCharacterSets( const QStringList &characterSets ) { sl_CharacterSets = characterSets; };
        void setManufacturer( const QString &manufacturer );
        void setFileSystem( int fs ) { i_filesystem=fs; }
        int i_filesystem;
        /** This is a list of all phonebook slots that can be selected with the
        * SelectSMSSlot job.
        */
        QStringList sl_pbSlots;
        /** This is a list of all supported character sets of this phone
        */
        QStringList sl_CharacterSets;
        /** This is a list of all available short message slots.
        */
        QStringList sl_SMSSlots;
        /** The manufacturer ID as returned by +CGMI
        */
        QString manufacturer;
        bool smsPDUMode, b_canSMGL, b_canSiemensVCF, b_canStoreSMS, b_canSendSMS, b_canDeleteSMS, b_canSDBR;
    friend class TestPhoneFeatures;
};

#endif
