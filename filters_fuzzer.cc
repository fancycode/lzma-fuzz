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

#include <algorithm>

#include "7zCrc.h"
#include "Bra.h"
#include "Delta.h"
#include "XzCrc64.h"

class FilterFuzzer {
 public:
  FilterFuzzer(const uint8_t *data, size_t size) : size_(size) {
    data_ = static_cast<uint8_t*>(malloc(size));
    assert(data_);
    memcpy(data_, data, size);
  }
  virtual ~FilterFuzzer() {
    free(data_);
  }

  const uint8_t *data() const { return data_; }
  size_t size() const { return size_; }

  virtual void RunFuzzer() = 0;

 private:
  uint8_t *data_;
  size_t size_;
};

class SevenzCrcFuzzer : public FilterFuzzer {
 public:
  SevenzCrcFuzzer(const uint8_t *data, size_t size) : FilterFuzzer(data, size) {
    CrcGenerateTable();
  }

  void RunFuzzer() override {
    CrcCalc(data(), size());
  }
};

class XzCrcFuzzer : public FilterFuzzer {
 public:
  XzCrcFuzzer(const uint8_t *data, size_t size) : FilterFuzzer(data, size) {
    Crc64GenerateTable();
  }

  void RunFuzzer() override {
    Crc64Calc(data(), size());
  }
};

class BraFuzzer : public FilterFuzzer {
 public:
  BraFuzzer(const uint8_t *data, size_t size) : FilterFuzzer(data, size) {}

  void RunFuzzer() override {
    Byte *tmp = static_cast<Byte*>(malloc(size()));
    assert(tmp);
    memcpy(tmp, data(), size());
    RunFilter(tmp, size());
    assert(memcmp(tmp, data(), size()) == 0);
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

    // We are using up to the first "kDeltaCount" bytes to determine the
    // "delta" value for the filter.
    uint8_t delta = 0;
    static const size_t kDeltaCount = 32;
    for (size_t i = 0; i < std::min(size, kDeltaCount); i++) {
      delta += data[i];
    }
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

  FilterFuzzer *fuzzers[] = {
    new BraArmFuzzer(data, size),
    new BraArmtFuzzer(data, size),
    new BraIa64Fuzzer(data, size),
    new BraPpcFuzzer(data, size),
    new BraSparcFuzzer(data, size),
    new BraX86Fuzzer(data, size),
    new DeltaFuzzer(data, size),
    new SevenzCrcFuzzer(data, size),
    new XzCrcFuzzer(data, size),
  };
  for (FilterFuzzer *fuzzer : fuzzers) {
    fuzzer->RunFuzzer();
    delete fuzzer;
  };
  return 0;
}
