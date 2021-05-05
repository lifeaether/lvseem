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
    const char *command;
    const char *device_name;
    const char *b_id;
    const char *b_password;
    const char *channel;
    const char *pan_id;
    const char *addr;
};

bool lvseem_option_parse( const int argc, const char * const argv[], struct lvseem_option * const option ) {
    if ( ! option ) {
        return false;
    }
    for ( int i = 1; i < argc; ++i ) {
        if ( strcmp( argv[i], "--device" ) == 0 && ++i < argc ) {
            option->device_name = argv[i];
        } else if ( strcmp( argv[i], "--id" ) == 0 && ++i < argc ) {
            option->b_id = argv[i];
        } else if ( strcmp( argv[i], "--password" ) == 0 && ++i < argc ) {
            option->b_password = argv[i];
        } else if ( strcmp( argv[i], "--channel" ) == 0 && ++i < argc ) {
            option->channel = argv[i];
        } else if ( strcmp( argv[i], "--pan_id" ) == 0 && ++i < argc ) {
            option->pan_id = argv[i];
        } else if ( strcmp( argv[i], "--addr" ) == 0 && ++i < argc ) {
            option->addr = argv[i];
        } else {
            option->command = argv[i];
        }
    }
    return true;
}

bool lvseem_option_validate( const struct lvseem_option * const option ) {
    if ( ! option ) {
        return false;
    }
    if ( ! option->command ) {
        return false;
    }
    if ( strcmp( option->command, "help" ) == 0 ) {
        return true;
    }
    if ( ! option->device_name ) {
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

    tty.c_cc[VTIME] = 1;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 115200
    cfsetispeed( &tty, B115200 );
    cfsetospeed( &tty, B115200 );

    if ( tcsetattr( serial_port, TCSANOW, &tty ) != 0 ) {
        return false;
    }

    return true;
}

void lvseem_usage( void )
{
    fprintf( stderr, "usage: %s command [options]\n", lvseem_command_name );
    fprintf( stderr, "\t%s status --device DeviceName\n", lvseem_command_name );
    fprintf( stderr, "\t%s help\n", lvseem_command_name );
}

static bool handler_event_ever( void *userdata, const char *version )
{
    fprintf( stdout, "%s\n", version );
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

    if ( strcmp( option.command, "help" ) == 0 ) {
        lvseem_usage();
        return EXIT_SUCCESS;
    }

    const int serial_port = open( option.device_name, O_RDWR );
    if ( serial_port == -1 ) {
        fprintf( stderr, "open: %s\n", strerror( errno ) );
        return EXIT_FAILURE;
    }

    if ( ! serial_port_initalize( serial_port ) ) {
        close( serial_port );
        return EXIT_FAILURE;
    }

    if ( option.b_id ) {
//        if ( ! bp35a1_set_b_id( serial_port, stdout, option.b_id ) ) {
//            close( serial_port );
//            return EXIT_FAILURE;
//        }
    }

    if ( option.b_password ) {
//        if ( ! bp35a1_set_b_password( serial_port, stdout, option.b_password ) ) {
//            close( serial_port );
//            return EXIT_FAILURE;
//        }
    }

    bool result = EXIT_SUCCESS;
    if ( strcmp( option.command, "status" ) == 0 ) {
        if ( ! bp35a1_write( serial_port, "SKVER\r\n", sizeof("SKVER\r\n")-1 ) ) {
            fprintf( stderr, "bp35a1_write: failed\n" );
            result = EXIT_FAILURE;
        }
    }

    struct bp35a1_handler handler = {};
    handler.event_ever = handler_event_ever;
    while ( bp35a1_parse( serial_port, &handler, NULL ) );

    if ( close( serial_port ) != 0 ) {
        fprintf( stderr, "close: failed\n" );
    }

    return result;
}
