#!/bin/bash
# This script modifys your ~/.pdrc file to add the proper path to make this library work. 
# Use at your own risks. 

# Usage : bash ./setup-pdrc.sh
 
echo -path $PWD >> ~/.pdrc
echo -helppath $PWD >> ~/.pdrc
echo -path $PWD 
echo -helppath $PWD

