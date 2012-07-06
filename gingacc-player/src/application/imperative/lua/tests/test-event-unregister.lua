--[[ test-event-unregister.lua -- Check event.unregister.
     Copyright (C) 2012 PUC-Rio/Laboratorio TeleMidia

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 51
Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. --]]

require 'tests'

local f = function () end
local g = function () end
local h = function () end

local function register5 ()
   event.register (f)
   event.register (g)
   event.register (h)
   event.register (f)
   event.register (g)                             -- H=<f,g,h,f,g>
   assert (hsz () == 5)
end

-- Invalid calls.
assert (pcall (event.unregister, nil) == false)
assert (pcall (event.unregister, 0) == false)
assert (pcall (event.unregister, {}, 0, {}) == false)
assert (pcall (event.unregister, {}, f, f) == false)
assert (pcall (event.unregister, nil, f, f) == false)


----------------------------------------------------------------------------
-- event.unregister (f:function) -> n:number
----------------------------------------------------------------------------

register5 ()                                      -- H=<f,g,h,f,g>

assert (event.unregister (f) == 2)                -- H=<g,h,g>
assert (hsz () == 3)
assert (hf (1) == g and hf (2) == h and hf (3) == g)

assert (event.unregister (h) == 1)                -- H=<g,g>
assert (hsz () == 2)
assert (hf (1) == g and hf (2) == g)

-- Function not registered; do nothing.
assert (event.unregister (f) == 0)
assert (hsz () == 2)
assert (hf (1) == g and hf (2) == g)

assert (event.unregister (g) == 2)                -- H=<>
assert (hsz () == 0)


----------------------------------------------------------------------------
-- event.unregister (f1:function, f2:function, ...) -> n:number
----------------------------------------------------------------------------

register5 ()                                      -- H=<f,g,h,f,g>

assert (event.unregister (g, g, g, g) == 2)       -- H=<f,h,f>
assert (hsz () == 3)
assert (hf (1) == f and hf (2) == h and hf (3) == f)

assert (event.unregister (h, f) == 3)
assert (hsz () == 0)


----------------------------------------------------------------------------
-- event.unregister () -> n:number
----------------------------------------------------------------------------

register5 ()
assert (event.unregister () == 5)
assert (hsz () == 0)
assert (hsz () == 0)

register5 ()
done ()
