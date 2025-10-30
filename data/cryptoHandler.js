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

  function generateKeyPair() {
    const keyPair = nacl.box.keyPair();
    return {
      publicKey: encodeBase64(keyPair.publicKey),
      privateKey: keyPair.secretKey,
    };
  }

  function deriveAESKey(myPrivateKey, peerPublicKeyB64) {
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
    generateKeyPair,
    deriveAESKey,
    encryptMessage,
    decryptMessage
  };

})();
