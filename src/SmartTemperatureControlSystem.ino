#include <WiFi.h>
#include <DHT.h>

// ===================== CONFIG =====================
#define DHTPIN 4          // Chân nối cảm biến DHT
#define DHTTYPE DHT11     // Hoặc DHT11 nếu bạn dùng DHT11
#define RELAY_PIN 5       // Relay điều khiển quạt
#define TEMP_THRESHOLD 30 // Ngưỡng nhiệt độ bật quạt (°C)

// WiFi config
const char* ssid = "Wifi_2.4G";
const char* password = "66668888";

// ===================================================
DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);

float temperature;
float humidity;
bool fanState = false; // Trạng thái quạt

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Quạt tắt ban đầu

  // Kết nối WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client Connected.");
    String request = client.readStringUntil('\r');
    client.flush();

    // Đọc sensor
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    // Kiểm tra lỗi sensor
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Điều khiển quạt theo nhiệt độ
    if (temperature >= TEMP_THRESHOLD) {
      fanState = true;
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      fanState = false;
      digitalWrite(RELAY_PIN, LOW);
    }

    // Tạo giao diện web
    String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5'/>"
                  "<title>ESP32 DHT WebServer</title>"
                  "<style>body{font-family:Arial;text-align:center;}</style></head><body>";
    html += "<h2>🌡️ ESP32 DHT Sensor Dashboard</h2>";
    html += "<p><b>Nhiệt độ:</b> " + String(temperature) + " °C</p>";
    html += "<p><b>Độ ẩm:</b> " + String(humidity) + " %</p>";
    html += "<p><b>Trạng thái quạt:</b> " + String(fanState ? "BẬT 🌀" : "TẮT ❌") + "</p>";
    html += "<p><i>Tự động làm mới mỗi 5s</i></p>";
    html += "</body></html>";

    // Gửi dữ liệu về client
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.println(html);
    client.stop();
    Serial.println("Client disconnected.");
  }
}
