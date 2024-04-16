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

#include "7zTypes.h"

class OutputBuffer {
 public:
  OutputBuffer();
  ~OutputBuffer();

  ISeqOutStream *stream() { return &stream_.vt; };
  const uint8_t *data() const { return data_; }
  size_t size() const { return position_; }

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
  size_t position_ = 0;
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
  OutputBufferStream *stream = Z7_CONTAINER_FROM_VTBL(p, OutputBufferStream, vt);
  return stream->buffer->Write(data, size);
}

size_t OutputBuffer::Write(const void *data, size_t size) {
  size_t available = size_ - position_;
  if (available < size) {
    if (!size_) {
      size_ = kInitialSize;
    }
    while (size_ - position_ < size) {
      size_ *= 2;
    }
    uint8_t *tmp = static_cast<uint8_t*>(malloc(size_));
    assert(tmp);
    if (data_) {
      memcpy(tmp, data_, position_);
      free(data_);
    }
    data_ = tmp;
  }

  memcpy(data_ + position_, data, size);
  position_ += size;
  return size;
}

class OutputByteBuffer {
 public:
  OutputByteBuffer();
  ~OutputByteBuffer();

  IByteOut *stream() { return &stream_.vt; };
  const uint8_t *data() const { return data_; }
  size_t size() const { return position_; }

 private:
  static const size_t kInitialSize = 128;

  typedef struct {
    IByteOut vt;
    OutputByteBuffer *buffer;
  } ByteOutStream;

  static void _Write(const IByteOut *pp, Byte b);
  void Write(Byte b);

  ByteOutStream stream_;
  uint8_t *data_ = nullptr;
  size_t size_ = 0;
  size_t position_ = 0;
};

OutputByteBuffer::OutputByteBuffer() {
  stream_.vt.Write = &OutputByteBuffer::_Write;
  stream_.buffer = this;
}

OutputByteBuffer::~OutputByteBuffer() {
  free(data_);
}

// static
void OutputByteBuffer::_Write(const IByteOut *p, Byte b) {
  ByteOutStream *stream = Z7_CONTAINER_FROM_VTBL(p, ByteOutStream, vt);
  stream->buffer->Write(b);
}

