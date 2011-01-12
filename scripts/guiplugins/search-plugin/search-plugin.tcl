# plugin to allow searching all the documentation using a regexp
# check the Help menu for the Search item to use it

package require Tk 8.4
package require pd_bindings
package require pd_menucommands

namespace eval ::dialog_search:: {
    variable selected_file {}
    variable selected_basedir {}
    variable basedir_list {}
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
    variable selected_basedir
    variable basedir_list
    set selection [$listingwidget curselection]
    if {$selection eq ""} {
        set selected_file ""
    } else {
        set line [$listingwidget get $selection]
        set selected_file [string replace $line [string first ":" $line] end]
        set selected_basedir [lindex $basedir_list $selection]
    }
}

proc ::dialog_search::open_line {} {
    variable selected_file
    variable selected_basedir
    if {$selected_file ne ""} {
        menu_doc_open $selected_basedir $selected_file
    }
}

proc ::dialog_search::readfile {filename} {
    set fp [open $filename]
    set file_contents [split [read $fp] \n]
    close $fp
    return $file_contents
}

proc ::dialog_search::search {searchtext} {
    variable basedir_list {}
    set widget .search.resultslistbox
    $widget delete 0 end
    foreach basedir [concat [file join $::sys_libdir doc] $::sys_searchpath $::sys_staticpath] {
        # Fix the directory name, this ensures the directory name is in the
        # native format for the platform and contains a final directory seperator
        set basedir [file normalize $basedir]
        foreach docfile [find_doc_files $basedir] {
            searchfile $searchtext [readfile $docfile] $widget \
                [string replace $docfile 0 [string length $basedir]] $basedir
        }
    }
}

proc ::dialog_search::searchfile {searchtext file_contents widget filename basedir} {
    variable basedir_list
    set n 0
    foreach line $file_contents {
        if {[regexp -nocase -- $searchtext $line]} {
            $widget insert end "$filename: $line"
            lappend basedir_list $basedir
            incr n
        }
    }
}

proc ::dialog_search::ok {mytoplevel} {
    # this is a placeholder for the standard dialog bindings
}

proc ::dialog_search::cancel {mytoplevel} {
    wm withdraw .search
}

proc ::dialog_search::open_search_dialog {mytoplevel} {
    if {[winfo exists $mytoplevel]} {
        wm deiconify $mytoplevel
        raise $mytoplevel
    } else {
        create_dialog $mytoplevel
    }
}

proc ::dialog_search::create_dialog {mytoplevel} {
    variable selected_file
    toplevel $mytoplevel
    wm title $mytoplevel [_ "Search"]
    entry $mytoplevel.searchtextentry -bg white -textvar searchtext \
        -highlightcolor blue -font 18 -borderwidth 3
    bind $mytoplevel.searchtextentry <Return> {::dialog_search::search $searchtext}
    # TODO add history like in the find box
    bind $mytoplevel.searchtextentry <Up> {set searchtext ""}
    listbox $mytoplevel.resultslistbox -yscrollcommand "$mytoplevel.yscrollbar set" \
        -bg white -highlightcolor blue -height 30 -width 80
    scrollbar $mytoplevel.yscrollbar -command "$mytoplevel.resultslistbox yview" \
        -takefocus 0
    bind $mytoplevel.resultslistbox <<ListboxSelect>> \
        "::dialog_search::selectline $mytoplevel.resultslistbox"
    bind $mytoplevel.resultslistbox <Key-Return> ::dialog_search::open_line
	bind $mytoplevel.resultslistbox <Double-ButtonRelease-1> ::dialog_search::open_line
    ::pd_bindings::dialog_bindings $mytoplevel "search"

    grid $mytoplevel.searchtextentry - -sticky ew
    grid $mytoplevel.resultslistbox $mytoplevel.yscrollbar -sticky news
    grid columnconfig . 0 -weight 1
    grid rowconfig    . 1 -weight 1
    
    focus $mytoplevel.searchtextentry
}

# create the menu item on load
set mymenu .menubar.help
set inserthere [$mymenu index [_ "Report a bug"]]
$mymenu insert $inserthere separator
$mymenu insert $inserthere command -label [_ "Search"] \
    -command {::dialog_search::open_search_dialog .search}
