
say_namespace summary {
  foreach {x y} {
    f "floating-point"
    ~ "signal"
    \# "grid"
  } {
    say_category basic<$x>
    say ~.do ""
    say ~.norm ""
    say ~.packunpack3 ""
    say ~ ""
    say ~.swap ""
    say ~.taa ""
  }

  say_category interfaces
  proc pu_say {} {
  }

  pu_say  antireflexive {t  } ""
  pu_say  antisymmetric {t  } ""
  pu_say    associative {t  } ""
  pu_say    commutative {t  } ""
  pu_say   distributive {t  } ""
  pu_say    equivalence {t  } ""
  pu_say     invertible {t  } ""
  pu_say partialorder   {t  } ""
  pu_say partialordereq {t  } ""
  pu_say      reflexive {t  } ""
  pu_say   totalorder   {t  } ""
  pu_say   totalordereq {t  } ""
  pu_say     transitive {t  } ""
  pu_say     trichotomy {t  } ""
  pu_say      operator1 {t  } ""
  pu_say      operator2 {t  } ""

  say_category cancellators
  say  associator ""
  say  commutator "ab-ba"
  say distributor ""
  say    invertor ""

  say_category misc
  say twice ""
  say 3times ""
  say 4times ""
  say ^ ""
  say error ""
  say protocols-tree ""
  say tree ""
}
