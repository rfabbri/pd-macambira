
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

  pu_say   antireflexive {t  } ""
  pu_say       reflexive {t  } ""

  pu_say     commutative {t  } ""
  pu_say anticommutative {t  } ""
  pu_say   antisymmetric {t  } ""

  pu_say     associative {t  } ""
  pu_say    distributive {t  } ""
  pu_say      invertible {t  } ""

  pu_say  partialorder   {t  } "partial order (open)"
  pu_say  partialordereq {t  } "partial order (closed)"
  pu_say    totalorder   {t  } "total order (open)"
  pu_say    totalordereq {t  } "total order (closed)"
  pu_say     equivalence {t  } "equivalence relation"

  pu_say      transitive {t  } "transitive: "
  pu_say      trichotomy {t  } "trichotomy: either equal or less or greater"
  pu_say       operator1 {t  } "1-input operator"
  pu_say       operator2 {t  } "2-input operator"

  say_category cancellators
  say  associator "(ab)c-a(bc)"
  say  commutator "ab-ba"
  say distributor "a&(b^c)-(ab^ac)"
  say    invertor "ab/b-a"

  say_category misc
  say twice ""
  say 3times ""
  say 4times ""
  say ^ ""
  say error ""
  say protocols-tree ""
  say tree ""
}
