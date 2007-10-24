 package require base64

 proc encode {} {
     .txt delete 1.0 end
     set fileID [open [tk_getOpenFile] RDONLY]
     fconfigure $fileID -translation binary
     set rawData [read $fileID]
     close $fileID
     set encodedData [base64::encode $rawData]
     .txt insert 1.0 $encodedData
 }

 proc clipcopy {} {
     clipboard clear
     clipboard append [.txt get 1.0 end]
 }

 wm title . "Base64 Gif Encoder"
 text .txt -wrap none -font "Courier 10"
 menu .mbar
 . configure -menu .mbar
 .mbar add command -label "Encode File" -command encode
 .mbar add command -label "Copy2Clipboard" -command clipcopy
 .mbar add command -label "Exit" -command {destroy .; exit}

 pack .txt -expand 1 -fill both

 encode
 ### End of Script
