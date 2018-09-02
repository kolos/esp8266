 /* ESP8266 Mosquitto TLS stuff
 * 
 * From:
 *  https://github.com/sgjava/nodemcu-mqtt-tls
 *  https://github.com/HarringayMakerSpace/awsiot/blob/master/Esp8266AWSIoTExample/Esp8266AWSIoTExample.ino
 *  https://github.com/knolleary/pubsubclient/issues/84#issuecomment-350549450
 */

bool loadcerts() {
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return false;
  }

  /** Client cert file (.crt.der) **/
  File cert = SPIFFS.open(CLIENT_CRT_DER_PATH, "r");
  if (!cert) {
    Serial.println("Failed to open cert file");
    return false;
  }
  Serial.print("Success to open cert file.. ");
  if (!wiFiClient.loadCertificate(cert)) {
    Serial.println("cert not loaded!");
    return false;
  }
  Serial.println("cert loaded!");

  /** Client cert key file (.key.der) **/
  File private_key = SPIFFS.open(CLIENT_KEY_DER_PATH, "r");
  if (!private_key) {
    Serial.println("Failed to open private cert file");
    return false;
  }
  Serial.print("Success to open private cert file.. ");
  if (!wiFiClient.loadPrivateKey(private_key)) {
   Serial.println("private key not loaded!");
  }
  Serial.println("private key loaded!");
   

  /** Server CA (ca.der) **/
  File ca = SPIFFS.open(SERVER_CA_PATH, "r");
  if (!ca) {
   Serial.println("Failed to open CA");
   return false;
  }
  Serial.print("Success to open CA.. ");
  if(!wiFiClient.loadCACert(ca)) {
    Serial.println("CA failed!");
    return false;
  }
  Serial.println("CA loaded!");

  return true;
}
