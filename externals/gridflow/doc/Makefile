all:: index.pd numop.pd numtype.pd

index.pd: make_index.tcl locale/english.tcl
	tclsh make_index.tcl > index.pd

numop.pd numtype.pd: moulinette.tcl
	tclsh moulinette.tcl