void OutputByteBuffer::Write(Byte b) {
  if (size_ == position_) {
    if (!size_) {
      size_ = kInitialSize;
    } else {
      size_ *= 2;
    }
    uint8_t *tmp = static_cast<uint8_t*>(malloc(size_));
    assert(tmp);
    if (data_) {
      memcpy(tmp, data_, position_);
      free(data_);
    }
    data_ = tmp;
  }

  data_[position_++] = b;
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
  InputBufferStream *stream = Z7_CONTAINER_FROM_VTBL(p, InputBufferStream, vt);
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

class InputByteBuffer {
 public:
  InputByteBuffer(const uint8_t *data, size_t size);

  IByteIn *stream() { return &stream_.vt; };

 private:
  typedef struct {
    IByteIn vt;
    InputByteBuffer *buffer;
  } ByteInStream;

  static Byte _Read(const IByteIn *p);
  Byte Read();

  ByteInStream stream_;
  const uint8_t *data_;
  size_t size_;
  size_t position_ = 0;
};

InputByteBuffer::InputByteBuffer(const uint8_t *data, size_t size)
  : data_(data), size_(size) {
  stream_.vt.Read = &InputByteBuffer::_Read;
  stream_.buffer = this;
}

// static
Byte InputByteBuffer::_Read(const IByteIn *p) {
  ByteInStream *stream = Z7_CONTAINER_FROM_VTBL(p, ByteInStream, vt);
  return stream->buffer->Read();
}

Byte InputByteBuffer::Read() {
  if (size_ == position_) {
    return 0;
  }

  return data_[position_++];
}

class InputLookBuffer {
 public:
  InputLookBuffer(const uint8_t *data, size_t size);

  ILookInStream *stream() { return &stream_.vt; };

 private:
  typedef struct {
    ILookInStream vt;
    InputLookBuffer *buffer;
  } InputLookBufferStream;

  static SRes _Look(const ILookInStream *p, const void **data, size_t *size);
  SRes Look(const void **data, size_t *size);
  static SRes _Skip(const ILookInStream *p, size_t offset);
  SRes Skip(size_t offset);
  static SRes _Read(const ILookInStream *p, void *data, size_t *size);
  SRes Read(void *data, size_t *size);
  static SRes _Seek(const ILookInStream *p, Int64 *pos, ESzSeek origin);
  SRes Seek(Int64 *pos, ESzSeek origin);

  InputLookBufferStream stream_;
  const uint8_t *data_;
  Int64 size_;
  Int64 position_ = 0;
};

InputLookBuffer::InputLookBuffer(const uint8_t *data, size_t size)
  : data_(data), size_(size) {
  stream_.vt.Look = &InputLookBuffer::_Look;
  stream_.vt.Skip = &InputLookBuffer::_Skip;
  stream_.vt.Read = &InputLookBuffer::_Read;
  stream_.vt.Seek = &InputLookBuffer::_Seek;
  stream_.buffer = this;
}

// static
SRes InputLookBuffer::_Look(const ILookInStream *p, const void **data,
    size_t *size) {
  InputLookBufferStream *stream =
      Z7_CONTAINER_FROM_VTBL(p, InputLookBufferStream, vt);
  return stream->buffer->Look(data, size);
}

SRes InputLookBuffer::Look(const void **data, size_t *size) {
  size_t available = size_ - position_;
  if (available < *size) {
    *size = available;
  }
  *data = data_ + position_;
  return SZ_OK;
}

// static
SRes InputLookBuffer::_Skip(const ILookInStream *p, size_t offset) {
  InputLookBufferStream *stream =
      Z7_CONTAINER_FROM_VTBL(p, InputLookBufferStream, vt);
  return stream->buffer->Skip(offset);
}

SRes InputLookBuffer::Skip(size_t offset) {
  size_t available = size_ - position_;
  if (offset > available) {
    offset = available;
  }
  position_ += offset;
  return SZ_OK;
}

// static
SRes InputLookBuffer::_Read(const ILookInStream *p, void *data, size_t *size) {
  InputLookBufferStream *stream =
      Z7_CONTAINER_FROM_VTBL(p, InputLookBufferStream, vt);
  return stream->buffer->Read(data, size);
}

SRes InputLookBuffer::Read(void *data, size_t *size) {
  size_t available = size_ - position_;
  if (available < *size) {
    *size = available;
  }
  if (*size > 0) {
    memcpy(data, data_ + position_, *size);
    position_ += *size;
  }
  return SZ_OK;
}

// static
SRes InputLookBuffer::_Seek(const ILookInStream *p, Int64 *pos,
    ESzSeek origin) {
  InputLookBufferStream *stream =
      Z7_CONTAINER_FROM_VTBL(p, InputLookBufferStream, vt);
  return stream->buffer->Seek(pos, origin);
}

SRes InputLookBuffer::Seek(Int64 *pos, ESzSeek origin) {
  switch (origin) {
    case SZ_SEEK_SET:
      if (*pos < 0) {
        *pos = 0;
      } else if (*pos > size_) {
        *pos = size_;
      }
      position_ = *pos;
      break;
    case SZ_SEEK_CUR:
      if (*pos > size_ || position_ + *pos > size_) {
        *pos = size_;
      } else if (position_ < -*pos) {
        *pos = 0;
      }
      position_ += *pos;
      break;
    case SZ_SEEK_END:
      if (*pos >= 0) {
        *pos = size_;
      } else if (-(*pos) > size_) {
        *pos = 0;
      } else {
        *pos = size_ + *pos;
      }
      position_ = *pos;
      break;
    default:
      return SZ_ERROR_PARAM;
  }
  return SZ_OK;
}
