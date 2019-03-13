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

#define N 128
#define M N / 8

int
main (void)
{
  Document *doc;
  Context *root;
  MediaSettings *settings;
  Media *m[N];

  tests_create_document (&doc, &root, &settings);

  g_assert (doc->getObjectByIdOrAlias ("__root__") == root);
  g_assert_null (doc->getObjectByIdOrAlias ("ncl"));
  root->addAlias ("ncl");
  g_assert (doc->getObjectByIdOrAlias ("__root__") == root);
  g_assert (doc->getObjectByIdOrAlias ("ncl") == root);

  g_assert (doc->getObjectByIdOrAlias ("__settings__") == settings);
  g_assert_null (doc->getObjectByIdOrAlias ("settings"));
  settings->addAlias ("settings");
  g_assert (doc->getObjectByIdOrAlias ("__settings__") == settings);
  g_assert (doc->getObjectByIdOrAlias ("settings") == settings);

  for (size_t i = 0; i < N; i++)
    {
      m[i] = new Media (xstrbuild ("m%d", (int) i));
      doc->addObject (m[i]);
      for (size_t j = 0; j < M; j++)
        {
          string alias = xstrbuild ("m%d:%d", (int) i, (int) j);
          m[i]->addAlias (alias);
        }
    }
  g_assert_cmpint (doc->getObjects ()->size (), ==, N + 2);

  for (size_t i = 0; i < N; i++)
    {
      string id = xstrbuild ("m%d", (int) i);
      g_assert (doc->getObjectByIdOrAlias (id) == m[i]);
      for (size_t j = 0; j < M; j++)
        {
          string alias = xstrbuild ("m%d:%d", (int) i, (int) j);
          g_assert (doc->getObjectByIdOrAlias (alias) == m[i]);
        }
    }

  g_assert_null (doc->getObjectByIdOrAlias ("a"));
  g_assert_null (doc->getObjectByIdOrAlias ("b"));
  g_assert_null (doc->getObjectByIdOrAlias ("c"));

  delete doc;

  exit (EXIT_SUCCESS);
}
