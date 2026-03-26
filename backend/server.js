const mqtt = require('mqtt');

const mqttClient = mqtt.connect('mqtt://broker.hivemq.com');

mqttClient.on('connect', () => {
  console.log("Connected to MQTT broker");
});

const express = require('express');
const cors = require('cors');

const authRoutes = require('./routes/auth');
const verifyToken = require('./middleware/verifyToken');

const app = express();

// Middleware
app.use(cors({
  origin: [
    "https://trio-home-iot.netlify.app",
    "http://localhost:5500"
  ]
}));
app.use(express.json());

// Routes
app.use('/', authRoutes);

// Test public route
app.get('/', (req, res) => {
  res.send("SCADA Backend Running");
});

// 🔐 Protected route example
app.get('/secure-data', verifyToken, (req, res) => {
  res.json({
    message: "Protected data access granted",
    user: req.user
  });
});

app.post('/control', verifyToken, (req, res) => {
  const { device, state } = req.body;

  const topic = "smarthome/plc1/control";

  mqttClient.publish(topic, JSON.stringify({ [device]: state }));

  console.log(`User ${req.user.user} → ${topic}: ${state}`);

  res.json({ success: true });
});

// Server start
const PORT = 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});