BEGIN {
    FS=" ";
    varName = "Commands";
    printf( "\r\r" );
    printf( "!STN%s class methodsFor: 'class initialization' stamp: 'monetToSqueak 11/7/2003 23:59'!\r", "Monet" );
    printf( "initialize\r" );
    printf( "\t%s _ OrderedCollection new.\r", varName );
    printf( "\t%s add: #(#none -1 ).\r", varName );

}
/^[0-9]/ {
    num = $1;
    if ( $4 == "#" ) {
	id = $5;
    } else {
	id = sprintf( "cmd%d", num );
    }
    printf( "\t%s add: #(#'%s' %d).\r", varName, tolower(id), num );
}
END {
    printf( "! !\r\r\r" );
}
