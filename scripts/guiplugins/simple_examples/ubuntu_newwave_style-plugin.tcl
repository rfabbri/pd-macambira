#
# this plugin aims to make Pd's menubar and menus look like the Ubuntu New
# Wave theme.

.menubar configure -borderwidth 0 -foreground white -background grey35 -font {Ubuntu 10}

foreach menuname {file edit put find media window help} {
    .menubar.$menuname configure -relief sunken -font {Ubuntu 10} -borderwidth 1 \
        -background "#f2f2f2" -activebackground "#F9B78D"
}
