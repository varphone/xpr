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
// implementation

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_base64.h>

static char gBase64DecodeTable[256];

static void InitBase64DecodeTable(void)
{
    int i;
    for (i = 0; i < 256; ++i) gBase64DecodeTable[i] = (char)0x80;
    // default value: invalid

    for (i = 'A'; i <= 'Z'; ++i) gBase64DecodeTable[i] = 0 + (i - 'A');
    for (i = 'a'; i <= 'z'; ++i) gBase64DecodeTable[i] = 26 + (i - 'a');
    for (i = '0'; i <= '9'; ++i) gBase64DecodeTable[i] = 52 + (i - '0');
    gBase64DecodeTable[(unsigned char)'+'] = 62;
    gBase64DecodeTable[(unsigned char)'/'] = 63;
    gBase64DecodeTable[(unsigned char)'='] = 0;
}

unsigned char* XPR_Base64Decode(char const* in, unsigned inSize,
                                unsigned int* resultSize, int trimTrailingZeros)
{
    static int haveInitializedBase64DecodeTable = 0;
    unsigned char* out = 0;
    int paddingCount = 0;
    int i = 0;
    int j = 0;
    int jMax = inSize - 3;
    int k = 0;
    char inTmp[4];
    char outTmp[4];

    //
    if (!haveInitializedBase64DecodeTable) {
        InitBase64DecodeTable();
        haveInitializedBase64DecodeTable = 1;
    }

    out = (unsigned char*)malloc(strlen(in)); // ensures we have enough space

    // in case "inSize" is not a multiple of 4 (although it should be)
    for (j = 0; j < jMax; j += 4) {
        for (i = 0; i < 4; ++i) {
            inTmp[i] = in[i+j];
            if (inTmp[i] == '=') ++paddingCount;
            outTmp[i] = gBase64DecodeTable[(unsigned char)inTmp[i]];
            if ((outTmp[i]&0x80) != 0) outTmp[i] = 0; // this happens only if there was an invalid character; pretend that it was 'A'
        }

        out[k++] = (outTmp[0]<<2) | (outTmp[1]>>4);
        out[k++] = (outTmp[1]<<4) | (outTmp[2]>>2);
        out[k++] = (outTmp[2]<<6) | outTmp[3];
    }

    if (trimTrailingZeros) {
        while (paddingCount > 0 && k > 0 && out[k-1] == '\0') { --k; --paddingCount; }
    }

    if (resultSize)
        *resultSize = k;

    return out;
}

static const char gBase64Char[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* XPR_Base64Encode(const unsigned char* origSigned, unsigned int origLength)
{
    const unsigned char* orig = (const unsigned char*)origSigned; // in case any input bytes have the MSB set
    const unsigned int numOrig24BitValues = origLength/3;
    int havePadding = origLength > numOrig24BitValues*3;
    int havePadding2 = origLength == numOrig24BitValues*3 + 2;
    const unsigned int numResultBytes = 4*(numOrig24BitValues + havePadding);
    char* result = 0;

    // Map each full group of 3 input bytes into 4 output base-64 characters:
    unsigned i = 0;

    if (orig == NULL)
        return NULL;

    result = (char*)malloc(numResultBytes+1); // allow for trailing '\0'

    for (i = 0; i < numOrig24BitValues; ++i) {
        result[4*i+0] = gBase64Char[(orig[3*i]>>2)&0x3F];
        result[4*i+1] = gBase64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
        result[4*i+2] = gBase64Char[((orig[3*i+1]<<2) | (orig[3*i+2]>>6))&0x3F];
        result[4*i+3] = gBase64Char[orig[3*i+2]&0x3F];
    }

    // Now, take padding into account.  (Note: i == numOrig24BitValues)
    if (havePadding) {
        result[4*i+0] = gBase64Char[(orig[3*i]>>2)&0x3F];
        if (havePadding2) {
            result[4*i+1] = gBase64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
            result[4*i+2] = gBase64Char[(orig[3*i+1]<<2)&0x3F];
        } else {
            result[4*i+1] = gBase64Char[((orig[3*i]&0x3)<<4)&0x3F];
            result[4*i+2] = '=';
        }
        result[4*i+3] = '=';
    }

    result[numResultBytes] = '\0';
    return result;
}
