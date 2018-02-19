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

#include "aux-ginga.h"
#include <sstream> // for stringstream

GINGA_NAMESPACE_BEGIN

// Logging -----------------------------------------------------------------

/**
 * @brief Parses G_STRFUNC string to generate a log prefix.
 * @param strfunc String generated by G_STRFUNC macro.
 * @return Log prefix.
 */
string
__ginga_strfunc (const string &strfunc)
{
  string result;
  size_t i;

  i = strfunc.find ("(");
  g_assert (i != std::string::npos);
  result = strfunc.substr (0, i);

  i = result.rfind (" ");
  if (i != std::string::npos)
    result = result.substr (i + 1);

  return result + "()";
}

// Numeric functions.

/**
 * @brief Tests whether floating-point numbers are equal up to a given
 * threshold.
 * @param x Floating-point number.
 * @param y Floating-point number.
 * @param epsilon Error threshold.
 * @return True if successful, or false otherwise.
 */
bool
floateq (double x, double y, double epsilon)
{
  return ABS (x - y) <= ABS (epsilon);
}

// Parsing and evaluation --------------------------------------------------

/**
 * @brief Parses boolean string.
 * @param s Boolean string.
 * @param result Variable to store the resulting boolean.
 * @return True if successful, or false otherwise.
 */
bool
try_parse_bool (const string &s, bool *result)
{
  if (xstrcaseeq (s, "true"))
    {
      tryset (result, true);
      return true;
    }
  else if (xstrcaseeq (s, "false"))
    {
      tryset (result, false);
      return true;
    }
  else
    {
      return false;
    }
}

/**
 * @brief Parses color string.
 * @param s Color string.
 * @param result Variable to store the resulting color.
 * @return True if successful, or false otherwise.
 */
bool
try_parse_color (const string &s, Color *result)
{
  if (s == "")
    {
      Color none = {0, 0, 0, 0};
      tryset (result, none);
      return true;
    }
  else
    {
      return gdk_rgba_parse (result, s.c_str ());
    }
}

/**
 * @brief Parses list of string items.
 * @param s List string.
 * @param sep Separator.
 * @param min Minimum number of items.
 * @param max Maximum number of items.
 * @param result Variable to store the resulting items.
 * @return True if successful, or false otherwise.
 */
bool
try_parse_list (const string &s, char sep, size_t min, size_t max,
                list<string> *result)
{
  list<string> items;
  size_t n;

  items = xstrsplit (s, sep);
  n = items.size ();
  if (n < min || n > max)
    return false;

  if (result == nullptr)
    return true;

  for (auto it : items)
    result->push_back (xstrstrip (it));

  return true;
}

/**
 * @brief Parses Lua-like table with string keys and values.
 * @param s Table string.
 * @param result Variable to store the resulting table.
 * @return True if successful, or false otherwise.
 *
 * @todo Add support for keys or values containing embedded commas and
 * scaped quotes.  Put another way, replace xstrsplit() call by real parsing
 * code.
 */
bool
try_parse_table (const string &s, map<string, string> *result)
{
  string str;
  map<string, string> tab;

  str = xstrstrip (s);
  if (str.front () != '{' || str.back () != '}')
    return false;

  str.assign (str.substr (1, str.length () - 2));
  for (auto &it : xstrsplit (str, ','))
    {
      list<string> pr;
      string key;
      string val;

      if (unlikely (!try_parse_list (it, '=', 2, 2, &pr)))
        return false;

      auto pr_it = pr.begin ();
      key = xstrstrip (*pr_it++);
      val = xstrstrip (*pr_it++);
      g_assert (pr_it == pr.end ());
      if (unlikely (!((val.front () == '\'' && val.back () == '\'')
                      || (val.front () == '"' && val.back () == '"'))))
        {
          return false;
        }
      tab[key] = val.substr (1, val.length () - 2);
    }

  tryset (result, tab);
  return true;
}

/**
 * Parses time string ("Ns" or "NN:NN:NN").
 * @param s Time string.
 * @param result Variable to store the resulting time.
 * @return True if successful, or false otherwise.
 */
bool
try_parse_time (const string &s, Time *result)
{
  gchar *dup;
  gchar *end;
  double secs;

  dup = g_strdup (s.c_str ());
  g_assert_nonnull (dup);
  g_strchomp (dup);

  secs = g_strtod (dup, &end);
  if (*end == '\0' || g_str_equal (end, "s"))
    goto success;

  if (*end != ':')
    goto failure;

  end++;
  secs = 3600 * secs + 60 * g_strtod (end, &end);
  if (*end != ':')
    goto failure;

  end++;
  secs += g_strtod (end, &end);
  if (*end != '\0')
    goto failure;

success:
  g_free (dup);
  tryset (result, (Time) (secs * GINGA_SECOND));
  return true;

failure:
  g_free (dup);
  return false;
}

