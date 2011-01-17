
package require pd_menus

namespace eval category_menu {
}

proc category_menu::load_menutree {} {
    # load object -> tags mapping from file in Pd's path
    set testfile [file join $::current_plugin_loadpath menutree.tcl]
    set f [open $testfile]
    set menutree [read $f]
    close $f
    unset f        
    return $menutree
}

proc category_menu::create {mymenu} {
    pdtk_post "category menu plugin\n"
    set menutree [load_menutree]

    $mymenu add separator
    foreach categorylist $menutree {
        set category [lindex $categorylist 0]
        menu $mymenu.$category
        $mymenu add cascade -label $category -menu $mymenu.$category
        foreach subcategorylist [lrange $categorylist 1 end] {
            set subcategory [lindex $subcategorylist 0]
            menu $mymenu.$category.$subcategory
            $mymenu.$category add cascade -label $subcategory -menu $mymenu.$category.$subcategory
            pdtk_post "subcategorylist: $subcategorylist\n"
            foreach item [lindex $subcategorylist 1] {
                pdtk_post "item: $item\n"
                $mymenu.$category.$subcategory add command -label $item \
                    -command "pdsend \"\$::focused_window obj \$::popup_xcanvas \$::popup_ycanvas $item\""
            }
        }
    }
}

category_menu::create .popup
