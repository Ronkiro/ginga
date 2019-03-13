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

#include "tests.h"

int
main (void)
{
  vector<Object *> objs;

  objs.push_back (new Media ("m"));
  objs.push_back (new MediaSettings ("stgs"));
  objs.push_back (new Context ("c"));
  objs.push_back (new Switch ("s"));

  for (auto obj : objs)
    {
      for (guint j = 0; j < samples_keys.size (); j++)
        {
          obj->sendKey (samples_keys[j], true);
        }
      delete obj;
    }
  exit (EXIT_SUCCESS);
}
