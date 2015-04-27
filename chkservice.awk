BEGIN {
    connectcfg = "MS/OPEN-R/MW/CONF/CONNECT.CFG";
    FS=" *, *";
}
/^Service/ {
    entry = $1;
    sub( /Service[^"]*"/, "", entry );
    sub( /"[^"]*$/, "", entry );
    #printf( "%s %s\n", FILENAME, entry );
    entries[ entry ] = 1;
    fns[ entry ] = FILENAME;
}
END {
    FS=" *";
    while ( getline < connectcfg > 0 ) {
	if ( $0 ~ /^#/ || $0 ~ /^ *$/ ) {
	     continue;
	}
	if ( !isSystemObject( $1 ) ) {
	    if ( entries[ $1 ] == 0 ) {
		printf( "unknown service %s in %s\n", $1, connectcfg );
	    } else {
		entries[ $1 ] = 2;
	    }
	}
	if ( !isSystemObject( $2 ) ) {
	    if ( entries[ $2 ] == 0 ) {
		printf( "unknown service %s in %s\n", $2, connectcfg );
	    } else {
		entries[ $2 ] = 2;
	    }
	}
    }
    printf( "Unused services:\n" );
    for ( entry in entries ) {
	if ( entries[ entry ] != 2 ) {
	    printf( " %s in %s\n", entry, fns[ entry ] );
	}
    }
}
function isSystemObject( entry ) {
    sub( /\..*$/, "", entry );
    if ( entry == "OVirtualRobotComm" ) {
	return 1;
    }
    return 0;
}
