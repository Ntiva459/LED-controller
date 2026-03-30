#include <HomeSpan.h>
#include <WiFi.h>

// === Wi-Fi credentials ===
const char* ssid = "DIGI_458f88";
const char* password = "5b2eba70";

// === RGB MOSFET pins ===
#define PIN_R 25
#define PIN_G 26
#define PIN_B 27

// Function to set the RGB LEDs
void setRGB(int r, int g, int b) {
  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

// RGB LED LightBulb service
struct RGBLED : Service::LightBulb {
  SpanCharacteristic *power;
  SpanCharacteristic *hue;
  SpanCharacteristic *saturation;
  SpanCharacteristic *brightness;

  RGBLED() {
    pinMode(PIN_R, OUTPUT);
    pinMode(PIN_G, OUTPUT);
    pinMode(PIN_B, OUTPUT);

    power = new Characteristic::On(false);          // ON/OFF
    hue = new Characteristic::Hue(0);              // 0–360°
    saturation = new Characteristic::Saturation(0); // 0–100%
    brightness = new Characteristic::Brightness(100); // 0–100%
  }

  boolean update() override {
    if (!power->getNewVal()) {
      setRGB(0, 0, 0); // Turn off LEDs
      return true;
    }

    // Convert HSV -> RGB (0–255)
    float h = hue->getNewVal();
    float s = saturation->getNewVal() / 100.0;
    float v = brightness->getNewVal() / 100.0;

    float r, g, b;
    int i = int(h / 60.0) % 6;
    float f = (h / 60.0) - i;
    float p = v * (1.0 - s);
    float q = v * (1.0 - f * s);
    float t = v * (1.0 - (1.0 - f) * s);

    switch (i) {
      case 0: r = v; g = t; b = p; break;
      case 1: r = q; g = v; b = p; break;
      case 2: r = p; g = v; b = t; break;
      case 3: r = p; g = q; b = v; break;
      case 4: r = t; g = p; b = v; break;
      case 5: r = v; g = p; b = q; break;
    }

    setRGB(int(r * 255), int(g * 255), int(b * 255));
    return true;
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected! IP address:");
  Serial.println(WiFi.localIP());

  homeSpan.begin(Category::Lighting);

  // Accessory info
  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Name("RGB LED Strip");
  new Characteristic::Manufacturer("Tivadar");
  new Characteristic::Model("ESP32 Dev Kit");
  new Characteristic::SerialNumber("003");

  // Add RGB LightBulb service
  new RGBLED();

  Serial.println("HomeKit RGB LightBulb ready! Scan QR in Serial Monitor.");
}

void loop() {
  homeSpan.poll();
}
