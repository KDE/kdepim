/***************************************************************************
                          harray.hxx  -  description
                             -------------------
    begin                : Fri Jun 30 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __H_ARRAY__
#define __H_ARRAY__

#include <stdlib.h>

template <class T> class harray
{
  private:
    T   **array;
    int   N,LN;
  private:
    void cleanup(void) { int i;
      for(i=0;i<N;i++) {
        if (array[i]!=NULL) { delete array[i]; }
      }
      delete array;
      LN=N=0;array=NULL;
    }

    void resize(int i) 
    {
     T **A=array;
     int c,n=i*2+1;

     array=new T *[n];

//       if (array==NULL) { THROW_FATAL("Out of memory resizing array"); }
       for(c=0;c<N;c++) { array[c]=A[c]; }
       for(c=N,N=n;c<N;c++) { array[c]=NULL; }
       if (A!=NULL) { delete A; }
    }
  public:
    harray()
    { LN=N=0;array=NULL; }

   ~harray()
    { cleanup(); }
  public:
    void swap(int i,int j)
    { T *h=array[i];
      array[i]=array[j];
      array[j]=h;
    }

    int len(void) 	
    { return LN; }
    int length(void) 	
    { return LN; }
  public:
    void operator =(harray<T> & A)
    {int i,l;
      cleanup();
      for(i=0,l=A.len();i<l;i++) {
        (*this)[i]=A[i];
      }
    }
  public:
    void insert(int I,T & v)
    {int i;
      if (I>=len()) {
        (*this)[I]=v;
      }
      else {
        if (len()==N) { resize(len()+1); }
        for(i=len();i>I;i--) {
          array[i]=array[i-1];
        }
        array[i]=NULL;
        (*this)[i]=v;
        LN+=1;
      }
    }
  public:
    T & operator [](int i)
    {
      //if (i<0) THROW_FATAL("Index out of range (<0)");
      if (i>=N) { resize(i); }
      if (array[i]==NULL) {
        array[i]=new T;
//        if (array[i]==NULL) {THROW_FATAL("Out of memory allocating element"); }
      }
      if (i>=LN) { LN=i+1; }
      return array[i][0];
    }
  public:
    void initialize(void)
    { cleanup(); }
    void reinitialize(void) 
    { cleanup(); }
  protected:
    void decrease(int n=1)  { LN-=n; }
};

template <class T>
void sort(harray<T> & A,bool decending=false,int l=-1,int h=-1)
{
  if (l==-1) { l=0;h=A.len(); }

  if ((h-l)<=1) { return; }
  else {int lm,hm;
    {int i=((h-l)>>1)+l,j,n=h-1;
       A.swap(i,n);
       T & An=A[n];
       if (decending) {
         for(i=j=l;i<n;i++) {
           if (A[i]>An) { A.swap(i,j);j+=1; }
         }
       }
       else {
         for(i=j=l;i<n;i++) {
           if (A[i]<An) { A.swap(i,j);j+=1; }
         }
       }
       hm=j;
       for(i=j;i<n;i++) {
         if (A[i]==An) { A.swap(i,j);j+=1; }
       }
       A.swap(j,n);
       lm=j+1;
    }
    sort(A,decending,lm,h);
    sort(A,decending,l,hm);
  }
}


#endif
