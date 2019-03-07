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

#ifndef GINGA_LUA_API_H
#define GINGA_LUA_API_H

#include "aux-ginga.h"
GINGA_BEGIN_DECLS
#include "aux-lua.h"
GINGA_END_DECLS

#include "Object.h"

GINGA_NAMESPACE_BEGIN

class Context;
class Document;
class Media;
class Switch;

/// The Lua interface to the internal model.
///
/// This class implements the Lua interface to the objects of the internal
/// model (Document, Object, Event, etc.).  We expose these C++ objects to
/// Lua using wrappers.  By _wrapper_ we mean a full Lua userdata consisting
/// of a single pointer, which points to the actual C++ object.  We call the
/// object associated with a Lua wrapper its _underlying object_.  We keep
/// the mapping between C++ objects and their wrappers in the
/// `LUA_REGISTRY`.  That is, for each C++ object `o` we set
/// `LUA_REGISTRY[o]=w`, where `w` is the Lua wrapper of object `o`.
///
class LuaAPI
{
public:

  /// Attaches Lua wrapper to \p doc.
  static void Document_attachWrapper (lua_State *L, Document *doc);

  /// Detaches Lua wrapper from \p doc.
  static void Document_detachWrapper (lua_State *L, Document *doc);

  /// Checks if the value at index \p i of stack is a Document wrapper.
  static Document *Document_check (lua_State *L, int i);

  //// Pushes the Lua wrapper of document onto stack.
  static void Document_push (lua_State *L, Document *doc);

  /// Calls a method of the Lua wrapper of the given document.
  static void Document_call (lua_State *L, Document *doc, const char *name,
                             int nargs, int nresults);

  /// Attaches Lua wrapper to \p ctx.
  static void Context_attachWrapper (lua_State *L, Context *ctx);

  /// Detaches Lua wrapper from \p ctx.
  static void Context_detachWrapper (lua_State *L, Context *ctx);

  /// Checks if the value at index \p i of stack is a Context wrapper.
  static Context *Context_check (lua_State *L, int i);

  /// Attaches Lua wrapper to \p swtch.
  static void Switch_attachWrapper (lua_State *L, Switch *swtch);

  /// Detaches Lua wrapper from \p swtch.
  static void Switch_detachWrapper (lua_State *L, Switch *swtch);

  /// Checks if the value at index \p i of stack is a Switch wrapper.
  static Switch *Switch_check (lua_State *L, int i);

  /// Attaches Lua wrapper to \p media.
  static void Media_attachWrapper (lua_State *L, Media *media);

  /// Detaches Lua wrapper from \p media.
  static void Media_detachWrapper (lua_State *L, Media *media);

  /// Checks if the value at index \p i of stack is a Media wrapper.
  static Media *Media_check (lua_State *L, int i);

  /// Attaches Lua wrapper to \p evt.
  static void Event_attachWrapper (lua_State *L, Event *evt);

  /// Detaches Lua wrapper from \p evt.
  static void Event_detachWrapper (lua_State *L, Event *evt);

  /// Checks if the value at index \p i of stack is an Event wrapper.
  static Event *Event_check (lua_State *L, int i);

  /// Checks if the value at index \p i of stack is an Event::Type.
  static Event::Type Event_Type_check (lua_State *L, int i);

  /// Checks if the value at index \p i of stack is an Event::State.
  static Event::State Event_State_check (lua_State *L, int i);

  //// Pushes the Lua wrapper of event onto stack.
  static void Event_push (lua_State *L, Event *doc);

  /// Pushes Event::Type onto stack.
  static void Event_Type_push (lua_State *L, Event::Type type);

  /// Pushes Event::State onto stack.
  static void Event_State_push (lua_State *L, Event::State state);

  /// Calls a method of the Lua wrapper of the given event.
  static void Event_call (lua_State *L, Event *doc, const char *name,
                          int nargs, int nresults);

private:

  // Document:

  /// Registry key for the Document metatable.
  static const char *_DOCUMENT;

  /// Lua code to run when loading Document metatable.
  static unsigned char Document_initMt_lua[];

  /// Length in bytes of LuaAPI::Document_initMt_lua.
  static unsigned int Document_initMt_lua_len;

  static int __l_Document_gc (lua_State *L);

  static int __l_Document_toString (lua_State *L);

  static int _l_Document_getUnderlyingObject (lua_State *L);

  static int _l_Document_getObject (lua_State *L);

  static int _l_Document_getObjects (lua_State *L);

  static int _l_Document_getRoot (lua_State *L);

  static int _l_Document_getSettings (lua_State *L);

  static int _l_Document_createObject (lua_State *L);

