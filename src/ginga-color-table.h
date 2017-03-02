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

#ifndef GINGA_COLOR_TABLE_H
#define GINGA_COLOR_TABLE_H

#include "ginga.h"

typedef struct _GingaColorTable
{
  const char *name;
  guchar r;
  guchar g;
  guchar b;
} GingaColorTable;

static const GingaColorTable ginga_color_table[] = {
  /* KEEP THIS SORTED ALPHABETICALLY */
  {"AliceBlue", 240, 248, 255},
  {"AntiqueWhite", 250, 235, 215},
  {"Aqua", 0, 255, 255},
  {"Aquamarine", 127, 255, 212},
  {"Azure", 240, 255, 255},
  {"Beige", 245, 245, 220},
  {"Bisque", 255, 228, 196},
  {"Black", 0, 0, 0},
  {"BlanchedAlmond", 255, 235, 205},
  {"Blue", 0, 0, 255},
  {"BlueViolet", 138, 43, 226},
  {"Brown", 165, 42, 42},
  {"BurlyWood", 222, 184, 135},
  {"CadetBlue", 95, 158, 160},
  {"Chartreuse", 127, 255, 0},
  {"Chocolate", 210, 105, 30},
  {"Coral", 255, 127, 80},
  {"CornflowerBlue", 100, 149, 237},
  {"Cornsilk", 255, 248, 220},
  {"Crimson", 220, 20, 60},
  {"Cyan", 0, 255, 255},
  {"DarkBlue", 0, 0, 139},
  {"DarkCyan", 0, 139, 139},
  {"DarkGoldenRod", 184, 134, 11},
  {"DarkGray", 169, 169, 169},
  {"DarkGreen", 0, 100, 0},
  {"DarkKhaki", 189, 183, 107},
  {"DarkMagenta", 139, 0, 139},
  {"DarkOliveGreen", 85, 107, 47},
  {"DarkOrange", 255, 140, 0},
  {"DarkOrchid", 153, 50, 204},
  {"DarkRed", 139, 0, 0},
  {"DarkSalmon", 233, 150, 122},
  {"DarkSeaGreen", 143, 188, 143},
  {"DarkSlateBlue", 72, 61, 139},
  {"DarkSlateGray", 47, 79, 79},
  {"DarkTurquoise", 0, 206, 209},
  {"DarkViolet", 148, 0, 211},
  {"DeepPink", 255, 20, 147},
  {"DeepSkyBlue", 0, 191, 255},
  {"DimGray", 105, 105, 105},
  {"DodgerBlue", 30, 144, 255},
  {"FireBrick", 178, 34, 34},
  {"FloralWhite", 255, 250, 240},
  {"ForestGreen", 34, 139, 34},
  {"Fuchsia", 255, 0, 255},
  {"Gainsboro", 220, 220, 220},
  {"GhostWhite", 248, 248, 255},
  {"Gold", 255, 215, 0},
  {"GoldenRod", 218, 165, 32},
  {"Gray", 128, 128, 128},
  {"Green", 0, 128, 0},
  {"GreenYellow", 173, 255, 47},
  {"HoneyDew", 240, 255, 240},
  {"HotPink", 255, 105, 180},
  {"IndianRed", 205, 92, 92},
  {"Indigo", 75, 0, 130},
  {"Ivory", 255, 255, 240},
  {"Khaki", 240, 230, 140},
  {"Lavender", 230, 230, 250},
  {"LavenderBlush", 255, 240, 245},
  {"LawnGreen", 124, 252, 0},
  {"LemonChiffon", 255, 250, 205},
  {"LightBlue", 173, 216, 230},
  {"LightCoral", 240, 128, 128},
  {"LightCyan", 224, 255, 255},
  {"LightGoldenRodYellow", 250, 250, 210},
  {"LightGray", 211, 211, 211},
  {"LightGreen", 144, 238, 144},
  {"LightPink", 255, 182, 193},
  {"LightSalmon", 255, 160, 122},
  {"LightSeaGreen", 32, 178, 170},
  {"LightSkyBlue", 135, 206, 250},
  {"LightSlateGray", 119, 136, 153},
  {"LightSteelBlue", 176, 196, 222},
  {"LightYellow", 255, 255, 224},
  {"Lime", 0, 255, 0},
  {"LimeGreen", 50, 205, 50},
  {"Linen", 250, 240, 230},
  {"Magenta", 255, 0, 255},
  {"Maroon", 128, 0, 0},
  {"MediumAquaMarine", 102, 205, 170},
  {"MediumBlue", 0, 0, 205},
  {"MediumOrchid", 186, 85, 211},
  {"MediumPurple", 147, 112, 219},
  {"MediumSeaGreen", 60, 179, 113},
  {"MediumSlateBlue", 123, 104, 238},
  {"MediumSpringGreen", 0, 250, 154},
  {"MediumTurquoise", 72, 209, 204},
  {"MediumVioletRed", 199, 21, 133},
  {"MidnightBlue", 25, 25, 112},
  {"MintCream", 245, 255, 250},
  {"MistyRose", 255, 228, 225},
  {"Moccasin", 255, 228, 181},
  {"NavajoWhite", 255, 222, 173},
  {"Navy", 0, 0, 128},
  {"OldLace", 253, 245, 230},
  {"Olive", 128, 128, 0},
  {"OliveDrab", 107, 142, 35},
  {"Orange", 255, 165, 0},
  {"OrangeRed", 255, 69, 0},
  {"Orchid", 218, 112, 214},
  {"PaleGoldenRod", 238, 232, 170},
  {"PaleGreen", 152, 251, 152},
  {"PaleTurquoise", 175, 238, 238},
  {"PaleVioletRed", 219, 112, 147},
  {"PapayaWhip", 255, 239, 213},
  {"PeachPuff", 255, 218, 185},
  {"Peru", 205, 133, 63},
  {"Pink", 255, 192, 203},
  {"Plum", 221, 160, 221},
  {"PowderBlue", 176, 224, 230},
  {"Purple", 128, 0, 128},
  {"Red", 255, 0, 0},
  {"RosyBrown", 188, 143, 143},
  {"RoyalBlue", 65, 105, 225},
  {"SaddleBrown", 139, 69, 19},
  {"Salmon", 250, 128, 114},
  {"SandyBrown", 244, 164, 96},
  {"SeaGreen", 46, 139, 87},
  {"SeaShell", 255, 245, 238},
  {"Sienna", 160, 82, 45},
  {"Silver", 192, 192, 192},
  {"SkyBlue", 135, 206, 235},
  {"SlateBlue", 106, 90, 205},
  {"SlateGray", 112, 128, 144},
  {"Snow", 255, 250, 250},
  {"SpringGreen", 0, 255, 127},
  {"SteelBlue", 70, 130, 180},
  {"Tan", 210, 180, 140},
  {"Teal", 0, 128, 128},
  {"Thistle", 216, 191, 216},
  {"Tomato", 255, 99, 71},
  {"Turquoise", 64, 224, 208},
  {"Violet", 238, 130, 238},
  {"Wheat", 245, 222, 179},
  {"White", 255, 255, 255},
  {"WhiteSmoke", 245, 245, 245},
  {"Yellow", 255, 255, 0},
  {"YellowGreen", 154, 205, 50},
};

