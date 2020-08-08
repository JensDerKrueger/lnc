#ifndef ActorArrayTemplate_h
#define ActorArrayTemplate_h

// check if we've included some ethernet library
// before (most likely ethernet2.h) for the new
// arduino.org shield, if yes don't inlcude the
// standart ethernet.h library
#ifndef ethernet_h
  #include <Ethernet.h>
#endif  // ethernet_h

#include "Arduino.h"

#ifdef AES
#include "AES.h"
#endif  // AES


template <class T>
class ActorArrayTemplate
{
public:
#ifdef ADAFRUIT_SLEEPYDOG_H
#ifdef AES
  ActorArrayTemplate(short sendPort,
#ifndef PURE_SENSOR
                     short receivePort,
#endif  // PURE_SENSOR
                     const String& password,
                     uint8_t randomPin, IPAddress server = IPAddress(192,168,0,100));
#else  // AES
  ActorArrayTemplate(short sendPort,
#ifndef PURE_SENSOR
                     short receivePort,
#endif  // PURE_SENSOR
                     const String& password,
                     IPAddress server = IPAddress(192,168,0,100));
#endif // AES
  
  virtual ~ActorArrayTemplate() {
    Watchdog.disable();
#ifdef AES
    delete m_aesSend;
#ifndef PURE_SENSOR
    delete m_aesReceive;
#endif // PURE_SENSOR
#endif // AES
  }

  int watchdogTimeout() {
    return m_watchdogTimeout;
  }
  
#else // ADAFRUIT_SLEEPYDOG_H
#ifdef AES
  ActorArrayTemplate(short sendPort,
#ifndef PURE_SENSOR
                     short receivePort,
#endif  // PURE_SENSOR
                     const String& password,
                     uint8_t randomPin, IPAddress server = IPAddress(192,168,0,100),
                     uint8_t watchdogID=0);

  virtual ~ActorArrayTemplate() {
    delete m_aesSend;
#ifndef PURE_SENSOR
    delete m_aesReceive;
#endif  // PURE_SENSOR
  }
  
#else  // AES
  ActorArrayTemplate(short sendPort,
#ifndef PURE_SENSOR
                     short receivePort,
#endif  // PURE_SENSOR
                     IPAddress server = IPAddress(192,168,0,100),
                     uint8_t watchdogID=0);
#endif // AES
#endif // ADAFRUIT_SLEEPYDOG_H
  
  virtual void setup();
  virtual void loop(uint32_t iSkipFrames=0, uint32_t iSkipTestFrames=0);

  bool IsConnected() const {return bWasSendConnected;}
  void kickWatchdog();
  
private:
#ifdef AES
  uint8_t m_randomPin;
  String m_password;
  String m_sendBuffer;
  AESCrypt* m_aesSend;
#ifndef PURE_SENSOR
  AESCrypt* m_aesReceive;
#endif  // PURE_SENSOR
#endif  // AES
  
  short sendPort;
  bool bWasSendConnected;
  
#ifndef PURE_SENSOR
  String m_receiveBuffer;
  short receivePort;
  bool bWasReceiveConnected;

  String removeHeaderAndFooter(const String& str) const;
  void receiveChar(char data, bool init);
  void receiveDataAndHeader(bool init);
#endif  // PURE_SENSOR
  
  void sendDataAndHeader();
  
  void checkConnections();
  void connectSender();
  
#ifndef PURE_SENSOR
  void connectReceiver();
  void cleanDisconnectReceiver();
#endif  // PURE_SENSOR
  
  void cleanDisconnectSender();
  
protected:
  T sendClient;
  IPAddress server;
  
#ifdef ADAFRUIT_SLEEPYDOG_H
  int m_watchdogTimeout;
#else // ADAFRUIT_SLEEPYDOG_H
  uint8_t watchdogID;
#endif  // ADAFRUIT_SLEEPYDOG_H

  virtual void sendData() = 0;
  virtual void sendInit() = 0;

  virtual bool connectSenderCall(const IPAddress& ip, short port);


  void initItem(const String& name, const String& unit);
  template <typename U> void sendItem(U value);
  
