#ifndef _GFSM_REGEX_COMPILER_H
#define _GFSM_REGEX_COMPILER_H

#include <gfsmScanner.h>
#include <gfsmAutomaton.h>
#include <gfsmAlphabet.h>
#include <gfsmAlgebra.h>

/** Regex automaton type */
typedef enum {
  gfsmREATEmpty,   ///< empty acceptor
  gfsmREATLabel,   ///< single label
  gfsmREATFull     ///< full automaton
} gfsmRegexAutomatonType;

/** Regex automaton value */
typedef union {
  gfsmLabelVal   lab; ///< single label
  gfsmAutomaton *fsm; ///< full automaton
} gfsmRegexAutomatonValue;

/** Regex automaton */
typedef struct {
  gfsmRegexAutomatonType  typ; ///< regex type
  gfsmRegexAutomatonValue val; ///< regex value
} gfsmRegexAutomaton;

/** Data structure for regex compiler */
typedef struct {
  gfsmScanner     scanner;     ///< scanner
  gfsmSRType      srtype;      ///< semiring type
  gfsmRegexAutomaton  rea;     ///< regex automaton under construction
  gfsmAlphabet       *abet;    ///< alphabet
  GString            *gstr;    ///< buffer
  gboolean            is_label : 1; ///< is this a singleton fsm? (if so, *fsm is a gfsmLabelVal)
} gfsmRegexCompiler;

/** New full-fleded automaton */
gfsmAutomaton *gfsm_regex_automaton_new_fsm(gfsmRegexCompiler *rec);

/** Get full-fledged automaton */
gfsmAutomaton *gfsm_regex_automaton_fsm(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea);

/** Full Epsilon recognizer */
gfsmAutomaton *gfsm_regex_automaton_epsilon_fsm(gfsmRegexCompiler *rec);

/** Full single-character recognizer */
gfsmAutomaton *gfsm_regex_automaton_label_fsm(gfsmRegexCompiler *rec, gfsmLabelVal lab);


/** Single-label recognizer */
gfsmRegexAutomaton gfsm_regex_automaton_label(gfsmRegexCompiler *rec, gfsmLabelVal lab);

/** Single-label concatenation (low-level) */
gfsmAutomaton *gfsm_regex_automaton_append_lab(gfsmRegexCompiler *rec,
					       gfsmAutomaton    *fsm,
					       gfsmLabelVal lab);

/** General concatenation */
gfsmRegexAutomaton gfsm_regex_automaton_concat(gfsmRegexCompiler *rec,
						gfsmRegexAutomaton rea1,
						gfsmRegexAutomaton rea2);

/** Closure */
gfsmRegexAutomaton gfsm_regex_automaton_closure(gfsmRegexCompiler *rec,
						 gfsmRegexAutomaton rea,
						 gboolean is_plus);

/** Power (n-ary closure) */
gfsmRegexAutomaton gfsm_regex_automaton_power(gfsmRegexCompiler *rec,
					       gfsmRegexAutomaton rea,
					       guint32 n);

/** Projection */
gfsmRegexAutomaton gfsm_regex_automaton_project(gfsmRegexCompiler *rec,
						gfsmRegexAutomaton rea,
						gfsmLabelSide which);


/** Optionality */
gfsmRegexAutomaton gfsm_regex_automaton_optional(gfsmRegexCompiler *rec,
						  gfsmRegexAutomaton rea);

/** Complement */
gfsmRegexAutomaton gfsm_regex_automaton_complement(gfsmRegexCompiler *rec,
						    gfsmRegexAutomaton rea);

/** Union */
gfsmRegexAutomaton gfsm_regex_automaton_union(gfsmRegexCompiler *rec,
					       gfsmRegexAutomaton rea1,
					       gfsmRegexAutomaton rea2);

/** Intersection */
gfsmRegexAutomaton gfsm_regex_automaton_intersect(gfsmRegexCompiler *rec,
						   gfsmRegexAutomaton rea1,
						   gfsmRegexAutomaton rea2);

/** Product */
gfsmRegexAutomaton gfsm_regex_automaton_product(gfsmRegexCompiler *rec,
						 gfsmRegexAutomaton rea1,
						 gfsmRegexAutomaton rea2);

/** Composition */
gfsmRegexAutomaton gfsm_regex_automaton_compose(gfsmRegexCompiler *rec,
						 gfsmRegexAutomaton rea1,
						 gfsmRegexAutomaton rea2);

/** Difference */
gfsmRegexAutomaton gfsm_regex_automaton_difference(gfsmRegexCompiler *rec,
						    gfsmRegexAutomaton rea1,
						    gfsmRegexAutomaton rea2);

/** Weight */
gfsmRegexAutomaton gfsm_regex_automaton_weight(gfsmRegexCompiler *rec,
						gfsmRegexAutomaton rea1,
						gfsmWeight w);




#endif /* _GFSM_REGEX_COMPILER_H */
