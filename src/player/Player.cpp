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
#include "Player.h"

#include "mb/Display.h"
using namespace ::ginga::mb;

GINGA_PLAYER_BEGIN

Player::Player (const string &mrl)
{
  this->mrl = mrl;
  this->window = NULL;
  this->_notifying = false;
  this->presented = false;
  this->visible = true;
  this->status = PL_SLEEPING;
  this->forcedNaturalEnd = false;
  this->scope = "";
  this->scopeType = PL_TYPE_PRESENTATION;
  this->scopeInitTime = 0;
  this->scopeEndTime = GINGA_TIME_NONE;

  _time = 0;

  //media attr
  this->texture = NULL;         // media content
  this->borderWidth = 0;
  this->bgColor = {0, 0, 0, 0};
  this->borderColor = {0, 0, 0, 0};
  this->z = 0;
  this->alpha = 255;

  animator = new PlayerAnimator ();
}

Player::~Player ()
{
  if (this->texture != NULL)
    Ginga_Display->destroyTexture (this->texture);

  _listeners.clear ();
  _properties.clear ();
}

void
Player::setMrl (const string &mrl, bool visible)
{
  this->mrl = mrl;
  this->visible = visible;
}

void
Player::addListener (IPlayerListener *listener)
{
  if (_notifying)
    return;
  _listeners.insert (listener);
}

void
Player::removeListener (IPlayerListener *listener)
{
  set<IPlayerListener *>::iterator i;

  if (!_notifying)
    {
      i = _listeners.find (listener);
      if (i != _listeners.end ())
        _listeners.erase (i);
    }
}

void
Player::notifyPlayerListeners (short code,
                               const string &parameter,
                               PlayerEventType type,
                               const string &value)
{
  string p;
  string v;

  this->_notifying = true;

  if (code == PL_NOTIFY_STOP)
    this->presented = true;

  if (_listeners.empty ())
    {
      this->_notifying = false;
      return;
    }

  for (auto i: this->_listeners)
    i->updateStatus (code, parameter, type, value);
}

void G_GNUC_NORETURN
Player::setMediaTime (arg_unused (GingaTime time))
{
  ERROR_NOT_IMPLEMENTED ("cannot seek in media content");
}

GingaTime
Player::getMediaTime ()
{
  return _time;
}

void
Player::incMediaTime (GingaTime incr)
{
  _time += incr;
}

void
Player::setScope (const string &scope,
                  PlayerEventType type,
                  GingaTime initTime,
                  GingaTime endTime)
{
  this->scope = scope;
  this->scopeType = type;
  this->scopeInitTime = initTime;
  this->scopeEndTime = endTime;
}

bool
Player::play ()
{
  this->forcedNaturalEnd = false;
  _time = 0;
  this->status = PL_OCCURRING;
  Ginga_Display->registerPlayer (this);
  return true;
}

void
Player::stop ()
{
  _time = 0;
  this->status = PL_SLEEPING;
  Ginga_Display->unregisterPlayer (this);
}

void
Player::abort ()
{
  stop ();
}

void
Player::pause ()
{
  this->status = PL_PAUSED;
}

void
Player::resume ()
{
  this->status = PL_OCCURRING;
}

string
Player::getPropertyValue (string const &name)
{
  return (_properties.count (name) != 0) ? _properties[name] : "";
}

void
Player::setPropertyValue (const string &name, const string &value)
{
  vector<string> params;

  if (!value.length ())
    return;

  if (name == "bounds")
    {
      params = xstrsplit (value, ',');
      if (params.size () == 4)
        {
          this->rect.x = xstrtoint (params[0], 10);
          this->rect.y = xstrtoint (params[1], 10);
          this->rect.w = xstrtoint (params[2], 10);
          this->rect.h = xstrtoint (params[3], 10);
        }
    }
  else if (name == "location")
    {
      g_assert_not_reached ();
    }
  else if (name == "size")
    {
      g_assert_not_reached ();
    }
  else if (name == "left")
    {
      this->rect.x = xstrtoint (value, 10);
    }
  else if (name == "top")
    {
      this->rect.y = xstrtoint (value, 10);
    }
  else if (name == "width")
    {
      this->rect.w = xstrtoint (value, 10);
    }
  else if (name == "height")
    {
      this->rect.h = xstrtoint (value, 10);
    }
  else if(name == "background" || name == "backgroundColor"
          || name == "bgColor")
    {
      g_assert (ginga_color_parse (value, &this->bgColor));
    }
  else if (name == "transparency")
    {
      if(xstrtouint8 (value, 10) <= 0)
        this->alpha = 255;
      else
        this->alpha = (guint8)(255 - (((xstrtod (value)/100)*255)));
    }
  _properties[name] = value;
}

bool
Player::isVisible ()
{
  return this->visible;
}

void
Player::setVisible (bool visible)
{
  this->visible = visible;
}

void
Player::forceNaturalEnd (bool forceIt)
{
  forcedNaturalEnd = forceIt;
  if (forceIt)
    notifyPlayerListeners (PL_NOTIFY_STOP, "", PL_TYPE_PRESENTATION, "");
}

bool
Player::isForcedNaturalEnd ()
{
  if (mrl == "")
    return false;
  return forcedNaturalEnd;
}

void
Player::setOutWindow (SDLWindow *win)
{
  if (win != NULL)
    {
      this->rect = win->getRect ();
      win->getZ (&this->z, &this->zorder);
    }
  this->window = win;
}

Player::PlayerStatus
Player::getMediaStatus ()
{
   return this->status;
}

void
Player::getZ (int *z, int *zorder)
{
  set_if_nonnull (z, this->z);
  set_if_nonnull (zorder, this->zorder);
}

void
Player::setAnimatorProperties(string dur, string name, string value)
{
  animator->addProperty (dur, name, value);
}

void
Player::redraw (SDL_Renderer *renderer)
{
  if (this->status == PL_SLEEPING)
    return;

  animator->update (&this->rect,
                    &this->bgColor.r,
                    &this->bgColor.g,
                    &this->bgColor.b,
                    &this->alpha);

  if (this->window != NULL)
    this->window->getBorder (&this->borderColor, &this->borderWidth);

  if (this->bgColor.a > 0)
    {
      // background color
      SDLx_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_BLEND);
      SDLx_SetRenderDrawColor (renderer,
                               this->bgColor.r,
                               this->bgColor.g,
                               this->bgColor.b,
                               this->alpha);
      SDLx_RenderFillRect (renderer, &this->rect);
    }

  if (this->texture != NULL)
    {
      SDLx_SetTextureAlphaMod (this->texture, this->alpha);
      SDLx_RenderCopy (renderer, this->texture, NULL, &this->rect);
    }

  if (this->borderWidth < 0)
    {
      this->borderWidth *= -1;
      SDLx_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_BLEND);
      SDLx_SetRenderDrawColor (renderer,
                               this->borderColor.r,
                               this->borderColor.g,
                               this->borderColor.b, 255);
      SDLx_RenderDrawRect (renderer, &this->rect);
    }
}

GINGA_PLAYER_END
