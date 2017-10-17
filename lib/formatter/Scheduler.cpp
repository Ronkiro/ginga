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
#include "Scheduler.h"
#include "Converter.h"

GINGA_FORMATTER_BEGIN


// Public.

Scheduler::Scheduler (GingaInternal *ginga)
{
  g_assert_nonnull (ginga);
  _ginga = ginga;
  _converter = nullptr;
  _doc = nullptr;
  _settings = nullptr;
}

Scheduler::~Scheduler ()
{
  for (auto obj: _objects)
    delete obj;
  _objects.clear ();

  if (_converter != nullptr)
    delete _converter;
}

bool
Scheduler::run (NclDocument *doc)
{
  string id;
  Context *body;
  const vector<Port *> *ports;
  vector<NclEvent *> *entryevts;

  g_assert_nonnull (doc);
  _doc = doc;

  id = _doc->getId ();
  body = _doc->getRoot ();
  g_assert_nonnull (body);

  // Get entry events (i.e., those mapped by ports).
  ports = body->getPorts ();
  g_assert_nonnull (ports);
  if (unlikely (ports->size () == 0))
    {
      WARNING ("document has no ports");
      return false;
    }

  // Insert dummy settings node.
  Media *dummy =  new Media (_doc, "__settings__", true);
  body->addNode (dummy);
  Property *prop = new Property (_doc, "service.currentFocus");
  prop->setValue ("");
  dummy->addAnchor (prop);

  // Create and load converter.
  _converter = new Converter (_ginga, new RuleAdapter ());
  entryevts = new vector<NclEvent *>;
  for (auto port: *ports)
    {
      Node *node;
      Anchor *iface;
      ExecutionObject *obj;
      NclEvent *evt;

      port->getTarget (&node, &iface);
      obj = _converter->obtainExecutionObject (node);
      g_assert_nonnull (obj);

      evt = _converter->obtainEvent
        (obj, iface, instanceof (Property *, iface)
         ? EventType::ATTRIBUTION : EventType::PRESENTATION, "");
      g_assert_nonnull (evt);
      entryevts->push_back (evt);
    }
  g_assert_false (entryevts->empty ()); // doc has no ports

  // Create execution object for settings node and initialize it.
  ExecutionObjectSettings *settings = nullptr;
  vector <Node *> *nodes = _doc->getSettingsNodes ();
  for (auto node: *nodes)
    {
      Media *content;

      if (settings == nullptr)
        {
          ExecutionObject *obj;
          obj = _converter->obtainExecutionObject (node);
          g_assert_nonnull (obj);
          settings = cast (ExecutionObjectSettings *, obj);
          g_assert_nonnull (settings);
        }

      content = (Media *) node;
      for (auto anchor: *content->getAnchors ())
        {
          Property *prop;
          string name;
          string value;

          if (!instanceof (Property *, anchor))
            continue;           // nothing to do

          prop = cast (Property *, anchor);
          name = prop->getName ();
          value = prop->getValue ();
          if (value == "")
            continue;           // nothing to do

          cast (ExecutionObject *, settings)
            ->setProperty (name, "", value, 0);
        }
    }
  delete nodes;

  // Set global settings object.
  g_assert_nonnull (settings);
  _settings = settings;

  // Start entry events.
  for (auto event: *entryevts)
    {
      NclAction *fakeAction;
      fakeAction = new NclAction (event, EventStateTransition::START, this);
      runAction (event, fakeAction);
      delete fakeAction;
    }
  delete entryevts;

  // Refresh current focus.
  settings->updateCurrentFocus ("");

  // Success.
  return true;
}

ExecutionObjectSettings *
Scheduler::getSettings ()
{
  return _settings;
}

const set<ExecutionObject *> *
Scheduler::getObjects ()
{
  return &_objects;
}

ExecutionObject *
Scheduler::getObjectById (const string &id)
{
  for (auto obj: _objects)
    if (obj->getId () == id)
      return obj;
  return nullptr;
}

ExecutionObject *
Scheduler::getObjectByIdOrAlias (const string &id)
{
  ExecutionObject *obj;
  if ((obj = this->getObjectById (id)) != nullptr)
    return obj;
  for (auto obj: _objects)
    if (obj->hasAlias (id))
      return obj;
  return nullptr;
}

