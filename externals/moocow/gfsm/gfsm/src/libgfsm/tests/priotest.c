#include <glib.h>
#include <stdio.h>
#include <string.h>

const char *prog=NULL;

//======================================================================
// typedefs
typedef enum {
  gfsmAFNone    = 0x0,  /**< no sort field */
  gfsmAFLower   = 0x1,  /**< sort by lower label */
  gfsmAFUpper   = 0x2,  /**< sort by upper label */
  gfsmAFWeight  = 0x3,  /**< sort by weight (refers to semiring) */
  gfsmAFSource  = 0x4,  /**< sort by arc source (if supported and meaningful) */
  gfsmAFTarget  = 0x5,  /**< sort by arc target (if supported and meaningful) */
  gfsmAFUser    = 0x6,  /**< user-defined sort function */
  gfsmAFAll     = 0x7,  /**< not really a sort field: mask of all valid sort fields */
  gfsmAFReverse = 0x8,  /**< not really a sort field: if set, indicates that arc comparisons should be reversed */
  gfsmAFMask    = 0xf   /**< not really a sort field: mask of valid sort fields & reverse flag */
} gfsmArcField;

typedef enum {
  gfsmAFNone    = 0x0,  /**< '_': no sort field */
  gfsmAFLower   = 0x1,  /**< 'l': sort by lower label */
  gfsmAFUpper   = 0x2,  /**< 'u': sort by upper label */
  gfsmAFWeight  = 0x3,  /**< 'w': sort by weight (refers to semiring) */
  gfsmAFSource  = 0x4,  /**< 's': sort by arc source (if supported and meaningful) */
  gfsmAFTarget  = 0x5,  /**< 't': sort by arc target (if supported and meaningful) */
  gfsmAFLowerR  = 0x6,  /**< 'L': reverse sort by lower label */
  gfsmAFUpperR  = 0x7,  /**< 'U': reverse sort by upper label */
  gfsmAFWeightR = 0x8,  /**< 'W': reverse sort semiring weight */
  gfsmAFSourceR = 0x9,  /**< 'S': reverse sort source state (if supported and meaningful) */
  gfsmAFTargetR = 0xa,  /**< 'T': reverse sort target state (if supported and meaningful) */
  gfsmAFUser    = 0xf   /**< 'x': pseudo-field for user-defined comparisons */
} gfsmArcFieldId;

#define gfsmArcFieldShift 4       //-- number of bits in a single logical ::gfsmArcField element
const guint32 gfsmNArcFields = 5; //-- maximum 'nth' paramter supported by ::gfsmArcFieldMask

typedef guint32 gfsmArcFieldMask; //-- mask of ::gfsmArcField values, left-shifted by ::gfsmArcFieldShift


const guint32 gfsmAFM_L   = gfsmAFLower;
const guint32 gfsmAFM_LU  = gfsmAFLower|(gfsmAFUpper<<gfsmArcFieldShift);
const guint32 gfsmAFM_LUW = gfsmAFLower|(gfsmAFUpper<<gfsmArcFieldShift)|(gfsmAFWeight<<(2*gfsmArcFieldShift));

gfsmArcFieldMask gfsm_arc_field_mask_new(guint nth, gfsmArcField field, gboolean reverse)
{
  gfsmArcFieldMask m = field;
  if (reverse) m |= gfsmAFReverse;
  return m << (nth*gfsmArcFieldShift);
}

gfsmArcFieldMask gfsm_arc_field_mask_add(gfsmArcFieldMask m, guint nth, gfsmArcField field, gboolean reverse)
{ return (m | gfsm_arc_field_mask_new(nth,field,reverse)); }

gfsmArcFieldMask gfsm_arc_field_mask_clear(gfsmArcFieldMask m, guint nth)
{ return m & ((~gfsmAFMask)<<(nth*gfsmArcFieldShift)); }

gfsmArcField gfsm_arc_field_mask_get_field(gfsmArcFieldMask m, guint nth)
{ return (m>>(nth*gfsmArcFieldShift))&gfsmAFAll; }

gboolean gfsm_arc_field_mask_get_reverse(gfsmArcFieldMask m, guint nth)
{ return ((m>>(nth*gfsmArcFieldShift))&gfsmAFReverse) ? TRUE : FALSE; }


//======================================================================
// parse
gfsmArcFieldMask parse_mask(const char *str)
{
  gfsmArcFieldMask m = 0;
  gint i;
  guint nth=0;
  /*
  gint   max_tokens = 32;
  gchar **toks = g_strsplit(str,",; \n\t",max_tokens);

  //-- parse
  for (i=0; toks[i] != NULL; i++) {
    gchar *tok = toks[i];
    g_strstrip(tok);
  }
  */
  for (i=0; str[i] && nth < gfsmNArcFields; i++) {
    switch (str[i]) {
    case 'l' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFLower,0); break;
    case 'L' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFLower,1); break;

    case 'u' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFUpper,0); break;
    case 'U' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFUpper,1); break;

    case 'w' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFWeight,0); break;
    case 'W' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFWeight,1); break;

    case 's' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFSource,0); break;
    case 'S' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFSource,1); break;

    case 't' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFTarget,0); break;
    case 'T' : m |= gfsm_arc_field_mask_new(nth++,gfsmAFTarget,1); break;

      //-- silently ignore these
    case 'x':
    case 'X':
    case '-':
    case ',':
    case ' ':
    case '\t':
    case '\n':
      break;

    default:
      g_printerr("%s: character '%c' is not in [sStTlLuUwW] in field string '%s' - skipping\n", prog, str[i], str);
      break;
    }
  }
  if (str[i] && nth==gfsmNArcFields) {
    g_printerr("%s: ignoring trailing characters '%s' in field string '%s'\n", prog, (str+i), str);
  }
  
  //-- cleanup
  //g_strfreev(toks);

  return m;
}

//======================================================================
// dump

const char *mask_field_str(gfsmArcFieldMask afm, guint nth)
{
  switch (gfsm_arc_field_mask_get_field(afm, nth)) {
  case gfsmAFNone:    return "none";
  case gfsmAFLower:   return "lower";
  case gfsmAFUpper:   return "upper";
  case gfsmAFWeight:  return "weight";
  case gfsmAFSource:  return "source";
  case gfsmAFTarget:  return "target";
  default: return "?";
  }
  return "?";
}
const char *mask_reverse_str(gfsmArcFieldMask afm, guint nth)
{
  return gfsm_arc_field_mask_get_reverse(afm, nth) ? ">" : "<";
}

void dump_mask(gfsmArcFieldMask afm, const char *str)
{
  printf("%s: str='%s': priorities = %u = %#0.6x = { %s%s, %s%s, %s%s, %s%s, %s%s }\n",
	 prog, str, afm, afm,
	 mask_field_str(afm,0), mask_reverse_str(afm,0),
	 mask_field_str(afm,1), mask_reverse_str(afm,1),
	 mask_field_str(afm,2), mask_reverse_str(afm,2),
	 mask_field_str(afm,3), mask_reverse_str(afm,3),
	 mask_field_str(afm,4), mask_reverse_str(afm,4)
	 );
}

//======================================================================
// MAIN
int main(int argc, char **argv) {
  int i;
  gfsmArcFieldMask afm = 0;

  prog = argv[0];
  for (i=1; i < argc; i++) {
    afm = parse_mask(argv[i]);
    dump_mask(afm, argv[i]);
  }
  return 0;
}
