/* Copyright (C) 2006-2018 PUC-Rio/Laboratorio TeleMidia

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
along with Ginga.  If not, see <https://www.gnu.org/licenses/>.  */

#include "LuaAPI.h"
#include "Event.h"

const char *LuaAPI::EVENT = "Ginga.Event";

const char *const LuaAPI::_Event_optTypes[] =
  {"attribution", "presentation", "selection", NULL};

Event::Type
LuaAPI::_Event_getOptIndexType (int i)
{
  switch (i)
    {
    case 0:
      return Event::ATTRIBUTION;
    case 1:
      return Event::PRESENTATION;
    case 2:
      return Event::SELECTION;
    default:
      g_assert_not_reached ();
    }
}

void
LuaAPI::_Event_attachWrapper (lua_State *L, Event *evt)
{
  static const struct luaL_Reg funcs[] =
    {
     {"__tostring",            LuaAPI::__l_Event_toString},
     {"__getUnderlyingObject", LuaAPI::__l_Event_getUnderlyingObject},
     {"getType",               LuaAPI::l_Event_getType},
     {"getObject",             LuaAPI::l_Event_getObject},
     {"getId",                 LuaAPI::l_Event_getId},
     {"getState",              LuaAPI::l_Event_getState},
     {NULL, NULL},
    };
  Event **wrapper;

  g_return_if_fail (L != NULL);
  g_return_if_fail (evt != NULL);

  // Load and initialize metatable, if not loaded yet.
  LuaAPI::loadLuaWrapperMt (L, funcs, LuaAPI::EVENT,
                            (const char *) LuaAPI::Event_initMt_lua,
                            (size_t) LuaAPI::Event_initMt_lua_len);

  wrapper = (Event **) lua_newuserdata (L, sizeof (Event **));
  g_assert_nonnull (wrapper);
  *wrapper = evt;
  luaL_setmetatable (L, LuaAPI::EVENT);

  // Set LUA_REGISTRY[evt]=wrapper.
  LuaAPI::attachLuaWrapper (L, evt);

  // Call evt:__attachData().
  LuaAPI::callLuaWrapper (L, evt, "_attachData", 0, 0);

  // Call evt.object:_addEvent (evt).
  LuaAPI::pushLuaWrapper (L, evt);
  LuaAPI::callLuaWrapper (L, evt->getObject (), "_addEvent", 1, 0);
}

void
LuaAPI::_Event_detachWrapper (lua_State *L, Event *evt)
{
  g_return_if_fail (L != NULL);
  g_return_if_fail (evt != NULL);

  // Call evt:__detachData().
  LuaAPI::callLuaWrapper (L, evt, "_detachData", 0, 0);

  // Call evt.object:_removeEvent (evt).
  LuaAPI::pushLuaWrapper (L, evt);
  LuaAPI::callLuaWrapper (L, evt->getObject (), "_removeEvent", 1, 0);

  // Set LUA_REGISTRY[evt] = nil.
  LuaAPI::detachLuaWrapper (L, evt);
}

Event *
LuaAPI::_Event_check (lua_State *L, int i)
{
  return *((Event **) luaL_checkudata (L, i, LuaAPI::EVENT));
}

int
LuaAPI::__l_Event_toString (lua_State *L)
{
  Event *evt;

  evt = LuaAPI::_Event_check (L, 1);
  lua_pushstring (L, evt->toString ().c_str ());

  return 1;
}

int
LuaAPI::__l_Event_getUnderlyingObject (lua_State *L)
{
  lua_pushlightuserdata (L, LuaAPI::_Event_check (L, 1));
  return 1;
}

int
LuaAPI::l_Event_getType (lua_State *L)
{
  Event *evt;
  const char *type;

  evt = LuaAPI::_Event_check (L, 1);
  type = Event::getTypeAsString (evt->getType ()).c_str ();
  lua_pushstring (L, type);

  return 1;
}

int
LuaAPI::l_Event_getObject (lua_State *L)
{
  Event *evt;

  evt = LuaAPI::_Event_check (L, 1);
  LuaAPI::pushLuaWrapper (L, evt->getObject ());

  return 1;
}

int
LuaAPI::l_Event_getId (lua_State *L)
{
  Event *evt;

  evt = LuaAPI::_Event_check (L, 1);
  lua_pushstring (L, evt->getId ().c_str ());

  return 1;
}

int
LuaAPI::l_Event_getState (lua_State *L)
{
  Event *evt;

  evt = LuaAPI::_Event_check (L, 1);
  switch (evt->getState ())
    {
    case Event::OCCURRING:
      lua_pushliteral (L, "occurring");
      break;
    case Event::PAUSED:
      lua_pushliteral (L, "paused");
      break;
    case Event::SLEEPING:
      lua_pushliteral (L, "sleeping");
      break;
    default:
      g_assert_not_reached ();
    }

  return 1;
}
