# Carrier Pigeon - Standalone
Carrier Pigeon is a Open Source E2EE LoraWAN based Messaging Firmware, created for the ESP32.

## Hardware Specs
Heltec LoRa V3.2 (ESP32-S3)

Waveshare 3.7" Display (416x240) (SPI)

M5Stack CardKB (I2C)

## Firmware Specs
> Majority of Crypto is performed using [monocipher.c](https://github.com/LoupVaillant/monocypher).
### Encryption
AEAD (XSalsa20-Poly1305)
### Key Exchange
Elliptic Curve Diffie–Hellman (X25519)
### Authentication
Blake2b MAC (PSK + session key)
### Key Material
ECDH-derived shared key + 12 Char PSK
### Nonces
(Session nonce) + (Per-message role + counter nonce)
### Integrity
Authenticated encryption (AEAD)

**Security Note**
Ensure 12 character passphrase is high entropy. (VERY RANDOM)
