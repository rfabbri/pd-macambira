
proc log {message} {
    exec sh -c "/bin/echo $message" >/dev/stderr
}

proc alert {message} {
    tk_messageBox -message $message -type ok -icon question
}

proc default_keybindings {} {
         return { exit CMD-q }
}

proc checkPort { port } {
 if { 
 [catch {set result [exec sh -c "/usr/sbin/netstat -an |grep $port"] } errn] } {
    # the command returns no result, so the port is not used
    return true
 } else { 
    return false
 }
}

proc waitForPort { port } {
    for {set x 0} {$x<30} {incr x} {
        after 100
        if {![checkPort $port]} {
            return true
        }
    }
    return false
}



proc loadpdtcl { } {
    global pd_guidir
    # wait for pd to start
    if {[waitForPort 5400]} {
        if { 
            [catch {load $pd_guidir/bin/pdtcl } errmsg] } {
            alert "can't connect to pd : $errmsg"
            exit(1)
        }
    } else {
        alert "Couldn't start Pd"
    } 
}



##### startup ##################################################################

global pd_guidir
global pd_port

# get the path to the Wish Shell so a relative path can be 
# used to launch Pd
regsub -- "Pd" [info nameofexecutable] "" wish_path
puts "$wish_path/../Resources/lib/pd/bin/pd.tk"

# set paths
set pd_guidir [file join [file dirname [file dirname [info script]]] lib pd]
set pd_exec_path [file join [file dirname [file dirname [info script]]] bin]

# launch pd -a dummy guicmd prevents starting the wish shell
if {[checkPort 5400]} {
    exec sh -c "cd $pd_exec_path;./pd -guicmd /bin/echo" >&/dev/stderr &
} else {
    alert "Can't start pd because the port 5400 is in use"
    exit(1)
}


# open gui
source [file join $pd_guidir bin pd.tk]

################################################################################



# depends on pd.tk
# tk::mac::OpenDocument is called when docs are dropped 
# on the Dock icon with the filenames put into the var args
proc tk::mac::OpenDocument {args} {
    foreach file $args {
        pd [concat pd open [pdtk_enquote [file tail $file]] \
		[pdtk_enquote  [file dirname $file]] \;]
        menu_doc_open [file dirname $file] [file tail $file]
    }
}
