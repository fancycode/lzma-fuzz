/**
 *
 * @copyright Copyright (c) 2019 Joachim Bauch <mail@joachim-bauch.de>
 *
 * @license GNU GPL version 3 or any later version
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Bra.h"

#if !defined(BRA_CONVERT)
#error "Need to define BRA_CONVERT to one of the XXX_Convert functions from Bra.h."
#endif

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (!size) {
    return 0;
  }

  Byte *tmp = static_cast<Byte*>(malloc(size));
  assert(tmp);
  memcpy(tmp, data, size);

  const UInt32 ip = 0;

  // Encode data.
  BRA_CONVERT(tmp, size, ip, 1);

  // Decode data.
  BRA_CONVERT(tmp, size, ip, 0);
  assert(memcmp(tmp, data, size) == 0);
  free(tmp);
  return 0;
}
