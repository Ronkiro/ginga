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

#include "aux-ginga.h"
#include "Parser.h"

#include "Context.h"
#include "Document.h"
#include "Media.h"
#include "MediaSettings.h"

#include <libxml/tree.h>
#include <libxml/parser.h>

GINGA_NAMESPACE_BEGIN

// Helper macros and functions.
#define toCString(s) deconst (char *, (s))
#define toXmlChar(s) (xmlChar *)(deconst (char *, (s).c_str ()))
#define toString(s)  string (deconst (char *, (s)))

static inline bool
xmlGetPropAsString (xmlNode *node, const string &name, string *result)
{
  xmlChar *str = xmlGetProp (node, toXmlChar (name));
  if (str == nullptr)
    return false;
  tryset (result, toString (str));
  g_free (str);
  return true;
}


// Parser internal state.

// Element cache.
typedef struct ParserCache
{
  string id;                    // node id
  string tag;                   // node tag
  xmlNode *node;                // node object
  map<string, string> attrs;    // node attributes
} ParserCache;

// Connector cache.
typedef struct ParserConnRole
{
  string role;                  // role label
  Event::Type eventType;        // role event type
  Event::Transition transition; // role transition
  bool condition;               // whether role is condition
  Predicate *predicate;         // role associated predicate (if condition)
  string value;                 // role value (if attribution)
  string key;                   // role key (if selection)
} ParserConnRole;

typedef struct ParserConnCache
{
  string id;                    // connector id
  list<ParserConnRole> roles;   // connector roles
} ParserConnCache;

// Link cache.
typedef struct ParserLinkBind
{
  xmlNode *node;                // bind node
  string role;                  // bind role
  string component;             // bind component
  string interface;             // bind interface
  map<string, string> params;   // bind parameters
} ParserLinkBind;

typedef struct ParserLinkCache
{
  string id;                    // link id
  string connector;             // link connector id
  Context *context;             // parent context
  map<string, string> params;   // link parameters
  list<ParserLinkBind> binds;   // link binds
} ParserLinkCache;

// Parser state.
typedef struct ParserState
{
  Document *doc;                // NCL document
  xmlDoc *xml;                  // DOM tree
  Rect rect;                    // screen dimensions
  Rect saved_rect;              // saved screen dimensions
  int genid;                    // last generated id
  string errmsg;                // last error message

  // objects
  list<Object *> objStack;      // object stack

  // element cache
  map<string, ParserCache> cache;              // cached elements
  map<string, list<ParserCache *>> cacheByTag; // cached elements (by tag)

  // connector cache
  map<string, ParserConnCache> connCache; // cached connectors
  ParserConnCache *currentConn;           // current connector
  list<Predicate *> predStack;            // predicate stack
  struct                                  // current predicate data
  {
    Predicate *pred;
    bool has_left;
    bool has_right;
    string left;
    string right;
    Predicate::Test test;
  } currentPred;

  // link cache
  map<string, ParserLinkCache> linkCache; // cached links
  ParserLinkCache *currentLink;           // current link
} ParserState;

// Initializes parser state.
#define PARSER_STATE_INIT(st,doc,xml,w,h)       \
  G_STMT_START                                  \
  {                                             \
    g_assert_nonnull ((doc));                   \
    (st)->doc = (doc);                          \
    g_assert_nonnull ((xml));                   \
    (st)->xml = (xml);                          \
    g_assert_cmpint ((w), >, 0);                \
    g_assert_cmpint ((h), >, 0);                \
    (st)->rect = {0,0,(w),(h)};                 \
    (st)->saved_rect = (st)->rect;              \
    (st)->genid = 0;                            \
    (st)->errmsg = "";                          \
    (st)->currentConn = nullptr;                \
    (st)->currentLink = nullptr;                \
  }                                             \
  G_STMT_END

// Sets parser state error message.
static inline G_GNUC_PRINTF (2,3) void
_st_err (ParserState *st, const char *fmt, ...)
{
  va_list args;
  char *c_str = nullptr;
  int n;

  va_start (args, fmt);
  n = g_vasprintf (&c_str, fmt, args);
  va_end (args);

  g_assert (n >= 0);
  g_assert_nonnull (c_str);
  st->errmsg.assign (c_str);
  g_free (c_str);
}