  virtual void connectNotification(int i, int counter) {}

#ifndef PURE_SENSOR
  unsigned int receiveElemCount;
  T receiveClient;
  bool extractDataLine(int& start, const String& input, String& output);
  virtual void receiveInit(const String& str);
  virtual void receiveData(const String& str) = 0;
#endif // PURE_SENSOR
  
};

#ifdef AES
static const String testHeader ="MESSAGE_OK";
#endif // AES
static const char dataBeginSend[] ="DATA BEGIN";
static const char dataEndSend[] ="DATA END";

static const char dataBegin[] = "DATA BEGIN\n"; // length 11
static const char dataEnd[] = "DATA END\n"; // length 9

template <class T>
void ActorArrayTemplate<T>::initItem(const String& name, const String& unit) {
#ifdef AES
  m_sendBuffer += name+String(", ")+unit+String("\n");
#else // AES
  sendClient.println(name+String(", ")+unit);
#endif // AES
}

template <class T> template <typename U>
void ActorArrayTemplate<T>::sendItem(U value) {
#ifdef AES
  m_sendBuffer += value+String("\n");
#else // AES
  sendClient.println(value);
#endif // AES
  
}


#ifdef ADAFRUIT_SLEEPYDOG_H
#ifdef AES
template <class T>
ActorArrayTemplate<T>::ActorArrayTemplate(short sp,
#ifndef PURE_SENSOR
                                          short rp,
#endif // PURE_SENSOR
                                          const String& password,
                                          uint8_t randomPin,
                                          IPAddress s) :
m_randomPin(randomPin),
m_password(password),
m_sendBuffer(""),
m_aesSend(0),
#ifndef PURE_SENSOR
m_aesReceive(0),
m_receiveBuffer(""),
receivePort(rp),
bWasReceiveConnected(false),
#endif // PURE_SENSOR
sendPort(sp),
bWasSendConnected(false),
server(s),
m_watchdogTimeout(0)
#ifndef PURE_SENSOR
,receiveElemCount(0)
#endif // PURE_SENSOR
{
}
#else // AES
template <class T>
ActorArrayTemplate<T>::ActorArrayTemplate(short sp, short rp, IPAddress s) :
sendPort(sp),
#ifndef PURE_SENSOR
m_receiveBuffer(""),
receivePort(rp),
bWasReceiveConnected(false),
#endif // PURE_SENSOR
bWasSendConnected(false),
server(s),
m_watchdogTimeout(0)
#ifndef PURE_SENSOR
,receiveElemCount(0)
#endif // PURE_SENSOR
{
}
#endif // AES
#else  // ADAFRUIT_SLEEPYDOG_H
#ifdef AES
template <class T>
ActorArrayTemplate<T>::ActorArrayTemplate(short sp,
#ifndef PURE_SENSOR
                                          short rp,
#endif // PURE_SENSOR
                                          const String& password,
                                          uint8_t randomPin,
                                          IPAddress s,
                                          uint8_t wdID) :
m_randomPin(randomPin),
m_password(password),
m_sendBuffer(""),
m_aesSend(0),
#ifndef PURE_SENSOR
m_aesReceive(0),
#endif // PURE_SENSOR
sendPort(sp),
bWasSendConnected(false),
#ifndef PURE_SENSOR
m_receiveBuffer(""),
receivePort(rp),
bWasReceiveConnected(false),
#endif // PURE_SENSOR
server(s),
watchdogID(wdID)
#ifndef PURE_SENSOR
,receiveElemCount(0)
#endif // PURE_SENSOR
{
  if (watchdogID != 0)
    pinMode(watchdogID, OUTPUT);
}
#else /// ADAFRUIT_SLEEPYDOG_H

template <class T>
ActorArrayTemplate<T>::ActorArrayTemplate(short sp,
#ifndef PURE_SENSOR
                                          short rp,
#endif // PURE_SENSOR
                                          IPAddress s,
                                          uint8_t wdID) :