// Asserted wrappers for parse_*.
#define _GINGA_PARSE_DEFN(Type, Name, Str)                                 \
  Type parse_##Name (const string &s)                                      \
  {                                                                        \
    Type result;                                                           \
    if (unlikely (!ginga::try_parse_##Name (s, &result)))                  \
      ERROR ("invalid %s string '%s'", Str, s.c_str ());                   \
    return result;                                                         \
  }

_GINGA_PARSE_DEFN (bool, bool, "boolean")
_GINGA_PARSE_DEFN (Color, color, "color")
_GINGA_PARSE_DEFN (Time, time, "time")

list<string>
parse_list (const string &s, char sep, size_t min, size_t max)
{
  list<string> result;
  if (unlikely (!ginga::try_parse_list (s, sep, min, max, &result)))
    ERROR ("invalid list string '%s'", s.c_str ());
  return result;
}

map<string, string>
parse_table (const string &s)
{
  map<string, string> result;
  if (unlikely (!ginga::try_parse_table (s, &result)))
    ERROR ("invalid Lua-like table string '%s'", s.c_str ());
  return result;
}

/**
 * @brief Parses number or percent string to an integer.
 * @param s Number or percent string.
 * @param base Base value used to convert percent to integer.
 * @param min Minimum resulting integer.
 * @param max Maximum resulting integer.
 * @return The resulting integer.
 */
int
parse_percent (const string &s, int base, int min, int max)
{
  bool percent;
  double d;
  int result;

  d = xstrtodorpercent (s, &percent);
  if (percent)
    {
      result = (int) lround (d * base);
    }
  else
    {
      result = (int) lround (d);
    }
  return (int) CLAMP (result, min, max);
}

/**
 * @brief Parses pixel string to an integer.
 * @param s Pixel string.
 * @return The resulting integer.
 */
guint8
parse_pixel (const string &s)
{
  return (guint8) parse_percent (s, 255, 0, 255);
}

// Strings -----------------------------------------------------------------

/**
 * @brief Converts string to a floating-point number.
 * @param s Number string.
 * @param dp Variable to store the resulting floating-point number.
 * @return True if successful, or false otherwise.
 */
bool
_xstrtod (const string &s, double *dp)
{
  const gchar *c_str;
  gchar *endptr;
  double d;

  c_str = s.c_str ();
  d = g_ascii_strtod (c_str, &endptr);
  if (endptr == c_str)
    return false;

  tryset (dp, d);
  return true;
}

/**
 * @brief Converts string to a 64-bit integer.
 * @param s Number string.
 * @param ip Variable to store the resulting integer.
 * @param base The base to be used in the conversion.
 * @return True if successful, or false otherwise.
 */
bool
_xstrtoll (const string &s, gint64 *ip, guint base)
{
  const gchar *c_str;
  gchar *endptr;
  gint64 i;

  c_str = s.c_str ();
  i = g_ascii_strtoll (c_str, &endptr, base);
  if (endptr == c_str)
    return false;

  tryset (ip, i);
  return true;
}

/**
 * @brief Converts string to an unsigned 64-bit integer.
 * @param s Number string.
 * @param ip Variable to store the resulting unsigned integer.
 * @param base The base to be used in the conversion.
 * @return True if successful, or false otherwise.
 */
bool
_xstrtoull (const string &s, guint64 *ip, guint base)
{
  const gchar *c_str;
  gchar *endptr;
  guint64 u;

  c_str = s.c_str ();
  u = g_ascii_strtoull (c_str, &endptr, base);
  if (endptr == c_str)
    return false;

  tryset (ip, u);
  return true;
}

// Asserted wrappers for _xstrtod, _xstrtoll, and _xstrtoull.
double
xstrtod (const string &s)
{
  double d;
  g_assert (_xstrtod (s, &d));
  return d;
}

#define _GINGA_XSTRTO_DEFN(Type, Typemin, Typemax)                         \
  g##Type xstrto##Type (const string &s, guint8 base)                      \
  {                                                                        \
    gint64 x = 0;                                                          \
    g_assert (_xstrtoll (s, &x, base));                                    \
    return (g##Type) (CLAMP (x, Typemin, Typemax));                        \
  }

_GINGA_XSTRTO_DEFN (int, G_MININT, G_MAXINT)
_GINGA_XSTRTO_DEFN (int8, G_MININT8, G_MAXINT8)
_GINGA_XSTRTO_DEFN (int64, G_MININT64, G_MAXINT64)

#define _GINGA_XSTRTOU_DEFN(Type, Typemax)                                 \
  g##Type xstrto##Type (const string &s, guint8 base)                      \
  {                                                                        \
    guint64 x = 0;                                                         \
    g_assert (_xstrtoull (s, &x, base));                                   \
    return (g##Type) (MIN (x, Typemax));                                   \
  }

_GINGA_XSTRTOU_DEFN (uint, G_MAXUINT)
_GINGA_XSTRTOU_DEFN (uint8, G_MAXUINT8)
_GINGA_XSTRTOU_DEFN (uint64, G_MAXUINT64)

/**
 * @brief Checks if string is a percent value.
 * @param s Value string.
 * @return True if successful, or false otherwise.
 */
bool
xstrispercent (const string &s)
{
  const gchar *str = s.c_str ();
  gchar *end;

  if (str[0] == '%')
    return false;

  g_ascii_strtod (str, &end);
  return *end == '%';
}

/**
 * @brief Converts number or percent string to floating-point number.
 * @param s Number or percent string.
 * @param perc Variable to store whether the converted value is a number or
 * percent value.
 * @result The resulting number.
 */
// Converts a number or percent string to a number.
gdouble
xstrtodorpercent (const string &s, bool *perc)
{
  gchar *end;
  gdouble x = g_ascii_strtod (s.c_str (), &end);
  if (*end == '%')
    {
      tryset (perc, true);
      return x / 100.;
    }
  else
    {
      tryset (perc, false);
      return x;
    }
}

/**
 * @brief Compares two strings ignoring case.
 * @param s1 First string.
 * @param s2 Second string.
 * @return -1, 0, or 1
 */
int
xstrcasecmp (const string &s1, const string &s2)
{
  return g_ascii_strcasecmp (s1.c_str (), s2.c_str ());
}

/**
 * @brief Tests string prefix.
 * @param s String.
 * @param prefix Prefix.
 * @return True if successful, or false otherwise.
 */
bool
xstrhasprefix (const string &s, const string &prefix)
{
  return g_str_has_prefix (s.c_str (), prefix.c_str ());
}

/**
 * @brief Tests string suffix.
 * @param s String.
 * @param suffix Suffix.
 * @return True if successful, or false otherwise.
 */
bool
xstrhassuffix (const string &s, const string &suffix)
{
  return g_str_has_suffix (s.c_str (), suffix.c_str ());
}

/**
 * @brief Assigns format to string.
 * @param s Resulting string.
 * @param format Format string.
 * @return The number of bytes assigned.
 */
int
xstrassign (string &s, const char *format, ...)
{
  va_list args;
  char *c_str = NULL;
  int n;

  va_start (args, format);
  n = g_vasprintf (&c_str, format, args);
  va_end (args);

  g_assert (n >= 0);
  g_assert_nonnull (c_str);
  s.assign (c_str);
  g_free (c_str);

  return n;
}

/**
 * @brief Builds string from format.
 * @param format Format string.
 * @return The resulting string.
 */
string
xstrbuild (const char *format, ...)
{
  va_list args;
  char *c_str = NULL;
  int n;
  string s;

  va_start (args, format);
  n = g_vasprintf (&c_str, format, args);
  va_end (args);

  g_assert (n >= 0);
  g_assert_nonnull (c_str);
  s.assign (c_str);
  g_free (c_str);

  return s;
}

/**
 * @brief Removes leading and trailing whitespace from string.
 */
string
xstrstrip (string s)
{
  gchar *dup = g_strdup (s.c_str ());
  s.assign (g_strstrip (dup));
  g_free (dup);
  return s;
}

/**
 * @brief Splits strings according to separator.
 * @param s String.
 * @param sep Separator.
 * @return The resulting vector.
 */
list<string>
xstrsplit (const string &s, char sep)
{
  list<string> result;
  stringstream ss (s);
  string tok;

  while (getline (ss, tok, sep))
    result.push_back (tok);

  return result;
}

// Paths -------------------------------------------------------------------

/**
 * @brief Returns the basename of path.
 */
string
xpathbasename (string path)
{
  gchar *dir = g_path_get_basename (path.c_str ());
  path.assign (dir);
  g_free (dir);
  return path;
}

/**
 * @brief Returns the dirname of path.
 */
string
xpathdirname (string path)
{
  gchar *dir = g_path_get_dirname (path.c_str ());
  path.assign (dir);
  g_free (dir);
  return path;
}

/**
 * @brief Checks if path is absolute.
 */
bool
xpathisabs (const string &path)
{
  return g_path_is_absolute (path.c_str ());
}

/**
 * @brief Checks if path is an URI.
 */
bool
xpathisuri (const string &path)
{
  gchar *dup = g_uri_parse_scheme (path.c_str ());
  return (dup == NULL) ? false : (g_free (dup), true);
}

/**
 * @brief Makes path absolute.
 */
string
xpathmakeabs (string path)
{
  if (!xpathisabs (path))
    {
      gchar *cwd = g_get_current_dir ();
      gchar *dup = g_build_filename (cwd, path.c_str (), NULL);
      g_free (cwd);
      path.assign (dup);
      g_free (dup);
    }
  return path;
}

/**
 * @brief Builds a path from the given components.
 */
string
xpathbuild (const string &a, const string &b)
{
  string path;
  gchar *dup = g_build_filename (a.c_str (), b.c_str (), NULL);
  path.assign (dup);
  g_free (dup);
  return path;
}

/**
 * @brief Builds an absolute path from the given components.
 */
string
xpathbuildabs (const string &a, const string &b)
{
  return xpathmakeabs ((a == ".") ? b : xpathbuild (a, b));
}

/**
 * @brief Transforms the uri in a path.
 * @param path
 * @return the string with path
 */
string
xpathfromuri (const string &uri)
{
  string path;
  GError *err;

  gchar *g_path = g_filename_from_uri (uri.c_str (), nullptr, &err);
  if (g_path == nullptr)
    {
      ERROR ("%s.", err->message);
      g_error_free (err);
    }

  path = string (g_path);
  g_free (g_path);
  return path;
}

/**
 * @brief Transforms the src attribute content on a uri with full path.
 * @param src
 * @param baseuri
 * @return the string with the uri
 */
string
xurifromsrc (const string &src, const string &baseuri = "")
{
  string uri, abs;

  if (xpathisuri (src))
    {
      uri = src;
    }
  else
    {
      if (xpathisabs (src))
        abs = src;
      else
        abs = xpathbuildabs (baseuri, src);

      GError *err;
      gchar *g_uri = g_filename_to_uri (abs.c_str (), NULL, &err);
      if (g_uri == nullptr)
        {
          ERROR ("%s.", err->message);
          g_error_free (err);
        }

      uri = string (g_uri);
      g_free (g_uri);
    }

  return uri;
}

/**
 * @brief Gets the content of an uri (which can be both local or remote).
 *
 * The \p uri must be a valid URI. (Fullpaths without file:// are not
 * considered valid by this function.)
 *
 * @param uri
 * @param content
 * @param err
 * @return If the operation was successful or not.
 */
bool
xurigetcontents (const string &uri, string &data)
{
  GError *err = NULL;
  char *gdata = NULL;
  gsize len;

  GFile *file = g_file_new_for_uri (uri.c_str ());
  gboolean ret
      = g_file_load_contents (file, NULL, &gdata, &len, NULL, &err);
  if (ret)
    {
      g_assert_null (err);
      data = string (gdata);
      g_free (gdata);
    }
  else
    {
      g_assert_nonnull (err);
      string msg = (!g_file_query_exists (file, NULL))
                       ? "uri content does not exist."
                       : err->message;

      WARNING ("cannot load content from uri '%s': %s", uri.c_str (),
               msg.c_str ());
    }

  g_object_unref (file);

  return ret;
}

/**
 * @brief Returns the parent URI of uri.
 * @param uri
 * @return the parent URI or empty string if fails.
 */
string
xurigetparent (const string &uri)
{
  string parent_uri = "";
  GFile *file = g_file_new_for_uri (uri.c_str ());
  GFile *parent = g_file_get_parent (file);

  if (parent)
    {
      gchar *gparent_uri = g_file_get_uri (parent);
      parent_uri = string (gparent_uri);
      g_free (gparent_uri);
    }

  g_object_unref (parent);
  g_object_unref (file);

  return parent_uri;
}

// User data ---------------------------------------------------------------

UserData::UserData ()
{
}

UserData::~UserData ()
{
  for (auto it : _udata)
    if (it.second.second != nullptr)
      it.second.second (it.second.first);
}

bool
UserData::getData (const string &key, void **value)
{
  auto it = _udata.find (key);
  if (it == _udata.end ())
    return false;
  tryset (value, it->second.first);
  return true;
}

bool
UserData::setData (const string &key, void *value, UserDataCleanFunc fn)
{
  auto it = _udata.find (key);
  if (it != _udata.end () && it->second.second)
    it->second.second (it->second.first);
  if (value == nullptr)
    {
      _udata.erase (it);
      return false;
    }
  else
    {
      _udata[key] = std::make_pair (value, fn);
      return it == _udata.end ();
    }
}

GINGA_NAMESPACE_END
