/***************************************************************************
                          baseexception.h  -  description
                             -------------------
    begin                : Don Aug 1 2002
    copyright            : (C) 2002 by Selzer Michael
    email                : selzer@student.uni-kl.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BASEEXCEPTION_H
#define BASEEXCEPTION_H

// C++ includes
#include <string>

using namespace std;
/**This class is the base for every exception in libpv
  *@author Selzer Michael
  */

namespace CasioPV {

class BaseException {
	public:
		/**
		   * Constructor.
		   */
		BaseException( string message, unsigned int errorcode );
		/**
		   * Destructor.
		   */
		virtual ~BaseException();

		virtual string getMessage();

		virtual unsigned int getErrorCode();

	private:
		string m_message;
		unsigned int m_errorcode;

};

}; // namespace CasioPV

#endif
