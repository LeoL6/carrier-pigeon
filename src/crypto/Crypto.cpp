#include "crypto.h"

#include "../message/Messages.h"

namespace Crypto
{
  static size_t decrypt(uint8_t* payload, uint8_t length, Message& outMsg)
  {
    if (!payload)
      return 0;

    // Clamp to fit in Message buffer (leave room for null terminator)
    size_t copyLen = length;
    if (copyLen >= MAX_MESSAGE_LEN)
      copyLen = MAX_MESSAGE_LEN - 1;

    memcpy(outMsg.text, payload, copyLen);
    outMsg.text[copyLen] = '\0';

    outMsg.outgoing = false;

    return copyLen;
  }

  void onData(const Packet::Packet& pkt)
  {
    Message decrypted;

    // Decrypt function
    size_t len = decrypt(pkt.payload, pkt.length, decrypted);

    if (len == 0) return; // failed / invalid

  // Pass to queue
    Messages::push(decrypted); 
  }
}

// #include "mbedtls/md.h"
// #include "mbedtls/hkdf.h"
// #include "mbedtls/entropy.h"
// #include "mbedtls/ctr_drbg.h"
// #include <string.h>

// // --- RNG Context (static, initialized once) ---
// static mbedtls_entropy_context entropy;
// static mbedtls_ctr_drbg_context ctr_drbg;
// static bool rng_initialized = false;

// static bool crypto_init_rng()
// {
//     if (rng_initialized) return true;

//     mbedtls_entropy_init(&entropy);
//     mbedtls_ctr_drbg_init(&ctr_drbg);

//     const char* pers = "lora_crypto";

//     int ret = mbedtls_ctr_drbg_seed(
//         &ctr_drbg,
//         mbedtls_entropy_func,
//         &entropy,
//         (const unsigned char*)pers,
//         strlen(pers)
//     );

//     if (ret != 0) return false;

//     rng_initialized = true;
//     return true;
// }

// // --- Random Bytes ---
// bool crypto_random_bytes(uint8_t* buf, size_t len)
// {
//     if (!crypto_init_rng()) return false;

//     return mbedtls_ctr_drbg_random(&ctr_drbg, buf, len) == 0;
// }

// // --- HMAC-SHA256 ---
// bool crypto_hmac_sha256(
//     const uint8_t* key, size_t key_len,
//     const uint8_t* data, size_t data_len,
//     uint8_t* out
// )
// {
//     const mbedtls_md_info_t* md_info =
//         mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

//     if (!md_info) return false;

//     int ret = mbedtls_md_hmac(
//         md_info,
//         key, key_len,
//         data, data_len,
//         out
//     );

//     return ret == 0;
// }

// // --- HKDF-SHA256 ---
// bool crypto_hkdf_sha256(
//     const uint8_t* ikm, size_t ikm_len,
//     const uint8_t* salt, size_t salt_len,
//     const uint8_t* info, size_t info_len,
//     uint8_t* out_key, size_t out_len
// )
// {
//     const mbedtls_md_info_t* md_info =
//         mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

//     if (!md_info) return false;

//     int ret = mbedtls_hkdf(
//         md_info,
//         salt, salt_len,
//         ikm, ikm_len,
//         info, info_len,
//         out_key, out_len
//     );

//     return ret == 0;
// }