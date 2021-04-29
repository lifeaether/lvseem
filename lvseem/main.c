//
//  main.c
//  lvseem
//
//  Created by lifeaether on 2021/04/29.
//

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char * const command_name = "lvseem";

struct lvseem_option {
    const char *device_name;
};

bool option_parse( const int argc, const char * const argv[], struct lvseem_option * const option ) {
    if ( ! option ) {
        return false;
    }
    for ( int i = 1; i < argc; ++i ) {
        option->device_name = argv[i];
    }
    return true;
}

bool option_validate( const struct lvseem_option * const option ) {
    if ( ! option ) {
        return false;
    }
    if ( ! option->device_name ) {
        return false;
    }
    return true;
}

void usage( void )
{
    fprintf( stderr, "usage: %s DeviceName\n", command_name );
}

int main(int argc, const char * argv[]) {
    struct lvseem_option option = {};
    if ( ! option_parse( argc, argv, &option ) ) {
        usage();
        return EXIT_FAILURE;
    }
    if ( ! option_validate( &option ) ) {
        usage();
        return EXIT_FAILURE;
    }
    
    const int serial_port = open( option.device_name, O_RDWR );
    if ( serial_port == -1 ) {
        fprintf( stderr, "error: open: %s\n", strerror( errno ) );
        return EXIT_FAILURE;
    }
    
    if ( close( serial_port ) != 0 ) {
        fprintf( stderr, "error: close: %s\n", strerror( errno ) );
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
