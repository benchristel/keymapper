/*
 * Convert a standard QWERTY keyboard to any keyboard layout you want!
 *
 * You will need:
 * - An Arduino Leonardo
 * - A USB Host Shield v2.0, available from Circuits@Home
 * - A MicroUSB cable to connect the Leonardo to your computer
 * - A USB keyboard
 *
 * Usage:
 * - Upload this sketch to your Arduino Leonardo
 * - Plug the Leonardo into the computer you want to use your keyboard layout with.
 * - Plug your keyboard into the USB host shield.
 *
 * Dependencies: https://github.com/felis/USB_Host_Shield_2.0
 */

#include <avr/pgmspace.h>
#include <Usb.h>
#include <hidboot.h>
#include <usbhub.h>

/* CONFIG */
//#define TEENSY    // Uncomment this line if you are using Teensy.
//#define DEBUG     // Uncomment this line to see output to the serial monitor.
                    // NOTE: in debug mode, the keyboard will not work until the serial monitor is started.
#define SERIAL_IO_RATE 115200

/* KEYCODES */

#define keyA        4
#define keyB        5
#define keyC        6
#define keyD        7
#define keyE        8
#define keyF        9
#define keyG        10
#define keyH        11
#define keyI        12
#define keyJ        13
#define keyK        14
#define keyL        15
#define keyM        16
#define keyN        17
#define keyO        18
#define keyP        19
#define keyQ        20
#define keyR        21
#define keyS        22
#define keyT        23
#define keyU        24
#define keyV        25
#define keyW        26
#define keyX        27
#define keyY        28
#define keyZ        29
#define key1        30
#define key2        31
#define key3        32
#define key4        33
#define key5        34
#define key6        35
#define key7        36
#define key8        37
#define key9        38
#define key0        39
#define keyPipe     49
#define keyColon    51
#define keyQuote    52
#define keyTilde    53
#define keyComma    54
#define keyPeriod   55
#define keySlash    56
#define keyReturn   40
#define keyEsc      41
#define keyBacksp   42
#define keyTab      43
#define keyCapsLock 57
#define keyF1       58
#define keyF2       59
#define keyF3       60
#define keyF4       61
#define keyF5       62
#define keyF6       63
#define keyF7       64
#define keyF8       65
#define keyF9       66
#define keyF10      67
#define keyF11      68
#define keyF12      69
#define keyF13      70
#define keyF14      71
#define keyF15      72
#define modLShift   0x02
#define modRShift   0x20
#define modLCtrl    0x01
#define modRCtrl    0x10
#define modLAlt     0x04
#define modRAlt     0x40
#define modLMeta    0x08
#define modRMeta    0x80

inline void SendKeysToHost (uint8_t *buf);

typedef enum
{
  qwerty = 0,
  booznik,
} KeyboardLayout;

PROGMEM const uint8_t qwertyKeymap[]  = {keyA, keyB, keyC, keyD, keyE, keyF, keyG, keyH, keyI, keyJ, keyK, keyL, keyM, keyN, keyO, keyP,     keyQ, keyR, keyS, keyT, keyU, keyV, keyW, keyX, keyY, keyZ, key1, key2, key3, key4, key5, key6, key7, key8, key9, key0, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, keyColon, 52, 53, 54, 55, 56, 57};
PROGMEM const uint8_t booznikKeymap[] = {keyA, keyB, keyC, keyS, keyF, keyT, keyD, keyH, keyO, keyN, keyE, keyI, keyM, keyK, keyY, keyColon, keyQ, keyG, keyR, keyP, keyL, keyV, keyW, keyX, keyJ, keyZ, key1, key2, key3, key4, key5, key6, key7, key8, key9, key0, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, keyU,     52, 53, 54, 55, 56, 57};

const uint8_t *Keymap[] =
{
  qwertyKeymap,
  booznikKeymap
};

KeyboardLayout CurrentLayout = booznik;
uint8_t KeyBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

class KbdRptParser : public KeyboardReportParser {
  protected:
    virtual void Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
    virtual void OnControlKeysChanged(uint8_t before, uint8_t after);
    virtual void OnKeyDown(uint8_t mod, uint8_t key);
    virtual void OnKeyUp(uint8_t mod, uint8_t key);
};

bool isMappableKey(uint8_t key) {
  return key >= keyA && key <= keyCapsLock;
}

inline bool isError(uint8_t *buf) {
  if (buf[2] == 1) return true;
  return false;
}

uint8_t mappedKey(uint8_t key) {
  return pgm_read_byte(Keymap[CurrentLayout] + key - keyA);
}

uint8_t mapModifierKeys(uint8_t *buf) {
  return buf[0];
}

void KbdRptParser::Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  for (uint8_t i = 0; i < 8; i++) {
    PrintHex<uint8_t>(buf[i], 0x80);
    Serial.print(" ");
  }
  Serial.println("");

  if (isError(buf)) return;

  KeyBuffer[0] = mapModifierKeys(buf);

  for (uint8_t i = 2; i < 8; i++) {
    if (isMappableKey(buf[i])) {
      KeyBuffer[i] = mappedKey(buf[i]);
    } else {
      KeyBuffer[i] = buf[i];
    }

    HandleLockingKeys(hid, KeyBuffer[i]);
  }

  SendKeysToHost(KeyBuffer);
};

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {
  Serial.println("modifier keys changed");
}

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key) {
  Serial.println("pressed");
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {
  Serial.println("released");  
}

inline void SendKeysToHost (uint8_t *buf) {
#ifdef TEENSY
  Keyboard.set_modifier(buf[0]);
  Keyboard.set_key1(buf[2]);
  Keyboard.set_key2(buf[3]);
  Keyboard.set_key3(buf[4]);
  Keyboard.set_key4(buf[5]);
  Keyboard.set_key5(buf[6]);
  Keyboard.set_key6(buf[7]);
  Keyboard.send_now();
#else
  HID_SendReport(2, buf, 8);
#endif
}

USB     Usb;
USBHub  hub(&Usb);
HIDBoot<HID_PROTOCOL_KEYBOARD>    ExtKeyboard(&Usb);
KbdRptParser Prs;

void setup() {
  Serial.begin(SERIAL_IO_RATE);
#ifdef DEBUG
  while (!Serial);
#endif
  Serial.println("Starting...");
    
  delay(5000);

  if (Usb.Init() < 0) Serial.println("OSC did not start.");

  delay(5000);
  
  if (!ExtKeyboard.SetReportParser(0, (HIDReportParser*)&Prs)) Serial.println("SetReportParser failed");
 
  Serial.println("Setup finished");
}

void loop() {
  Usb.Task();
}
