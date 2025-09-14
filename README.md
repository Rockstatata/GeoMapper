# 🚗 GeoMapper: An Obstacle Avoidance & Environment Mapping Car (ESP32)

An autonomous car that **detects and avoids obstacles** while also **scanning and mapping** its environment in real time.  
Built using **ESP32, ultrasonic sensors, and an MPU gyroscope**, with data streamed wirelessly to a **browser interface (JavaScript)** for visualization.

---

## 📌 Features
- 🔄 **Autonomous Navigation** – avoids obstacles using ultrasonic sensors  
- 📡 **Real-Time Mapping** – scans surroundings and maps them in 2D  
- 📶 **Wireless Data Transfer** – ESP32 streams live sensor readings to PC  
- 🌐 **Web-Based Visualization** – interactive map displayed in the browser  
- ⚡ **Lightweight & Low-Cost** – built using ESP32 + basic sensors  

---

## 🛠️ Hardware Components
- 1 × ESP32 Dev Board  
- 3 × Ultrasonic Sensors (HC-SR04)  
- 1 × MPU3050 / MPU6050 Gyroscope Sensor  
- 1 × Motor Driver (L298N / L293D / DRV8833)  
- 2 × DC Motors + Wheels + Chassis  
- 1 × Li-ion / LiPo Battery Pack  
- Jumper wires, breadboard, optional 3D-printed mounts  

---

## ⚙️ Software & Tools
- **ESP32 Arduino Core** (Arduino IDE / PlatformIO)  
- **WebSocket / Serial Communication**  
- **JavaScript (Canvas/D3.js/Three.js)** for browser visualization  
- **HTML + CSS** for UI  
- (Optional) **Python (Matplotlib/PyQtGraph)** alternative for visualization  

---

## 🔧 System Architecture

```

\[ Ultrasonic Sensors ] --
\[ Gyroscope (MPU3050) ] ----> \[ ESP32 ] ---> WiFi (WebSocket) ---> \[ Browser UI ]
\[ Motor Driver + Motors ] ---/

```

---

## 🚀 How It Works
1. Car moves forward until an obstacle is detected.  
2. Sonar sensors scan the environment (front, left, right).  
3. Gyroscope provides orientation (angle/heading).  
4. ESP32 sends sensor data as JSON via WiFi to PC.  
5. Browser receives data via WebSocket and **plots a live 2D map**.  

---

## 📂 Project Structure
```

📁 ObstacleMappingCar
├── 📁 esp32-code       # Arduino/PlatformIO code for ESP32
├── 📁 web-visualizer   # Browser interface (HTML, CSS, JS)
├── 📁 docs             # Diagrams, system design, notes
└── README.md           # Project documentation

```

---

## 🖥️ Web Interface Preview
*(screenshot/gif to be added here after implementation)*  
The web UI will display:
- Real-time sonar distances  
- Car’s current heading  
- Incremental map of the room  

---

## 🏗️ Setup & Installation
### 1. Flash ESP32
- Install [Arduino IDE](https://www.arduino.cc/en/software)  
- Add **ESP32 Board Support** via Board Manager  
- Open `esp32-code/obstacle_mapping.ino` and upload to ESP32  

### 2. Run Web Visualizer
- Open `web-visualizer/index.html` in your browser  
- Connect to ESP32’s WebSocket server (update IP in `script.js`)  

---

## 🧪 Demo
- Car drives autonomously in a room  
- Avoids collisions  
- Browser shows live mapping of walls and free space  

---

## 🔮 Future Improvements
- 🛰️ Add SLAM algorithms for precise mapping  
- 📍 Integrate odometry for accurate positioning  
- 📷 Add camera for computer vision-based navigation  
- 📡 Cloud data logging for analysis  

---

## 🤝 Contribution
Pull requests are welcome!  
If you’d like to improve the visualization or control system, feel free to fork and submit changes.  

---

## 📜 License
This project is licensed under the **MIT License** – feel free to use, modify, and share.  

---

