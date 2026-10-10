#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

const char* AP_SSID = "FREE-WIFI";

DNSServer dnsServer;
ESP8266WebServer server(80);

const byte DNS_PORT = 53;

const char HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>Wi-Fi</title>

<style>

body {
  margin: 0;
  font-family: Arial, sans-serif;
  background: #f2f2f2;

  display: flex;
  justify-content: center;
  align-items: center;

  min-height: 100vh;
}

.box {
  width: 90%;
  max-width: 380px;

  background: white;

  padding: 30px;

  border-radius: 15px;

  text-align: center;

  box-shadow: 0 5px 20px rgba(0,0,0,.15);
}

button {

  width: 100%;

  padding: 16px;

  border: 0;

  border-radius: 10px;

  background: #007bff;

  color: white;

  font-size: 18px;

}

button:active {
  transform: scale(.97);
}

#status {

  margin-top: 15px;

  font-size: 15px;

}

</style>

</head>

<body>

<div class="box">

<h1>Wi-Fi</h1>

<p>
Silakan tekan tombol di bawah.
</p>

<button onclick="playPrank()">
Sambungkan Wi-Fi
</button>

<p id="status">
...
</p>

</div>


<script>

function playPrank() {

  document.getElementById("status").innerHTML =
    "Connecting...";

  console.log("BUTTON DIKLIK");

  var audio = new Audio("/prank.mp3");

  audio.volume = 1.0;

  audio.play()
    .then(function() {

      document.getElementById("status").innerHTML =
        "Berhasil tersambung!";

    })
    .catch(function(error) {

      document.getElementById("status").innerHTML =
        "Koneksi gagal, silahkan klik sambungkan lagi";

      console.log(error);

    });

}

</script>

</body>
</html>
)rawliteral";


void setup() {

  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("=== CAPTIVE PORTAL ===");

  // =========================
  // LITTLEFS
  // =========================

  if (!LittleFS.begin()) {

    Serial.println("LittleFS gagal!");

    return;
  }

  Serial.println("LittleFS OK");

  if (LittleFS.exists("/prank.mp3")) {

    File file = LittleFS.open("/prank.mp3", "r");

    Serial.print("prank.mp3 ditemukan: ");
    Serial.print(file.size());
    Serial.println(" bytes");

    file.close();

  } else {

    Serial.println("prank.mp3 TIDAK ditemukan!");

  }


  // =========================
  // ACCESS POINT
  // =========================

  WiFi.mode(WIFI_AP);

  WiFi.softAP(AP_SSID);

  Serial.print("SSID: ");
  Serial.println(AP_SSID);

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());


  // =========================
  // DNS CAPTIVE PORTAL
  // =========================

  dnsServer.start(
    DNS_PORT,
    "*",
    WiFi.softAPIP()
  );


  // =========================
  // MAIN PAGE
  // =========================

  server.on("/", HTTP_GET, []() {

    server.send_P(
      200,
      "text/html",
      HTML
    );

  });


  // =========================
  // MP3
  // =========================

  server.on("/prank.mp3", HTTP_GET, []() {

    File file = LittleFS.open(
      "/prank.mp3",
      "r"
    );

    if (!file) {

      server.send(
        404,
        "text/plain",
        "prank.mp3 tidak ditemukan"
      );

      return;
    }

    server.streamFile(
      file,
      "audio/mpeg"
    );

    file.close();

  });


  // =========================
  // CAPTIVE PORTAL REDIRECT
  // =========================

  server.onNotFound([]() {

    server.sendHeader(
      "Location",
      "http://192.168.4.1/",
      true
    );

    server.send(
      302,
      "text/plain",
      ""
    );

  });


  server.begin();

  Serial.println("Web server aktif");

}


void loop() {

  dnsServer.processNextRequest();

  server.handleClient();

}