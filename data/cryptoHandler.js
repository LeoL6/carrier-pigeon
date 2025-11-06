// Ensure nacl is already loaded (nacl-fast.min.js + nacl-util.min.js)

window.cryptoHandler = (function() {
  function encodeBase64(bytes) {
    return btoa(String.fromCharCode.apply(null, bytes));
  }

  function decodeBase64(str) {
    const binary = atob(str);
    const len = binary.length;
    const bytes = new Uint8Array(len);
    for (let i = 0; i < len; i++) bytes[i] = binary.charCodeAt(i);
    return bytes;
  }

  function encodeUTF8(arr) {
    return new TextDecoder().decode(arr);
  }

  function decodeUTF8(str) {
    return new TextEncoder().encode(str);
  }

  // --- HMAC helper ---
  function hmacSHA256(key, data) {
    const blockSize = 64;
    if (key.length > blockSize) key = nacl.hash(key).slice(0, 32);
    if (key.length < blockSize) {
      const tmp = new Uint8Array(blockSize);
      tmp.set(key);
      key = tmp;
    }
    const o_key_pad = new Uint8Array(blockSize);
    const i_key_pad = new Uint8Array(blockSize);
    for (let i = 0; i < blockSize; i++) {
      o_key_pad[i] = key[i] ^ 0x5c;
      i_key_pad[i] = key[i] ^ 0x36;
    }
    const inner = nacl.hash(new Uint8Array([...i_key_pad, ...data]));
    const outer = nacl.hash(new Uint8Array([...o_key_pad, ...inner]));
    return outer.slice(0, 32);
  }

  // --- derive session key using ECDH sharedSecret, static PSK (shared on both devices), and the salt (derived from both parties session nonces)---
  function deriveSessionKey(sharedSecret, psk, salt) {
    // combine inputs: sharedSecret || psk || salt
    const data = new Uint8Array([
      ...sharedSecret,
      ...psk,
      ...salt
    ]);
    return hmacSHA256(psk, data); // 32-byte key
  }

  function generateKeyPair() {
    const keyPair = nacl.box.keyPair();
    return {
      publicKey: encodeBase64(keyPair.publicKey),
      privateKey: keyPair.secretKey,
    };
  }

  function deriveSharedSecret(myPrivateKey, peerPublicKeyB64) {
    const peerPublicKey = decodeBase64(peerPublicKeyB64);
    return nacl.box.before(peerPublicKey, myPrivateKey);
  }

  function encryptMessage(sharedKey, plaintext) {
    const nonce = nacl.randomBytes(nacl.box.nonceLength);
    const messageUint8 = decodeUTF8(plaintext);
    const box = nacl.secretbox(messageUint8, nonce, sharedKey);

    return {
      nonce: encodeBase64(nonce),
      data: encodeBase64(box),
    };
  }

  function decryptMessage(sharedKey, packet) {
    const nonce = decodeBase64(packet.nonce);
    const box = decodeBase64(packet.data);
    const message = nacl.secretbox.open(box, nonce, sharedKey);

    if (!message) throw new Error("Decryption failed.");
    return encodeUTF8(message);
  }

  // Expose only what’s needed
  return {
    encodeBase64,
    decodeBase64,
    generateKeyPair,
    deriveSharedSecret,
    deriveSessionKey,
    encryptMessage,
    decryptMessage
  };

})();
