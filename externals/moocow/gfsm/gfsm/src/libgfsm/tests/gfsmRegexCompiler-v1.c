#include <gfsmRegexCompiler.h>
#include <gfsmArith.h>
#include <gfsmUtils.h>

#define RETURN(rec,_rea) (rec)->rea=(_rea); return (_rea);

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_epsilon(gfsmRegexCompiler *rec)
{
  rec->rea.typ = gfsmREATEmpty;
  rec->rea.val.lab = 0;
  return rec->rea;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_automaton_new_fsm(gfsmRegexCompiler *rec)
{
  gfsmAutomaton *fsm = gfsm_automaton_new_full(gfsmAutomatonDefaultFlags,
					       rec->srtype,
					       gfsmAutomatonDefaultSize);
  fsm->flags.is_transducer = FALSE;
  return fsm;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_automaton_epsilon_fsm(gfsmRegexCompiler *rec)
{
  gfsmAutomaton *fsm = gfsm_regex_automaton_new_fsm(rec);
  fsm->root_id = gfsm_automaton_add_state(fsm);
  gfsm_automaton_set_final_state(fsm,fsm->root_id,TRUE);
  return fsm;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_automaton_label_fsm(gfsmRegexCompiler *rec, gfsmLabelVal lab)
{
  gfsmAutomaton *fsm = gfsm_regex_automaton_new_fsm(rec);
  gfsmStateId    labid;
  fsm->root_id = gfsm_automaton_add_state(fsm);
  labid        = gfsm_automaton_add_state(fsm);
  gfsm_automaton_add_arc(fsm, fsm->root_id, labid, lab, lab, fsm->sr->one);
  gfsm_automaton_set_final_state(fsm,labid,TRUE);
  return fsm;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_automaton_fsm(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea)
{
  switch (rea.typ) {
  case gfsmREATEmpty:
    return gfsm_regex_automaton_epsilon_fsm(rec);
    break;
  case gfsmREATLabel:
    return gfsm_regex_automaton_label_fsm(rec, rea.val.lab);
    break;
  case gfsmREATFull:
  default:
    return rea.val.fsm;
    break;
  }
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_regex_automaton_expand(gfsmRegexCompiler *rec, gfsmRegexAutomaton *rea)
{
  rea->val.fsm = gfsm_regex_automaton_fsm(rec,*rea);
  rea->typ     = gfsmREATFull;
  return rea->val.fsm;
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_label(gfsmRegexCompiler *rec, gfsmLabelVal lab)
{
  rec->rea.typ = gfsmREATLabel;
  rec->rea.val.lab = lab;
  return rec->rea;
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_concat(gfsmRegexCompiler *rec,
					       gfsmRegexAutomaton rea1,
					       gfsmRegexAutomaton rea2)
{
  switch (rea2.typ) {
  case gfsmREATEmpty:
    break;
  case gfsmREATLabel:
    gfsm_regex_automaton_append_lab(rec, gfsm_regex_automaton_expand(rec,&rea1), rea2.val.lab);
    break;
  case gfsmREATFull:
  default:
    gfsm_automaton_concat(gfsm_regex_automaton_expand(rec,&rea1), rea2.val.fsm);
    gfsm_automaton_free(rea2.val.fsm);
    break;
  }
  
  RETURN(rec,rea1);
}


//--------------------------------------------------------------
struct _gfsm_regex_append_lab_data {
  gfsmAutomaton     *fsm;
  gfsmLabelVal       lab;
  gfsmStateId        newid;
};

gboolean _gfsm_regex_append_lab_foreach_func(gfsmStateId qid, gpointer pw,
					     struct _gfsm_regex_append_lab_data *data)
{
  gfsm_automaton_get_state(data->fsm,qid)->is_final = FALSE;
  gfsm_automaton_add_arc(data->fsm, qid, data->newid, data->lab, data->lab, gfsm_ptr2weight(pw));
  return FALSE;
}

gfsmAutomaton *gfsm_regex_automaton_append_lab(gfsmRegexCompiler *rec, gfsmAutomaton *fsm, gfsmLabelVal lab)
{
  struct _gfsm_regex_append_lab_data data = { fsm, lab, gfsm_automaton_add_state(fsm) };
  gfsm_weightmap_foreach(fsm->finals,
			 (GTraverseFunc)_gfsm_regex_append_lab_foreach_func,
			 &data);
  gfsm_weightmap_clear(fsm->finals);
  gfsm_automaton_set_final_state(fsm, data.newid, TRUE);
  return fsm;
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_closure(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea, gboolean is_plus)
{
  gfsm_automaton_closure(gfsm_regex_automaton_expand(rec,&rea),is_plus);
  RETURN(rec,rea);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_power(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea, guint32 n)
{
  gfsm_automaton_n_closure(gfsm_regex_automaton_expand(rec,&rea),n);
  RETURN(rec,rea);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_project(gfsmRegexCompiler *rec,
						gfsmRegexAutomaton rea,
						gfsmLabelSide which)
{
  gfsm_automaton_project(gfsm_regex_automaton_expand(rec,&rea),which);
  RETURN(rec,rea);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_optional(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea)
{
  gfsm_automaton_optional(gfsm_regex_automaton_expand(rec,&rea));
  RETURN(rec,rea);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_complement(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea)
{
  gfsm_automaton_complement_full(gfsm_regex_automaton_expand(rec,&rea),rec->abet);
  RETURN(rec,rea);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_union(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea1, gfsmRegexAutomaton rea2)
{
  gfsm_automaton_union(gfsm_regex_automaton_expand(rec,&rea1),gfsm_regex_automaton_expand(rec,&rea2));
  gfsm_automaton_free(gfsm_regex_automaton_expand(rec,&rea2));
  RETURN(rec,rea1);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_intersect(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea1, gfsmRegexAutomaton rea2)
{
  gfsm_automaton_intersect(gfsm_regex_automaton_expand(rec,&rea1),gfsm_regex_automaton_expand(rec,&rea2));
  gfsm_automaton_free(gfsm_regex_automaton_expand(rec,&rea2));
  RETURN(rec,rea1);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_product(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea1, gfsmRegexAutomaton rea2)
{
  _gfsm_automaton_product(gfsm_regex_automaton_expand(rec,&rea1),gfsm_regex_automaton_expand(rec,&rea2));
  gfsm_automaton_free(gfsm_regex_automaton_expand(rec,&rea2));
  RETURN(rec,rea1);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_compose(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea1, gfsmRegexAutomaton rea2)
{
  gfsm_automaton_compose(gfsm_regex_automaton_expand(rec,&rea1),gfsm_regex_automaton_expand(rec,&rea2));
  gfsm_automaton_free(gfsm_regex_automaton_expand(rec,&rea2));
  RETURN(rec,rea1);
}

//--------------------------------------------------------------
gfsmRegexAutomaton gfsm_regex_automaton_difference(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea1, gfsmRegexAutomaton rea2)
{
  gfsm_automaton_difference(gfsm_regex_automaton_expand(rec,&rea1),gfsm_regex_automaton_expand(rec,&rea2));
  gfsm_automaton_free(gfsm_regex_automaton_expand(rec,&rea2));
  RETURN(rec,rea1);
}

//--------------------------------------------------------------
/** Weight */
gfsmRegexAutomaton gfsm_regex_automaton_weight(gfsmRegexCompiler *rec, gfsmRegexAutomaton rea, gfsmWeight w)
{
  gfsm_automaton_arith_final(gfsm_regex_automaton_expand(rec,&rea), gfsmAOSRTimes, w, FALSE);
  RETURN(rec,rea);
}
