/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
#ifndef BACREAL_H
#define BACREAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int decode_real_safe(
        uint8_t * apdu,
        uint32_t len_value,
        float *real_value);

    int decode_real(
        uint8_t * apdu,
        float *real_value);

    int decode_context_real(
        uint8_t * apdu,
        uint8_t tag_number,
        float *real_value);
    int encode_bacnet_real(
        float value,
        uint8_t * apdu);
    int decode_double(
        uint8_t * apdu,
        double *real_value);
    int decode_context_double(
        uint8_t * apdu,
        uint8_t tag_number,
        double *double_value);
    int decode_double_safe(
        uint8_t * apdu,
        uint32_t len_value,
        double *double_value);

    int encode_bacnet_double(
        double value,
        uint8_t * apdu);

#ifdef TEST
#include "ctest.h"

    void testBACreal(
        Test * pTest);
    void testBACdouble(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
