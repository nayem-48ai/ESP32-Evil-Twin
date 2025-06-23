
ESP32 Smart Evil Twin

A feature-rich Evil Twin attack platform running on the ESP32 for educational and ethical security testing purposes.

<div align="center">


![alt text](https://img.shields.io/badge/License-MIT-yellow.svg)


![alt text](https://img.shields.io/badge/Platform-ESP32-blue.svg)


![alt text](https://img.shields.io/badge/Framework-Arduino-00979D.svg)


![alt text](https://img.shields.io/badge/Version-1.4-brightgreen.svg)

</div>

<br>

<div align="center">
<svg width="400" height="120" xmlns="http://www.w3.org/2000/svg">
<style>
.warning-text { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji"; font-size: 16px; font-weight: bold; fill: #d73a49; text-anchor: middle; }
.warning-icon { fill: #f6f8fa; stroke: #d73a49; stroke-width: 2; }
.pulse { animation: pulse 1.5s infinite; }
@keyframes pulse {
0% { opacity: 1; }
50% { opacity: 0.3; }
100% { opacity: 1; }
}
</style>
<rect width="100%" height="100%" fill="#fff0f1" stroke="#d73a49" stroke-width="2" rx="10" ry="10" />
<path class="warning-icon" d="M75,20 L125,100 L25,100 Z" transform="translate(15,0) scale(0.8)"/>
<text x="50" y="70" font-size="40" fill="#d73a49" text-anchor="middle" class="pulse">!</text>
<text x="250" y="50" class="warning-text">WARNING: ETHICAL USE ONLY</text>
<text x="250" y="75" class="warning-text" style="font-size:12px; font-weight:normal;">This tool is for educational purposes on</text>
<text x="250" y="92" class="warning-text" style="font-size:12px; font-weight:normal;">networks you own or have permission to test.</text>
</svg>
</div>

Overview

The ESP32 Smart Evil Twin is a powerful Wi-Fi security auditing tool that demonstrates the principles of an Evil Twin attack. It provides a modern, dashboard-style web interface for control, automates the process of impersonating a network, and includes a real-time password verification feature to confirm captured credentials.

This project transforms a standard ESP32 into a standalone platform for practical cybersecurity education.

Features

Modern Dashboard UI: A clean, responsive control panel built with Bootstrap 5 for easy management from any device.

Wi-Fi Scanning: Scans for and lists nearby Wi-Fi networks to select a target.

Evil Twin AP: Creates an open Wi-Fi network by appending " 5G" to the target's SSID to lure users.

Realistic Captive Portal: Redirects all connected clients to a convincing Wi-Fi login page to capture the password. Includes a show/hide password toggle.

Real-Time Password Verification: After capturing a password, the ESP32 attempts to connect to the legitimate network to verify if the password is correct.

Credential Storage: Successfully captured and verified credentials are listed on the main dashboard.

Persistent Control AP Settings: The SSID and password for the control access point can be changed via the web UI and are saved to the ESP32's non-volatile storage.

Screenshots
<table style="width:100%; border: none;">
<tr>
<td align="center"><b>Main Control Panel</b></td>
<td align="center"><b>Network Scan Results</b></td>
<td align="center"><b>Captive Portal Login</b></td>
</tr>
<tr>
<td><img src="https://i.imgur.com/g8e1WzL.png" alt="Main Dashboard"></td>
<td><img src="https://i.imgur.com/Bf0y3yH.png" alt="Scan Results"></td>
<td><img src="https://i.imgur.com/xO8t3R9.png" alt="Captive Portal"></td>
</tr>
</table>

Hardware & Software Requirements
Hardware

An ESP32 Development Board (e.g., NodeMCU-32S, WEMOS LOLIN D32)

Software

```Arduino IDE```

```ESP32 Board Manager for Arduino```

The following libraries are used but are typically included with the ESP32 core installation:

```WiFi.h```

```WebServer.h```

```DNSServer.h```

```Preferences.h```

Installation & Usage

Setup Arduino IDE: Make sure your Arduino IDE is set up with the ESP32 board manager.

Open the Sketch: Open the ESP32_Evil_Twin.ino file in the Arduino IDE.

Select Board & Port: Choose your ESP32 board from Tools > Board and the correct COM port from Tools > Port.

Upload the Code: Click the upload button to flash the sketch to your ESP32.

Connect to the Control AP:

Using your phone or computer, scan for Wi-Fi networks.

Connect to the access point with the default SSID: Deauth

Use the default password: 12345678

Access the Control Panel:

Open a web browser and navigate to http://192.168.4.1.

You will see the main dashboard.

Launch an Attack:

Click the "Start Scan" button.

A list of nearby networks will appear.

Click the "Start" button next to the network you wish to impersonate.

The ESP32 will restart its Wi-Fi in attack mode, hosting the fake AP. The control panel will be inaccessible until the attack is stopped.

Capture Credentials:

When a user connects to the fake AP, they will be redirected to the captive portal.

If they enter a password, the ESP32 will capture it and attempt to verify it.

If the password is correct, it will be saved and displayed on the dashboard after the device returns to control mode.

If incorrect, the portal will show an error, allowing the user to try again.

How It Works

The device operates in two primary modes:

Control Mode: The ESP32 hosts a password-protected access point (Deauth). This AP serves the control panel, allowing the operator to scan networks, view captured credentials, and configure settings.

Attack Mode: After a target is selected, the device switches modes:

It stops the control AP.

It starts a new, open AP with the target's SSID and a " 5G" suffix.

A DNS server is activated to redirect all traffic from any connected client to the ESP32's captive portal.

The captive portal serves a login page asking for the Wi-Fi password.

Upon submission, the handleCapture function is triggered. The ESP32 temporarily goes into WIFI_STA mode to test the credentials against the real network.

Based on the verification result, it either returns to Control Mode (on success) or restarts the attack (on failure).

⚠️ Disclaimer

This project is intended strictly for educational purposes and for ethical security testing on networks that you own or have explicit permission to test. The creator of this software is not responsible for any misuse or damage caused by this program. Using this tool on networks without authorization is illegal. Always act responsibly and ethically.
