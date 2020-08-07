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

  bool beep;
  float temperature;
  float humidity;

protected:

  virtual void receiveData(const String& str) {
    String s;
    int start = 0;
    if (!extractDataLine(start, str, s)) return;
    beep = s.toInt() == 0 ? false : true;     
  }

  virtual void sendInit() {
    initItem("Temperatur", "Grad C");
    initItem("Luftfeuchtgkeit", "%");    
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

void loop() {
    actor.temperature = sensor.getTemp();;
    actor.humidity = sensor.getRH();
    actor.loop(50);

    if (actor.beep) {
      tone(BEEP_PIN, 3400, 1000);
    }
    delay(100);
}
