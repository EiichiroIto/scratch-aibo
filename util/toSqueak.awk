BEGIN {
    FS = " ";
    varName = "";
}
/^\/\/.*Squeak:.*ConditionIds/ {
    varName = $3;
    idPrefix = $4;
    printf( "\r\r" );
    printf( "!STNCondition class methodsFor: 'class initialization' stamp: 'toSqueak 11/7/2003 23:59'!\r", $3 );
    printf( "initialize%s\r", varName );
    printf( "\t%s _ OrderedCollection new.\r", varName );
    printf( "\t%s add: #(#'%s' %d).\r", varName, "default", 0 );
    next;
}
/^\/\/.*Squeak:.*ConditionCodes/ {
    varName = "aCollection";
    idPrefix = $4;
    printf( "! !\r\r\r" );
    printf( "!STNCondition class methodsFor: 'class initialization' stamp: 'toSqueak 11/7/2003 23:59'!\r", $3 );
    printf( "initialize%s\r", $5 );
    printf( "\taCollection _ %s at: #'%s' put: (OrderedCollection new).\r",
	    $3, tolower($5) );
    next;
}
/^\/\/.*Squeak:.*Head/ {
    varName = "Commands";
    idPrefix = $4;
    printf( "! !\r\r\r" );
    printf( "!STN%s class methodsFor: 'class initialization' stamp: 'toSqueak 11/7/2003 23:59'!\r", $3 );
    printf( "initialize\r" );
    printf( "\t%s _ OrderedCollection new.\r", varName );
    next;
}
/^\/\/.*Squeak:.*Face/ {
    varName = "Commands";
    idPrefix = $4;
    printf( "! !\r\r\r" );
    printf( "!STN%s class methodsFor: 'class initialization' stamp: 'toSqueak 11/7/2003 23:59'!\r", $3 );
    printf( "initialize\r" );
    printf( "\t%s _ OrderedCollection new.\r", varName );
    next;
}
/^const int / {
    id = $3;
    gsub( idPrefix, "", id );
    num = $5;
    gsub( /;/, "", num );
    printf( "\t%s add: #(#'%s' %d).\r", varName, tolower(id), num );
}
END {
    printf( "! !\r\r\r" );
}
