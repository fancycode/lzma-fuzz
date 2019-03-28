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

#include "LzmaEnc.h"
#include "LzmaDec.h"

#include "common-alloc.h"

#include <stdio.h>

class OutputBuffer {
 public:
  OutputBuffer();
  ~OutputBuffer();

  ISeqOutStream *stream() { return &stream_.vt; };
  const uint8_t *data() const { return data_; }
  size_t size() const { return size_; }

 private:
  static const size_t kInitialSize = 128;

  typedef struct {
    ISeqOutStream vt;
    OutputBuffer *buffer;
  } OutputBufferStream;

  static size_t _Write(const ISeqOutStream *pp, const void *data, size_t size);
  size_t Write(const void *data, size_t size);

  OutputBufferStream stream_;
  uint8_t *data_ = nullptr;
  size_t size_ = 0;
  size_t available_ = 0;
};

OutputBuffer::OutputBuffer() {
  stream_.vt.Write = &OutputBuffer::_Write;
  stream_.buffer = this;
}

OutputBuffer::~OutputBuffer() {
  free(data_);
}

// static
size_t OutputBuffer::_Write(const ISeqOutStream *p, const void *data,
    size_t size) {
  OutputBufferStream *stream = CONTAINER_FROM_VTBL(p, OutputBufferStream, vt);
  return stream->buffer->Write(data, size);
}

size_t OutputBuffer::Write(const void *data, size_t size) {
  if (available_ < size) {
    if (!available_) {
      available_ = kInitialSize;
    }
    while (available_ < size) {
      available_ *= 2;
    }
    uint8_t *tmp = static_cast<uint8_t*>(malloc(available_));
    assert(tmp);
    if (data_) {
      memcpy(tmp, data_, size_);
      free(data_);
    }
    data_ = tmp;
  }

  memcpy(data_ + size_, data, size);
  size_ += size;
  return size;
}

class InputBuffer {
 public:
  InputBuffer(const uint8_t *data, size_t size);

  ISeqInStream *stream() { return &stream_.vt; };

 private:
  typedef struct {
    ISeqInStream vt;
    InputBuffer *buffer;
  } InputBufferStream;

  static SRes _Read(const ISeqInStream *p, void *data, size_t *size);
  SRes Read(void *data, size_t *size);

  InputBufferStream stream_;
  const uint8_t *data_;
  size_t size_;
};

InputBuffer::InputBuffer(const uint8_t *data, size_t size)
  : data_(data), size_(size) {
  stream_.vt.Read = &InputBuffer::_Read;
  stream_.buffer = this;
}

// static
SRes InputBuffer::_Read(const ISeqInStream *p, void *data, size_t *size) {
  InputBufferStream *stream = CONTAINER_FROM_VTBL(p, InputBufferStream, vt);
  return stream->buffer->Read(data, size);
}

SRes InputBuffer::Read(void *data, size_t *size) {
  if (size_ < *size) {
    *size = size_;
  }
  if (*size > 0) {
    memcpy(data, data_, *size);
    size_ -= *size;
    data_ += *size;
  }
  return SZ_OK;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size <= 10) {
    return 0;
  }

  CLzmaEncProps props = {0};
  Byte props_data[LZMA_PROPS_SIZE];
  SizeT props_size = LZMA_PROPS_SIZE;

  LzmaEncProps_Init(&props);
  props.level = data[0] <= 9 ? data[0] : 9;
  props.lc = data[1] <= 8 ? data[1] : 8;
  props.lp = data[2] <= 4 ? data[2] : 4;
  props.pb = data[3] <= 4 ? data[3] : 4;
  props.algo = data[4] ? 1 : 0;
  props.fb = 5 + data[5];
  props.btMode = data[6] ? 1 : 0;
  props.numHashBytes = 2 + (data[7] % 3);
  props.mc = 1 + data[8];
  props.writeEndMark = data[9] ? 1 : 0;
  props.dictSize = 1 << 24;
  data += 10;
  size -= 10;
  LzmaEncProps_Normalize(&props);

  CLzmaEncHandle enc = LzmaEnc_Create(&CommonAlloc);
  if (!enc) {
    return 0;
  }

  OutputBuffer out_buffer;
  InputBuffer in_buffer(data, size);
  Byte *dest = nullptr;
  SizeT srcLen;
  SizeT destLen;
  ELzmaStatus status;

  SRes res = LzmaEnc_SetProps(enc, &props);
  if (res != SZ_OK) {
    goto exit;
  }

  LzmaEnc_SetDataSize(enc, size);
  if (LzmaEnc_WriteProperties(enc, props_data, &props_size) != SZ_OK) {
    goto exit;
  }
  assert(props_size == LZMA_PROPS_SIZE);

  res = LzmaEnc_Encode(enc, out_buffer.stream(), in_buffer.stream(), nullptr,
      &CommonAlloc, &CommonAlloc);
  assert(res == SZ_OK);
  assert(out_buffer.size() > 0);

  // Decompress and compare with input data.
  dest = static_cast<Byte*>(malloc(size));
  assert(dest);
  destLen = size;
  srcLen = out_buffer.size();

  res = LzmaDecode(dest, &destLen, out_buffer.data(), &srcLen, props_data,
      props_size, LZMA_FINISH_END, &status, &CommonAlloc);
  assert(res == SZ_OK);
  assert(status == LZMA_STATUS_FINISHED_WITH_MARK ||
      status == LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK);
  assert(srcLen == out_buffer.size());
  assert(memcmp(dest, data, size) == 0);

exit:
  LzmaEnc_Destroy(enc, &CommonAlloc, &CommonAlloc);
  free(dest);
  return 0;
}