  static int _l_Document_createEvent (lua_State *L);

  // Object:

  /// Lua code to run when loading Object metatable.
  static unsigned char Object_initMt_lua[];

  /// Length in bytes of LuaAPI::Object_initMt_lua.
  static unsigned int Object_initMt_lua_len;

  /// Gets the registry key of the metatable of \p obj.
  static const char *_Object_getRegistryKey (Object *obj);

  /// Attaches Lua wrapper to \p obj.
  static void _Object_attachWrapper (lua_State *L, Object *obj);

  /// Detaches Lua wrapper from \p obj.
  static void _Object_detachWrapper (lua_State *L, Object *obj);

  /// Checks if the value at index \p i of stack is an Object wrapper.
  static Object *_Object_check (lua_State *L, int i);

  /// Checks if the value at index \p i of stack is an Object::Type.
  static Object::Type _Object_Type_check (lua_State *L, int i);

  /// Pushes Object::Type onto stack.
  static void _Object_Type_push (lua_State *L, Object::Type type);

  static int _l_Object_getUnderlyingObject (lua_State *L);

  static int _l_Object_getType (lua_State *L);

  static int _l_Object_getDocument (lua_State *L);

  static int _l_Object_getParent (lua_State *L);

  static int _l_Object_getId (lua_State *L);

  static int _l_Object_getProperty (lua_State *L);

  static int _l_Object_setProperty (lua_State *L);

  static int _l_Object_getEvents (lua_State *L);

  static int _l_Object_getEvent (lua_State *L);

  // Context, switch, and media:

  /// Registry key for the Context metatable.
  static const char *_CONTEXT;

  /// Registry key for the Switch metatable.
  static const char *_SWITCH;

  /// Registry key for the Media metatable.
  static const char *_MEDIA;

  // Event:

  /// Registry key for the Event metatable.
  static const char *_EVENT;

  /// Lua code to run when loading Event metatable.
  static unsigned char Event_initMt_lua[];

  /// Length in bytes of LuaAPI::Event_initMt_lua.
  static unsigned int Event_initMt_lua_len;

  static int _l_Event_getUnderlyingObject (lua_State *L);

  static int _l_Event_getType (lua_State *L);

  static int _l_Event_getObject (lua_State *L);

  static int _l_Event_getId (lua_State *L);

  static int _l_Event_getQualifiedId (lua_State *L);

  static int _l_Event_getState (lua_State *L);

  static int _l_Event_setState (lua_State *L);

  static int _l_Event_getBeginTime (lua_State *L);

  static int _l_Event_setBeginTime (lua_State *L);

  static int _l_Event_getEndTime (lua_State *L);

  static int _l_Event_setEndTime (lua_State *L);

  static int _l_Event_getLabel (lua_State *L);

  static int _l_Event_setLabel (lua_State *L);

  // Auxiliary:

  /// Lua code to run when loading any metatable.
  static unsigned char initMt_lua[];

  /// Length in bytes of LuaAPI::Lua_initMt_lua.
  static unsigned int initMt_lua_len;

  /// Loads the metatable of a Lua wrapper (if not already loaded).
  ///
  /// @param L Lua state
  /// @param funcs The functions to install in the metatable.
  /// @param name The name for the metatable in LUA_REGISTRY.
  /// @param chunk A Lua chunk to run immediately after the metatable is
  ///              loaded, or NULL (no chunk).  If given, \p chunk is loaded
  ///              and called with the newly created metatable as first
  ///              argument.
  /// @param len Then length of \p chunk in bytes.
  ///
  static void _loadLuaWrapperMt (lua_State *L,
                                 const luaL_Reg *funcs, const char *name,
                                 const char *chunk, size_t len);


  /// Pops a value from the stack and sets it as the Lua wrapper of \p ptr.
  static void _attachLuaWrapper (lua_State *L, void *ptr);

  /// Detaches Lua wrapper from \p ptr.
  static void _detachLuaWrapper (lua_State *L, void *ptr);

  /// Pushes the Lua wrapper of \p ptr onto stack.
  static void _pushLuaWrapper (lua_State *L, void *ptr);

  /// Calls a method of the Lua wrapper of \p ptr.
  ///
  /// @param L Lua state.
  /// @param ptr The pointer whose wrapper is to be called.
  /// @param name The name of the method to call.
  /// @param nargs The number of arguments to the method.
  /// @param nresults The number of results of the method.
  ///
  static void _callLuaWrapper (lua_State *L, void *ptr, const char *name,
                               int nargs, int nresults);
};

GINGA_NAMESPACE_END

#endif // GINGA_LUA_API_H
