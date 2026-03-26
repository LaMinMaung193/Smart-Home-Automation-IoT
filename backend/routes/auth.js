const express = require('express');
const jwt = require('jsonwebtoken');

const router = express.Router();
const SECRET_KEY = "scada_secret";

router.post('/login', (req, res) => {
  const { username, password } = req.body;

  if (username === "Team Trio" && password === "hello123") {
    const token = jwt.sign({ user: username }, SECRET_KEY, { expiresIn: "2h" });
    res.json({ token });
  } else {
    res.status(401).json({ error: "Invalid credentials" });
  }
});


module.exports = router;

