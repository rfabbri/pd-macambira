#!/bin/sh


#==============================================================================#
# functions

print_usage() {
	 echo "Usage: "
	 echo "To load a config file:"
	 echo "   $0 load CONFIG_NAME"
	 echo " "
	 echo "To save the current config to file:"
	 echo "   $0 save CONFIG_NAME"
	 echo " "
	 echo "To delete the current config:"
	 echo "   $0 delete CONFIG_NAME"
	 echo " "
	 echo "To list existing configs:"
	 echo "   $0 list"
	 echo " "
	 echo "To use the .pdrc instead, add '--pdrc':"
	 echo "   $0 --pdrc load CONFIG_NAME"
	 echo "   $0 --pdrc save CONFIG_NAME"
	 echo "   $0 --pdrc delete CONFIG_NAME"
	 echo "   $0 --pdrc list"
	 exit
}

#==============================================================================#
# THE PROGRAM

if [ $# -eq 0 ]; then
	 print_usage
else
	 # get the command line arguments
	 if [ $1 == "--pdrc" ]; then
		  CONFIG_DIR=~
		  CONFIG_FILE=.pdrc
		  COMMAND=$2
		  CONFIG_NAME=$3
	 else
		  COMMAND=$1
		  CONFIG_NAME=$2
    # location of pref file that Pd reads
		  case `uname` in
				Darwin)
					 CONFIG_DIR=~/Library/Preferences
					 CONFIG_FILE=org.puredata.pd.plist
					 ;;
				Linux)
					 CONFIG_DIR=~
					 CONFIG_FILE=.pdsettings
					 ;;
				*)
					 echo "Not supported on this platform."
					 exit
					 ;;
		  esac
	 fi
	 
    # everything happens in this dir
	 cd $CONFIG_DIR
	 
	 save_file="$CONFIG_DIR/$CONFIG_FILE-$CONFIG_NAME"
	 case $COMMAND in
		  load)
				if [ -e "$save_file" ]; then
					 test -e "$CONFIG_FILE" && mv "$CONFIG_FILE" /tmp
					 rm "$CONFIG_FILE"
					 cp "$save_file" "$CONFIG_FILE" && \
						  echo "Pd config \"$save_file\" loaded." 
				else
					 echo "\"$save_file\" doesn't exist.  No action taken."
				fi
				;;
		  save)
				if [ -e "$CONFIG_DIR/$CONFIG_FILE" ]; then
					 cp "$CONFIG_FILE" "$save_file" && \
						  echo "Pd config \"$CONFIG_NAME\" saved." 
				else
					 echo "\"$CONFIG_FILE\" doesn't exist.  No action taken."
				fi
				;;
		  delete)
				if [ -e "$save_file" ]; then
					 rm "$save_file" && \
						  echo "Pd config \"$save_file\" deleted." 
				else
					 echo "\"$save_file\" doesn't exist.  No action taken."
				fi
				;;
 		  list)
				echo "Available configs:"
				\ls -1 ${CONFIG_FILE}*
				;;
		  *) print_usage ;;
	 esac
fi
