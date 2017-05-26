/* Copyright (C) 2006-2017 PUC-Rio/Laboratorio TeleMidia

This file is part of Ginga (Ginga-NCL).

Ginga is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Ginga is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License
along with Ginga.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _SIMPLERULE_H_
#define _SIMPLERULE_H_

#include "Rule.h"

GINGA_NCL_BEGIN

class SimpleRule : public Rule
{
private:
  string attribute;
  short ruleOperator;
  string value;

public:
  SimpleRule (const string &id, const string &attr, short op, const string &val);

  virtual ~SimpleRule (){};
  string getAttribute ();
  short getOperator ();
  string getValue ();
  void setOperator (short newOp);
  void setValue (const string &newValue);
  void setAttribute (const string &someAttribute);
};

GINGA_NCL_END

#endif //_SIMPLERULE_H_
