# ESP32 Space Invaders LED Game

A Space Invaders-style game using an ESP32 and a 168-LED NeoPixel strip. Control the game through a web interface while watching the action unfold on your LED strip!

![ESP32](https://img.shields.io/badge/ESP32-WiFi-blue)
![Arduino](https://img.shields.io/badge/Framework-Arduino-orange)
![NeoPixel](https://img.shields.io/badge/LED-NeoPixel-green)

## 🎮 Game Overview

Transform your LED strip into an interactive Space Invaders game where:
- **🟢 You** defend the left side of the LED strip
- **🔴 Enemies** advance from the right side  
- **🔵 Bullets** travel across the strip to destroy enemies
- **📱 Control** everything through a web browser

## 🛠️ Hardware Requirements

### Components
- ESP32 Development Board
- WS2812B NeoPixel LED Strip (168 LEDs)
- Power Supply (5V, sufficient amperage for LED strip)
- Jumper Wires

### Wiring
```
ESP32    →    LED Strip
GPIO 4   →    Data In (DI)
5V       →    5V Power
GND      →    GND
```

⚠️ **Important**: Ensure your power supply can handle the current draw of all 168 LEDs at maximum brightness (~10A for full white at 100% brightness).

## 📋 Software Requirements

### Development Environment
- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- ESP32 Board Support Package
- Adafruit NeoPixel Library

### PlatformIO Dependencies
```ini
lib_deps = 
    adafruit/Adafruit NeoPixel@^1.12.0
```

## 🚀 Installation & Setup

### 1. Clone or Download the Project
```bash
git clone <repository-url>
cd ESP32_Invader
```

### 2. Configure WiFi
Edit `src/main.cpp` and update your WiFi credentials:
```cpp
#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASSWORD "YourWiFiPassword"
```

### 3. Build & Flash
```bash
# Build the firmware
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output (optional)
pio device monitor
```

### 4. Find Your ESP32
After flashing, the ESP32 will connect to your WiFi network. Find its IP address:
- Check your router's connected devices list
- Use a network scanner app like Fing
- Check serial monitor during startup (if connected)

## 🎯 How to Play

### Starting the Game
1. Connect power to your ESP32 and LED strip
2. Wait for WiFi connection (green LED on ESP32)
3. Open web browser and navigate to `http://<ESP32_IP_ADDRESS>`
4. Game starts automatically!

### Controls
- **🔫 SHOOT!** - Fire a bullet from your position
- **🔄 RESTART** - Start a new game (when game over)

### Game Mechanics
| Element | Color | Position | Behavior |
|---------|-------|----------|----------|
| Player | 🟢 Green | LED 0 (leftmost) | Fixed position |
| Enemies | 🔴 Red | LEDs 158-167 (right side) | Move left every 2 seconds |
| Bullets | 🔵 Blue | Travel left→right | Fast movement (200ms intervals) |

### Scoring System
- **100 points** per enemy destroyed
- **3 lives** to start
- **Game Over** when an enemy reaches position 0
- **New enemies** spawn randomly

## 🏗️ Technical Architecture

### Game Engine
- **Update Rate**: 50ms main loop
- **Enemy Movement**: 2000ms intervals
- **Bullet Movement**: 200ms intervals
- **Max Concurrent Bullets**: 5
- **Max Concurrent Enemies**: 8

### LED Strip Mapping
- **Total LEDs**: 168
- **Player Position**: Fixed at index 0
- **Enemy Spawn Zone**: Indices 158-167
- **Data Pin**: GPIO 4
- **LED Type**: WS2812B (GRB color order)

### Web Interface
- **Framework**: Vanilla HTML/CSS/JavaScript
- **Auto-refresh**: Every 2 seconds
- **LED Preview**: Shows first 50 LEDs
- **Responsive Design**: Mobile-friendly

## 📁 Project Structure

```
ESP32_Invader/
├── src/
│   └── main.cpp              # Main game logic and web server
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This documentation
```

## 🔧 Configuration Options

### Game Settings (in `src/main.cpp`)
```cpp
#define LED_PIN    4           // NeoPixel data pin
#define LED_COUNT  168         // Total LED count
#define WIFI_SSID "Veblu"      // WiFi network name
#define WIFI_PASSWORD "VaLu2014" // WiFi password
```

### LED Settings
```cpp
strip.setBrightness(150);     // LED brightness (0-255)
```

### Timing Adjustments
```cpp
// Enemy movement speed
if (currentTime - lastEnemyMove > 2000) // 2 seconds

// Bullet speed  
if (currentTime - lastBulletMove > 200)  // 200ms

// Web page refresh rate
setInterval(()=>location.reload(), 2000); // 2 seconds
```

## 🐛 Troubleshooting

### Common Issues

**LEDs Not Working**
- Check data pin connection (GPIO 4)
- Verify power supply is adequate
- Confirm LED strip is WS2812B compatible

**WiFi Connection Issues**
- Verify SSID and password are correct
- Check 2.4GHz WiFi availability
- Move ESP32 closer to router

**Web Page Not Loading**
- Confirm ESP32 is connected to WiFi
- Check IP address is correct
- Try a different browser

**Game Performance Issues**
- Reduce LED brightness
- Decrease enemy count
- Check power supply stability

### Serial Monitor Output
Connect to serial monitor (115200 baud) to see:
- WiFi connection status
- IP address assignment
- Game state changes
- Error messages

## 🎨 Customization Ideas

### Gameplay Variations
```cpp
// Different movement patterns
enemies[i] += random(-2, 3);  // Random movement

// Power-ups
if (random(1000) < 1) {
    spawnPowerUp();  // Create special ability
}

// Difficulty levels
int enemySpeed = 2000 - (score / 100) * 100; // Faster as score increases
```

### Visual Effects
```cpp
// Explosion animation
void showExplosion(int position) {
    for (int i = 0; i < 5; i++) {
        strip.setPixelColor(position-i, strip.Color(255, 255, 0));
        strip.setPixelColor(position+i, strip.Color(255, 255, 0));
    }
    strip.show();
    delay(100);
}
```

## 📄 License

This project is open source. Feel free to modify and distribute.

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- Sound effects
- More enemy types
- Power-ups
- High score tracking
- Multiplayer support

## 📞 Support

For issues and questions:
1. Check this README
2. Review the code comments
3. Test with serial monitor
4. Create an issue with detailed description

---

## 🚀 Development History

This project was built incrementally through the following phases:

### Phase 1: Basic LED Control ✅
- Simple NeoPixel test with all white LEDs
- Verified 168 LED strip functionality on GPIO 4
- Confirmed power and data connections

### Phase 2: LED Position Testing ✅  
- First LED green, last LED red
- Verified individual LED addressing
- Confirmed strip orientation and mapping

### Phase 3: WiFi & Web Server ✅
- Connected to WiFi network "Veblu"
- Hosted basic HTML web interface
- Served system information and status

### Phase 4: Space Invaders Implementation ✅
- Implemented game mechanics on LED strip
- Added web-based shooting controls
- Created real-time LED visualization
- Added score tracking and game states

### Current Status: ✅ FULLY FUNCTIONAL
The game is now complete with:
- Working Space Invaders gameplay on 168 LEDs
- Web-based controls with shooting button
- Real-time visual feedback on LED strip
- Score tracking and game over system
- Mobile-friendly web interface

---

**Enjoy your LED Space Invaders game! 🎮✨**