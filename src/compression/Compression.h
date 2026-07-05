#pragma once
#include <cstdint>
#include <cstddef>

namespace Compression 
{
  // <========================>
  //   Constants
  // <========================>

  // Max bytes compress() will ever need to work with (matches your 64-byte
  // message cap). Give it +4 bytes of slack so smaz can attempt compression
  // on incompressible input without immediately failing.
  constexpr size_t MAX_INPUT_LEN  = 64;
  constexpr size_t WORK_BUFFER_LEN = MAX_INPUT_LEN + 4;

  // <========================>
  //   Functions
  // <========================>
  
  // Returns compressed length, or 0 if compression failed / didn't help.
  // outBuf must be at least WORK_BUFFER_LEN bytes.
  size_t compress(const uint8_t* in, size_t inLen, uint8_t* outBuf, size_t outBufLen);

  // Returns decompressed length, or 0 on failure.
  size_t decompress(const uint8_t* in, size_t inLen, uint8_t* outBuf, size_t outBufLen);
}