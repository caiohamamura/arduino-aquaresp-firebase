/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
#include <defines.h>
#include "processMessage.h"

// Define Firebase Data object
FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

const char *FIREBASE_PATH = "/6tPZnchirsOK1gcWzMFuUzCWdjf1/message";

#include <NTPClient.h>
#include <WiFiUdp.h> //Socket UDP
const int TIME_ZONE = -3;
const char *NTP_SERVER_URL = "pool.ntp.br";
const int NTP_INTERVAL = 60000;
WiFiUDP udp;
NTPClient ntpClient(udp, NTP_SERVER_URL, TIME_ZONE * 3600, NTP_INTERVAL);

void streamCallback(FirebaseStream data)
{
  String msgString = data.stringData();
  const char *msg = msgString.c_str();
  Serial.print("Received message => ");
  Serial.println(msgString);
  processMessage(strdup(msg));
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup()
{
  Serial.begin(74880);
  analogWriteFreq(21000);
  analogWriteRange(1023);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

// Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif

  if (!Firebase.RTDB.beginStream(&stream, FIREBASE_PATH))
    Serial.printf("Stream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  ntpClient.begin();

  Serial.println("Waiting for first update");
  while (!ntpClient.update())
  {
    Serial.print(".");
    ntpClient.forceUpdate();
    delay(500);
  }

  Serial.println();
  Serial.println("First Update Complete");
}

void loop()
{
  if (autoLED)
  {
    int value = ledPwm;
    if (ntpClient.getHours() > hourBegin && ntpClient.getHours() < (hourEnd - 1))
    {
      value = 0;
    }
    else if (ntpClient.getHours() == hourBegin)
    {
      value = (int)round(1023 - (1023 * ((ntpClient.getMinutes() * 60) + ntpClient.getSeconds()) / 3600.0));
    }
    else if (ntpClient.getHours() == (hourEnd - 1))
    {
      value = (int)round(1023 * ((ntpClient.getMinutes() * 60) + ntpClient.getSeconds()) / 3600.0);
    }
    else
    {
      value = 1023;
    }
    if (ledPwm != value)
    {
      Serial.printf("Changing value from: %d => %d\n", ledPwm, value);
      ledPwm = value;
      analogWrite(LED_PIN, ledPwm);
    }
  }

  Firebase.ready();
  delay(1);
}