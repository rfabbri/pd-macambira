# plugin to allow searching all the documentation using a regexp
# check the Help menu for the Search item to use it

package require Tk 8.4
package require pd_bindings
package require pd_menucommands

namespace eval ::dialog_search:: {
    variable selected_file {}
}

# TODO it works funny when libdirs are symlinks

# find_doc_files
# basedir - the directory to start looking in
proc ::dialog_search::find_doc_files { basedir } {
    # Fix the directory name, this ensures the directory name is in the
    # native format for the platform and contains a final directory seperator
    set basedir [string trimright [file join $basedir { }]]
    set fileList {}

    # Look in the current directory for matching files, -type {f r}
    # means ony readable normal files are looked at, -nocomplain stops
    # an error being thrown if the returned list is empty
    foreach fileName [glob -nocomplain -type {f r} -path $basedir *.txt *.pd] {
        lappend fileList $fileName
    }

    # Now look for any sub direcories in the current directory
    foreach dirName [glob -nocomplain -type {d  r} -path $basedir *] {
        # Recusively call the routine on the sub directory and append any
        # new files to the results
        set subDirList [find_doc_files $dirName]
        if { [llength $subDirList] > 0 } {
            foreach subDirFile $subDirList {
                lappend fileList $subDirFile
            }
        }
    }
    return $fileList
}

proc ::dialog_search::selectline {listingwidget} {
    variable selected_file
    set selection [$listingwidget curselection]
    if {$selection eq ""} {
        set selected_file ""
    } else {
        set line [$listingwidget get $selection]
        set selected_file [string replace $line [string first ":" $line] end]
    }
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
    foreach dir [concat [file join $::sys_libdir doc] $::sys_searchpath $::sys_staticpath] {
        # Fix the directory name, this ensures the directory name is in the
        # native format for the platform and contains a final directory seperator
        set dir [file normalize $dir]
        foreach docfile [find_doc_files $dir] {
            readfile $docfile data
            searchfile $searchtext $widget \
                [string replace $docfile 0 [string length $dir]]
        }
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
    wm title .search [_ "Search"]
    entry .search.searchtextentry -bg white -textvar searchtext
    bind .search.searchtextentry <Return> {::dialog_search::search $searchtext}
    # TODO add history like in the find box
    bind .search.searchtextentry <Up> {set searchtext ""}
    listbox .search.resultslistbox -yscrollcommand ".search.yscrollbar set" \
        -bg white -highlightcolor blue -height 30 -width 80
    scrollbar .search.yscrollbar -command ".search.resultslistbox yview" \
        -takefocus 0
    bind .search.resultslistbox <<ListboxSelect>> \
        "::dialog_search::selectline .search.resultslistbox"
    bind .search.resultslistbox <Key-Return> \
        {menu_doc_open $::sys_libdir $::dialog_search::selected_file}
	bind .search.resultslistbox <Double-ButtonRelease-1> \
        {menu_doc_open $::sys_libdir $::dialog_search::selected_file}
    ::pd_bindings::dialog_bindings .search "search"

    grid .search.searchtextentry - -sticky ew
    grid .search.resultslistbox .search.yscrollbar -sticky news
    grid columnconfig . 0 -weight 1
    grid rowconfig    . 1 -weight 1
    
    focus .search.searchtextentry
}

set mymenu .menubar.help
set inserthere [$mymenu index [_ "Report a bug"]]
$mymenu insert $inserthere separator
$mymenu insert $inserthere command -label [_ "Search"] \
    -command {::dialog_search::open_search_dialog .search}
