/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2014 Live Networks, Inc.  All rights reserved.
// Base64 encoding and decoding
// C++ header

#ifndef XPR_BASE64_H
#define XPR_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

/// @brief returns a newly allocated array - of size "resultSize" - that
///        the caller is responsible for delete[]ing.
///        As above, but includes the size of the input string (i.e., the number of bytes to decode) as a parameter.
///        This saves an extra call to "strlen()" if we already know the length of the input string.
unsigned char* XPR_Base64Decode(const char* in, unsigned int inSize,
                                unsigned int* resultSize, int trimTrailingZeros);

/// @brief returns a 0-terminated string that
/// the caller is responsible for delete[]ing.
char* XPR_Base64Encode(const unsigned char* orig, unsigned int origLength);

#ifdef __cplusplus
}
#endif

#endif