bool
Scheduler::getObjectPropertyByRef (const string &ref, string *result)
{
  size_t i;
  string id;
  string name;
  ExecutionObject *object;

  if (ref[0] != '$' || (i = ref.find ('.')) == string::npos)
    return false;

  id = ref.substr (1, i - 1);
  name = ref.substr (i + 1);
  object = this->getObjectByIdOrAlias (id);
  if (object == nullptr)
    return false;

  tryset (result, object->getProperty (name));
  return true;
}

bool
Scheduler::addObject (ExecutionObject *obj)
{
  g_assert_nonnull (obj);
  if (_objects.find (obj) != _objects.end ()
      || getObjectByIdOrAlias (obj->getId ()) != nullptr)
    {
      return false;
    }
  _objects.insert (obj);
  return true;
}

void
Scheduler::sendTickEvent (GingaTime total, GingaTime diff, GingaTime frame)
{
  vector<ExecutionObject *> buf;
  for (auto obj: _objects)
    if (obj->isOccurring ())
      buf.push_back (obj);
  for (auto obj: buf)
    {
      g_assert (!instanceof (ExecutionObjectSettings *, obj));
      obj->sendTickEvent (total, diff, frame);
    }
  _settings->sendTickEvent (total, diff, frame);
}

void
Scheduler::sendKeyEvent (const string &key, bool press)
{
  vector<ExecutionObject *> buf;
  for (auto obj: _objects)
    if (instanceof (ExecutionObjectSettings *, obj) || obj->isOccurring ())
      buf.push_back (obj);
  for (auto obj: buf)
    obj->sendKeyEvent (key, press);
}

void
Scheduler::scheduleAction (NclAction *action)
{
  runAction (action->getEvent (), action);
}


// Private.

void
Scheduler::runAction (NclEvent *event, NclAction *action)
{
  ExecutionObject *obj;
  string name;

  obj = event->getExecutionObject ();
  g_assert_nonnull (obj);

  name = EventUtil::getEventStateTransitionAsString
    (action->getEventStateTransition ());

  TRACE ("running %s over %s",
         name.c_str (), obj->getId ().c_str ());

  if (instanceof (SelectionEvent *, event))
    {
      event->start ();
      delete action;
      return;
    }

  if (instanceof (ExecutionObjectSwitch *, obj)
      && instanceof (SwitchEvent *, event))
    {
      this->runActionOverSwitch ((ExecutionObjectSwitch *) obj,
                                 (SwitchEvent *) event, action);
      return;
    }

  if (instanceof (ExecutionObjectContext *, obj))
    {
      this->runActionOverComposition
        ((ExecutionObjectContext *) obj, action);
      return;
    }

  if (instanceof (AttributionEvent *, event))
    {
      AttributionEvent *attevt;
      Property *property;

      string name;
      string from;
      string to;

      GingaTime dur;

      g_assert (action->getEventStateTransition ()
                == EventStateTransition::START);

      attevt = (AttributionEvent *) event;
      if (event->getCurrentState () != EventState::SLEEPING)
        return;                 // nothing to do

      property = cast (Property *, attevt->getAnchor ());
      g_assert_nonnull (property);

      name = property->getName ();
      from = property->getValue ();
      to = action->getValue ();
      if (to[0] == '$')
        this->getObjectPropertyByRef (to, &to);

      string s;
      s = action->getDuration ();
      if (s[0] == '$')
        this->getObjectPropertyByRef (s, &s);
      dur = ginga_parse_time (s);

      attevt->start ();
      obj->setProperty (name, from, to, dur);

      // TODO: Wrap this in a closure to be called at the end of animation.
      attevt->stop ();
      return;
    }

  switch (action->getEventStateTransition ())
    {
    case EventStateTransition::START:
      obj->prepare (event);
      g_assert (obj->start ());
      break;
    case EventStateTransition::STOP:
      obj->stop ();
      break;
    case EventStateTransition::PAUSE:
      g_assert (obj->pause ());
      break;
    case EventStateTransition::RESUME:
      g_assert (obj->resume ());
      break;
    case EventStateTransition::ABORT:
      g_assert (obj->abort ());
      break;
    default:
      g_assert_not_reached ();
    }
}

