#include <WiFi.h>
#include <Led4digit74HC595.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_sntp.h"
#include "lwip/apps/sntp.h"
#include "driver/gpio.h"

const char* ssid = "Boon Labs";
const char* password = "Swajal@123";
Led4digit74HC595 myLedDisplay(26, 5, 35);  // Pins:(SCLK, RCLK, DIO)
unsigned long previousMillis = 0;
const long interval = 60000;  // Interval for updating time in milliseconds (1 minute)
unsigned long previousMillis1 = 0;
const long interval1 = 1000;  // Interval for toggling dot in milliseconds (1 second)
#define NTP_SERVER "time.google.com"

// Function prototypes
static bool obtain_time();
static void initialize_sntp();
static void time_sync_notification_cb(struct timeval* tv);
void get_internal_timestamp(char** out_val);

bool dot_state = false;

void setup() {
  Serial.begin(115200);
  delay(100);
  myLedDisplay.setDecimalPoint(0);

  // Connect to Wi-Fi
  Serial.println();
  Serial.println("Connecting to " + String(ssid));
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wi-Fi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());
  obtain_time();  // Call obtain_time() when WiFi is connected
}

void loop() {
  myLedDisplay.loopShow();  // Automatic multiplexing

  unsigned long currentMillis = millis();

  // Check if one minute has passed since the last update
  if (currentMillis - previousMillis >= interval) {
    // Update the time and reset the timer
    char* rtc_timestamp;
    get_internal_timestamp(&rtc_timestamp);
    free(rtc_timestamp);

    previousMillis = currentMillis;
  }

  unsigned long currentMillis1 = millis();
  // Check if one second has passed since the last toggle
  if (currentMillis1 - previousMillis1 >= interval1) {
    // Toggle the dot
    dot_state = !dot_state;

    // Set decimal point between hour and minute digits
    if (dot_state) {
      myLedDisplay.setDecimalPoint(3);
    } else {
      myLedDisplay.setDecimalPoint(0);
    }

    // Update previousMillis to currentMillis
    previousMillis1 = currentMillis1;
  }
}

void display_time(struct tm timeinfo) {
  int hours = timeinfo.tm_hour;
  int minutes = timeinfo.tm_min;

  // Convert hours to 12-hour format
  bool is_pm = hours >= 12;
  if (hours > 12) {
    hours -= 12;
  } else if (hours == 0) {
    hours = 12;
  }

  // Construct the display number for LED
  int display_number = hours * 100 + minutes;

  // Toggle the state of the dot every second
  dot_state = !dot_state;

  // Set decimal point between hour and minute digits
  if (dot_state) {
    myLedDisplay.setDecimalPoint(3);
  } else {
    myLedDisplay.setDecimalPoint(0);
  }

  // Add decimal point to the LED display
  myLedDisplay.setNumber(display_number);

  // Sleep for a short period to allow the display to update
  delay(100);
}

// Definition of obtain_time() function
static bool obtain_time() {
  initialize_sntp();  // Initializing SNTP server
  // Wait for time to be set
  int retry = 0;
  const int retry_count = 10;
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
    Serial.printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
    delay(1000);  // Delay of 1 second
  }

  if (retry == retry_count)
    return false;

  // Fetch and store the current time
  time_t now;
  time(&now);
  now = now + (5.5 * 60 * 60);  // Adjust timezone
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  Serial.printf("Current time: %s", asctime(&timeinfo));
  return true;
}

// Definition of initialize_sntp() function
static void initialize_sntp(void) {
  Serial.println("Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  Serial.printf("Your NTP Server is %s\n", NTP_SERVER);
  sntp_setservername(0, NTP_SERVER);
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  sntp_init();
}

// Definition of time_sync_notification_cb() function
void time_sync_notification_cb(struct timeval* tv) {
  Serial.println("Notification of a time synchronization event");

  // Call your update function here
  char* rtc_timestamp;
  get_internal_timestamp(&rtc_timestamp);
  free(rtc_timestamp);
}

void get_internal_timestamp(char** out_val) {
  time_t now;
  struct tm timeinfo = { 0 };  // To store the date/time elements
  ESP_LOGI(RTC, "Fetching date/time from internal RTC...");

  // Obtain current time in UTC
  time(&now);
  now = now + (5.5 * 60 * 60);  // Adjusted for IST (+05:30 hours)
  localtime_r(&now, &timeinfo);

  // Format timestamp string
  size_t ts_size = snprintf(NULL, 0, "%04d-%02d-%02dT%02d:%02d:%02d+05:30",
                            timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // Allocate memory for timestamp string
  *out_val = (char*)malloc(ts_size + 1);

  if (*out_val == NULL) {
    ESP_LOGE("RTC", "Memory Allocation failed!");
    *out_val = strdup("1970-01-01T00:00:00+05:30");  // Set default value
    return;
  }

  // Write timestamp string to allocated memory
  snprintf(*out_val, ts_size + 1, "%04d-%02d-%02dT%02d:%02d:%02d+05:30",
           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  ESP_LOGI("RTC", "Got internal timestamp ---> %s", *out_val);

  Serial.printf("Current time: %s", asctime(&timeinfo));
  display_time(timeinfo);  // Display time on LED display
}
