/* Root page (inline HTML, stored in flash) */
const char IndexHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Soil Moisture – ESP32</title>
  <style>
    body{font-family:sans-serif;text-align:center;margin-top:2rem;}
    h1{font-size:1.6rem;margin-bottom:1rem;}
    p{font-size:2.5rem;color:#4842f5;margin:0;}
    span{font-weight:bold;}
  </style>
</head>
<body>
  <h1>Soil Moisture with ESP32</h1>
  <p>Moisture Level: <span id="val">--</span>%</p>

  <script>
    function update() {
      fetch("/readMoisture")
        .then(r => r.text())
        .then(t => document.getElementById("val").textContent = t);
    }
    setInterval(update, 500); // update every 0.5 s
    update(); // first run
  </script>
</body>
</html>
)rawliteral";
