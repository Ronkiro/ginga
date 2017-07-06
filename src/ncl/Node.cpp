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
#include "Node.h"
#include "CompositeNode.h"

GINGA_NCL_BEGIN

/**
 * @brief Creates a new component.
 * @param id Component id.
 */
Node::Node (const string &id) : Entity (id)
{
  _parent = nullptr;
}

/**
 * @brief Destroys component.
 */
Node::~Node ()
{
  _anchors.clear ();
}

/**
 * @brief Gets component parent.
 */
CompositeNode *
Node::getParent ()
{
  return _parent;
}

/**
 * @brief Sets component parent.  (Can only be called once.)
 */
void
Node::setParent (CompositeNode *parent)
{
  g_assert_null (_parent);
  g_assert_nonnull (parent);
  _parent = parent;
}

/**
 * @brief Adds anchor to component.
 * @param anchor Anchor.
 */
void
Node::addAnchor (Anchor *anchor)
{
  g_assert_nonnull (anchor);
  _anchors.push_back (anchor);
}

/**
 * @brief Gets all anchors.
 */
const vector<Anchor *> *
Node::getAnchors ()
{
  return &_anchors;
}

/**
 * @brief Gets anchor.
 * @param id Anchor id.
 * @return Anchor if successful, or null if not found.
 */
Anchor *
Node::getAnchor (const string &id)
{
  for (auto anchor: _anchors)
    if (anchor->getId () == id)
      return anchor;
  return nullptr;
}


// FIXME: Remove this with NclNodeNesting stuff.

vector<Node *> *
Node::getPerspective ()
{
  vector<Node *> *perspective;

  if (_parent == NULL)
    {
      perspective = new vector<Node *>;
    }
  else
    {
      perspective = ((CompositeNode *)_parent)->getPerspective ();
    }
  perspective->push_back ((Node *)this);
  return perspective;
}

GINGA_NCL_END
