/*
    Dit is het "feleciteer Erwin voor zijn 50e verjaardag" project!
    Geschreven door Dennis en Francien op 18 maart 2017 (ja, we waren RUIM op tijd klaar ;-)!)

*/
// ------------------Francien: ---------------------------
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is <<----- Add your address here.  Find it from I2C Scanner. --> gezocht: is 0x27
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

const int ledSnoerPin = 5; // pin voor ledsnoer

int state = 0; // gebruikt om scherm van high op low te zetten
int n = 1;
int teller = 0; // teller voor de message loop
String wens;  // wens = de boodschap die via LoRa binnenkomt.
String wensRegel1;
String wensRegel2;
int wensLengte; // // aantal leestekens in de wens
boolean extraScroll; // extra scrollen

LiquidCrystal_I2C  lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

// ------------------Dennis: -----------------------------
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include "keys.h"

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static uint8_t mydata[] = "?";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 9,
  .dio = {6, 7, 8},
};

// hebben we een bericht terug van lora?
boolean berichtVanLora;
String voorDisplay = "";

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.println(F("Received "));
        Serial.println(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
        berichtVanLora = true; // bericht terug van lora - doorgeven aan het display
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      break;
  }
}

void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    Serial.println(F("Packet queued"));
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
  // ------------------setup Dennis: -----------------------------
  Serial.begin(115200);
  Serial.println(F("Starting"));

#ifdef VCC_ENABLE
  // For Pinoccio Scout boards
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(1000);
#endif

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
#ifdef PROGMEM
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
#else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif

#if defined(CFG_eu868)
  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  // NA-US channels 0-71 are configured automatically
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.
#elif defined(CFG_us915)
  // NA-US channels 0-71 are configured automatically
  // but only one group of 8 should (a subband) should be active
  // TTN recommends the second sub band, 1 in a zero based count.
  // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
  LMIC_selectSubBand(1);
#endif

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  //  LMIC.dn2Dr = DR_SF12;
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF9, 14);

  // Start job
  do_send(&sendjob);

  // nog geen bericht terug van lora
  berichtVanLora = false;

  // ------------------setup Francien: ---------------------------
  lcd.begin (16, 2);     //  mijn LCD = 16 tekens breed, 2 regels hoog
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.noCursor ();      // cursor uit
  lcd.setBacklight(HIGH); // backlight aan

  pinMode(ledSnoerPin, OUTPUT);
  digitalWrite(ledSnoerPin, HIGH);

  kondigBerichtAan();
  toonBericht("Hoera Erwin is                   11001 ");
  delay(1000);
  lcd.clear();
  lcd.home ();

  /*
    kondigBerichtAan();             // testcode, mag weg
    toonBericht("Test in setup!");  // testcode, mag weg
    delay(1000);                    // testcode, mag weg
    lcd.clear();                    // testcode, mag weg

    voorDisplay = "SETUP: ja wat is dat nou voor test! "; //  testcode, MOET weg
    toonBericht(voorDisplay); // testcode, MOET weg
    delay(1000); // testcode, mag weg
  */

  wens = ""; // leeg maken
  wensRegel1 = "";
  wensRegel2 = "";
  extraScroll = false;
}

void loop() {
  // ------------------loop Dennis: -------------------------------
  os_runloop_once(); // doe alle lora stuff +
  moetErEenBerichtNaarHetDisplay(); // zet nieuwe wens in de variabele "voorDisplay" (mits er nieuw bericht is)
  // ------------------loop Dennis/ Francien: -------------------
  if (extraScroll) {
    scroll();
    extraScroll = false;
  }

  // Serial.print("voorDisplay BUITEN de loop zegt: ");  // testcode, mag weg
  // Serial.println(voorDisplay);  // testcode, mag weg
  //  Serial.print("berichtVanLora zegt: ");  // testcode, mag weg
  //  Serial.println(berichtVanLora);  // testcode, mag weg
  // voorDisplay = "ja wat is dat nou voor test! ";
  // toonBericht(voorDisplay); // testcode, MOET weg
  //delay(20000);

  if (berichtVanLora) {   // als lora bericht binnenkomt
    Serial.println("voorDisplay IN de LOOP zegt: ");  // testcode, mag weg
    Serial.println(voorDisplay);  // testcode, mag weg
    kondigBerichtAan();   // kondig aan (+ knipper backlight zodat je t binnen ziet komen)
    toonBericht(voorDisplay); // toon bericht in display
    berichtVanLora = false;
    extraScroll = true;
  }

}

