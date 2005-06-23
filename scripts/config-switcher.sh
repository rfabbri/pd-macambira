#!/bin/sh


#==============================================================================#
# functions

print_usage() {
	 echo "Usage: "
	 echo "To select a config file:"
	 echo "   $0 select CONFIG_NAME"
	 echo "To save the current config to file:"
	 echo "   $0 save CONFIG_NAME"
	 echo "To delete the current config:"
	 echo "   $0 delete CONFIG_NAME"
	 echo "To list existing configs:"
	 echo "   $0 list"
	 exit
}

#==============================================================================#
# THE PROGRAM

# location of pref file that Pd reads
case `uname` in
	 Darwin)
		  CONFIG_DIR=~/Library/Preferences
		  CONFIG_FILE=org.puredata.pd.plist
		  ;;
	 *)
		  CONFIG_DIR=~
		  CONFIG_FILE=.pdrc
		  ;;
esac

# everything happens in this dir
cd $CONFIG_DIR

if [ $# -gt 1 ]; then
	 save_file="$CONFIG_FILE-$2"
	 case $1 in
		  select)
				if [ -e "$save_file" ]; then
					 test -e "$CONFIG_FILE" && mv "$CONFIG_FILE" /tmp
					 ln -s "$save_file" "$CONFIG_FILE" && \
						  echo "Pd config \"$save_file\" selected." 
				else
					 echo "\"$save_file\" doesn't exist.  No action taken."
				fi
				;;
		  save)
				if [ -e "$CONFIG_DIR/$CONFIG_FILE" ]; then
					 cp "$CONFIG_FILE" "$save_file" && \
						  echo "Pd config \"$2\" saved." 
				else
					 echo "\"$CONFIG_FILE\" doesn't exist.  No action taken."
				fi
				;;
		  delete)
				if [ -e "$save_file" ]; then
					 rm "$save_file" && \
						  echo "Pd config \"$save_file\" deleted." 
				else
					 echo "\"$CONFIG_FILE\" doesn't exist.  No action taken."
				fi
				;;
		  *) print_usage ;;
	 esac
else
	 case $1 in
 		  list)
				echo "Available configs:"
				\ls -1 ${CONFIG_FILE}*
				;;
		  *)
				print_usage
				;; 
	 esac
fi
