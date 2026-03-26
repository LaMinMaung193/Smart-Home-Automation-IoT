const client = mqtt.connect("wss://broker.hivemq.com:8884/mqtt");

let tempData = [];
let ldrData = [];

const tempChart = new Chart(document.getElementById("tempChart"), {
  type: 'line',
  data: { labels: [], datasets: [{ label: 'Temp', data: [] }] }
});

const ldrChart = new Chart(document.getElementById("ldrChart"), {
  type: 'line',
  data: { labels: [], datasets: [{ label: 'LDR', data: [] }] }
});

client.on('connect', () => {
  console.log("Connected");
  client.subscribe("smarthome/plc1/status");
});

client.on('message', (topic, message) => {

  const data = JSON.parse(message.toString());

  // STATUS
  document.getElementById("plcStatus").innerText = data.plc_online ? "Online" : "Offline";
  document.getElementById("modeStatus").innerText = data.mode === 0 ? "AUTO" : "MANUAL";
  document.getElementById("tempStatus").innerText = data.temperature;

  document.getElementById("pirStatus").innerText = data.pir;
  document.getElementById("ldrStatus").innerText = data.ldr;
  document.getElementById("doorStatus").innerText = data.door;

  document.getElementById("lightStatus").innerText = data.light;
  document.getElementById("fanStatus").innerText = data.fan;
  document.getElementById("buzzerStatus").innerText = data.buzzer;

  // BUTTON FEEDBACK
  toggle("lightOnBtn", data.light);
  toggle("fanOnBtn", data.fan);
  toggle("buzzerOnBtn", data.buzzer);

  // FIRE ALERT
  const fireDiv = document.getElementById("fireAlert");
  const alarm = document.getElementById("alarmSound");

  if (data.fire === 1) {
    fireDiv.classList.remove("hidden");
    alarm.play();
  } else {
    fireDiv.classList.add("hidden");
    alarm.pause();
  }

  // PLC OFFLINE
  const offlineDiv = document.getElementById("offlineAlert");
  if (!data.plc_online) {
    offlineDiv.classList.remove("hidden");
  } else {
    offlineDiv.classList.add("hidden");
  }

  // CHART UPDATE
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

function toggle(id, state) {
  document.getElementById(id).classList.toggle("active", state);
}

function setMode(mode) {
  client.publish("smarthome/plc1/control", JSON.stringify({ mode }));
}

function controlDevice(device, state) {
  client.publish("smarthome/plc1/control", JSON.stringify({ [device]: state }));
}