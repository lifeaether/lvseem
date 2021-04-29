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

#include "bp35a1.h"

static const char * const lvseem_command_name = "lvseem";

struct lvseem_option {
    const char *device_name;
};

bool lvseem_option_parse( const int argc, const char * const argv[], struct lvseem_option * const option ) {
    if ( ! option ) {
        return false;
    }
    for ( int i = 1; i < argc; ++i ) {
        option->device_name = argv[i];
    }
    return true;
}

bool lvseem_option_validate( const struct lvseem_option * const option ) {
    if ( ! option ) {
        return false;
    }
    if ( ! option->device_name ) {
        return false;
    }
    return true;
}

void lvseem_usage( void )
{
    fprintf( stderr, "usage: %s DeviceName\n", lvseem_command_name );
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

bool serial_port_close( void *item )
{
    int *serial_port = (int *)item;
    if ( ! serial_port ) {
        return true;
    }
    if ( *serial_port == -1 ) {
        return true;
    }
    if ( close( *serial_port ) != 0 ) {
        fprintf( stderr, "close: %s\n", strerror( errno ) );
        return false;
    }
    return true;
}

bool serial_port_initalize( const int serial_port )
{
    struct termios tty = {};
    if ( tcgetattr( serial_port, &tty ) != 0 ) {
        fprintf( stderr, "tcgetattr: %s\n", strerror( errno ) );
        return false;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 115200
    cfsetispeed( &tty, B115200 );
    cfsetospeed( &tty, B115200 );

    if ( tcsetattr( serial_port, TCSANOW, &tty ) != 0 ) {
        return false;
    }

    return true;
}

int main(int argc, const char * argv[]) {
    struct lvseem_option option = {};
    if ( ! lvseem_option_parse( argc, argv, &option ) ) {
        lvseem_usage();
        return EXIT_FAILURE;
    }
    if ( ! lvseem_option_validate( &option ) ) {
        lvseem_usage();
        return EXIT_FAILURE;
    }

    int serial_port = -1;
    struct lvseem_resource resources[] = {
        { serial_port_close, &serial_port },
    };
    
    serial_port = open( option.device_name, O_RDWR );
    if ( serial_port == -1 ) {
        lvseem_resource_close_all( resources, sizeof( resources ) / sizeof( resources[0] ) );
        fprintf( stderr, "open: %s\n", strerror( errno ) );
        return EXIT_FAILURE;
    }

    if ( ! serial_port_initalize( serial_port ) ) {
        lvseem_resource_close_all( resources, sizeof( resources ) / sizeof( resources[0] ) );
        return EXIT_FAILURE;
    }

    {
        char version[32] = {};
        if ( ! bp35a1_command( serial_port, "SKVER", version, sizeof(version) ) ) {
            fprintf( stderr, "bp35a1_command: FAILED\n" );
            lvseem_resource_close_all( resources, sizeof( resources ) / sizeof( resources[0] ) );
            return EXIT_FAILURE;
        }
        fprintf( stdout, "%s\n", version );
    }

    {
        char response[128] = {};
        if ( ! bp35a1_command( serial_port, "SKINFO", response, sizeof(response) ) ) {
            fprintf( stderr, "bp35a1_command: FAILED\n" );
            lvseem_resource_close_all( resources, sizeof( resources ) / sizeof( resources[0] ) );
            return EXIT_FAILURE;
        }
        fprintf( stdout, "%s\n", response );
    }

    if ( ! lvseem_resource_close_all( resources, sizeof( resources ) / sizeof( resources[0] ) ) ) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
