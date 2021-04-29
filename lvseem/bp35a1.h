//
//  bp35a1.h
//  lvseem
//
//  Created by lifeaether on 2021/04/29.
//

#ifndef bp35a1_h
#define bp35a1_h

#include <stdint.h>
#include <stdbool.h>

//bool bp35a1_set_echo_back( const int fd, bool enabled );

bool bp35a1_command_write( const int fd, const char *input );
bool bp35a1_command_read_line( const int fd, uint8_t * const response, size_t *size );

bool bp35a1_skver( const int fd, char *version, size_t size );

#endif /* bp35a1_h */
