//
//  bp35a1.h
//  lvseem
//
//  Created by lifeaether on 2021/04/29.
//

#ifndef bp35a1_h
#define bp35a1_h

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

ssize_t bp35a1_write_bytes( const int fd, const uint8_t * const bytes, const size_t size );
bool bp35a1_write_string( const int fd, const char * const string );
ssize_t bp35a1_read_bytes( const int fd, uint8_t * const bytes, const size_t size );
bool bp35a1_read_string( const int fd, char * const string, const size_t size );

void bp35a1_read_to_end( const int fd );

void bp35a1_print_information( const int fd, FILE *fp );

#endif /* bp35a1_h */
