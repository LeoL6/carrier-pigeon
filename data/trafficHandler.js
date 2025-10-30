window.trafficHandler = (function() {
    let socket = null;

    let myId = Math.floor(Math.random() * 1e9); // large random ID to avoid collisions
    let role = null; // "init" or "resp"

    let privateKey = null;
    let myPubKey = null;
    let peerPubKey = null;
    let aesKey = null;

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
        const encrypted = cryptoHandler.encryptMessage(aesKey, text);

        //const { ciphertext, iv } = await encryptMessage(sharedKey, text);
        //sendToRelay({ type: "encrypted", ciphertext, iv }); LEARN MORE ABT THIS IV
        sendToRelay({ type: "MSG", data: encrypted });
    }

    async function handlePubKey(msg) {
        peerPubKey = msg.key;
        aesKey = await cryptoHandler.deriveAESKey(privateKey, peerPubKey);
        console.log("Derived AES key established.");
    }

    function handleEncryptedMessage(msg) {
        const plaintext = cryptoHandler.decryptMessage(aesKey, msg.data);
        appendMessage(plaintext, "received");
    }

    function startKeyExchange() {
        sendToRelay({ type: "PKEY", key: myPubKey });
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
