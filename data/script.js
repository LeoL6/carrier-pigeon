/* WEBSOCKET SERVER */

let socket = new WebSocket("ws://" + location.hostname + ":81/");

let lastHeartbeat = null; // timestamp of last received ACK
const HEARTBEAT_TIMEOUT = 10000; // 10 seconds

socket.onopen = () => {
  console.log("Connected to WebSocket");
};

function onHeartbeatReceived() {
    lastHeartbeat = Date.now();
    peerConnected(); // enable input, hide overlay
}

function peerConnected() {
    document.getElementById("msg").disabled = false;
    document.getElementById("send-btn").disabled = false;
    document.getElementById("status-overlay").textContent = "Peer Connected";
}

function peerDisconnected() {
    document.getElementById("msg").disabled = true;
    document.getElementById("send-btn").disabled = true;
    document.getElementById("status-overlay").textContent = "Waiting for peer to connect";
}

/* MSG */

function appendMessage(message, type) {
  let chat = document.getElementById("chat");
  let msg = document.createElement("div");
  msg.className = `message ${type}`
  msg.textContent = message;
  chat.appendChild(msg);
  chat.scrollTop = chat.scrollHeight; // auto-scroll
}

/* MSG RECEIVE */

socket.onmessage = (event) => {
    const msg = event.data;
    if (msg === 'ACK') {
      onHeartbeatReceived();
    } else {
      appendMessage(event.data, "received");
    }
};

/* MSG SEND */

const input = document.getElementById("msg");
const counter = document.getElementById("char-count");

function sendMsg() {
  if (input.value.trim() !== "") {
    appendMessage(input.value, "sent");

    /* send through socket */
    socket.send(input.value);

    /* reset counter */
    counter.textContent = "0 / 180";
    counter.style = "color: #eee;"

    input.value = "";
  }
}

input.addEventListener("keyup", function(event) {
    if (event.key === "Enter") {
        event.preventDefault();
        sendMsg();
    }
})

input.addEventListener("input", () => {
    counter.textContent = `${msg.value.length} / 180`;
    if (msg.value.length < 100) { counter.style = "color: #eee;" } else
    if (msg.value.length < 150) { counter.style = "color: #e0cc18ff;" } else
    if (msg.value.length < 180) { counter.style = "color: #e08600ff;" } 
    else { counter.style = "color: #e20e0eff;" } 
});

/* check to see if peer is offline periodically */

setInterval(() => {
    if (!lastHeartbeat || (Date.now() - lastHeartbeat > HEARTBEAT_TIMEOUT)) {
        peerDisconnected(); // disable input, show overlay
    }
}, 4000); // check every 4 seconds

setInterval(() => {
    socket.send("ACK");
}, 5000); // ACKS every 5 seconds

// DO ACKS FROM SOCKET -> LORA -> SOCKET
