/*****************************************************************************/
/* D2K: A Doom Source Port for the 21st Century                              */
/*                                                                           */
/* Copyright (C) 2014: See COPYRIGHT file                                    */
/*                                                                           */
/* This file is part of D2K.                                                 */
/*                                                                           */
/* D2K is free software: you can redistribute it and/or modify it under the  */
/* terms of the GNU General Public License as published by the Free Software */
/* Foundation, either version 2 of the License, or (at your option) any      */
/* later version.                                                            */
/*                                                                           */
/* D2K is distributed in the hope that it will be useful, but WITHOUT ANY    */
/* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS */
/* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    */
/* details.                                                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License along   */
/* with D2K.  If not, see <http://www.gnu.org/licenses/>.                    */
/*                                                                           */
/*****************************************************************************/


#ifndef M_BITMAP_H__
#define M_BITMAP_H__

#define bit_index_to_byte_index(i) (((i) - ((i) & 0x7)) >> 3)

#define bit_size_to_byte_size(size) ((((size) - (((size) & 0x7))) >> 3) + 1)

#define def_bitmap(name, size) uint8_t name[bit_size_to_byte_size((size))]

#define def_bitmap_zero(name, size)            \
  uint8_t name[bit_size_to_byte_size((size))]; \
  memset(name, 0, sizeof(name))

#define bitmap_flag_to_bit_flag(flag) (1 << ((flag) & 0x7))

#define bitmap_get_bit(bitmap, i) \
  (bitmap[bit_index_to_byte_index((i))] & bitmap_flag_to_bit_flag((i)))

#define bitmap_set_bit(bitmap, i) \
  (bitmap[bit_index_to_byte_index((i))] |= bitmap_flag_to_bit_flag((i)))

#define bitmap_clear_bit(bitmap, i) \
  (bitmap[bit_index_to_byte_index((i))] &= ~bitmap_flag_to_bit_flag((i)))

#define print_bitmap(bitmap) do {                          \
  printf("[");                                             \
  for (size_t _i = 0; _i < sizeof(bitmap); _i++) {         \
    for (size_t _j = 1; _j < 1 << 8; _j <<= 1) {           \
      printf(" %c", bitmap[_i] & _j ? '1' : '0');          \
    }                                                      \
  }                                                        \
  puts(" ]");                                              \
} while (0)

#endif

/* vi: set et ts=2 sw=2: */
