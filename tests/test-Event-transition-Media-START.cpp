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

#include "Event.h"
#include "Media.h"
#include "Parser.h"

#define PARSE(fmt, doc, str)                                            \
  G_STMT_START                                                          \
  {                                                                     \
    string buf = str;                                                   \
    string errmsg;                                                      \
    *fmt = new Formatter (0, nullptr, nullptr);                         \
    g_assert_nonnull (*fmt);                                            \
    if (!(*fmt)->start (buf.c_str (), buf.length (), &errmsg))          \
      {                                                                 \
        g_printerr ("*** Unexpected error: %s", errmsg.c_str ());       \
        g_assert_not_reached ();                                        \
      }                                                                 \
    *doc = (*fmt)->getDocument ();                                      \
    g_assert_nonnull (*doc);                                            \
  }                                                                     \
  G_STMT_END

int
main (void)
{
  
  // Presentation events ---------------------------------------------------

  // @lambda: START from state OCCURRING.
  {
    Formatter *fmt;
    Document *doc;
    PARSE (&fmt, &doc, "\
<ncl>\n\
 <body>\n\
  <media id='m'/>\n\
 </body>\n\
</ncl>");

    Media *m = cast (Media *, doc->getObjectById ("m"));
    g_assert_nonnull (m);

    Event *lambda = m->getLambda ();
    g_assert_nonnull (lambda);

    g_assert (lambda->getState () == Event::SLEEPING);
    g_assert (lambda->transition (Event::START));
    g_assert (lambda->getState () == Event::OCCURRING);

    // Main check.
    g_assert_false (lambda->transition (Event::START));
    g_assert (lambda->getState () == Event::OCCURRING);

    delete fmt;
  }

  // @lambda: START from state PAUSED.
  {
  }

  // @lambda: START from state SLEEPING.
  {
    Formatter *fmt;
    Document *doc;
    PARSE (&fmt, &doc, "\
<ncl>\n\
 <body>\n\
  <media id='m'>\n\
   <property name='p1' value='0'/>\n\
   <area id='a1'/>\n\
   <area id='a2'/>\n\
  </media>\n\
 </body>\n\
</ncl>");

    // Check lambda
    Media *m = cast (Media *, doc->getObjectById ("m"));
    g_assert_nonnull (m);

    Event *lambda = m->getLambda ();
    g_assert_nonnull (lambda);

    g_assert (lambda->getState () == Event::SLEEPING);
    g_assert (lambda->transition (Event::START));
    g_assert (lambda->getState () == Event::OCCURRING);

    // Check anchors
    // In the reaction that lambda goes to OCCURRING, 
    // the anchors must be on SLEEPING.
    // They only change to OCCURRING on the reaction of 
    // time advance, i.e. sendTickEvent.
    Event *a1 = m->getPresentationEvent ("a1");
    g_assert_nonnull (a1);
    g_assert (a1->getState () == Event::SLEEPING);

    Event *a2 = m->getPresentationEvent ("a1");
    g_assert_nonnull (a2);
    g_assert (a2->getState () == Event::SLEEPING);

    Event *p1 = m->getAttributionEvent ("p1");
    g_assert_nonnull (p1);
    g_assert (p1->getState () == Event::SLEEPING);

    fmt->sendTick (1, 1, 1);

    g_assert (lambda->getState () == Event::OCCURRING);
    g_assert (a1->getState () == Event::OCCURRING);
    g_assert (a2->getState () == Event::OCCURRING);
    g_assert (p1->getState () == Event::SLEEPING);

    delete fmt;
  }


  // Attribution events ----------------------------------------------------


  // Selection events ------------------------------------------------------

  exit (EXIT_SUCCESS);
}
