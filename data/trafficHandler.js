window.trafficHandler = (function() {
    let socket = null;

    // CHANGE Priovate Shared Key to whatever pre-determined value you want
    // const PSK_BASE64 = "jyPUkXrIFV8ysZxOfQLpqFbMN5AaS2L/jilz0FuRbyY="; // both sides must have the same one
    // const PSK = cryptoHandler.decodeBase64(PSK_BASE64);
    const PSK = new Uint8Array([
        0x8f, 0x23, 0xd4, 0x91, 0x7a, 0xc8, 0x14, 0x5f,
        0x32, 0xb1, 0x9c, 0x4e, 0x7d, 0x02, 0xe9, 0xa8,
        0x56, 0xcc, 0x37, 0x90, 0x1a, 0x4b, 0x62, 0xff,
        0x8e, 0x29, 0x73, 0xd0, 0x5b, 0x91, 0x6f, 0xac
    ]);

    let myId = Math.floor(Math.random() * 1e9); // large random ID to avoid collisions
    let role = null; // "init" or "resp"

    let sessionNonce = null;
    let privateKey = null;
    let myPubKey = null;
    let peerSessionNonce = null;
    let peerPubKey = null;

    let salt = null;
    let sharedSecret = null;
    let sessionKey = null;

    let state = "connecting"; // connecting | keyx | ready

    let lastHeartbeat = null; // timestamp of last received HB
    const HEARTBEAT_DELAY = 5000; // 3 seconds
    const HEARTBEAT_TIMEOUT = 10000; // 10 seconds

    // --- Initialize WebSocket connection ---
    async function initConnection() {
        socket = new WebSocket("ws://" + location.hostname + ":81/");

        socket.onopen = () => {
            console.log("Connected to relay");

            const { privateKey: priv, publicKey } = cryptoHandler.generateKeyPair();
            privateKey = priv;
            myPubKey = publicKey;

            sessionNonce = nacl.randomBytes(16); // Raw Bytes

            console.log("Generated new keypair for this session:", myPubKey);
            state = "connecting";
            sendToRelay({ type: "HI", id: myId });
        };

        socket.onmessage = (event) => {
            try {
                const msg = JSON.parse(event.data);
                onRelayMessage(msg);
            } catch (err) {
                console.error("Bad message from relay:", err);
            }
        };

        socket.onclose = () => {
            console.warn("WS Connection closed");
        };
    }

    function onHeartbeatReceived() {
        lastHeartbeat = Date.now();
        if (state === "ready") {
            peerConnected(); // enable input, hide overlay
        } 
    }

    function peerConnected() {
        // ADD UPDATE MESSAGES FOR CONNECTING, ACKED, KEYX, AND READY

        document.getElementById("msg").disabled = false;
        document.getElementById("send-btn").disabled = false;
        document.getElementById("status-overlay").textContent = "Peer Connected";
    }

    function peerDisconnected() {
        document.getElementById("msg").disabled = true;
        document.getElementById("send-btn").disabled = true;
        document.getElementById("status-overlay").textContent = "Waiting for peer to connect";
    }

    function sendToRelay(obj) {
        if (!socket || socket.readyState !== WebSocket.OPEN) {
            console.error("Socket not connected!");
            return;
        }

        socket.send(JSON.stringify(obj));
        console.log(obj);
    }

    function sendEncryptedMessage(text) {
        const encrypted = cryptoHandler.encryptMessage(sessionKey, text);

        //const { ciphertext, iv } = await encryptMessage(sharedKey, text);
        //sendToRelay({ type: "encrypted", ciphertext, iv }); LEARN MORE ABT THIS IV
        sendToRelay({ type: "MSG", data: encrypted });
    }

    // function toHex(u8) {
    // return Array.from(u8)
    //     .map(b => b.toString(16).padStart(2, '0'))
    //     .join('');
    // }

    async function handlePubKey(msg) {
        peerPubKey = msg.key;

        peerSessionNonce = msg.nonce;
        peerSessionNonce = cryptoHandler.decodeBase64(peerSessionNonce);

        if (role === "init") { salt = new Uint8Array([...sessionNonce, ...peerSessionNonce]); }
        else { salt = new Uint8Array([...peerSessionNonce, ...sessionNonce]); } 

        sharedSecret = await cryptoHandler.deriveSharedSecret(privateKey, peerPubKey);
        console.log("Derived Shared Secret established.");
        
        sessionKey = await cryptoHandler.deriveSessionKey(sharedSecret, PSK, salt);
        console.log("Derived Session Key established.");

        // Testing shit
        // appendMessage(toHex(peerPubKey), "received");
        // appendMessage(toHex(privateKey), "received");
        // appendMessage(toHex(sessionNonce), "received");
        // appendMessage(toHex(peerSessionNonce), "received");
        // appendMessage(toHex(salt), "received");
        // appendMessage(toHex(sharedSecret), "received");
        // appendMessage(toHex(sessionKey), "received");
    }

    function handleEncryptedMessage(msg) {
        const plaintext = cryptoHandler.decryptMessage(sessionKey, msg.data);
        appendMessage(plaintext, "received");
    }

    function startKeyExchange() {
        const encodedNonce = cryptoHandler.encodeBase64(sessionNonce); // Text-Safe base64
        sendToRelay({ type: "PKEY", key: myPubKey, nonce: encodedNonce });
    }

    function onRelayMessage(msg) {
        switch (msg.type) {
            case "HB":
                onHeartbeatReceived()
                break;

            case "HI":
                if (!role) {
                    // Assign roles deterministically once
                    role = myId > msg.id ? "init" : "resp";
                }

                if (role === "resp" && state === "connecting") {
                    console.log("Received HI, replying with ACK");
                    sendToRelay({ type: "ACK" });
                    state = "acked";
                }
                break;

            case "ACK":
                if (role === "init") {
                    console.log("ACK received, starting key exchange...");
                    state = "keyx";
                    startKeyExchange();
                }
                break;

            case "PKEY":
                console.log("Public key received, handling...");
                handlePubKey(msg);

                if (role === "resp") {
                    startKeyExchange();
                }

                state = "ready";
                lastHeartbeat = Date.now();
                peerConnected();
                break;

            case "MSG":
                if (state === "ready") {
                    handleEncryptedMessage(msg);
                } else {
                    console.warn("Received message before key exchange was ready.");
                }
                break;

            default:
                console.warn("Unknown message type:", msg.type);
        }
    }

    setInterval(() => {
        if (state === "connecting") {sendToRelay( { type: "HI", id: myId}); }
        else if (state === "ready") {sendToRelay( { type: "HB"}); }
    }, HEARTBEAT_DELAY); // HB / HI every 5 seconds depending on state


    /* check to see if peer is offline periodically */
    setInterval(() => {
        if (state === "ready" && (!lastHeartbeat || (Date.now() - lastHeartbeat > HEARTBEAT_TIMEOUT))) {
            state = "connecting";
            console.log("Peer Disconnected?");
            peerDisconnected(); // disable input, show overlay
        }
    }, HEARTBEAT_DELAY); // check every 5 seconds

    return {
        initConnection,
        sendEncryptedMessage
    };
})();
