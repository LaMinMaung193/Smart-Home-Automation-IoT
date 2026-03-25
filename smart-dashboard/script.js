const client = mqtt.connect('ws://broker.hivemq.com:8000/mqtt');

client.on('connect', function () {
  console.log("Connected to MQTT");

  client.subscribe("smarthome/status");
  client.subscribe("smarthome/sensors");
  client.subscribe("smarthome/availability");
});

client.on('message', function (topic, message) {

  let data = JSON.parse(message.toString());

  if (topic === "smarthome/availability") {
    document.getElementById("plcStatus").innerText =
      data.plc_online ? "Online" : "Offline";
  }

  if (topic === "smarthome/sensors") {
    document.getElementById("pirStatus").innerText = data.pir;
    document.getElementById("ldrStatus").innerText = data.ldr;
    document.getElementById("doorStatus").innerText = data.door;
    document.getElementById("tempStatus").innerText = data.temperature;
  }

  if (topic === "smarthome/status") {
    document.getElementById("modeStatus").innerText =
      data.mode === 0 ? "AUTO" : "MANUAL";

    document.getElementById("fireStatus").innerText =
      data.fire === 1 ? "🔥 FIRE" : "Normal";
  }

});

function setMode(modeValue) {
  client.publish("smarthome/control", JSON.stringify({ mode: modeValue }));
}

function controlDevice(device, state) {
  let payload = {};
  payload[device] = state;
  client.publish("smarthome/control", JSON.stringify(payload));
}