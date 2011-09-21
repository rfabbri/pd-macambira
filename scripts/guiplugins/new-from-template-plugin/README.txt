
This plugin inserts a menu item to the File menu called "New from template".
When you select this menu item, it creates a new patch from the template.pd
file that is included in this plugin.  You can modify the template.pd however
you want and it'll use it.

Also, you can set the default folder that it creates the new patch in by
replacing this line:

    variable defaultfolder [file join $::env(HOME) Documents]

with something like:

    variable defaultfolder /path/to/my/favorite/folder

