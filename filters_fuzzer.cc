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

#include "7zCrc.h"
#include "Bra.h"
#include "Delta.h"
#include "XzCrc64.h"

class FilterFuzzer {
 public:
  FilterFuzzer(const uint8_t *data, size_t size) : data_(data), size_(size) {}
  virtual ~FilterFuzzer() = default;

  virtual void RunFuzzer() = 0;

 protected:
  const uint8_t *data_;
  size_t size_;
};

class SevenzCrcFuzzer : public FilterFuzzer {
 public:
  SevenzCrcFuzzer(const uint8_t *data, size_t size) : FilterFuzzer(data, size) {
    CrcGenerateTable();
  }

  void RunFuzzer() override {
    CrcCalc(data_, size_);
  }
};

class XzCrcFuzzer : public FilterFuzzer {
 public:
  XzCrcFuzzer(const uint8_t *data, size_t size) : FilterFuzzer(data, size) {
    Crc64GenerateTable();
  }

  void RunFuzzer() override {
    Crc64Calc(data_, size_);
  }
};

class BraFuzzer : public FilterFuzzer {
 public:
  BraFuzzer(const uint8_t *data, size_t size) : FilterFuzzer(data, size) {}

  void RunFuzzer() override {
    Byte *tmp = static_cast<Byte*>(malloc(size_));
    assert(tmp);
    memcpy(tmp, data_, size_);
    RunFilter(tmp, size_);
    assert(memcmp(tmp, data_, size_) == 0);
    free(tmp);
  }

 protected:
  static const UInt32 kIp = 0;

  virtual void RunFilter(uint8_t *data, size_t size) = 0;
};

class BraArmFuzzer : public BraFuzzer {
 public:
  BraArmFuzzer(const uint8_t *data, size_t size) : BraFuzzer(data, size) {}

 protected:
  void RunFilter(uint8_t *data, size_t size) override {
    // Encode data.
    ARM_Convert(data, size, kIp, 1);

    // Decode data.
    ARM_Convert(data, size, kIp, 0);
  }
};

class BraArmtFuzzer : public BraFuzzer {
 public:
  BraArmtFuzzer(const uint8_t *data, size_t size) : BraFuzzer(data, size) {}

 protected:
  void RunFilter(uint8_t *data, size_t size) override {
    // Encode data.
    ARMT_Convert(data, size, kIp, 1);

    // Decode data.
    ARMT_Convert(data, size, kIp, 0);
  }
};

class BraIa64Fuzzer : public BraFuzzer {
 public:
  BraIa64Fuzzer(const uint8_t *data, size_t size) : BraFuzzer(data, size) {}

 protected:
  void RunFilter(uint8_t *data, size_t size) override {
    // Encode data.
    IA64_Convert(data, size, kIp, 1);

    // Decode data.
    IA64_Convert(data, size, kIp, 0);
  }
};

class BraPpcFuzzer : public BraFuzzer {
 public:
  BraPpcFuzzer(const uint8_t *data, size_t size) : BraFuzzer(data, size) {}

 protected:
  void RunFilter(uint8_t *data, size_t size) override {
    // Encode data.
    PPC_Convert(data, size, kIp, 1);

    // Decode data.
    PPC_Convert(data, size, kIp, 0);
  }
};

class BraSparcFuzzer : public BraFuzzer {
 public:
  BraSparcFuzzer(const uint8_t *data, size_t size) : BraFuzzer(data, size) {}

 protected:
  void RunFilter(uint8_t *data, size_t size) override {
    // Encode data.
    SPARC_Convert(data, size, kIp, 1);

    // Decode data.
    SPARC_Convert(data, size, kIp, 0);
  }
};

class BraX86Fuzzer : public BraFuzzer {
 public:
  BraX86Fuzzer(const uint8_t *data, size_t size) : BraFuzzer(data, size) {}

 protected:
  void RunFilter(uint8_t *data, size_t size) override {
    UInt32 state;
    // Encode data.
    x86_Convert_Init(state);
    x86_Convert(data, size, kIp, &state, 1);

    // Decode data.
    x86_Convert_Init(state);
    x86_Convert(data, size, kIp, &state, 0);
  }
};

class DeltaFuzzer : public BraFuzzer {
 public:
  DeltaFuzzer(const uint8_t *data, size_t size) : BraFuzzer(data, size) {}

 protected:
  void RunFilter(uint8_t *data, size_t size) override {
    if (!size) {
      return;
    }

    uint8_t delta = data[0];
    data++;
    size--;
    if (!delta || !size) {
      return;
    }

    Byte state[DELTA_STATE_SIZE];
    Delta_Init(state);
    Delta_Encode(state, delta, data, size);

    Delta_Init(state);
    Delta_Decode(state, delta, data, size);
  }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (!size) {
    return 0;
  }

  uint8_t type = data[0];
  data++;
  size--;

  FilterFuzzer *fuzzer;
  switch (type) {
    case 0:
      fuzzer = new SevenzCrcFuzzer(data, size);
      break;
    case 1:
      fuzzer = new XzCrcFuzzer(data, size);
      break;
    case 2:
      fuzzer = new BraArmFuzzer(data, size);
      break;
    case 3:
      fuzzer = new BraArmtFuzzer(data, size);
      break;
    case 4:
      fuzzer = new BraIa64Fuzzer(data, size);
      break;
    case 5:
      fuzzer = new BraPpcFuzzer(data, size);
      break;
    case 6:
      fuzzer = new BraSparcFuzzer(data, size);
      break;
    case 7:
      fuzzer = new BraX86Fuzzer(data, size);
      break;
    case 8:
      fuzzer = new DeltaFuzzer(data, size);
      break;
    default:
      return 0;
  }

  fuzzer->RunFuzzer();
  delete fuzzer;
  return 0;
}
