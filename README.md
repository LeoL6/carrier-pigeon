# Carrier Pigeon 
Carrier Pigeon is a Open Source E2EE LoraWAN based Messaging Application, created for the ESP32.
## Specs
> Majority of Crypto is performed using [monocipher.c](https://github.com/LoupVaillant/monocypher).
### Encryption
AEAD (XSalsa20-Poly1305)
### Key Exchange
Diffie–Hellman (NaCl keypairs)
### Authentication
HMAC (PSK + session key)
### Key Material
ECDH-derived shared key (X25519) + 12 Char PSK
### Nonces
Session nonce + per-message role + counter nonce 
### Integrity
Authenticated encryption (AEAD)

**Security Note**
Ensure 12 character passphrase is high entropy. (VERY RANDOM)
