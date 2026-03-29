// 🔐 AUTH CHECK
const token = localStorage.getItem("token");
if (!token) {
  window.location.href = "login.html";
}

// 📡 MQTT
const client = mqtt.connect("wss://broker.hivemq.com:8884/mqtt");


// 🔊 Enable sound on first user interaction (ALL DEVICES FIX)
function unlockAudio() {
  const alarm = document.getElementById("alarmSound");
  if (!alarm) return;

  alarm.muted = false;

  alarm.play().then(() => {
    alarm.pause();
    alarm.currentTime = 0;
    console.log("🔊 Sound enabled");
  }).catch((e) => {
    console.log("Audio unlock failed:", e);
  });
}

// Support ALL devices (Laptop + Android + iPhone + iPad)
document.body.addEventListener("click", unlockAudio, { once: true });
document.body.addEventListener("touchstart", unlockAudio, { once: true });
document.body.addEventListener("keydown", unlockAudio, { once: true });

const tempChart = new Chart(document.getElementById("tempChart"), {
  type: 'line',
  data: {
    labels: [],
    datasets: [{ label: 'Temperature (°C)', data: [] }]
  }
});

const ldrChart = new Chart(document.getElementById("ldrChart"), {
  type: 'line',
  data: {
    labels: [],
    datasets: [{ label: 'LDR', data: [] }]
  }
});

function setStatus(id, text, isOn = null) {
  const el = document.getElementById(id);
  if (!el) return;

  el.innerText = text;

  if (isOn !== null) {
    el.style.color = isOn ? "lime" : "red";
  }
}

client.on('connect', () => {
  console.log("MQTT Connected");
  client.subscribe("smarthome/plc1/status");
});

client.on('message', (topic, message) => {
  const data = JSON.parse(message.toString());

  setStatus("plcStatus", data.plc_online ? "🟢 Online" : "🔴 Offline", data.plc_online);
  setStatus("modeStatus", data.mode === 0 ? "AUTO" : "MANUAL");
  setStatus("tempStatus", data.temperature + " °C");

  setStatus("pirStatus", data.pir ? "Motion" : "No Motion", data.pir);
  setStatus("ldrStatus", data.ldr ? "Dark" : "Light Detected", data.ldr);
  setStatus("doorStatus", data.door ? "Closed" : "Open", data.door);
  setStatus("smokeStatus", data.smoke ? "Smoke Detected" : "Clear", !data.smoke);

  setStatus("lightStatus", data.light ? "🟢 ON" : "🔴 OFF", data.light);
  setStatus("fanStatus", data.fan ? "🟢 ON" : "🔴 OFF", data.fan);
  setStatus("buzzerStatus", data.buzzer ? "🟢 ON" : "🔴 OFF", data.buzzer);

  const fireDiv = document.getElementById("fireAlert");
  const alarm = document.getElementById("alarmSound");

  if (data.fire === 1) {
    fireDiv.classList.remove("hidden");
    if (alarm.paused) {
      alarm.play().catch(() => {});
    }
  } else {
    fireDiv.classList.add("hidden");
    alarm.pause();
    alarm.currentTime = 0;
  }
  
  const smokeDiv = document.getElementById("smokeAlert");

  if (data.smoke === 1) {
    smokeDiv.classList.remove("hidden");
  } else {
    smokeDiv.classList.add("hidden");
  }

  document.getElementById("offlineAlert")
    .classList.toggle("hidden", data.plc_online);

  let time = new Date().toLocaleTimeString();

  tempChart.data.labels.push(time);
  tempChart.data.datasets[0].data.push(data.temperature);

  ldrChart.data.labels.push(time);
  ldrChart.data.datasets[0].data.push(data.ldr);

  if (tempChart.data.labels.length > 10) {
    tempChart.data.labels.shift();
    tempChart.data.datasets[0].data.shift();
    ldrChart.data.labels.shift();
    ldrChart.data.datasets[0].data.shift();
  }

  tempChart.update();
  ldrChart.update();
});

function controlDevice(device, state) {
  fetch("https://smart-home-automation-iot.onrender.com/control", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      "Authorization": "Bearer " + token
    },
    body: JSON.stringify({ device, state })
  });
}

function setMode(mode) {
  controlDevice("mode", mode);
}

function logout() {
  localStorage.removeItem("token");
  window.location.href = "login.html";
}