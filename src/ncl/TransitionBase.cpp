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

#include "ginga.h"
#include "TransitionBase.h"

GINGA_NCL_BEGIN

TransitionBase::TransitionBase (const string &id) : Base (id)
{
  _transitionSet = new vector<Transition *>;
}

TransitionBase::~TransitionBase ()
{
  vector<Transition *>::iterator i;

  if (_transitionSet != NULL)
    {
      i = _transitionSet->begin ();
      while (i != _transitionSet->end ())
        {
          delete *i;
          ++i;
        }

      delete _transitionSet;
      _transitionSet = NULL;
    }
}

bool
TransitionBase::addTransition (Transition *transition)
{
  if (transition == NULL)
    {
      return false;
    }

  vector<Transition *>::iterator i;
  i = _transitionSet->begin ();
  while (i != _transitionSet->end ())
    {
      if (*i == transition)
        {
          return false;
        }
      ++i;
    }

  _transitionSet->push_back (transition);
  return true;
}

bool
TransitionBase::addBase (Base *base, const string &alias, const string &location)
{
  if (instanceof (TransitionBase *, base))
    {
      return Base::addBase (base, alias, location);
    }
  return false;
}

void
TransitionBase::clear ()
{
  _transitionSet->clear ();
  Base::clear ();
}

Transition *
TransitionBase::getTransitionLocally (const string &transitionId)
{
  vector<Transition *>::iterator i;
  Transition *transition;

  i = _transitionSet->begin ();
  while (i != _transitionSet->end ())
    {
      transition = *i;
      if (transition->getId () == transitionId)
        {
          return transition;
        }
      ++i;
    }
  return NULL;
}

Transition *
TransitionBase::getTransition (const string &transitionId)
{
  string::size_type index;
  string prefix, suffix;
  TransitionBase *base;

  index = transitionId.find_first_of ("#");
  if (index == std::string::npos)
    {
      return getTransitionLocally (transitionId);
    }

  prefix = transitionId.substr (0, index);
  index++;
  suffix = transitionId.substr (index, transitionId.length () - index);
  if (_baseAliases.count (prefix) != 0)
    {
      base = (TransitionBase *)(_baseAliases[prefix]);
      return base->getTransition (suffix);
    }
  else if (_baseLocations.count (prefix) != 0)
    {
      base = (TransitionBase *)(_baseLocations[prefix]);
      return base->getTransition (suffix);
    }
  else
    {
      return NULL;
    }
}

vector<Transition *> *
TransitionBase::getTransitions ()
{
  return _transitionSet;
}

bool
TransitionBase::removeTransition (Transition *transition)
{
  vector<Transition *>::iterator i;
  i = _transitionSet->begin ();
  while (i != _transitionSet->end ())
    {
      if (*i == transition)
        {
          _transitionSet->erase (i);
          return true;
        }
      ++i;
    }

  return false;
}

GINGA_NCL_END
