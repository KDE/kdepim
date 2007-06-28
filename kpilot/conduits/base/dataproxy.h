#ifndef _DATAPROXY_H
#define _DATAPROXY_H


#include "Record.h"

class CUDCounter;

class DataProxy {
  protected:
    CUDCounter * fCounter;


  public:
    DataProxy();

    /**
     * Adds the record to the database and returns the internal id for the added record.
     */
    QVariant addRecord();

     deleteRecord();

     editRecord();

    void syncFinished();

    void setIterateMode();

    virtual void commitChanges() = 0;

    /**
     * Looks for a matching record. Should return 0 if there is no match.
     */
    virtual Record lookForMatch() = 0;

    virtual void loadAllRecords() = 0;

    /**
     * Dependend on the iterateMode it should give the next record, the next modified record or 0 if there are no more records to iterate over.
     */
    virtual Record nextRecord() = 0;

    virtual Record readRecordById() = 0;

    /**
     * Returns QString::Null if everything is ok, or a message explaining the cause (invalid count, or volatility exceeds configured threshold).
     */
    virtual QString volatilityMessage() = 0;


  protected:
    Record * fRecords;

};
#endif
