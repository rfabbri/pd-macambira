mkdir c:\pd\externs\maxlib
mkdir c:\pd\doc\5.reference\maxlib
cd help
copy *.pd c:\pd\doc\5.reference\maxlib
copy examplescore.txt c:\pd\doc\5.reference\maxlib
copy automata.txt c:\pd\externs\maxlib
cd ..
copy maxlib.dll c:\pd\externs\maxlib
copy README c:\pd\externs\maxlib
copy HISTORY c:\pd\externs\maxlib
copy LICENSE c:\pd\externs\maxlib
echo start Pd with flag "-lib c:\pd\externs\maxlib\maxlib"