sendPort(sp),
bWasSendConnected(false),
#ifndef PURE_SENSOR
m_receiveBuffer(""),
receivePort(rp),
bWasReceiveConnected(false),
#endif  // PURE_SENSOR
server(s),
watchdogID(wdID)
#ifndef PURE_SENSOR
,receiveElemCount(0)
#endif  // PURE_SENSOR
{
  if (watchdogID != 0)
    pinMode(watchdogID, OUTPUT);
}
#endif  // AES
#endif  // ADAFRUIT_SLEEPYDOG_H

#ifndef PURE_SENSOR
template <class T>
String ActorArrayTemplate<T>::removeHeaderAndFooter(const String& str) const {
  if (str.length() >= strlen(dataBegin)+strlen(dataEnd))
    return str.substring(strlen(dataBegin), str.length() - strlen(dataEnd) );
  else
    return "";
}

template <class T>
void ActorArrayTemplate<T>::receiveChar(char data, bool init) {
  if (data == '\r') return; // ignore windows line feeds
  
  m_receiveBuffer += data;
  
  if (m_receiveBuffer.endsWith(dataBegin)) {
    // set receive buffer to dataBegin, possibly removing any leftover
    // data before that string
    m_receiveBuffer = dataBegin;
    return;
  }
  
  // terminal string found?
  if (m_receiveBuffer.endsWith(dataEnd) ) {
    String message = removeHeaderAndFooter(m_receiveBuffer);
    
#ifdef AES
    if (init) {
      size_t pos = message.indexOf(";");
      String strIV = message.substring(0,pos);
      
      SimpleVec iv;
      base64_decode(strIV, iv);
      
      message = message.substring(pos+1);
        
      delete m_aesReceive;
      m_aesReceive =  new AESCrypt(iv.constData(), m_password);
    }

    if (message[message.length()-1] == '\n')
      message = message.substring(0, message.length()-1);
        
    message = m_aesReceive->decryptString(message);
    
    int pos = message.indexOf(testHeader);
    if (pos==-1) {
      return;
    }
    
    if (pos+testHeader.length() < message.length()) {
      message = message.substring(pos+testHeader.length()) + "\n";
    }
#endif  // AES
    
    if (init) {
      kickWatchdog();
      receiveInit(message);
    } else {
      kickWatchdog();
      receiveData(message);
    }
    m_receiveBuffer = "";
  }
  
  // sanity check
  if (m_receiveBuffer.length() > 5000) {
    m_receiveBuffer = "";
  }
  
}

template <class T>
void ActorArrayTemplate<T>::receiveDataAndHeader(bool init) {
  while (receiveClient.connected() && receiveClient.available()) {
    char c = receiveClient.read();
    receiveChar(c, init);
    if (init && receiveElemCount > 0) return;
  }
}

template <class T>
void ActorArrayTemplate<T>::connectReceiver() {
  if (receivePort == 0) return;
  
  int i = 0, j = 0;
  while (!receiveClient.connected() || receiveElemCount == 0) {
    connectNotification(2, i++);
    
    if (!receiveClient.connected()) {
      if (receiveClient.connect(server, receivePort)) {
        bWasReceiveConnected = true;
        receiveDataAndHeader(true);
      } else {
        j++;
        delay(1000);
        if (j > 10) {  // timeout after 10 seconds
          sendClient.stop();
          receiveClient.stop();
          return; // give up and start from scratch
        }
      }
      connectNotification(3, 0);
    } else {
      receiveDataAndHeader(true);
      j++;
      delay(1000);
      if (j > 10) {
        sendClient.stop();
        receiveClient.stop();
        return; // give up and start from scratch
      }
    }
  }
}

template <class T>
void ActorArrayTemplate<T>::cleanDisconnectReceiver() {
  if (bWasReceiveConnected) {
    receiveClient.stop();
    bWasReceiveConnected = false;
    receiveElemCount = 0;
  }
}

template <class T>
bool ActorArrayTemplate<T>::extractDataLine(int& start, const String& input, String& output) {
  int end = input.indexOf("\n",start);
  if (end < 0) return false;
  output = input.substring(start,end);
  start = end+1;
  return true;
}

