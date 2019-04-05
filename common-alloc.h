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

#include <stdint.h>
#include <stdlib.h>

#include "7zAlloc.h"
#include "7zTypes.h"

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)

// Arbitrary limit of 2 GByte for memory allocations. Used to prevent the
// MemorySanitizer from aborting with errors like "requested allocation size
// 0xffffffffffffffff exceeds maximum supported size of 0x200000000".
static const size_t kMaxAllowedMemory = 2 * 1024 * 1024 * 1024L;

static void *LzmaAlloc(ISzAllocPtr p, size_t size) {
  if (size > kMaxAllowedMemory) {
    return nullptr;
  }

  return SzAlloc(nullptr, size);
}

static void LzmaFree(ISzAllocPtr p, void *address) {
  SzFree(nullptr, address);
}

static ISzAlloc CommonAlloc = {LzmaAlloc, LzmaFree};

#else

#include "7zAlloc.h"

static ISzAlloc CommonAlloc = {SzAlloc, SzFree};

#endif
