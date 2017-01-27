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

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "ginga.h"

#include "SDLDisplay.h"
#include "InputManager.h"

#include "SDLWindow.h"
#include "SDLSurface.h"

#include "IContinuousMediaProvider.h"
#include "IFontProvider.h"
#include "IImageProvider.h"

#include "InputManager.h"
#include "SDLInputEvent.h"
#include "SDLEventBuffer.h"

#include "system/Thread.h"
using namespace ::ginga::system;

GINGA_MB_BEGIN

class DisplayManager
{
public:
  DisplayManager ();

protected:
  pthread_mutex_t mapMutex;
  pthread_mutex_t genMutex;

  map<GingaSurfaceID, SDLSurface *> surMap;
  pthread_mutex_t surMapMutex;

  map<GingaProviderID, IMediaProvider *> provMap;
  pthread_mutex_t provMapMutex;

  static set<IInputEventListener *> iListeners;
  static pthread_mutex_t ilMutex;

  static set<IMotionEventListener *> mListeners;
  static pthread_mutex_t mlMutex;

  static bool initMutex;
  bool running;

  bool isWaiting;
  pthread_cond_t wsSignal;
  pthread_mutex_t wsMutex;

  static void checkInitMutex ();

public:
  virtual ~DisplayManager ();

  void releaseHandler ();

  static void addIEListenerInstance (IInputEventListener *listener);

  static void removeIEListenerInstance (IInputEventListener *listener);

  static bool hasIEListenerInstance (IInputEventListener *listener,
                                     bool removeInstance = false);

  static void addMEListenerInstance (IMotionEventListener *listener);

  static void removeMEListenerInstance (IMotionEventListener *listener);

  void releaseScreen ();
  void releaseMB ();
  void clearWidgetPools ();

public:
  UnderlyingWindowID getScreenUnderlyingWindow ();

protected:
  GingaSurfaceID provIdRefCounter;

public:
  IMediaProvider *getIMediaProviderFromId (const GingaProviderID &provId);
  SDLSurface *getISurfaceFromId (const GingaSurfaceID &surfaceId);

  // Interfacing output.
  SDLWindow* createWindow (int x, int y, int w, int h, double z);

  UnderlyingWindowID createUnderlyingSubWindow (int x, int y, int w, int h,
                                                double z);

  bool hasWindow (SDLWindow* window);

  void releaseWindow (SDLWindow *window);

  void registerSurface (SDLSurface *);

  GingaSurfaceID createSurface ();

  GingaSurfaceID createSurface (int w, int h);

  GingaSurfaceID createSurfaceFrom (GingaSurfaceID underlyingSurface);

  bool hasSurface (const GingaSurfaceID &surId);
  bool releaseSurface (SDLSurface *surface);

  void lowerWindowToBottom (SDLWindow* winId);

  /* Interfacing content */
  GingaProviderID createContinuousMediaProvider (const char *mrl,
                                                 bool isRemote);
  void releaseContinuousMediaProvider (GingaProviderID provider);
  GingaProviderID createFontProvider (const char *mrl, int fontSize);
  void releaseFontProvider (GingaProviderID provider);
  GingaProviderID createImageProvider (const char *mrl);
  void releaseImageProvider (GingaProviderID provider);
  GingaSurfaceID createRenderedSurfaceFromImageFile (const char *mrl);

public:
  InputManager *getInputManager ();
  SDLEventBuffer *createEventBuffer ();
  SDLInputEvent *createInputEvent (void *event, const int symbol);
  SDLInputEvent *createApplicationEvent (int type, void *data);
  int fromMBToGinga (int keyCode);
  int fromGingaToMB (int keyCode);

  // windows
  void addWindowCaps (SDLWindow* winId, int caps);
  void setWindowCaps (SDLWindow* winId, int caps);
  int getWindowCap (SDLWindow* winId, const string &capName);

