
package require Tk 8.5
package require tile
package require pd_menucommands

set selected_file {}

# findFiles
# basedir - the directory to start looking in
# pattern - A pattern, as defined by the glob command, that the files must match
proc findFiles { basedir pattern } {

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

proc ui {} {
    toplevel .searchwindow
    wm title .searchwindow [_ "Search Window"]
    entry .searchwindow.searchtextentry -bg white -textvar searchtext
    bind .searchwindow.searchtextentry <Return> \
        {search $searchtext .searchwindow.resultslistbox}
    # TODO add history like in the find box
    bind .searchwindow.searchtextentry <Up> {set searchtext ""}
    listbox .searchwindow.resultslistbox -yscrollcommand ".searchwindow.yscrollbar set" \
        -bg white -height 20 -width 40
    scrollbar .searchwindow.yscrollbar -command ".searchwindow.resultslistbox yview"
    bind .searchwindow.resultslistbox <<ListboxSelect>> \
        {selectline [.searchwindow.resultslistbox get \
                         [.searchwindow.resultslistbox curselection]]}
    bind .searchwindow.resultslistbox <Key-Return> \
        {menu_doc_open $::sys_libdir "$::selected_file"}
	bind .searchwindow.resultslistbox <Double-ButtonRelease-1> \
        {menu_doc_open $::sys_libdir "$::selected_file"}

    grid .searchwindow.searchtextentry - -sticky ew
    grid .searchwindow.resultslistbox .searchwindow.yscrollbar -sticky news
    grid columnconfig . 0 -weight 1
    grid rowconfig    . 1 -weight 1
}
proc selectline {line} {
    set ::selected_file [string replace $line [string first ":" $line] end]
}
proc readfile {file varName} {
    upvar \#0 $varName data
    set fp [open $file]
    set data [split [read $fp] \n]
    close $fp
}
proc search {searchtext widget} {
    $widget delete 0 end
    foreach docfile $::allDocFiles {
        readfile $docfile data
        searchfile $searchtext $widget \
            [string replace $docfile 0 [string length $::sys_libdir]]
    }
}
proc searchfile {searchtext widget filename} {
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

#set sys_libdir "/home/hans/code/pure-data/trunk/pd/doc"
set allDocFiles [findFiles $sys_libdir "*.pd"]
#readfile $f data
#ui

set mymenu .menubar.help
set inserthere [$mymenu index [_ "Report a bug"]]
$mymenu insert $inserthere separator
$mymenu insert $inserthere command -label [_ " Search"] -command ui
