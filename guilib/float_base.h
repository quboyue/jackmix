/*
    Copyright ( C ) 2004 Arnold Krille <arnold@arnoldarts.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation;
    version 2 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef FLOATBASE_H
#define FLOATBASE_H

#include "common.h"

class FloatElement {
public:
//	FloatElement();
//	~FloatElement();
public:
	virtual void setPrecision( int ) =0;
	virtual void setPageStep( float ) =0;
	virtual void setMinimum( float ) =0;
	virtual void setMaximum( float ) =0;
	virtual void setValue( float ) =0;
//signals:
//	void valueChanged( float );
};

#endif