void moetErEenBerichtNaarHetDisplay() {
  if (berichtVanLora == true && LMIC.dataLen) {
    voorDisplay = "";
    char bericht[255];

    int i;
    for (i = 0 ; i < 255; i++) {
      bericht[i] = 0;
    }
    for (i = 0; i < LMIC.dataLen && i < 254; i++) {
      Serial.print((char)LMIC.frame[LMIC.dataBeg + i]);
      bericht[i] = (char)LMIC.frame[LMIC.dataBeg + i];
    }
    // regeleinde achter de char array
    if (LMIC.dataLen >= 254) {
      bericht[254] = 0;
    } else {
      bericht[LMIC.dataLen] = 0;
    }
    // omzetten naar string
    String berichtString = "";
    for (i = 0; i < LMIC.dataLen && i < 254; i++) {
      berichtString += bericht[i];
    }
    voorDisplay = berichtString; // voorDisplay = de string die francien verder gebruikt. In deze regel zet dennis het nieuwe bericht in "mijn" string
    LMIC.dataLen = 0;
  }
  //  displayNuBenJij(); // hier: gaat francien haar method aanroepen
}

// ------------(losse methods)------Francien: ---------------------------
void kondigBerichtAan() {  // laat zien (op scherm + door knipperen) dat er nieuw bericht aankomt
  lcd.clear();                      // maak scherm leeg
  lcd.home ();                      // cursor naar begin
  lcd.print("Nieuw LoRa be- ");
  lcd.setCursor ( 0, 1 );
  lcd.print("richt  ");
  for (int i = 1; i < 51; i++) {    // 50 x want Erwin is 50 --->>> NU LAGER OM TE TESTEN< TERUGZETTEN NAAR 50!
    if (i % 5 == 0) {
      state = ! state;                // als 0, dan 1, als 1 dan 0
      lcd.setBacklight(state);        // Backlight aan/ uit (toggle tussen state)
      digitalWrite(ledSnoerPin, state); // laat LEDsnoer mee knipperen, zodra er bericht binnenkomt
    }
    delay(50); // 50 ms want 50 jaar duurt wel heel lang ;-)
    lcd.setCursor ( 9, 1 );
    lcd.print(i);
  }
  lcd.setBacklight(HIGH);              // eindig altijd met display AAN
  digitalWrite(ledSnoerPin, HIGH);
  delay(10 * 50);                   // zodat je kunt zien dat ie tot 50 telt
}

void toonBericht(String wensInvoer) { // toon wens op LCD scherm
  wens = wensInvoer;
  knip();  // zet de wens in 2 variabelen van elk 1 regel
  lcd.clear();
  lcd.home ();
  lcd.setCursor(0, 0);
  lcd.print(wensRegel1); // eerste regel
  lcd.setCursor ( 0, 1 );
  lcd.print(wensRegel2); // tweede regel
  delay(500);

  if (wens.length() > 16) {   // alleen scrollen als tekst niet in scherm past
    scroll();
  }
}

void scroll() {
  delay(3 * 50);
  for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(3 * 50);
  }
  delay(20 * 50);
  for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
    // scroll one position right:
    lcd.scrollDisplayRight();
    // wait a bit:
    delay(3 * 50);
  }
  delay(3 * 50);
}

void knip() {
  wensRegel1 = "";
  wensRegel2 = "";
  wensLengte = wens.length();
  if (wensLengte <= 32) {
    wensRegel1 = wens.substring(0, wensLengte); // wensLengte is het einde van de substring
  } else if (wensLengte <= 64) {
    wensRegel1 = wens.substring(0, 32);
    wensRegel2 = wens.substring(32, wensLengte);
  } else {
    wensRegel1 = wens.substring(0, 32);
    wensRegel2 = wens.substring(32, 64);
  }
}

