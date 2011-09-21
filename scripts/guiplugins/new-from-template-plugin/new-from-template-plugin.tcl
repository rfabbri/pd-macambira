
package require pd_menucommands

namespace eval new-from-template {
    variable template
    variable defaultfolder [file join $::env(HOME) Documents]
}

proc ::new-from-template::paste_template {} {
    variable template
    variable defaultfolder

    if { ! [file isdirectory $defaultfolder]} {set defaultfolder $::env(HOME)}
    pdsend "pd filename [_ "new-from-template"] [enquote_path $defaultfolder]"
    foreach line [split [read [open $template r]] "\n"] {
        pdsend $line
    }
    pdsend "#X pop 1"
}

proc ::new-from-template::create {mymenu} {
    variable template
    set inserthere [$mymenu index [_ "Open"]]
    $mymenu insert $inserthere command -label [_ "New from template"] \
        -command {::new-from-template::paste_template}
    set template \
        [file join $::current_plugin_loadpath template.pd]
}

::new-from-template::create .menubar.file