  void drawWindow (SDLWindow* winId);
  void setWindowBounds (SDLWindow* winId, int x, int y, int w,
                        int h);
  void showWindow (SDLWindow* winId);
  void hideWindow (SDLWindow* winId);
  void raiseWindowToTop (SDLWindow* winId);
  void renderWindowFrom (SDLWindow* winId,
                         const GingaSurfaceID &surId);
  void setWindowBgColor (SDLWindow* winId, guint8 r, guint8 g,
                         guint8 b, guint8 alpha);
  void setWindowBorder (SDLWindow* winId, guint8 r, guint8 g,
                        guint8 b, guint8 alpha, int width);
  void setWindowCurrentTransparency (SDLWindow* winId,
                                     guint8 transparency);
  void setWindowColorKey (SDLWindow* winId, guint8 r, guint8 g,
                          guint8 b);
  void setWindowX (SDLWindow* winId, int x);
  void setWindowY (SDLWindow* winId, int y);
  void setWindowW (SDLWindow* winId, int w);
  void setWindowH (SDLWindow* winId, int h);
  void setWindowZ (SDLWindow* winId, double z);
  void disposeWindow (SDLWindow* winId);
  void setGhostWindow (SDLWindow* winId, bool ghost);
  void validateWindow (SDLWindow* winId);
  int getWindowX (SDLWindow* winId);
  int getWindowY (SDLWindow* winId);
  int getWindowW (SDLWindow* winId);
  int getWindowH (SDLWindow* winId);
  double getWindowZ (SDLWindow* winId);
  guint8 getWindowTransparencyValue (SDLWindow* winId);
  void resizeWindow (SDLWindow* winId, int width, int height);
  string getWindowDumpFileUri (SDLWindow* winId, int quality,
                               int dumpW, int dumpH);
  void clearWindowContent (SDLWindow* winId);
  void revertWindowContent (SDLWindow* winId);
  void deleteWindow (SDLWindow* winId);
  void moveWindowTo (SDLWindow* winId, int x, int y);
  void setWindowMirrorSrc (SDLWindow* winId,
                           SDLWindow* mirrorSrc);
  // surfaces
  void *getSurfaceContent (const GingaSurfaceID &surId);
  SDLWindow* getSurfaceParentWindow (const GingaSurfaceID &surId);
  void deleteSurface (const GingaSurfaceID &surId);
  bool setSurfaceParentWindow (const GingaSurfaceID &surId,
                               SDLWindow* winId);
  void clearSurfaceContent (const GingaSurfaceID &surId);
  void getSurfaceSize (const GingaSurfaceID &surId, int *width,
                       int *height);
  void addSurfaceCaps (const GingaSurfaceID &surId, const int caps);
  void setSurfaceCaps (const GingaSurfaceID &surId, const int caps);
  int getSurfaceCap (const GingaSurfaceID &surId, const string &cap);
  int getSurfaceCaps (const GingaSurfaceID &surId);
  void setSurfaceBgColor (const GingaSurfaceID &surId, guint8 r, guint8 g,
                          guint8 b, guint8 alpha);
  void setSurfaceFont (const GingaSurfaceID &surId, GingaSurfaceID font);
  void setColor (const GingaSurfaceID &surId, guint8 r, guint8 g, guint8 b,
                 guint8 alpha);
  void setExternalHandler (const GingaSurfaceID &surId, bool extHandler);
  void blitSurface (const GingaSurfaceID &surId, int x, int y,
                    GingaSurfaceID src = 0, int srcX = -1, int srcY = -1,
                    int srcW = -1, int srcH = -1);
  void flipSurface (const GingaSurfaceID &surId);
  void setSurfaceContent (const GingaSurfaceID &surId, void *surface);
  Color *getSurfaceColor (const GingaSurfaceID &surId);
  bool hasSurfaceExternalHandler (const GingaSurfaceID &surId);
  void setSurfaceColor (const GingaSurfaceID &surId, guint8 r, guint8 g,
                        guint8 b, guint8 alpha);
  // providers
  void setProviderSoundLevel (const GingaProviderID &provId, double level);
  void getProviderOriginalResolution (const GingaProviderID &provId,
                                      int *width, int *height);
  double getProviderTotalMediaTime (const GingaProviderID &provId);
  double getProviderSoundLevel (const GingaProviderID &provId);
  int64_t getProviderVPts (const GingaProviderID &provId);
  void setProviderMediaTime (const GingaProviderID &provId, double pos);
  double getProviderMediaTime (const GingaProviderID &provId);
  void pauseProvider (const GingaProviderID &provId);
  void stopProvider (const GingaProviderID &provId);
  void resumeProvider (const GingaProviderID &provId,
                       GingaSurfaceID surface);
  void setProviderAVPid (const GingaProviderID &provId, int aPid, int vPid);
  void feedProviderBuffers (const GingaProviderID &provId);
  bool checkProviderVideoResizeEvent (const GingaProviderID &provId,
                                      const GingaSurfaceID &frame);
  int getProviderStringWidth (const GingaProviderID &provId,
                              const char *text, int textLength = 0);
  void playProviderOver (const GingaProviderID &provId,
                         const GingaSurfaceID &surface);
  void playProviderOver (const GingaProviderID &provId,
                         const GingaSurfaceID &surface, const char *text,
                         int x, int y, short align);
  int getProviderHeight (const GingaProviderID &provId);
  bool getScreen (SDLDisplay **screen);

protected:
  void lock ();
  void unlock ();
};

// Global screen manager.
extern DisplayManager *_Ginga_Display_M;
#define Ginga_Display_M                                                      \
  GINGA_ASSERT_GLOBAL_NONNULL (_Ginga_Display_M, DisplayManager *)

GINGA_MB_END

#endif /* DISPLAY_MANAGER_H*/
