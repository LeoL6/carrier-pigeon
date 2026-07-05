#include "Compression.h"
extern "C" 
{
  #include "smaz/smaz.h"
}

namespace Compression 
{
  size_t compress(const uint8_t* in, size_t inLen, uint8_t* outBuf, size_t outBufLen)
  {
    if (inLen == 0 || inLen > MAX_INPUT_LEN) return 0;

    int result = smaz_compress(
      (char*)in, (int)inLen,
      (char*)outBuf, (int)outBufLen
    );

    // smaz returns outlen+1 on failure (buffer too small / no room)
    if (result <= 0 || (size_t)result > outBufLen) return 0;

    // Only worth it if it actually shrank the message
    if ((size_t)result >= inLen) return 0;

    return (size_t)result;
  }

  size_t decompress(const uint8_t* in, size_t inLen, uint8_t* outBuf, size_t outBufLen)
  {
    if (inLen == 0) return 0;

    int result = smaz_decompress(
      (char*)in, (int)inLen,
      (char*)outBuf, (int)outBufLen
    );

    if (result <= 0 || (size_t)result > outBufLen) return 0;

    return (size_t)result;
  }
}
