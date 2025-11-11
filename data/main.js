/* WEBSOCKET SERVER */

trafficHandler.initConnection();

/* MSG */

function appendMessage(message, type) {
  let chat = document.getElementById("chat");
  let msg = document.createElement("div");
  msg.className = `message ${type}`
  msg.textContent = message;
  chat.appendChild(msg);
  chat.scrollTop = chat.scrollHeight; // auto-scroll
}

/* MSG SEND */

const input = document.getElementById("msg");
const counter = document.getElementById("char-count");

/* So mobile devices dont fuck me */

// Scroll back to top when input/textarea loses focus
document.addEventListener('focusout', (event) => {
  if (event.target.tagName === 'INPUT') {
    // Delay a bit so iOS finishes resizing the viewport
    setTimeout(() => {
      window.scrollTo({ top: 0, behavior: 'smooth' });
    }, 100);
  }
});

async function sendMsg() {
  if (input.value.trim() !== "") {
    appendMessage(input.value, "sent");

    /* send through socket to blind relay */
    trafficHandler.sendEncryptedMessage(input.value);

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
