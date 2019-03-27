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

#include "Delta.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  uint8_t delta;
  if (size < sizeof(delta)) {
    return 0;
  }

  memcpy(&delta, data, sizeof(delta));
  data += sizeof(delta);
  size -= sizeof(delta);
  if (delta == 0 || size == 0) {
    return 0;
  }

  Byte *tmp = static_cast<Byte*>(malloc(size));
  assert(tmp);
  memcpy(tmp, data, size);

  Byte state[DELTA_STATE_SIZE];
  Delta_Init(state);
  Delta_Encode(state, delta, tmp, size);

  Delta_Init(state);
  Delta_Decode(state, delta, tmp, size);
  assert(memcmp(tmp, data, size) == 0);
  free(tmp);
  return 0;
}
