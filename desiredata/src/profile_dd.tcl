
if 1 {
	puts "profiler version [package require profiler]"
	profiler::init
	# try just: prof
	# or try: prof calls
	proc prof {{arg totalRuntime}} {
		set dump [profiler::dump]
		#foreach {a b} $dump {foreach {c d} $b {set prof($a:$c) $d}}
		set top [profiler::sortFunctions $arg]
		foreach entry $top {
			mset {k v} $entry
			if {!$v} {continue}
			puts [format "%8d  %s" $v $k]
		}
	}
}
if 0 {
	load matjuprofiler/matjuprofiler.so
}
