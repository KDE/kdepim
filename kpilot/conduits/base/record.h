#ifndef _RECORD_H
#define _RECORD_H


class Record {
  public:
    virtual QVariant id() = 0;

    virtual bool isModified() = 0;

};
#endif
