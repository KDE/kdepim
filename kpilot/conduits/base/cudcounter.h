#ifndef _CUDCOUNTER_H
#define _CUDCOUNTER_H


class CUDCounter {
  public:
    CUDCounter();

    void setStartCount();

    void setEndCount();

    void created();

     updated();

     deleted();

    bool hasValidCount();

    int volatility();

};
#endif
