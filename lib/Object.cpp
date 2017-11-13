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

#include "aux-ginga.h"
#include "Object.h"

#include "Composition.h"
#include "Event.h"
#include "Media.h"
#include "MediaSettings.h"
#include "Switch.h"

GINGA_NAMESPACE_BEGIN


// Public.

Object::Object (const string &id)
{
  _id = id;
  _formatter = nullptr;
  _parent = nullptr;
  _time = GINGA_TIME_NONE;

  this->addPresentationEvent ("@lambda", 0, GINGA_TIME_NONE);
  _lambda = this->getPresentationEvent ("@lambda");
  g_assert_nonnull (_lambda);
}

Object::~Object ()
{
  this->doStop ();
}

string
Object::getId ()
{
  return _id;
}

Formatter *
Object::getFormatter ()
{
  return _formatter;
}

void
Object::initFormatter (Formatter *formatter)
{
  g_assert_nonnull (formatter);
  g_assert_null (_formatter);
  _formatter = formatter;
}

Composition *
Object::getParent ()
{
  return _parent;
}

void
Object::initParent (Composition *parent)
{
  g_assert_nonnull (parent);
  g_assert_null (_parent);
  _parent = parent;
}

const vector <string> *
Object::getAliases ()
{
  return &_aliases;
}

bool
Object::hasAlias (const string &alias)
{
  for (auto curr: _aliases)
    if (curr == alias)
      return true;
  return false;
}

void
Object::addAlias (const string &alias)
{
  tryinsert (alias, _aliases, push_back);
}


Event *
Object::obtainEvent (Event::Type type, NclAnchor *anchor,
                     const string &key)
{
  Event *event;

  if (type == Event::SELECTION)
    event = this->getEvent (type, key);
  else
    event = this->getEvent (type, anchor->getId ());

  if (event != nullptr)
    return event;

  g_assert (instanceof (Media *, this));

  switch (type)
    {
    case Event::PRESENTATION:
      {
        NclArea *area = cast (NclArea *, anchor);
        g_assert_nonnull (area);
        this->addPresentationEvent (anchor->getId (),
                                    area->getBegin (),
                                    area->getEnd ());
        event = this->getPresentationEvent (anchor->getId ());
        g_assert_nonnull (event);
        break;
      }
    case Event::ATTRIBUTION:
      {
        NclProperty *property = cast (NclProperty *, anchor);
        g_assert_nonnull (property);
        this->addAttributionEvent (property->getId ());
        event = this->getAttributionEvent (property->getId ());
        g_assert_nonnull (event);
        event->setParameter ("value", property->getValue ());
        break;
      }
    case Event::SELECTION:
      {
        this->addSelectionEvent (key);
        event = this->getSelectionEvent (key);
        g_assert_nonnull (event);
        event->setParameter ("key", key);
        break;
      }
    default:
      g_assert_not_reached ();
    }

  g_assert_nonnull (event);
  return event;
}

Event *
Object::getEvent (Event::Type type, const string &id)
{
  for (auto evt: _events)
    if (evt->getType () == type && evt->getId () == id)
      return evt;
  return nullptr;
}

Event *
Object::getAttributionEvent (const string &propName)
{
  return this->getEvent (Event::ATTRIBUTION, propName);
}

void
Object::addAttributionEvent (const string &propName)
{
  Event *evt;

  if (this->getAttributionEvent (propName))
    return;

  evt = new Event (Event::ATTRIBUTION, this, propName);
  _events.insert (evt);
}

Event *
Object::getPresentationEvent (const string &id)
{
  return this->getEvent (Event::PRESENTATION, id);
}

void
Object::addPresentationEvent (const string &id, Time begin, Time end)
{
  Event *evt;

  if (this->getPresentationEvent (id))
    return;

  evt = new Event (Event::PRESENTATION, this, id);
  evt->setInterval (begin, end);
  _events.insert (evt);
}

Event *
Object::getSelectionEvent (const string &key)
{
  return this->getEvent (Event::SELECTION, key);
}

void
Object::addSelectionEvent (const string &key)
{
  Event *evt;

  if (this->getSelectionEvent (key))
    return;

  evt = new Event (Event::SELECTION, this, key);
  _events.insert (evt);
}

Event *
Object::getLambda ()
{
  g_assert_nonnull (_lambda);
  return _lambda;
}

bool
Object::isOccurring ()
{
  g_assert_nonnull (_lambda);
  return _lambda->getState () == Event::OCCURRING;
}

bool
Object::isPaused ()
{
  g_assert_nonnull (_lambda);
  return _lambda->getState () == Event::PAUSED;
}

bool
Object::isSleeping ()
{
  g_assert_nonnull (_lambda);
  return _lambda->getState () == Event::SLEEPING;
}

string
Object::getProperty (const string &name)
{
  auto it = _properties.find (name);
  if (it == _properties.end ())
    return "";
  return it->second;
}

void
Object::setProperty (const string &name, const string &value, Time dur)
{
  g_assert (GINGA_TIME_IS_VALID (dur));
  _properties[name] = value;
}

bool
Object::getData (const string &key, void **value)
{
  MAP_GET_IMPL (_userdata, key, value);
}

bool
Object::setData (const string &key, void *value)
{
  MAP_SET_IMPL (_userdata, key, value);
}

list<pair<Action, Time>> *
Object::getDelayedActions ()
{
  return &_delayed;
}

void
Object::addDelayedAction (Event *event, Event::Transition transition,
                          const string &value, Time delay)
{
  Action act;

  act.event = event;
  act.transition = transition;
  act.value = value;
  _delayed.push_back (std::make_pair (act, _time + delay));
}

void
Object::sendKeyEvent (unused (const string &key), unused (bool press))
{
}

void
Object::sendTickEvent (unused (Time total), Time diff, unused (Time frame))
{
  g_assert (this->isOccurring ());
  g_assert (GINGA_TIME_IS_VALID (_time));
  _time += diff;

  list<Action> trigger;
  for (auto it: _delayed)
    {
      if (_time >= it.second)
        {
          it.second = GINGA_TIME_NONE;
          trigger.push_back (it.first);
        }
    }

  for (auto action: trigger)
    {
      _formatter->evalAction (action);
      if (!this->isOccurring ())
        return;
    }

  for (auto it = _delayed.begin (); it != _delayed.end ();)
    {
      if (it->second == GINGA_TIME_NONE)
        it = _delayed.erase (it);
      else
        ++it;
    }
}


// Private.

void
Object::doStart ()
{
  _time = 0;
}

void
Object::doStop ()
{
  _time = GINGA_TIME_NONE;
  for (auto evt: _events)
    evt->reset ();
  _delayed.clear ();
}

GINGA_NAMESPACE_END
