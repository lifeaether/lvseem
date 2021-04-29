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
#include <termios.h>
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

struct lvseem_resource {
    bool (*close)( void *item );
    void *item;
};

bool lvseem_resource_close( const struct lvseem_resource * const resource )
{
    if ( ! resource ) {
        return true;
    }
    if ( ! resource->item ) {
        return true;
    }
    if ( ! resource->close ) {
        return false;
    }
    return resource->close( resource->item );
}

bool lvseem_resource_close_all( const struct lvseem_resource * const resources, const int count )
{
    bool failed = false;
    for ( int i = 0; i < count; ++i ) {
        const bool result = lvseem_resource_close( &(resources[i]) );
        failed = failed || (! result);
    }
    return ! failed;
}

bool close_serial_port( void *item )
{
    int *serial_port = (int *)item;
    if ( ! serial_port ) {
        return true;
    }
    if ( *serial_port == -1 ) {
        return true;
    }
    if ( close( *serial_port ) != 0 ) {
        fprintf( stderr, "error: close: %s\n", strerror( errno ) );
        return false;
    }
    return true;
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

    int serial_port = -1;
    struct lvseem_resource resources[] = {
        { close_serial_port, &serial_port },
    };
    
    serial_port = open( option.device_name, O_RDWR );
    if ( serial_port == -1 ) {
        lvseem_resource_close_all( resources, sizeof( resources ) / sizeof( resources[0] ) );
        fprintf( stderr, "error: open: %s\n", strerror( errno ) );
        return EXIT_FAILURE;
    }

    if ( ! lvseem_resource_close_all( resources, sizeof( resources ) / sizeof( resources[0] ) ) ) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
