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

#ifndef FORMATTER_CONDITION
#define FORMATTER_CONDITION

#include "FormatterEvent.h"

#include "ncl/Predicate.h"
using namespace ::ginga::ncl;

GINGA_FORMATTER_BEGIN

class FormatterCondition;
class IFormatterConditionListener
{
public:
  virtual void conditionSatisfied (FormatterCondition *) = 0;
};

class FormatterCondition: IFormatterEventListener
{
public:
  FormatterCondition (Predicate *, FormatterEvent *,
                      NclEventStateTransition);
  virtual ~FormatterCondition ();

  Predicate *getPredicate ();
  FormatterEvent *getEvent ();

  void setTriggerListener (IFormatterConditionListener *);
  void conditionSatisfied ();

  // IFormatterEventListener
  virtual void eventStateChanged (FormatterEvent *,
                                  NclEventStateTransition) override;
private:
  Predicate *_predicate;
  FormatterEvent *_event;
  NclEventStateTransition _transition;
  IFormatterConditionListener *_listener;
};

GINGA_FORMATTER_END

#endif // FORMATTER_CONDITION
