/****************************************************************************
** Copyright (C) 2001-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Tools library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
**********************************************************************/

#ifndef __KDABTOOLS__CPP__CHECKER_H__
#define __KDABTOOLS__CPP__CHECKER_H__

#include "kdtoolsglobal.h"

#include <cstdlib>

/**
   Usage:

   \code
   class Foo ... {
      KDAB_MAKE_CHECKABLE( Foo )
   public:
      // ...
   };

   Foo::Foo() {
      KDAB_CHECK_CTOR;
      // ...
   }

   Foo::~Foo() {
      KDAB_CHECK_DTOR;
      // ...
   }

   KDAB_DEFINE_CHECKS( Foo ) {
      assert( something );
      assert( another thing );
   }

   void Foo::setBar( int bar ) {
      KDAB_CHECK_THIS; // include as first line in every method

      // ...
   }

   int Foo::bar() const {
      KDAB_CHECK_THIS;

      // ...
   }
   \endcode
*/
class __KDAB__CheckerImplBase
{
public:
    virtual ~__KDAB__CheckerImplBase() {}
    virtual void checkInvariants() const = 0;
};

template <typename T_Class>
class __KDAB__CheckerImpl : public __KDAB__CheckerImplBase
{
    KDAB_DISABLE_COPY(__KDAB__CheckerImpl);
    const T_Class *const p;
public:
    __KDAB__CheckerImpl(const T_Class *t)
        : __KDAB__CheckerImplBase(), p(t) {}
    void checkInvariants() const
    {
        try {
            p->__KDAB_Checker__checkInvariants__();
        } catch (...) {
            std::abort();
        }
    }
};

template <bool check_in_ctor, bool check_in_dtor>
class __KDAB__Checker
{
    KDAB_DISABLE_COPY(__KDAB__Checker);
    const __KDAB__CheckerImplBase *const checker;
public:
    template <typename T_Class>
    __KDAB__Checker(const T_Class *t)
        : checker(new __KDAB__CheckerImpl<T_Class>(t))
    {
        if (check_in_ctor) {
            checker->checkInvariants();
        }
    }
    ~__KDAB__Checker()
    {
        if (check_in_dtor) {
            checker->checkInvariants();
        }
        delete checker;
    }
};

#define KDAB_MAKE_CHECKABLE( Class ) \
    private:                         \
    void __KDAB_Checker__checkInvariants__() const;    \
    friend class __KDAB__CheckerImpl<Class>;

#define KDAB_DEFINE_CHECKS( Class )                             \
    void Class::__KDAB_Checker__checkInvariants__() const

#ifndef NDEBUG
# define __KDAB_CHECK_HELPER__( ctor, dtor ) \
    const __KDAB__Checker<ctor,dtor> __checker_uglified__( this )
#else
# define __KDAB_CHECK_HELPER__( ctor, dtor ) \
    do {} while (0)
#endif

#define KDAB_CHECK_THIS __KDAB_CHECK_HELPER__( true, true )
#define KDAB_CHECK_CTOR __KDAB_CHECK_HELPER__( false, true )
#define KDAB_CHECK_DTOR __KDAB_CHECK_HELPER__( true, false )

#endif /* __KDABTOOLS__CPP__CHECKER_H__ */

