# Carrier Pigeon 
Carrier Pigeon is a Open Source E2EE LoraWAN based Messaging Application, created for the ESP32.
## Specs
> Majority of Crypto is performed using [TweetNacl.js](https://github.com/dchest/tweetnacl-js).
### Encryption
NaCl secretbox (XSalsa20-Poly1305)
### Key Exchange
Diffie–Hellman (NaCl keypairs)
### Authentication
HMAC (PSK + session key)
### Key Material
DH-derived shared key + PSK
### Nonces
Session nonce + per-message nonce 
###Integrity
Authenticated encryption (AEAD)

## Getting Started
To get started using carrier pigeon you will first want to change the Access Point password to something of your choosing.
Find this line in **webserver_setup.h**.
```
WiFi.softAP("Carrier Pigeon", "PUT YOUR PASSWORD HERE");
```
After that, navigate to the **/data** folder, and replace this variable within **trafficHandler.js**, with a fresh new 32 bit PSK.
```
const PSK = new Uint8Array([
    0x8f, 0x23, 0xd4, 0x91, 0x7a, 0xc8, 0x14, 0x5f,
    0x32, 0xb1, 0x9c, 0x4e, 0x7d, 0x02, 0xe9, 0xa8,
    0x56, 0xcc, 0x37, 0x90, 0x1a, 0x4b, 0x62, 0xff,
    0x8e, 0x29, 0x73, 0xd0, 0x5b, 0x91, 0x6f, 0xac
]);
```
## How to Use
Once connected to your ESP32's access point, navigate to ```192.168.4.1```;
Furthermore, once your peer has repeated the same steps, you will alerted that the receiving party has connected. 

You are now ready to start chatting with **Carrier Pigeon**!
