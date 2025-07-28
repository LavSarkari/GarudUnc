# 🦅 GarudUnc - AI-Powered Security System

> **An intelligent security system built on ESP32-CAM that detects motion and door access, identifies humans vs animals using AI, and sends real-time alerts to Telegram.**


[![Arduino](https://img.shields.io/badge/Arduino-Compatible-blue)](https://www.arduino.cc/)
[![AI](https://img.shields.io/badge/AI-Powered-orange)](https://imagga.com/)
[![Telegram](https://img.shields.io/badge/Telegram-Integration-blue)](https://telegram.org/)

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [Installation](#-installation)
- [Configuration](#-configuration)
- [Usage](#-usage)
- [API Integration](#-api-integration)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [License](#-license)

## 🎯 Overview

GarudUnc is a sophisticated IoT security solution that combines motion detection, door access monitoring, and AI-powered image classification. When motion or door access is detected, the system captures an image, analyzes it using the Imagga AI service to distinguish between humans and animals, and sends contextual alerts to Telegram.

### Key Capabilities:
- **Real-time Motion Detection** with PIR sensor
- **Door/Window Access Monitoring** with magnetic sensor
- **AI-Powered Classification** (Human vs Animal detection)
- **Instant Telegram Alerts** with photos and context
- **Flash-Enhanced Imaging** for better detection accuracy
- **WiFi Connectivity** for remote monitoring

## ✨ Features

### 🔍 **Smart Detection**
- **PIR Motion Sensor**: Detects movement with 15-second cooldown
- **Door/Window Sensor**: Monitors access points with magnetic reed switch
- **Flash LED**: Automatically activates for better image quality

### 🤖 **AI Integration**
- **Imagga API**: Advanced image classification service
- **Human Detection**: Identifies people with confidence scores
- **Animal Detection**: Recognizes pets and wildlife
- **Contextual Alerts**: Provides detailed classification results

### 📱 **Remote Monitoring**
- **Telegram Bot**: Real-time photo alerts with captions
- **Status Messages**: System state and connection updates
- **Secure Communication**: HTTPS encryption for all data

### ⚡ **Performance**
- **PSRAM Support**: High-resolution image processing
- **Optimized WiFi**: Power-efficient network connectivity
- **Fast Response**: Sub-second detection and alert times

## 🔧 Hardware Requirements

### **Required Components**
| Component | Model | Purpose |
|-----------|-------|---------|
| **ESP32-CAM** | ESP32-CAM-MB | Main controller with camera |
| **PIR Sensor** | HC-SR501 | Motion detection |
| **Door Sensor** | Magnetic reed switch | Door/window monitoring |
| **Flash LED** | 5mm White LED | Image illumination |
| **Power Supply** | 5V/2A | Stable power source |

### **Optional Components**
- **PSRAM Module**: For higher resolution images
- **External Antenna**: Better WiFi range
- **Enclosure**: Weather protection

### **Pin Connections**
```
ESP32-CAM Pin Mapping:
├── PIR Sensor: GPIO 13
├── Door Sensor: GPIO 12 (with pull-up)
├── Flash LED: GPIO 4
└── Camera: Built-in OV2640
```

## 💻 Software Requirements

### **Arduino IDE Setup**
1. **Arduino IDE** (1.8.x or 2.x)
2. **ESP32 Board Package** (2.0.x or later)
3. **Required Libraries**:
   - `WiFi` (built-in)
   - `WiFiClientSecure` (built-in)
   - `HTTPClient` (built-in)
   - `ArduinoJson` (v6.x)
   - `base64` (v1.x)
   - `esp_camera` (built-in)

### **External Services**
- **WiFi Network**: 2.4GHz connection
- **Imagga API**: Image classification service
- **Telegram Bot**: Alert delivery system

## 🚀 Installation

### **1. Hardware Assembly**
```bash
# Connect components according to pin mapping
PIR Sensor → GPIO 13
Door Sensor → GPIO 12 (with 10kΩ pull-up resistor)
Flash LED → GPIO 4 (with 220Ω current limiting resistor)
```

### **2. Software Setup**
```bash
# Clone the repository
git clone https://github.com/lavsarkari/GarudUnc.git
cd GarudUnc

# Open main.ino in Arduino IDE
# Select ESP32-CAM board with PSRAM enabled
# Install required libraries via Library Manager
```

### **3. Configuration**
Edit `main.ino` and update the following variables:
```cpp
// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Telegram Bot Configuration
const char* botToken = "YOUR_BOT_TOKEN";
const char* chatID = "YOUR_CHAT_ID";

// Imagga API Configuration
String imagga_user = "YOUR_IMAGGA_USER";
String imagga_secret = "YOUR_IMAGGA_SECRET";
```

### **4. Board Configuration**
Select the appropriate camera model in `board_config.h`:
```cpp
// Uncomment your board model
#define CAMERA_MODEL_ESP_EYE  // Default
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_AI_THINKER
// ... other models
```

## ⚙️ Configuration

### **WiFi Setup**
1. **Network Requirements**:
   - 2.4GHz WiFi network
   - Stable internet connection
   - No captive portal

2. **Power Management**:
   ```cpp
   WiFi.setSleep(false); // Prevents WiFi sleep
   ```

### **Telegram Bot Setup**
1. **Create Bot**:
   - Message @BotFather on Telegram
   - Use `/newbot` command
   - Get bot token

2. **Get Chat ID**:
   - Message your bot
   - Visit: `https://api.telegram.org/bot<TOKEN>/getUpdates`
   - Extract chat_id from response

### **Imagga API Setup**
1. **Sign Up**: Visit [Imagga.com](https://imagga.com/)
2. **Get Credentials**: API key and secret
3. **Configure**: Update variables in code

### **Sensor Configuration**
```cpp
// Pin Definitions
#define PIR_PIN         13    // Motion sensor
#define FLASH_PIN       4     // Flash LED
#define DOOR_SENSOR_PIN 12    // Door sensor

// Timing Configuration
const unsigned long motionCooldown = 15000; // 15 seconds
```

## 📱 Usage

### **System Operation**
1. **Power On**: System initializes camera and connects to WiFi
2. **Standby Mode**: Continuously monitors sensors
3. **Detection**: Captures image and sends to AI service
4. **Classification**: Determines if human or animal detected
5. **Alert**: Sends photo with context to Telegram

### **Alert Types**
| Event | Message | Photo |
|-------|---------|-------|
| **Motion (Human)** | 🚶 Person Detected (85.2%) | ✅ |
| **Motion (Animal)** | 🐾 Animal Detected (92.1%) | ✅ |
| **Motion (Unknown)** | ⚠ Motion Detected (Unknown) | ✅ |
| **Door Open** | 🚪 Door/Window OPENED! | ✅ |
| **Door Close** | 🔒 Door/Window CLOSED | ❌ |

### **Debug Mode**
Enable debug output by setting:
```cpp
#define DEBUG true
```

Debug information includes:
- API responses
- Classification results
- Network status
- Error messages

## 🔌 API Integration

### **Imagga API**
- **Service**: Image classification and tagging
- **Endpoint**: `https://api.imagga.com/v2/`
- **Authentication**: Basic Auth with API credentials
- **Features**: Human/animal detection with confidence scores

### **Telegram Bot API**
- **Service**: Message and photo delivery
- **Endpoint**: `https://api.telegram.org/bot<TOKEN>/`
- **Authentication**: Bot token
- **Features**: Photo upload with captions

### **API Response Examples**
```json
// Imagga Classification Response
{
  "result": {
    "tags": [
      {
        "tag": {"en": "person"},
        "confidence": 85.2
      }
    ]
  }
}

// Telegram Alert Format
🚶 Person Detected (85.2%)
[Photo attached]
```

## 🔧 Troubleshooting

### **Common Issues**

#### **WiFi Connection Problems**
```cpp
// Check WiFi status
if (WiFi.status() != WL_CONNECTED) {
  Serial.println("WiFi disconnected");
  WiFi.reconnect();
}
```

#### **Camera Initialization Failed**
- Verify PSRAM is enabled in board settings
- Check camera pin connections
- Ensure sufficient power supply

#### **API Request Failures**
- Verify API credentials
- Check internet connectivity
- Monitor API rate limits

#### **False Detections**
- Adjust PIR sensor sensitivity
- Modify motion cooldown timing
- Improve lighting conditions

### **Debug Commands**
```cpp
// Enable serial output
Serial.begin(115200);

// Check system status
echo("System Status: " + String(WiFi.status()));
```

### **Performance Optimization**
- **Image Quality**: Adjust `jpeg_quality` (10-63)
- **Frame Size**: Modify `frame_size` for speed vs quality
- **Cooldown**: Tune `motionCooldown` for sensitivity

## 🤝 Contributing

We welcome contributions! Please follow these steps:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/AmazingFeature`)
3. **Commit** your changes (`git commit -m 'Add AmazingFeature'`)
4. **Push** to the branch (`git push origin feature/AmazingFeature`)
5. **Open** a Pull Request

### **Development Guidelines**
- Follow Arduino coding standards
- Add comments for complex logic
- Test on multiple ESP32-CAM variants
- Update documentation for new features

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Espressif Systems** for ESP32-CAM platform
- **Imagga** for AI image classification service
- **Telegram** for messaging platform
- **Arduino Community** for libraries and support

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/GarudUnc/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/GarudUnc/discussions)
- **Documentation**: [Wiki](https://github.com/yourusername/GarudUnc/wiki)

---

<div align="center">

**Made with ❤️ for smart home security**

[![GitHub stars](https://img.shields.io/github/stars/yourusername/GarudUnc?style=social)](https://github.com/lavsarkari/GarudUnc)
[![GitHub forks](https://img.shields.io/github/forks/yourusername/GarudUnc?style=social)](https://github.com/lavsarkari/GarudUnc)

</div>
