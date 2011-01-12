
package require Tk 8.5
package require tile
package require pd_bindings
package require pd_menucommands

namespace eval ::dialog_search:: {
    variable selected_file {}
}

# findFiles
# basedir - the directory to start looking in
# pattern - A pattern, as defined by the glob command, that the files must match
proc ::dialog_search::findFiles { basedir pattern } {

    # Fix the directory name, this ensures the directory name is in the
    # native format for the platform and contains a final directory seperator
    set basedir [string trimright [file join [file normalize $basedir] { }]]
    set fileList {}

    # Look in the current directory for matching files, -type {f r}
    # means ony readable normal files are looked at, -nocomplain stops
    # an error being thrown if the returned list is empty
    foreach fileName [glob -nocomplain -type {f r} -path $basedir $pattern] {
        lappend fileList $fileName
    }

    # Now look for any sub direcories in the current directory
    foreach dirName [glob -nocomplain -type {d  r} -path $basedir *] {
        # Recusively call the routine on the sub directory and append any
        # new files to the results
        set subDirList [findFiles $dirName $pattern]
        if { [llength $subDirList] > 0 } {
            foreach subDirFile $subDirList {
                lappend fileList $subDirFile
            }
        }
    }
    return $fileList
}

proc ::dialog_search::selectline {line} {
    variable selected_file
    set selected_file [string replace $line [string first ":" $line] end]
}

proc ::dialog_search::readfile {file varName} {
    upvar \#0 $varName data
    set fp [open $file]
    set data [split [read $fp] \n]
    close $fp
}

proc ::dialog_search::search {searchtext} {
    set widget .search.resultslistbox
    $widget delete 0 end
    foreach docfile [findFiles $::sys_libdir "*.pd"] {
        readfile $docfile data
        searchfile $searchtext $widget \
            [string replace $docfile 0 [string length $::sys_libdir]]
    }
}

proc ::dialog_search::searchfile {searchtext widget filename} {
    global data
    set n 0
    foreach line $data {
        if {[regexp -nocase -- $searchtext $line]} {
            $widget insert end "$filename: $line"
            incr n
        }
    }
#    $widget insert end "Found $n lines"
    $widget see end
}

proc ::dialog_search::ok {mytoplevel} {
    pdtk_post "::dialog_search::ok\n"
}

proc ::dialog_search::cancel {mytoplevel} {
    wm withdraw .search
}

proc ::dialog_search::open_search_dialog {mytoplevel} {
    if {[winfo exists .search]} {
        wm deiconify .search
        raise .search
    } else {
        create_dialog
    }
}

proc ::dialog_search::create_dialog {} {
    variable selected_file
    toplevel .search
    wm title .search [_ "Search Window"]
    entry .search.searchtextentry -bg white -textvar searchtext
    bind .search.searchtextentry <Return> {::dialog_search::search $searchtext}
    # TODO add history like in the find box
    bind .search.searchtextentry <Up> {set searchtext ""}
    listbox .search.resultslistbox -yscrollcommand ".search.yscrollbar set" \
        -bg white -height 20 -width 40
    scrollbar .search.yscrollbar -command ".search.resultslistbox yview"
    bind .search.resultslistbox <<ListboxSelect>> \
        {::dialog_search::selectline [.search.resultslistbox get \
                                   [.search.resultslistbox curselection]]}
    bind .search.resultslistbox <Key-Return> \
        {menu_doc_open $::sys_libdir $::dialog_search::selected_file}
	bind .search.resultslistbox <Double-ButtonRelease-1> \
        {menu_doc_open $::sys_libdir $::dialog_search::selected_file}
    ::pd_bindings::dialog_bindings .search "search"

    grid .search.searchtextentry - -sticky ew
    grid .search.resultslistbox .search.yscrollbar -sticky news
    grid columnconfig . 0 -weight 1
    grid rowconfig    . 1 -weight 1
}

set mymenu .menubar.help
set inserthere [$mymenu index [_ "Report a bug"]]
$mymenu insert $inserthere separator
$mymenu insert $inserthere command -label [_ " Search"] \
    -command {::dialog_search::open_search_dialog .search}
