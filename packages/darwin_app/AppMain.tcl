# from http://aspn.activestate.com/ASPN/Mail/Message/tcl-mac/2115298

# tk::mac::OpenDocument is called when docs are dropped 
# on the Dock icon with the filenames put into the var args
proc tk::mac::OpenDocument {args} {
    foreach file $args {
        puts $file
    }
}

proc default_keybindings {} {
	 return { exit CMD-q }
}

#console show

# get the path to the Wish Shell so a relative path can be 
# used to launch Pd
regsub -- "Wish Shell" [info nameofexecutable] "" wish_path
exec open $wish_path/../Resources/Pd.term

puts "wish_path $wish_path"

#if {[string first "-psn" [lindex $argv 0]] == 0} {
#     set argv [lrange $argv 1 end]
#}



exit
