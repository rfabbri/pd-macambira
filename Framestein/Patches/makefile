all: vframe vsig vsnapshot vrgb vcolor vframeread vdrawarray arraysize buildstr l2s

# FLAGS:
#
# note: maximum optimization (/Ox) causes crashes with plugins..

F = /DNT /DPD /LD /Gd /GD /Ogitb1 /Gs /Ic:\pd\src /I..\Plugins
L = c:\pd\bin\pd.lib

vframe:
	cl vframe.c $(F) /link /export:vframe_setup $(L) user32.lib

vsig:
	cl vsig~.c $(F) /link /export:vsig_tilde_setup $(L)

vsnapshot:
	cl vsnapshot~.c $(F) /link /export:vsnapshot_tilde_setup $(L)

vrgb:
	cl vrgb~.c $(F) /link /export:vrgb_tilde_setup $(L) user32.lib

vcolor:
	cl vcolor~.c $(F) /link /export:vcolor_tilde_setup $(L) user32.lib

vframeread:
	cl vframeread~.c $(F) /link /export:vframeread_tilde_setup $(L)

vdrawarray:
	cl vdrawarray.c $(F) /link /export:vdrawarray_setup $(L)

arraysize:
	cl arraysize.c $(F) /link /export:arraysize_setup $(L)

buildstr:
	cl buildstr.c $(F) /link /export:buildstr_setup $(L)

l2s:
	cl l2s.c $(F) /link /export:l2s_setup $(L)
