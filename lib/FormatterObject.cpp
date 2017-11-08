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
#include "FormatterObject.h"

#include "FormatterComposition.h"
#include "FormatterEvent.h"
#include "FormatterMedia.h"
#include "FormatterMediaSettings.h"
#include "FormatterSwitch.h"

GINGA_NAMESPACE_BEGIN


// Public.

FormatterObject::FormatterObject (const string &id)
{
  _id = id;
  _formatter = nullptr;
  _parent = nullptr;
  _time = GINGA_TIME_NONE;

  this->addPresentationEvent ("@lambda", 0, GINGA_TIME_NONE);
  _lambda = this->getPresentationEvent ("@lambda");
  g_assert_nonnull (_lambda);
}

FormatterObject::~FormatterObject ()
{
  this->doStop ();
}

string
FormatterObject::getId ()
{
  return _id;
}

Formatter *
FormatterObject::getFormatter ()
{
  return _formatter;
}

void
FormatterObject::initFormatter (Formatter *formatter)
{
  g_assert_nonnull (formatter);
  g_assert_null (_formatter);
  _formatter = formatter;
}

FormatterComposition *
FormatterObject::getParent ()
{
  return _parent;
}

void
FormatterObject::initParent (FormatterComposition *parent)
{
  g_assert_nonnull (parent);
  g_assert_null (_parent);
  _parent = parent;
}

const vector <string> *
FormatterObject::getAliases ()
{
  return &_aliases;
}

bool
FormatterObject::hasAlias (const string &alias)
{
  for (auto curr: _aliases)
    if (curr == alias)
      return true;
  return false;
}

void
FormatterObject::addAlias (const string &alias)
{
  tryinsert (alias, _aliases, push_back);
}


FormatterEvent *
FormatterObject::obtainEvent (FormatterEvent::Type type, NclAnchor *anchor,
                              const string &key)
{
  FormatterEvent *event;

  if (type == FormatterEvent::Type::SELECTION)
    event = this->getEvent (type, key);
  else
    event = this->getEvent (type, anchor->getId ());

  if (event != nullptr)
    return event;

  g_assert (instanceof (FormatterMedia *, this));

  switch (type)
    {
    case FormatterEvent::Type::PRESENTATION:
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
    case FormatterEvent::Type::ATTRIBUTION:
      {
        NclProperty *property = cast (NclProperty *, anchor);
        g_assert_nonnull (property);
        this->addAttributionEvent (property->getId ());
        event = this->getAttributionEvent (property->getId ());
        g_assert_nonnull (event);
        event->setParameter ("value", property->getValue ());
        break;
      }
    case FormatterEvent::Type::SELECTION:
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

FormatterEvent *
FormatterObject::getEvent (FormatterEvent::Type type, const string &id)
{
  for (auto evt: _events)
    if (evt->getType () == type && evt->getId () == id)
      return evt;
  return nullptr;
}

FormatterEvent *
FormatterObject::getAttributionEvent (const string &propName)
{
  return this->getEvent (FormatterEvent::Type::ATTRIBUTION, propName);
}

void
FormatterObject::addAttributionEvent (const string &propName)
{
  FormatterEvent *evt;

  if (this->getAttributionEvent (propName))
    return;

  evt = new FormatterEvent (FormatterEvent::Type::ATTRIBUTION, this, propName);
  _events.insert (evt);
}

FormatterEvent *
FormatterObject::getPresentationEvent (const string &id)
{
  return this->getEvent (FormatterEvent::Type::PRESENTATION, id);
}

void
FormatterObject::addPresentationEvent (const string &id, GingaTime begin,
                                       GingaTime end)
{
  FormatterEvent *evt;

  if (this->getPresentationEvent (id))
    return;

  evt = new FormatterEvent (FormatterEvent::Type::PRESENTATION, this, id);
  evt->setInterval (begin, end);
  _events.insert (evt);
}

FormatterEvent *
FormatterObject::getSelectionEvent (const string &key)
{
  return this->getEvent (FormatterEvent::Type::SELECTION, key);
}

void
FormatterObject::addSelectionEvent (const string &key)
{
  FormatterEvent *evt;

  if (this->getSelectionEvent (key))
    return;

  evt = new FormatterEvent (FormatterEvent::Type::SELECTION, this, key);
  _events.insert (evt);
}

FormatterEvent *
FormatterObject::getLambda ()
{
  g_assert_nonnull (_lambda);
  return _lambda;
}

bool
FormatterObject::isOccurring ()
{
  g_assert_nonnull (_lambda);
  return _lambda->getState () == FormatterEvent::State::OCCURRING;
}

bool
FormatterObject::isPaused ()
{
  g_assert_nonnull (_lambda);
  return _lambda->getState () == FormatterEvent::State::PAUSED;
}

bool
FormatterObject::isSleeping ()
{
  g_assert_nonnull (_lambda);
  return _lambda->getState () == FormatterEvent::State::SLEEPING;
}

string
FormatterObject::getProperty (const string &name)
{
  auto it = _properties.find (name);
  if (it == _properties.end ())
    return "";
  return it->second;
}

void
FormatterObject::setProperty (const string &name, const string &value,
                              GingaTime dur)
{
  g_assert (GINGA_TIME_IS_VALID (dur));
  _properties[name] = value;
}

list<pair<FormatterAction *, GingaTime>> *
FormatterObject::getDelayedActions ()
{
  return &_delayed;
}

void
FormatterObject::addDelayedAction (FormatterEvent *event,
                                   FormatterEvent::Transition transition,
                                   const string &value,
                                   GingaTime delay)
{
  FormatterAction *action = new FormatterAction (event, transition);
  action->setParameter ("value", value);
  _delayed.push_back (std::make_pair (action, _time + delay));
}

void
FormatterObject::sendKeyEvent (unused (const string &key),
                               unused (bool press))
{
}

void
FormatterObject::sendTickEvent (unused (GingaTime total),
                                GingaTime diff,
                                unused (GingaTime frame))
{
  g_assert (this->isOccurring ());
  g_assert (GINGA_TIME_IS_VALID (_time));
  _time += diff;

  list<FormatterAction *> trigger;
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
        {
          delete it->first;
          it = _delayed.erase (it);
        }
      else
        {
          ++it;
        }
    }
}


// Private.

void
FormatterObject::doStart ()
{
  _time = 0;
}

void
FormatterObject::doStop ()
{
  _time = GINGA_TIME_NONE;
  for (auto evt: _events)
    evt->reset ();
  for (auto item: _delayed)
    delete item.first;
  _delayed.clear ();
}

GINGA_NAMESPACE_END
