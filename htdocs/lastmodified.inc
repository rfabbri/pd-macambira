
<BR>
<P CLASS="small">
<?php
putenv("TZ=GMT");
echo "Last Modified: ";
echo date( "F d Y H:i:s", getlastmod() );
echo " GMT";
?>
</P>