#define ST_ERR(st, fmt, ...)\
  (_st_err ((st), fmt, ## __VA_ARGS__), false)

#define ST_ERR_LINE(st, line, fmt, ...)\
  ST_ERR ((st), "Syntax error at line %d: " fmt, (line), ## __VA_ARGS__)

#define ST_ERR_ELT(st, elt, fmt, ...)                   \
  ST_ERR_LINE ((st), (elt)->line, "Element <%s>: " fmt, \
               toCString ((elt)->name), ## __VA_ARGS__)

#define ST_ERR_ELT_UNKNOWN(st, elt)\
  ST_ERR_ELT ((st), (elt), "Unknown element")

#define ST_ERR_ELT_MISSING_PARENT(st, elt)\
  ST_ERR_ELT ((st), (elt), "Missing parent")

#define ST_ERR_ELT_BAD_PARENT(st, elt, parent)\
  ST_ERR_ELT ((st), (elt), "Bad parent <%s>", (parent))

#define ST_ERR_ELT_UNKNOWN_ATTR(st, elt, attr)\
  ST_ERR_ELT ((st), (elt), "Unknown attribute '%s'", (attr))

#define ST_ERR_ELT_MISSING_ATTR(st, elt, attr)\
  ST_ERR_ELT ((st), (elt), "Missing attribute '%s'", (attr))

#define ST_ERR_ELT_BAD_ATTR(st, elt, attr, val, explain)                \
  ST_ERR_ELT ((st), (elt), "Bad value '%s' for attribute '%s'%s",       \
              (val), (attr), (explain != nullptr)                       \
              ? (" (" + string (explain) + ")").c_str () : "")

#define ST_ERR_ELT_UNKNOWN_CHILD(st, elt, child)\
  ST_ERR_ELT ((st), (elt), "Unknown child <%s>", (child))

#define ST_ERR_ELT_MISSING_CHILD(st, elt, child)\
  ST_ERR_ELT ((st), (elt), "Missing child <%s>", (child))

// Generates unique id.
static string
st_gen_id (ParserState *st)
{
  return xstrbuild ("__unamed-%d__", st->genid++);
}

// Pushes object onto stack.
static void
st_obj_stack_push (ParserState *st, Object *obj)
{
  g_assert_nonnull (obj);
  st->objStack.push_back (obj);
}

// Gets object on top of stack.
static Object *
st_obj_stack_peek (ParserState *st)
{
  g_assert (!st->objStack.empty ());
  return st->objStack.back ();
}

// Pops object from stack.
static Object *
st_obj_stack_pop (ParserState *st)
{
  Object *obj = st_obj_stack_peek (st);
  st->objStack.pop_back ();
  return obj;
}

// Index element cache by id.
static bool
st_cache_index (ParserState *st, const string &id,
                ParserCache **result)
{
  auto it = st->cache.find (id);
  if (it == st->cache.end ())
    return false;
  tryset (result, &it->second);
  return true;
}

// Index element cache by tag.
static const list<ParserCache *> *
st_cache_index_by_tag (ParserState *st, const string &tag)
{
  auto it = st->cacheByTag.find (tag);
  if (it == st->cacheByTag.end ())
    return nullptr;
  return &it->second;
}

// Resolve id-ref using element cache.
static bool
st_cache_resolve_idref (ParserState *st, const string &id,
                        set<string> tags, xmlNode **result_node,
                        map<string,string> **result_attrs)
{
  ParserCache *entry;

  if (!st_cache_index (st, id, &entry))
    return false;
  if (entry->node->type != XML_ELEMENT_NODE)
    return false;
  if (tags.find (entry->tag) == tags.end ())
    return false;

  tryset (result_node, entry->node);
  tryset (result_attrs, &entry->attrs);
  return true;
}

// Index connector cache by id.
static const ParserConnCache *
st_conn_cache_index (ParserState *st, const string &id)
{
  auto it = st->connCache.find (id);
  if (it == st->connCache.end ())
    return nullptr;
  return &it->second;
}

// Index link cache by id.
static const ParserLinkCache *
st_link_cache_index (ParserState *st, const string &id)
{
  auto it = st->linkCache.find (id);
  if (it == st->linkCache.end ())
    return nullptr;
  return &it->second;
}


// NCL syntax.

// Element push function.
typedef bool (ParserPushFunc) (ParserState *, xmlNode *,
                               map<string, string> *);
// Element pop function.
typedef bool (ParserPopFunc) (ParserState *, xmlNode *,
                              map<string, string> *, list<xmlNode *> *);
// Attribute info.
typedef struct ParserSyntaxAttr
{
  string name;                              // attribute name
  bool required;                            // whether attribute is required
  bool (*check) (const string &, string *); // syntax check function
} ParserSyntaxAttr;

// Element info.
typedef struct ParserSyntaxElt
{
  ParserPushFunc *push;                // push function
  ParserPopFunc *pop;                  // pop function
  int flags;                           // processing flags
  vector<string> parents;              // possible parents
  vector<ParserSyntaxAttr> attributes; // attributes
} ParserSyntaxElt;

// Element processing flags.
typedef enum
{
  PARSER_SYNTAX_FLAG_CACHE  = 1<<0, // cache element
  PARSER_SYNTAX_FLAG_GEN_ID = 2<<0, // generate id if not present
} ParserSyntaxFlag;

// Forward declarations.
#define PARSER_SYNTAX_ATTR_CHECK_DECL(attr)             \
  static bool G_PASTE (parser_syntax_attr_check_, attr) \
    (const string &, string *);

#define PARSER_PUSH_DECL(elt)                           \
  static bool G_PASTE (parser_push_, elt)               \
    (ParserState *, xmlNode *, map<string, string> *);  \


#define PARSER_POP_DECL(elt)                            \
  static bool G_PASTE (parser_pop_, elt)                \
    (ParserState *, xmlNode *, map<string, string> *,   \
     list<xmlNode *> *);

PARSER_SYNTAX_ATTR_CHECK_DECL (id)

PARSER_PUSH_DECL (ncl)
PARSER_POP_DECL  (ncl)
PARSER_PUSH_DECL (region)
PARSER_POP_DECL  (region)
PARSER_PUSH_DECL (descriptorParam)
PARSER_PUSH_DECL (causalConnector)
PARSER_POP_DECL  (causalConnector)
PARSER_PUSH_DECL (compoundCondition)
PARSER_POP_DECL  (compoundCondition)
PARSER_PUSH_DECL (assessmentStatement)
PARSER_POP_DECL  (assessmentStatement)
PARSER_PUSH_DECL (attributeAssessment)
PARSER_PUSH_DECL (valueAssessment)
PARSER_PUSH_DECL (simpleCondition)
PARSER_PUSH_DECL (simpleAction)
PARSER_PUSH_DECL (context)
PARSER_PUSH_DECL (port)
PARSER_POP_DECL  (context)
PARSER_PUSH_DECL (media)
PARSER_POP_DECL  (media)
PARSER_PUSH_DECL (area)
PARSER_PUSH_DECL (property)
PARSER_PUSH_DECL (link)
PARSER_POP_DECL  (link)
PARSER_PUSH_DECL (linkParam)
PARSER_PUSH_DECL (bind)
PARSER_PUSH_DECL (bindParam)

// Common attribute entries.
#define PARSER_SYNTAX_ATTR_ID\
  {"id", true, parser_syntax_attr_check_id}
#define PARSER_SYNTAX_ATTR_OPT_ID\
  {"id", false, parser_syntax_attr_check_id}
#define PARSER_SYNTAX_ATTR_ROLE\
  {"role", true, parser_syntax_attr_check_id}


static map<string, ParserSyntaxElt> parser_syntax =
{
 {"ncl",                                        // element name
  {parser_push_ncl,                             // push function
   parser_pop_ncl,                              // pop function
   0,                                           // flags
   {},                                          // possible parents
   {PARSER_SYNTAX_ATTR_OPT_ID,                  // attributes
    {"title", false, nullptr},
    {"xmlns", false, nullptr}}},
 },
 //
 // Head.
 //
 {"head",
  {nullptr, nullptr,
   0,
   {"ncl"}, {}},
 },
 {"regionBase",
  {nullptr, nullptr,
   0,
   {"head"},
   {PARSER_SYNTAX_ATTR_OPT_ID,
    {"device", false, nullptr},
    {"region", false, nullptr}}},
 },
 {"region",
  {parser_push_region, parser_pop_region,
   PARSER_SYNTAX_FLAG_CACHE,
   {"region", "regionBase"},
   {PARSER_SYNTAX_ATTR_ID,
    {"title", false, nullptr},
    {"left", false, nullptr},
    {"right", false, nullptr},
    {"top", false, nullptr},
    {"bottom", false, nullptr},
    {"height", false, nullptr},
    {"width", false, nullptr},
    {"zIndex", false, nullptr}}},
 },
 {"descriptorBase",
  {nullptr, nullptr,
   0,
   {"head"},
   {PARSER_SYNTAX_ATTR_OPT_ID}},
 },
 {"descriptor",
  {nullptr, nullptr,
   PARSER_SYNTAX_FLAG_CACHE,
   {"descriptorBase"},
   {PARSER_SYNTAX_ATTR_ID,
    {"left", false, nullptr},
    {"right", false, nullptr},
    {"top", false, nullptr},
    {"bottom", false, nullptr},
    {"height", false, nullptr},
    {"width", false, nullptr},
    {"zIndex", false, nullptr},
    {"region", false, nullptr},
    {"player", false, nullptr},
    {"explicitDur", false, nullptr},
    {"freeze", false, nullptr},
    {"moveLeft", false, nullptr},
    {"moveRight", false, nullptr},
    {"moveUp", false, nullptr},
    {"moveDown", false, nullptr},
    {"focusIndex", false, nullptr},
    {"focusBorderColor", false, nullptr},
    {"focusBorderWidth", false, nullptr},
    {"focusBorderTransparency", false, nullptr},
    {"focusSrc", false, nullptr},
    {"focusSelSrc", false, nullptr},
    {"selBorderColor", false, nullptr},
    {"transIn", false, nullptr},
    {"transOut", false, nullptr}}},
 },
 {"descriptorParam",
  {parser_push_descriptorParam, nullptr,
   0,
   {"descriptor"},
   {{"name", true, nullptr},
    {"value", true, nullptr}}},
 },
 {"connectorBase",
  {nullptr, nullptr,
   0,
   {"head"},
   {PARSER_SYNTAX_ATTR_OPT_ID}},
 },
 {"causalConnector",
  {parser_push_causalConnector, parser_pop_causalConnector,
   PARSER_SYNTAX_FLAG_CACHE,
   {"connectorBase"},
   {PARSER_SYNTAX_ATTR_ID}},
 },
 {"connectorParam",
  {nullptr, nullptr,
   0,
   {"causalConnector"},
   {{"name", true, nullptr}}},
 },
 {"compoundCondition",
  {parser_push_compoundCondition, parser_pop_compoundCondition,
   0,
   {"causalConnector", "compoundCondition"},
   {{"operator", false, nullptr}, // ignored
    {"delay", false, nullptr}}},  // ignored
 },
 {"compoundStatement",
  {nullptr, nullptr,
   0,
   {"compoundCondition", "compoundStatement"},
   {{"operator", true, nullptr},
    {"isNegated", false, nullptr}}},
 },
 {"assessmentStatement",
  {parser_push_assessmentStatement, parser_pop_assessmentStatement,
   0,
   {"compoundCondition", "compoundStatement"},
   {{"comparator", true, nullptr}}},
 },
 {"attributeAssessment",
  {parser_push_attributeAssessment, nullptr,
   0,
   {"assessmentStatement"},
   {PARSER_SYNTAX_ATTR_ROLE,
    {"eventType", false, nullptr},     // ignored
    {"key", false, nullptr},           // ignored
    {"attributeType", false, nullptr}, // ignored
    {"offset", false, nullptr}}},      // ignored
 },
 {"valueAssessment",
  {parser_push_valueAssessment, nullptr,
   0,
   {"assessmentStatement"},
   {{"value", true, nullptr}}},
 },
 {"simpleCondition",
  {parser_push_simpleCondition, nullptr,
   0,
   {"causalConnector", "compoundCondition"},
   {PARSER_SYNTAX_ATTR_ROLE,
    {"eventType", false, nullptr},
    {"key", false, nullptr},
    {"transition", false, nullptr},
    {"delay", false, nullptr},       // ignored
    {"min", false, nullptr},         // ignored
    {"max", false, nullptr},         // ignored
    {"qualifier", false, nullptr}}}, // ignored
 },
 {"compoundAction",
  {nullptr, nullptr,
   0,
   {"causalConnector", "compoundAction"},
   {{"operator", false, nullptr}, // ignored
    {"delay", false, nullptr}}},  // ignored
 },
 {"simpleAction",
  {parser_push_simpleAction, nullptr,
   0,
   {"causalConnector", "compoundAction"},
   {PARSER_SYNTAX_ATTR_ROLE,
    {"eventType", false, nullptr},
    {"actionType", false, nullptr},
    {"value", false, nullptr},
    {"delay", false, nullptr},       // ignored
    {"duration", false, nullptr},    // ignored
    {"min", false, nullptr},         // ignored
    {"max", false, nullptr},         // ignored
    {"min", false, nullptr},         // ignored
    {"qualifier", false, nullptr},   // ignored
    {"repeat", false, nullptr},      // ignored
    {"repeatDelay", false, nullptr}, // ignored
    {"by", false, nullptr}}},        // ignored
 },
 //
 // Body.
 //
 {"body",                       // -> Context
  {parser_push_context, parser_pop_context,
   0,
   {"ncl"},
   {PARSER_SYNTAX_ATTR_OPT_ID}},
 },
 {"context",                    // -> Context
  {parser_push_context, parser_pop_context,
   PARSER_SYNTAX_FLAG_CACHE,
   {"body", "context"},
   {PARSER_SYNTAX_ATTR_ID}},
 },
 {"port",
  {parser_push_port, nullptr,
   PARSER_SYNTAX_FLAG_CACHE,
   {"body", "context"},
   {PARSER_SYNTAX_ATTR_ID,
    {"component", true, nullptr},
    {"interface", false, nullptr}}},
 },
 {"media",                      // -> Media
  {parser_push_media, parser_pop_media,
   PARSER_SYNTAX_FLAG_CACHE,
   {"body", "context", "switch"},
   {PARSER_SYNTAX_ATTR_ID,
    {"src", false, nullptr},
    {"type", false, nullptr},
    {"descriptor", false, nullptr}}},
 },
 {"area",
  {parser_push_area, nullptr,
   0,
   {"media"},
   {PARSER_SYNTAX_ATTR_ID,
    {"begin", false, nullptr},
    {"end", false, nullptr}}},
 },
 {"property",
  {parser_push_property, nullptr,
   0,
   {"body", "context", "media"},
   {{"name", true, nullptr},
    {"value", false, nullptr}}},
 },
 {"link",
  {parser_push_link, parser_pop_link,
   PARSER_SYNTAX_FLAG_CACHE | PARSER_SYNTAX_FLAG_GEN_ID,
   {"body", "context"},
   {PARSER_SYNTAX_ATTR_OPT_ID,
    {"xconnector", true, nullptr}}},
 },
 {"linkParam",
  {parser_push_linkParam, nullptr,
   0,
   {"link"},
   {{"name", true, nullptr},
    {"value", true, nullptr}}}
 },
 {"bind",
  {parser_push_bind, nullptr,
   0,
   {"link"},
   {PARSER_SYNTAX_ATTR_ROLE,
    {"component", true, nullptr},
    {"interface", false, nullptr}}},
 },
 {"bindParam",
  {parser_push_bindParam, nullptr,
   0,
   {"bind"},
   {{"name", true, nullptr},
    {"value", true, nullptr}}},
 },
};

// Indexes element map.
static bool
parser_syntax_index (const string &tag, ParserSyntaxElt **result)
{
  map<string, ParserSyntaxElt>::iterator it;
  if ((it = parser_syntax.find (tag)) == parser_syntax.end ())
    return false;
  tryset (result, &it->second);
  return true;
}

// Gets possible children of a given element.
static map<string, bool>
parser_syntax_get_possible_children (const string &tag)
{
  map<string, bool> result;
  for (auto it: parser_syntax)
    for (auto parent: it.second.parents)
      if (parent == tag)
        result[it.first] = true;
  return result;
}

// Checks if id is valid.
static bool
parser_syntax_attr_check_id (const string &id, string *errmsg)
{
  const char *str;
  char c;

  if (id == "")
    {
      tryset (errmsg, "must not be empty");
      return false;
    }

  str = id.c_str ();
  while ((c = *str++) != '\0')
    {
      if (!(g_ascii_isalnum (c) || c == '-' || c == '_'
            || c == ':' || c == '.'))
        {
          tryset (errmsg, xstrbuild ("must not contain '%c'", c));
          return false;
        }
    }
  return true;
}

// Parses "role" attribute.
static bool
parser_syntax_parse_role (const string &role, Event::Type *type,
                          Event::Transition *transition)
{
  static map<string, pair<int,int>> reserved =
    {
     {"onBegin",
      {(int) Event::PRESENTATION,
       (int) Event::START}},
     {"onEnd",
      {(int) Event::PRESENTATION,
       (int) Event::STOP}},
     {"onAbort",
      {(int) Event::PRESENTATION,
       (int) Event::ABORT}},
     {"onPause",
      {(int) Event::PRESENTATION,
       (int) Event::PAUSE}},
     {"onResumes",
      {(int) Event::PRESENTATION,
       (int) Event::RESUME}},
     {"onBeginAttribution",
      {(int) Event::ATTRIBUTION,
       (int) Event::START}},
     {"onEndAttribution",
      {(int) Event::SELECTION,
       (int) Event::STOP}},
     {"onSelection",
      {(int) Event::SELECTION,
       (int) Event::START}},
     {"start",
      {(int) Event::Type::PRESENTATION,
       (int) Event::Transition::START}},
     {"stop",
      {(int) Event::Type::PRESENTATION,
       (int) Event::Transition::STOP}},
     {"abort",
      {(int) Event::Type::PRESENTATION,
       (int) Event::Transition::ABORT}},
     {"pause",
      {(int) Event::Type::PRESENTATION,
       (int) Event::Transition::PAUSE}},
     {"resume",
      {(int) Event::Type::PRESENTATION,
       (int) Event::Transition::RESUME}},
     {"set",
      {(int) Event::Type::ATTRIBUTION,
       (int) Event::Transition::START}},
    };
  map<string, pair<int,int>>::iterator it;
  if ((it = reserved.find (role)) == reserved.end ())
    return false;
  tryset (type, (Event::Type) it->second.first);
  tryset (transition, (Event::Transition) it->second.second);
  return true;
}

// Parses "eventType" attribute.
static bool
parser_syntax_parse_event_type (const string &str, Event::Type *result)
{
  static map<string, Event::Type> good =
    {
     {"presentation", Event::PRESENTATION},
     {"attribution", Event::ATTRIBUTION},
     {"selection", Event::SELECTION},
    };
  auto it = good.find (str);
  if (it == good.end ())
    return false;
  tryset (result, it->second);
  return true;
}

// Parses "transition" attribute.
static bool
parser_syntax_parse_transition (const string &str,
                                Event::Transition *result)
{
  static map<string, Event::Transition> good =
    {
     {"starts", Event::START},
     {"stops", Event::STOP},
     {"aborts", Event::ABORT},
     {"pauses", Event::PAUSE},
     {"resumes", Event::RESUME},
    };
  auto it = good.find (str);
  if (it == good.end ())
    return false;
  tryset (result, it->second);
  return true;
}

// Parses and solve references to ghost binds.
static bool
parser_syntax_parse_ghost (const map<string, string> *ghosts,
                           const map<string, string> *params)
{
  bool modified = false;
  for (auto it: *params)
    {
      auto ghost = ghosts->find ('$' + it.first);
      if (ghost == ghosts->end ())
        continue;
      it.second = ghost->second;
      modified = true;
    }
  return modified;
}

// Parses and solve references to bind or link parameters.
static bool
parser_syntax_parse_parameter (const string &value,
                               const map<string, string> *bindParams,
                               const map<string, string> *linkParams,
                               string *result)
{
  string key;

  if (value[0] != '$')
    return false;

  key = value.substr (1, value.length () - 1);
  auto it = bindParams->find (key);
  if (it != bindParams->end ())
    {
      tryset (result, it->second);
      return true;
    }
  else
    {
      auto it = linkParams->find (key);
      if (it != linkParams->end ())
        {
          tryset (result, it->second);
          return true;
        }
    }

  return false;
}


// Attribute map helper functions.

static inline bool
parser_attrmap_index (map<string, string> *attrs, const string &name,
                      string *result)
{
  MAP_GET_IMPL (*attrs, name, result);
}

static inline string
parser_attrmap_get (map<string, string> *attrs, const string &name)
{
  string value;
  g_assert (parser_attrmap_index (attrs, name, &value));
  return value;
}

static inline string
parser_attrmap_opt_get (map<string, string> *attrs, const string &name,
                        const string &defvalue)
{
  string result;
  return parser_attrmap_index (attrs, name, &result) ? result : defvalue;
}


// Parse <ncl>.

static bool
parser_push_ncl (ParserState *st, unused (xmlNode *node),
                 map<string, string> *attrs)
{
  Context *root;
  string id;

  root = st->doc->getRoot ();
  g_assert_nonnull (root);

  if (parser_attrmap_index (attrs, "id", &id))
    root->addAlias (id);

  st_obj_stack_push (st, root);
  return true;
}

static bool
parser_pop_ncl (ParserState *st, unused (xmlNode *node),
                unused (map<string, string> *attrs),
                unused (list<xmlNode *> *children))
{
  const list<ParserCache *> *cachedDescriptors;
  const list<ParserCache *> *cachedMedias;
  const list<ParserCache *> *cachedLinks;

  // Resolve descriptor reference to region.
  cachedDescriptors = st_cache_index_by_tag (st, "descriptor");
  if (cachedDescriptors != nullptr)
    {
      for (auto entry: *cachedDescriptors)
        {
          string region_id;
          xmlNode *region_node;
          map<string, string> *region_attrs;

          if (!parser_attrmap_index (&entry->attrs, "region", &region_id))
            continue;           // nothing to do

          if (unlikely (!st_cache_resolve_idref (st, region_id, {"region"},
                                                 &region_node,
                                                 &region_attrs)))
            {
              ST_ERR_ELT_BAD_ATTR (st, entry->node, "region",
                                   region_id.c_str (), "no such region");
              return false;
            }
          for (auto it: *region_attrs)
            {
              if (it.first == "id")
                continue;           // nothing to do
              entry->attrs[it.first] = it.second;
            }

        }
    }

  // Resolve media reference to descriptor.
  cachedMedias = st_cache_index_by_tag (st, "media");
  if (cachedMedias != nullptr)
    {
      for (auto entry: *cachedMedias)
        {
          string id;
          Media *media;

          string desc_id;
          xmlNode *desc_node;
          map<string, string> *desc_attrs;

          id = parser_attrmap_get (&entry->attrs, "id");
          media = cast (Media *, st->doc->getObjectByIdOrAlias (id));
          g_assert_nonnull (media);

          if (!parser_attrmap_index (&entry->attrs, "descriptor", &desc_id))
            continue;           // nothing to do

          if (unlikely (!st_cache_resolve_idref
                        (st, desc_id, {"descriptor"}, &desc_node,
                         &desc_attrs)))
            {
              ST_ERR_ELT_BAD_ATTR (st, entry->node, "descriptor",
                                   desc_id.c_str (), "no such descriptor");
              return false;
            }

          for (auto it: *desc_attrs)
            {
              if (it.first == "id" || it.first == "region")
                continue;           // nothing to do
              if (media->getAttributionEvent (it.first) != nullptr)
                continue;           // already defined
              media->addAttributionEvent (it.first);
              media->setProperty (it.first, it.second);
            }
        }
    }

  // Resolve link reference to connector.
  cachedLinks = st_cache_index_by_tag (st, "link");
  if (cachedLinks != nullptr)
    {
      for (auto entry: *cachedLinks)
        {
          string id;
          Context *ctx;
          const ParserLinkCache *link;
          const ParserConnCache *conn;

          list<pair<const ParserConnRole *, const ParserLinkBind *>> bound;
          map<string,string> ghosts; // ghost binds

          list<Action> conditions;
          list<Action> actions;

          id = parser_attrmap_get (&entry->attrs, "id");

          link = st_link_cache_index (st, id);
          g_assert_nonnull (link);

          ctx = link->context;
          g_assert_nonnull (ctx);

          conn = st_conn_cache_index (st, link->connector);
          if (unlikely (conn == nullptr))
            {
              ST_ERR_ELT_BAD_ATTR (st, entry->node, "xconnector",
                                   link->connector.c_str (),
                                   "no such connector");
              return false;
            }

          // Collect active binds (vs ghost binds).
          for (auto &bind: link->binds)
            {
              const ParserConnRole *role;
              bool found = false;

              for (auto &rl: conn->roles)
                {
                  if (bind.role == rl.role)
                    {
                      found = true;
                      role = &rl;
                      break;
                    }
                }

              if (found)        // active bind
                {
                  bound.push_back (std::make_pair (role, &bind));
                }
              else              // ghost bind
                {
                  Object *obj = ctx->getChildById (bind.component);
                  if (unlikely (obj == nullptr))
                    {
                      ST_ERR_ELT_BAD_ATTR (st, bind.node, "component",
                                           bind.component.c_str (),
                                           "no such component");
                      return false;
                    }
                  if (bind.interface == "")
                    {
                      ST_ERR_ELT_BAD_ATTR (st, bind.node, "interface",
                                           bind.interface.c_str (),
                                           "ghost bind requires "
                                           "nonempty interface");
                      return false;
                    }
                  ghosts[bind.role]
                    = "$" + bind.component
                    + "." + bind.interface;
                }
            }

          // Update link parameters.
          parser_syntax_parse_ghost (&ghosts, &link->params);

          // Check if link matches connector.
          if (unlikely (bound.size () < conn->roles.size ()))
            {
              ST_ERR_ELT_BAD_ATTR (st, entry->node, "xconnector",
                                   link->connector.c_str (),
                                   "link does not match connector");
              return false;
            }

          // Process active binds.
          for (auto it: bound)
            {
              const ParserConnRole *role;
              const ParserLinkBind *bind;
              Object *obj;
              Event *evt;

              role = it.first;
              bind = it.second;

              // Update bind parameters
              parser_syntax_parse_ghost (&ghosts, &bind->params);

              // Check component.
              obj = ctx->getChildById (bind->component);
              if (unlikely (obj == nullptr))
                {
                  ST_ERR_ELT_BAD_ATTR (st, bind->node, "component",
                                       bind->component.c_str (),
                                       "no such component");
                  return false;
                }

              // Check interface.
              evt = nullptr;
              switch (role->eventType)
                {
                case Event::PRESENTATION:
                  if (bind->interface == "")
                    {
                      evt = obj->getLambda ();
                      g_assert_nonnull (evt);
                    }
                  else
                    {
                      evt = obj->getPresentationEvent (bind->interface);
                      if (unlikely (evt == nullptr))
                        {
                          string extra = "no such area in object '"
                            + obj->getId () + "'";
                          ST_ERR_ELT_BAD_ATTR (st, bind->node, "interface",
                                               bind->interface.c_str (),
                                               extra.c_str ());
                          return false;
                        }
                    }
                  break;

                case Event::ATTRIBUTION:
                  if (unlikely (bind->interface == ""
                      || ((evt = obj->getAttributionEvent (bind->interface))
                          == nullptr)))
                    {
                      string extra = "no such property in object '"
                        + obj->getId () + "'";
                      ST_ERR_ELT_BAD_ATTR (st, bind->node, "interface",
                                           bind->interface.c_str (),
                                           extra.c_str ());
                      return false;
                    }
                  if (role->condition)
                    {
                      string value;
                      value = role->value;
                      parser_syntax_parse_parameter (value, &bind->params,
                                                     &link->params, &value);
                    }
                  else
                    {
                      g_assert_not_reached ();
                    }
                  break;

                case Event::SELECTION:
                  g_assert_not_reached ();
                  break;

                default:
                  g_assert_not_reached ();
                }

              g_assert_nonnull (evt);
            }
        }
    }

  st_obj_stack_pop (st);
  return true;
}


// Parse <region>.

static bool
parser_push_region (ParserState *st, xmlNode *node,
                    map<string, string> *attrs)
{
  static int last_zorder = 0;
  Rect screen;
  Rect parent;
  Rect rect;
  string str;

  g_assert_nonnull (node->parent);
  if (toString (node->parent->name) != "region") // this is a root region
    screen = st->saved_rect = st->rect;
  else
    screen = st->saved_rect;

  rect = parent = st->rect;
  if (parser_attrmap_index (attrs, "left", &str))
    {
      rect.x += ginga::parse_percent (str, parent.width, 0, G_MAXINT);
    }
  if (parser_attrmap_index (attrs, "top", &str))
    {
      rect.y += ginga::parse_percent (str, parent.height, 0, G_MAXINT);
    }
  if (parser_attrmap_index (attrs, "width", &str))
    {
      rect.width = ginga::parse_percent (str, parent.width, 0, G_MAXINT);
    }
  if (parser_attrmap_index (attrs, "height", &str))
    {
      rect.height = ginga::parse_percent (str, parent.height, 0, G_MAXINT);
    }
  if (parser_attrmap_index (attrs, "right", &str))
    {
      rect.x += parent.width - rect.width
        - ginga::parse_percent (str, parent.width, 0, G_MAXINT);
    }
  if (parser_attrmap_index (attrs, "bottom", &str))
    {
      rect.y += parent.height - rect.height
        - ginga::parse_percent (str, parent.height, 0, G_MAXINT);
    }

  st->rect = rect;
  (*attrs)["zorder"] = xstrbuild ("%d", last_zorder++);
  (*attrs)["left"] = xstrbuild
    ("%.2f%%", ((double) rect.x / screen.width) * 100.);
  (*attrs)["top"] = xstrbuild
    ("%.2f%%", ((double) rect.y / screen.height) * 100.);
  (*attrs)["width"] = xstrbuild
    ("%.2f%%", ((double) rect.width / screen.width) * 100.);
  (*attrs)["height"] = xstrbuild
    ("%.2f%%", ((double) rect.height / screen.height) * 100.);

  return true;
}

static bool
parser_pop_region (ParserState *st, xmlNode *node,
                   unused (map<string, string> *attrs),
                   unused (list<xmlNode *> *children))
{
  g_assert_nonnull (node->parent);
  if (toString (node->parent->name) != "region") // root region
    st->rect = st->saved_rect;
  return true;
}


// Parse <descriptorParam>.

static bool
parser_push_descriptorParam (ParserState *st, xmlNode *node,
                             map<string, string> *attrs)
{
  string desc_id;
  ParserCache *entry;
  string name;
  string value;

  g_assert (xmlGetPropAsString (node->parent, "id", &desc_id));
  g_assert (st_cache_index (st, desc_id, &entry));
  name = parser_attrmap_get (attrs, "name");
  value = parser_attrmap_get (attrs, "value");
  entry->attrs[name] = value;

  return true;
}


// Parse <causalConnector>.

static bool
parser_push_causalConnector (ParserState *st,
                             unused (xmlNode *node),
                             map<string, string> *attrs)
{
  string id;

  id = parser_attrmap_get (attrs, "id");
  g_assert_null (st_conn_cache_index (st, id));
  g_assert_null (st->currentConn);
  st->currentConn = &st->connCache[id];

  return true;
}

static bool
parser_pop_causalConnector (unused (ParserState *st),
                            unused (xmlNode *node),
                            unused (map<string, string> *attrs),
                            unused (list<xmlNode *> *children))
{
  bool status;
  int nconds;
  int nacts;

  status = true;
  nconds = 0;
  nacts = 0;

  for (auto &role: st->currentConn->roles)
    {
      if (role.condition)
        nconds++;
      else
        nacts++;
    }

  if (unlikely (nconds == 0))
    {
      status = ST_ERR_ELT_MISSING_CHILD (st, node, "simpleCondition");
      goto done;
    }

  if (unlikely (nacts == 0))
    {
      status = ST_ERR_ELT_MISSING_CHILD (st, node, "simpleAction");
      goto done;
    }

 done:
  g_assert_nonnull (st->currentConn);
  st->currentConn = nullptr;
  return status;
}


// Parser <compoundCondition>.

static bool
parser_push_compoundCondition (ParserState *st,
                               unused (xmlNode *node),
                               unused (map<string, string> *attrs))
{
  st->predStack.push_back (new Predicate (Predicate::CONJUNCTION));
  return true;
}

static bool
parser_pop_compoundCondition (unused (ParserState *st),
                              unused (xmlNode *node),
                              unused (map<string, string> *attrs),
                              unused (list<xmlNode *> *children))
{
  g_assert (!st->predStack.empty ());
  delete st->predStack.back ();
  st->predStack.pop_back ();
  return true;
}


// Parse <assessmentStatement>.

static bool
parser_push_assessmentStatement (ParserState *st, xmlNode *node,
                                 map<string, string> *attrs)
{
  string comp;
  Predicate::Test test;

  Predicate *parent;
  Predicate *pred;

  comp = parser_attrmap_get (attrs, "comparator");
  if (comp == "eq")
    test = Predicate::EQ;
  else if (comp == "ne")
    test = Predicate::NE;
  else if (comp == "lt")
    test = Predicate::LT;
  else if (comp == "lte")
    test = Predicate::LE;
  else if (comp == "gt")
    test = Predicate::GT;
  else if (comp == "gte")
    test = Predicate::GE;
  else
    return ST_ERR_ELT_BAD_ATTR
      (st, node, "comparator", comp.c_str (), "no such comparator");

  g_assert (!st->predStack.empty ());
  parent = st->predStack.back ();
  g_assert_nonnull (parent);

  switch (parent->getType ())
    {
    case Predicate::NEGATION:
    case Predicate::CONJUNCTION:
    case Predicate::DISJUNCTION:
      break;
    default:
      g_assert_not_reached ();
    }

  pred = new Predicate (Predicate::ATOM);
  parent->addChild (pred);

  // Reset current predicate.
  st->currentPred.pred = pred;
  st->currentPred.has_left = false;
  st->currentPred.has_right = false;
  st->currentPred.test = test;

  return true;
}

static bool
parser_pop_assessmentStatement (ParserState *st, xmlNode *node,
                                unused (map<string, string> *attrs),
                                unused (list<xmlNode *> *children))
{
  if (unlikely (!st->currentPred.has_left))
    return ST_ERR_ELT_MISSING_CHILD (st, node, "attributeAssessment");

  if (unlikely (!st->currentPred.has_right))
    return ST_ERR_ELT_MISSING_CHILD (st, node, "valueAssessment");

  st->currentPred.pred->setTest
    (st->currentPred.left,
     st->currentPred.test,
     st->currentPred.right);

  return true;
}


// Parse <attributeAssessment>.

static bool
parser_push_attributeAssessment (ParserState *st,
                                 unused (xmlNode *node),
                                 map<string, string> *attrs)
{
  string role;

  if (unlikely (st->currentPred.has_left && st->currentPred.has_right))
    {
      // TODO: WARN
      return true;              // nothing to do
    }

  role = parser_attrmap_get (attrs, "role");
  if (!st->currentPred.has_left)
    {
      st->currentPred.left = "$" + role;
      st->currentPred.has_left = true;
    }
  else
    {
      st->currentPred.right = "$" + role;
      st->currentPred.has_right = true;
    }

  return true;
}


// Parse <valueAssessment>.

static bool
parser_push_valueAssessment (unused (ParserState *st),
                             unused (xmlNode *node),
                             unused (map<string, string> *attrs))
{
  string value;

  if (unlikely (st->currentPred.has_left && st->currentPred.has_right))
    {
      // TODO: WARN
      return true;              // nothing to do
    }

  value = parser_attrmap_get (attrs, "value");
  if (!st->currentPred.has_left)
    {
      st->currentPred.left = value;
      st->currentPred.has_left = true;
    }
  else
    {
      st->currentPred.right = value;
      st->currentPred.has_right = true;
    }

  return true;
}


// Parse <simpleCondition>.

static bool
parser_push_simpleCondition (ParserState *st, xmlNode *node,
                             map<string, string> *attrs)
{
  ParserConnRole role;
  string transition;
  string key;

  role.role = parser_attrmap_get (attrs, "role");
  role.condition = (toString (node->name) == "simpleCondition");
  transition = (role.condition) ? "transition" : "actionType";

  if (!parser_syntax_parse_role (role.role, &role.eventType,
                                 &role.transition))
    {
      string str;

      if (unlikely (!parser_attrmap_index (attrs, "eventType", &str)))
        return ST_ERR_ELT_MISSING_ATTR (st, node, "eventType");
      if (unlikely (!parser_syntax_parse_event_type (str, &role.eventType)))
        return ST_ERR_ELT_BAD_ATTR
          (st, node, "eventType", str.c_str (), "no such event type");

      if (unlikely (!parser_attrmap_index (attrs, transition, &str)))
        return ST_ERR_ELT_MISSING_ATTR (st, node, transition.c_str ());
      if (unlikely (!parser_syntax_parse_transition (str,
                                                     &role.transition)))
        {
          return ST_ERR_ELT_BAD_ATTR (st, node, transition.c_str (),
                                      str.c_str (), "no such transition");
        }
    }
  else                          // reserved role
    {
      string str;

      if (unlikely (parser_attrmap_index (attrs, "eventType", &str)))
        return ST_ERR_ELT_BAD_ATTR
          (st, node, "eventType", str.c_str (),
           ("role '" + role.role
            +"' is reserved and cannot be overwritten").c_str ());

      if (unlikely (parser_attrmap_index (attrs, transition, &str)))
        return ST_ERR_ELT_BAD_ATTR
          (st, node, transition.c_str (), str.c_str (),
           ("role '" + role.role
            + "' is reserved and cannot be overwritten").c_str ());
    }

  if (!role.condition && role.eventType == Event::ATTRIBUTION)
    {
      if (unlikely (!parser_attrmap_index (attrs, "value", &role.value)))
        return ST_ERR_ELT_MISSING_ATTR (st, node, "value");
    }

  role.predicate = nullptr;
  if (parser_attrmap_index (attrs, "key", &key))
    role.key = key;

  g_assert_nonnull (st->currentConn);
  st->currentConn->roles.push_back (role);

  return true;
}


// Parse <simpleAction>.

static bool
parser_push_simpleAction (ParserState *st, xmlNode *node,
                          map<string, string> *attrs)
{
  return parser_push_simpleCondition (st, node, attrs);
}


// Parse <context>.

static void
parser_push_context_cleanup (void *ptr)
{
  delete (list <string> *) ptr;
}

static bool
parser_push_context (ParserState *st, xmlNode *node,
                     map<string, string> *attrs)
{
  Object *ctx;
  list<string> *ports;

  if (toString (node->name) == "body")
    {
      string id;

      ctx = cast (Context *, st_obj_stack_peek (st));
      g_assert_nonnull (ctx);

      if (parser_attrmap_index (attrs, "id", &id))
        ctx->addAlias (id);
    }
  else
    {
      Composition *parent;

      parent = cast (Composition *, st_obj_stack_peek (st));
      g_assert_nonnull (parent);

      ctx = new Context (parser_attrmap_get (attrs, "id"));
      parent->addChild (ctx);
    }

  // Create port list.
  ports = new list<string> ();
  g_assert (ctx->setData ("ports", ports, parser_push_context_cleanup));

  st_obj_stack_push (st, ctx);
  return true;
}

static bool
parser_pop_context (unused (ParserState *st), unused (xmlNode *node),
                    unused (map<string, string> *attrs),
                    unused (list<xmlNode *> *children))
{
  bool status;
  Context *ctx;
  list<string> *ports;

  status = true;
  ctx = cast (Context *, st_obj_stack_peek (st));
  g_assert_nonnull (ctx);

  // Resolve port's references.
  g_assert (ctx->getData ("ports", (void **) &ports));
  for (auto port_id: *ports)
    {
      ParserCache *entry;
      Object *target_obj;
      Event *target_evt;
      string comp_id;
      string iface_id;

      g_assert (st_cache_index (st, port_id, &entry));
      comp_id = parser_attrmap_get (&entry->attrs, "component");
      target_obj = ctx->getChildById (comp_id);
      if (unlikely (target_obj == nullptr))
        {
          status = ST_ERR_ELT_BAD_ATTR
            (st, entry->node, "component", comp_id.c_str (),
             "no such object in scope");
          goto done;
        }

      if (!parser_attrmap_index (&entry->attrs, "interface", &iface_id))
        iface_id = "@lambda";

      target_evt = target_obj->getEvent (Event::PRESENTATION, iface_id);
      if (target_evt == nullptr)
        {
          target_evt = target_obj->getEvent (Event::ATTRIBUTION, iface_id);
          if (target_evt == nullptr)
            {
              status = ST_ERR_ELT_BAD_ATTR
                (st, entry->node, "interface", iface_id.c_str (),
                 ("no such interface in object '"
                  + comp_id + "'").c_str ());
              goto done;
            }
        }
      ctx->addPort (target_evt); // success
    }

 done:
  g_assert_false (ctx->setData ("ports", nullptr, nullptr));
  st_obj_stack_pop (st);
  return status;
}


// Parse <port>.

static bool
parser_push_port (ParserState *st, unused (xmlNode *node),
                  map<string, string> *attrs)
{
  Context *ctx;
  list<string> *ports;

  ctx = cast (Context *, st_obj_stack_peek (st));
  g_assert_nonnull (ctx);
  g_assert (ctx->getData ("ports", (void **) &ports));

  ports->push_back (parser_attrmap_get (attrs, "id"));
  return true;
}


// Parse <media>.

static bool
parser_push_media (ParserState *st, unused (xmlNode *node),
                   map<string, string> *attrs)
{
  Composition *parent;
  Media *media;
  string id;
  string type;

  id = parser_attrmap_get (attrs, "id");

  if (parser_attrmap_index (attrs, "type", &type)
      && type == "application/x-ginga-settings")
    {
      media = st->doc->getSettings ();
      g_assert_nonnull (media);
      media->addAlias (id);
    }
  else
    {
      string src = "";
      if (parser_attrmap_index (attrs, "src", &src)
          && !xpathisuri (src) && !xpathisabs (src))
        {
          string dir;
          if (st->xml->URL == nullptr)
            dir = "";
          else
            dir = xpathdirname (toString (st->xml->URL));
          src = xpathbuildabs (dir, src);
        }
      media = new Media (id, type, src);
      g_assert_nonnull (media);
    }

  parent = cast (Composition *, st_obj_stack_peek (st));
  g_assert_nonnull (parent);
  parent->addChild (media);

  st_obj_stack_push (st, media);
  return true;
}

static bool
parser_pop_media (unused (ParserState *st), unused (xmlNode *node),
                  unused (map<string, string> *attrs),
                  unused (list<xmlNode *> *children))
{
  g_assert (instanceof (Media *, st_obj_stack_pop (st)));
  return true;
}


// Parse <area>.

static bool
parser_push_area (ParserState *st, unused (xmlNode *node),
                  map<string, string> *attrs)
{
  Media *media;
  string id;
  string str;
  Time begin, end;

  media = cast (Media *, st_obj_stack_peek (st));
  g_assert_nonnull (media);

  id = parser_attrmap_get (attrs, "id");
  begin = parser_attrmap_index (attrs, "begin", &str)
    ? ginga::parse_time (str) : 0;
  end = parser_attrmap_index (attrs, "end", &str)
    ? ginga::parse_time (str) : GINGA_TIME_NONE;

  media->addPresentationEvent (id, begin, end);

  return true;
}


// Parse <property>.

static bool
parser_push_property (ParserState *st, unused (xmlNode *node),
                      map<string, string> *attrs)
{
  Object *obj;
  string name;
  string value;

  obj = cast (Object *, st_obj_stack_peek (st));
  g_assert_nonnull (obj);

  name = parser_attrmap_get (attrs, "name");
  value = parser_attrmap_opt_get (attrs, "value", "");

  obj->addAttributionEvent (name);
  obj->setProperty (name, value);

  return true;
}


// Parse <link>.

static bool
parser_push_link (ParserState *st, unused (xmlNode *node),
                  map<string, string> *attrs)
{
  string id;
  Context *ctx;

  id = parser_attrmap_get (attrs, "id");
  g_assert_null (st_link_cache_index (st, id));

  ctx = cast (Context *, st_obj_stack_peek (st));
  g_assert_nonnull (ctx);

  g_assert_null (st->currentLink);
  st->currentLink = &st->linkCache[id];

  st->currentLink->id = id;
  st->currentLink->context = ctx;
  st->currentLink->connector = parser_attrmap_get (attrs, "xconnector");

  return true;
}

static bool
parser_pop_link (unused (ParserState *st), unused (xmlNode *node),
                 unused (map<string, string> *attrs),
                 unused (list<xmlNode *> *children))
{
  g_assert_nonnull (st->currentLink);
  st->currentLink = nullptr;
  return true;
}


// Parser <linkParam>.

static bool
parser_push_linkParam (ParserState *st, unused (xmlNode *node),
                       map<string, string> *attrs)
{
  string name;
  string value;

  name = parser_attrmap_get (attrs, "name");
  value = parser_attrmap_get (attrs, "value");

  g_assert_nonnull (st->currentLink);
  st->currentLink->params[name] = value;

  return true;
}


// Parse <bind>.

static bool
parser_push_bind (ParserState *st, xmlNode *node,
                  map<string, string> *attrs)
{
  ParserLinkBind bind;

  bind.node = node;
  bind.role = parser_attrmap_get (attrs, "role");
  bind.component = parser_attrmap_get (attrs, "component");
  parser_attrmap_index (attrs, "interface", &bind.interface);

  g_assert_nonnull (st->currentLink);
  st->currentLink->binds.push_back (bind);

  return true;
}


// Parse <bindParam>.

static bool
parser_push_bindParam (ParserState *st, unused (xmlNode *node),
                       map<string, string> *attrs)
{
  string name;
  string value;
  ParserLinkBind *bind;

  name = parser_attrmap_get (attrs, "name");
  value = parser_attrmap_get (attrs, "value");

  g_assert_nonnull (st->currentLink);
  g_assert (st->currentLink->binds.size () > 0);
  bind = &st->currentLink->binds.back ();
  bind->params[name] = value;

  return true;
}


// Internal functions.

static bool
processElt (ParserState *st, xmlNode *node)
{
  string tag;
  ParserSyntaxElt *elt_syntax;
  bool status;

  map<string, string> _attrs;
  map<string, string> *attrs = &_attrs;
  map<string, bool> possible;
  list<xmlNode *> children;

  status = true;
  tag = toString (node->name);
  if (unlikely (!parser_syntax_index (tag, &elt_syntax)))
    {
      status = ST_ERR_ELT_UNKNOWN (st, node);
      goto done;
    }

  // Check parent.
  g_assert_nonnull (node->parent);
  if (elt_syntax->parents.size () > 0)
    {
      string parent;
      bool found;

      if (unlikely (node->parent->type != XML_ELEMENT_NODE))
        {
          status = ST_ERR_ELT_MISSING_PARENT (st, node);
          goto done;
        }

      parent = toString (node->parent->name);
      found = false;
      for (auto par: elt_syntax->parents)
        {
          if (parent == par)
            {
              found = true;
              break;
            }
        }
      if (unlikely (!found))
        {
          status = ST_ERR_ELT_BAD_PARENT
            (st, node, toCString (node->parent->name));
          goto done;
        }
    }

  // Build attr-map.
  for (auto attr_syntax: elt_syntax->attributes)
    {
      string value;
      string explain;

      if (!xmlGetPropAsString (node, attr_syntax.name, &value))
        {
          if (attr_syntax.name == "id"
              && elt_syntax->flags & PARSER_SYNTAX_FLAG_GEN_ID)
            {
              (*attrs)["id"] = st_gen_id (st);
              continue;
            }
          if (!attr_syntax.required)
            {
              continue;
            }
          status = ST_ERR_ELT_MISSING_ATTR
            (st, node, attr_syntax.name.c_str ());
          goto done;
        }
      (*attrs)[attr_syntax.name] = value;
      if (unlikely (attr_syntax.check != nullptr
                    && !attr_syntax.check (value, &explain)))
        {
          status = ST_ERR_ELT_BAD_ATTR
            (st, node, attr_syntax.name.c_str (),
             value.c_str (), explain.c_str ());
          goto done;
        }
    }

  // Check for unknown attributes.
  for (xmlAttr *prop = node->properties; prop != nullptr; prop = prop->next)
    {
      string name = toString (prop->name);
      if (unlikely (attrs->find (name) == attrs->end ()))
        {
          status = ST_ERR_ELT_UNKNOWN_ATTR (st, node, name.c_str ());
          goto done;
        }
    }

  // Collect id.
  if (parser_attrmap_index (attrs, "id", nullptr))
    {
      string id;

      id = parser_attrmap_get (attrs, "id");

      // Check if id is unique.
      if (unlikely (st->doc->getObjectByIdOrAlias (id)))
        {
          status = ST_ERR_ELT_BAD_ATTR
            (st, node, "id", id.c_str (), "duplicated id");
          goto done;
        }

      // Insert attr-map and element's node into cache.
      if (elt_syntax->flags & PARSER_SYNTAX_FLAG_CACHE)
        {
          string tag = toString (node->name);
          st->cache[id] = {id, tag, node, map<string, string> (_attrs)};
          st->cacheByTag[tag].push_back (&st->cache[id]);
          attrs = &(st->cache[id].attrs);
        }
    }
  else
    {
      g_assert_false (elt_syntax->flags & PARSER_SYNTAX_FLAG_CACHE);
    }

  // Push element.
  if (unlikely (elt_syntax->push && !elt_syntax->push (st, node, attrs)))
    {
      status = false;
      goto done;
    }

  // Collect children.
  possible = parser_syntax_get_possible_children (tag);
  for (xmlNode *child = node->children; child; child = child->next)
    {
      if (child->type != XML_ELEMENT_NODE)
        continue;

      string child_tag = toString (child->name);
      if (unlikely (possible.find (child_tag) == possible.end ()))
        {
          status = ST_ERR_ELT_UNKNOWN_CHILD (st, node, child->name);
          goto done;
        }

      if (unlikely (!processElt (st, child)))
        {
         status = false;
         goto done;
        }

      children.push_back (child);
    }

  // Pop element.
  if (elt_syntax->pop)
    status = elt_syntax->pop (st, node, attrs, &children);

 done:
  return status;
}

static Document *
processDoc (xmlDoc *xml, int width, int height, string *errmsg)
{
  ParserState st;
  Document *doc;
  xmlNode *root;

  doc = new Document ();
  PARSER_STATE_INIT (&st, doc, xml, width, height);
  root = xmlDocGetRootElement (xml);
  g_assert_nonnull (root);

  if (!processElt (&st, root))
    {
      tryset (errmsg, xstrstrip (st.errmsg));
      if (st.doc != nullptr)
        delete st.doc;
      while (!st.predStack.empty ())
        {
          delete st.predStack.back ();
          st.predStack.pop_back ();
        }
      return nullptr;
    }

  g_assert_nonnull (st.doc);
  return st.doc;
}


// External API.

Document *
Parser::parseBuffer (const void *buf, size_t size,
                     int width, int height, string *errmsg)
{
# define FLAGS (XML_PARSE_NOERROR | XML_PARSE_NOWARNING)
  xmlDoc *xml;
  Document *doc;

  xml = xmlReadMemory ((const char *) buf, (int) size,
                       nullptr, nullptr, FLAGS);
  if (unlikely (xml == nullptr))
    {
      xmlError *err = xmlGetLastError ();
      g_assert_nonnull (err);
      tryset (errmsg, "XML error: " + xstrstrip (string (err->message)));
      return nullptr;
    }

  doc = processDoc (xml, width, height, errmsg);
  xmlFreeDoc (xml);
  return doc;
}

Document *
Parser::parseFile (const string &path, int width, int height,
                   string *errmsg)
{
  xmlDoc *xml;
  Document *doc;

  xml = xmlReadFile (path.c_str (), nullptr, FLAGS);
  if (unlikely (xml == nullptr))
    {
      xmlError *err = xmlGetLastError ();
      g_assert_nonnull (err);
      tryset (errmsg, "XML error: " + xstrstrip (string (err->message)));
      return nullptr;
    }

  doc = processDoc (xml, width, height, errmsg);
  xmlFreeDoc (xml);
  return doc;
}

GINGA_NAMESPACE_END
