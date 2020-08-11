#include <Ethernet.h>
byte mac[] = { 0x98, 0x76, 0xB6, 0x10, 0x60, 0xa8};

#define AES
#include <ActorArray.h>

#define BEEP_PIN 14

#include "SparkFun_Si7021_Breakout_Library.h"
Weather sensor;

class MyActorArray : public ActorArray {
public:
  MyActorArray() :
    ActorArray(10018, 10019, "svdm7zq34vse.bu34", A0, IPAddress(134,91,11,186)),
    beep(false),
    temperature(0),
    humidity(0) 
    {}

  uint32_t beep;
  float temperature;
  float humidity;

protected:

  virtual void receiveData(const String& str) {
    String s;
    int start = 0;
    if (!extractDataLine(start, str, s)) return;
    beep = s.toInt();
  }

  virtual void sendInit() {
    initItem("Temperatur", "Grad C");
    initItem("Rel. Luftfeuchtigkeit", "%");
  }

  virtual void sendData() {
    sendItem(temperature);
    sendItem(humidity);
  }
} actor;


void setup() {
  pinMode(BEEP_PIN, OUTPUT);
  Ethernet.init(5);
  Ethernet.begin(mac);
  actor.setup();
  sensor.begin();
}

void tetris() {
  int t1 = 172;
  int t2 = 2*t1;
  int t3 = 3*t1;
   
  int a5 = 880;
  int b5 = 988;
  int c6 = 1047;
  int d6 = 1175;
  int e6 = 1319;
   
  tone(BEEP_PIN, e6); delay(t2);
  tone(BEEP_PIN, b5); delay(t1);
  tone(BEEP_PIN, c6); delay(t1);
  tone(BEEP_PIN, d6); delay(t2);
  tone(BEEP_PIN, c6); delay(t1);
  tone(BEEP_PIN, b5); delay(t1);
  tone(BEEP_PIN, a5); delay(t2);
  tone(BEEP_PIN, a5); delay(t1);
  tone(BEEP_PIN, c6); delay(t1);
  tone(BEEP_PIN, e6); delay(t2);
  tone(BEEP_PIN, d6); delay(t1);
  tone(BEEP_PIN, c6); delay(t1);
  tone(BEEP_PIN, b5); delay(t3);
  tone(BEEP_PIN, c6); delay(t1);
  tone(BEEP_PIN, d6); delay(t2);
  tone(BEEP_PIN, e6); delay(t2);
  tone(BEEP_PIN, c6); delay(t2);
  tone(BEEP_PIN, a5); delay(t2);
  tone(BEEP_PIN, a5); delay(t2);
  noTone(BEEP_PIN);
}

void loop() {
    actor.temperature = sensor.getTemp();;
    actor.humidity = sensor.getRH();
    actor.loop(50);

    switch (actor.beep) {
      case 1 : tone(BEEP_PIN, 3400, 100); break;
      case 2 : tetris();
    }
    delay(100);
}