void
Scheduler::runActionOverComposition (ExecutionObjectContext *ctxObj,
                                     NclAction *action)
{
  NclEvent *event;
  EventType type;
  EventStateTransition acttype;

  Node *node;
  Entity *entity;
  Composition *compNode;

  event = action->getEvent ();
  g_assert_nonnull (event);

  type = event->getType ();

  if (type == EventType::ATTRIBUTION)
    {
      ERROR_NOT_IMPLEMENTED
        ("context property attribution is not supported");
    }

  if (type == EventType::SELECTION)
    {
      WARNING ("trying to select composition '%s'",
               ctxObj->getId ().c_str ());
      return;
    }

  node = ctxObj->getNode ();
  g_assert_nonnull (node);

  entity = cast (Entity *, node);
  g_assert_nonnull (entity);

  compNode = cast (Composition *, entity);
  g_assert_nonnull (compNode);

  acttype = action->getEventStateTransition ();
  if (acttype == EventStateTransition::START) // start all ports
    {
      //ctxObj->suspendLinkEvaluation (false);
      for (auto port: *compNode->getPorts ())
        {
          Node *node;
          Anchor *iface;
          ExecutionObject *child;
          NclEvent *evt;

          port->getTarget (&node, &iface);
          child = _converter->obtainExecutionObject (node);
          g_assert_nonnull (child);

          if (!instanceof (Area *, iface))
            continue;           // nothing to do

          evt = _converter->obtainEvent (child, iface,
                                         EventType::PRESENTATION, "");
          g_assert_nonnull (evt);
          g_assert (instanceof (PresentationEvent *, evt));

          runAction (evt, action);

        }
    }
  else if (acttype == EventStateTransition::STOP) // stop all children
    {
      //ctxObj->suspendLinkEvaluation (true);
      for (auto child: *ctxObj->getChildren ())
        {
          NclEvent *evt;
          evt = child->getLambda ();
          if (evt == nullptr)
            continue;
          runAction (evt, action);
        }
      //ctxObj->suspendLinkEvaluation (false);
    }
  else if (acttype == EventStateTransition::ABORT)
    {
      ERROR_NOT_IMPLEMENTED ("action 'abort' is not supported");
    }
  else if (acttype == EventStateTransition::PAUSE)
    {
      ERROR_NOT_IMPLEMENTED ("action 'pause' is not supported");
    }
  else if (acttype == EventStateTransition::RESUME)
    {
      ERROR_NOT_IMPLEMENTED ("action 'resume' is not supported");
    }
  else
    {
      g_assert_not_reached ();
    }
}

void
Scheduler::runActionOverSwitch (ExecutionObjectSwitch *switchObj,
                                SwitchEvent *event,
                                NclAction *action)
{
  ExecutionObject *selectedObject;
  NclEvent *selectedEvent;

  selectedObject = switchObj->getSelectedObject ();
  if (selectedObject == nullptr)
    {
      selectedObject = _converter->processExecutionObjectSwitch (switchObj);
      g_assert_nonnull (selectedObject);
    }

  selectedEvent = event->getMappedEvent ();
  if (selectedEvent != nullptr)
    {
      runAction (selectedEvent, action);
    }
  else
    {
      runSwitchEvent (switchObj, event, selectedObject, action);
    }

  if (action->getEventStateTransition () == EventStateTransition::STOP
      || action->getEventStateTransition () == EventStateTransition::ABORT)
    {
      switchObj->select (nullptr);
    }
}

void
Scheduler::runSwitchEvent (unused (ExecutionObjectSwitch *switchObj),
                           SwitchEvent *switchEvent,
                           ExecutionObject *selectedObject,
                           NclAction *action)
{
  NclEvent *selectedEvent;
  SwitchPort *switchPort;
  vector<Port *>::iterator i;
  ExecutionObject *endPointObject;

  selectedEvent = nullptr;
  switchPort = (SwitchPort *)(switchEvent->getAnchor ());
  for (auto mapping: *switchPort->getPorts ())
    {
      if (mapping->getNode () != selectedObject->getNode ())
        continue;

      Node *node;
      Anchor *iface;
      mapping->getTarget (&node, &iface);

      endPointObject = _converter->obtainExecutionObject (node);
      g_assert_nonnull (endPointObject);
      selectedEvent = _converter
        ->obtainEvent (endPointObject, iface, switchEvent->getType (),
                       switchEvent->getKey ());
      break;
    }

  if (selectedEvent != nullptr)
    {
      switchEvent->setMappedEvent (selectedEvent);
      runAction (selectedEvent, action);
    }
}

GINGA_FORMATTER_END
