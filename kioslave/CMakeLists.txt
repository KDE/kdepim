add_subdirectory( imap4 )

if(SASL2_FOUND)
	add_subdirectory( sieve )
endif(SASL2_FOUND)

add_subdirectory( mbox ) 