static G_GNUC_PURE int
ginga_color_table_compar (const void *e1, const void *e2)
{
  const GingaColorTable *c1;
  const GingaColorTable *c2;

  c1 = (const GingaColorTable *) e1;
  c2 = (const GingaColorTable *) e2;

  return g_ascii_strcasecmp (c1->name, c2->name);
}

/* Formating of Hex Input to 8 bytes standard (#RRGGBBAA) */
static inline const char *
ginga_color_hex_formatter(string hex){
   //remove #
   if(hex[0]=='#') hex.erase(0, 1);
   // 16 colors GrayScale
   if(hex.length() == 1){
      hex.insert(1, string(1,hex[0]) );
      hex.insert(2, string(1,hex[0]) );
      hex.insert(3, string(1,hex[0]) );
      hex.insert(4, string(1,hex[0]) );
      hex.insert(5, string(1,hex[0]) );
      hex.append("FF");
   } // 256 colors GrayScale
    if(hex.length() == 2){
      hex.insert(2, string(1,hex[0]) );
      hex.insert(3, string(1,hex[1]) );
      hex.insert(4, string(1,hex[0]) );
      hex.insert(5, string(1,hex[1]) );
      hex.append("FF");
   } // RGB 16 colors per channel
   else if(hex.length() == 3){
      hex.insert(3, string(1,hex[2]) );
      hex.insert(2, string(1,hex[1]) );
      hex.insert(1, string(1,hex[0]) );
      hex.append("FF");
  } // RGB 16 colors per channel and A with 16 levels
   else if(hex.length() == 4){
      hex.insert(4, string(1,hex[3]) );
      hex.insert(3, string(1,hex[2]) );
      hex.insert(2, string(1,hex[1]) );
      hex.insert(1, string(1,hex[0]) );
   } //RGB 256 colors per channel and A with 16 levels
   else if(hex.length() == 5){
      hex.insert(5, string(1,hex[4]) );
      hex.append("FF");
   } //RGB 256 colors per channel
   else if(hex.length() == 6){
      hex.append("FF");
   } //RGB 256 colors per channel and A with 16 levels
   else if(hex.length() == 7){
       hex.insert(7, string(1,hex[6]) );
   } //RGB 256 colors per channel and A with 256 levels
   else if(hex.length() > 8){
      return  hex.substr (0,8).c_str(); //limit to 8 characters
   }
   return hex.c_str();
}