template <class T>
void ActorArrayTemplate<T>::receiveInit(const String& str) {
  int start = 0;
  String s;
  receiveElemCount = 0;
  
  while (extractDataLine(start, str, s) && receiveClient.connected()) {
    receiveElemCount++;
  }
}
#endif // PURE_SENSOR

template <class T>
bool ActorArrayTemplate<T>::connectSenderCall(const IPAddress& ip, short port) {
  return sendClient.connect(ip, port);
}

template <class T>
void ActorArrayTemplate<T>::connectSender() {
  int i = 0;
  while (!sendClient.connected()) {
    connectNotification(1, i++);
    
    if (connectSenderCall(server, sendPort)) {
      bWasSendConnected = true;
      sendClient.println(dataBeginSend);
#ifdef AES
      byte iv[16]; // TEST: = {64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79};
      AESCrypt::genIV(iv, m_randomPin);
      
      delete m_aesSend;
      m_aesSend = new AESCrypt(iv, m_password);
      
      m_sendBuffer = testHeader;
      sendInit();
      
      sendClient.print(base64_encode(iv, 16));
      sendClient.print(";");
      String encString = m_aesSend->encryptString(m_sendBuffer);
      sendClient.println(encString);
      
#else  // AES
      sendInit();
#endif  // AES
      sendClient.println(dataEndSend);
      sendClient.flush();
    } else {
      delay(1000);
    }
  }
}

template <class T>
void ActorArrayTemplate<T>::cleanDisconnectSender() {
  if (bWasSendConnected) {
    sendClient.stop();
    bWasSendConnected = false;
  }
}

template <class T>
void ActorArrayTemplate<T>::checkConnections() {
#ifndef PURE_SENSOR
  if (receivePort == 0) {
#endif  // PURE_SENSOR
    if (!sendClient.connected()) {
      cleanDisconnectSender();
    }
    connectSender();
#ifndef PURE_SENSOR  
  } else {
    if (!receiveClient.connected() || !sendClient.connected()) {
      cleanDisconnectReceiver();
      cleanDisconnectSender();
    }
    connectReceiver();
    connectSender();
  }
#endif  // PURE_SENSOR
}

template <class T>
void ActorArrayTemplate<T>::sendDataAndHeader() {
#ifndef PURE_SENSOR
  if (receivePort == 0 && sendClient.connected())
#endif  // PURE_SENSOR
  kickWatchdog();
  
  sendClient.println(dataBeginSend);
  
#ifdef AES
  m_sendBuffer = testHeader;
  sendData();
  m_sendBuffer = m_aesSend->encryptString(m_sendBuffer);
  sendClient.println(m_sendBuffer);
#else // AES
  sendData();
#endif // AES
  
  sendClient.println(dataEndSend);
  sendClient.flush();
}

template <class T>
void ActorArrayTemplate<T>::setup() {
#ifdef ADAFRUIT_SLEEPYDOG_H
  m_watchdogTimeout = Watchdog.enable();
#endif // ADAFRUIT_SLEEPYDOG_H
}

template <class T>
void ActorArrayTemplate<T>::loop(uint32_t iSkipFrames, uint32_t iSkipTestFrames) {
  static uint32_t j = 0;
  if (j >= iSkipTestFrames) {
    checkConnections();
    j = 0;
  } else {
    j++;
  }
  
  static uint32_t i = 0;
  if (i >= iSkipFrames) {
    sendDataAndHeader();
    i = 0;
  } else {
    i++;
  }
#ifndef PURE_SENSOR
  receiveDataAndHeader(false);
#endif  // PURE_SENSOR
}


template <class T>
void ActorArrayTemplate<T>::kickWatchdog() {
#ifdef ADAFRUIT_SLEEPYDOG_H
  Watchdog.reset();
#else  // ADAFRUIT_SLEEPYDOG_H
  if (watchdogID) {
    digitalWrite(watchdogID, HIGH);
    delay(20);
    digitalWrite(watchdogID, LOW);
  }
#endif  // ADAFRUIT_SLEEPYDOG_H
}


typedef ActorArrayTemplate<EthernetClient> ActorArray;


#endif // ActorArrayTemplate_h