/* Gets the matching SDL_Color to the given RGB or RGBA input (255,255,255 
or 255,255,255,255).  If the input is valid and returns true, otherwise 
returns false.  */
static inline gboolean
ginga_rgba_to_sdl_color(const string value,  SDL_Color *color){
   gchar **pixels = g_strsplit(value.c_str(),",",-1 );
   if(g_strv_length(pixels) < 3 )
         return FALSE;

   color->r = xstrto_uint8 (pixels[0]);
   color->g = xstrto_uint8 (pixels[1]);
   color->b = xstrto_uint8 (pixels[2]);
   if(pixels[3] != NULL)
     color->a = xstrto_uint8 (pixels[3]);
   else
     color->a=255;

   g_strfreev(pixels);
   return TRUE;
}

/* Gets the matching SDL_Color to the given RGB or RGBA text input [rgb(255,255,255) 
or rgba(255,255,255,255)]. If the input is valid and returns true, otherwise 
returns false.  */
static inline gboolean
ginga_rgbatext_to_sdl_color(const string value, SDL_Color *color){
      gchar **pixels = g_strsplit( value.substr
                                   (value.find("(")+1,
                                    value.find(")")-value.find("(")-1).c_str()
                                   ,",",-1 );
   if(g_strv_length(pixels) < 3 )
         return FALSE;

   color->r = xstrto_uint8 (pixels[0]);
   color->g = xstrto_uint8 (pixels[1]);
   color->b = xstrto_uint8 (pixels[2]);
   if(pixels[3] != NULL)
     color->a = xstrto_uint8 (pixels[3]);
   else
     color->a=255;

   g_strfreev(pixels);
   return TRUE;
}

/* Gets the matching SDL_Color to the given HEX code input.  If the input
is valid returns true, otherwise returns false.  */
static inline gboolean
ginga_hex_to_sdl_color(const string hex, SDL_Color *color){
   const char *c = ginga_color_hex_formatter(hex);
   if (strlen(c) < 8)
     return FALSE;
   int hexValue=0;

   for(int i=0; i<8; i++, ++c){
         int  thisC = *c;
        thisC = toupper (thisC);
        hexValue <<= 4;
        if( thisC >= 48 &&  thisC <= 57 )
            hexValue += thisC - 48;
        else if( thisC >= 65 && thisC <= 70)
            hexValue += thisC - 65 + 10;
        else return FALSE;
    }

   color->r = (guint8)((hexValue >> 24) & 0xFF); // extract the RR byte
   color->g = (guint8)((hexValue >> 16) & 0xFF); // extract the GG byte
   color->b = (guint8)((hexValue >> 8) & 0xFF);  // extract the BB byte
   color->a = (guint8)((hexValue) & 0xFF);       // extract the AA byte

   return TRUE;
}

/* Convert byte color to percentage  */
static inline double
ginga_color_percent(guint c){
    if(c>0) return (double)c/255;
    else return 0;
}

/* Gets the color value associated with the given name.  If NAME is in color
   table, stores its color components into *R, *G, *B, and returns true,
   otherwise returns false.  */
static inline gboolean
ginga_color_table_index (const char *name, guchar *r, guchar *g, guchar *b)
{
  GingaColorTable key;
  GingaColorTable *match;

  key.name = name;
  match = (GingaColorTable *)
    bsearch (&key, ginga_color_table, G_N_ELEMENTS (ginga_color_table),
             sizeof (*ginga_color_table), ginga_color_table_compar);
  if (match == NULL)
    return FALSE;

  *r = match->r;
  *g = match->g;
  *b = match->b;

  return TRUE;
}

/* Gets the matching SDL_Color to the given input.  If Value is a
 HEX (#FFF) the function calls 'ginga_hex_to_sdl_color'. If  Value is a RGB_TEXT,
 like 'rgb(255,255,255)', the function calls 'ginga_rgbatext_to_sdl_color'. If the
 value is a COLOR NAME the function calls 'ginga_color_table_index'. Return true if
 the input is valid, otherwise returns false. */
static inline gboolean
ginga_color_input_to_sdl_color(const string value, SDL_Color *color){
   if(value[0] == '#') //by hex
         return ginga_hex_to_sdl_color(value, color);
   else if(value.substr (0,3)=="rgb") //by rgbatxt
         return ginga_rgbatext_to_sdl_color(value, color);
   else  if(ginga_color_table_index (value.c_str(), &color->r, &color->g, &color->b ))  //by name
         return TRUE;
   else
         return ginga_rgba_to_sdl_color(value, color);      
}

#endif /* GINGA_COLOR_TABLE_H */