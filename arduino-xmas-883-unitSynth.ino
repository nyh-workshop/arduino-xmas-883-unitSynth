#include <LittleFS.h>
#include <MD_MIDIFile.h>
#include <M5UnitSynth.h>
#include <hardware/pll.h>
#include <hardware/vreg.h>

#include "midiJukebox.h"
#include "xmasLights_fastLED.h"

#define LED_PIN 25
#define UNIT_SYNTH_PIN_IN 4

const int LED_TOGGLE_TIME_SETUP_DONE = 1000;
const int LED_TOGGLE_TIME_SETUP_ERROR = 250;
int LedToggleTime = 0;
bool LedState = false;
static uint32_t previousLedToggleTime = 0;

M5UnitSynth synth;
MD_MIDIFile SMF;

PlayList* shuffleSongs = nullptr;

const char* dirPath = "/";

constexpr bool DEBUG_MODE = true;
constexpr bool ENABLE_FASTLED = true;
constexpr bool IS_BATTERY_OPERATED = false;

void midiSilence(void)
// Turn everything off on every channel.
// Some midi files are badly behaved and leave notes hanging, so between songs turn
// off all the notes and sound
{
  midi_event ev;

  // All sound off
  // When All Sound Off is received all oscillators will turn off, and their volume
  // envelopes are set to zero as soon as possible.
  ev.size = 0;
  ev.data[ev.size++] = 0xb0;
  ev.data[ev.size++] = 120;
  ev.data[ev.size++] = 0;

  for (ev.channel = 0; ev.channel < 16; ev.channel++)
    midiCallback(&ev);
}

void midiCallback(midi_event* pev)
// Called by the MIDIFile library when a file event needs to be processed
// thru the midi communications interface.
// This callback is set up in the setup() function.
{
  // Define constants for MIDI channel voice message IDs
  const uint8_t NOTE_OFF = 0x80;  // note on
  const uint8_t NOTE_ON = 0x90;   // note off. NOTE_ON with velocity 0 is same as NOTE_OFF

  switch (pev->data[0]) {
    case NOTE_OFF:  // [1]=note no, [2]=velocity
      // playNote(pev->data[1], SILENT);
      synth.setNoteOff(0, pev->data[1], pev->data[2]);
      Serial.printf("NOTE_OFF ch: %d\n", pev->data[1]);
      break;

    case NOTE_ON:  // [1]=note_no, [2]=velocity
      // Note ON with velocity 0 is the same as off
      // playNote(pev->data[1], (pev->data[2] == 0) ? SILENT : ACTIVE);
      // Velocity is not included for now!
      synth.setNoteOn(0, pev->data[1], pev->data[2]);
      Serial.printf("NOTE_ON trk: %d, ch: %d, no: %d\n", pev->track, pev->channel, pev->data[1]);
      break;

    default:
      break;
  }
}

void nextSong(String midiFileName) {

  int err = SMF.load(midiFileName.c_str());
  if (midiFileName == "NoMoreMidiFileInDir") {
    Serial.println("No more midi files in directory!");
    return;
  }

  Serial.printf("Filename: %s\n", SMF.getFilename());
  Serial.printf("Format: %d\n", SMF.getFormat());

  if (err != MD_MIDIFile::E_OK)
    Serial.printf("SMF load error during playing: %d\n", err);
  else
    SMF.restart();
}

static enum { S_IDLE,
              S_PLAYING,
              S_NEXT,
              S_RESTART } state = S_IDLE;

static enum { SETUP_PROGRESS,
              SETUP_DONE_WITH_ERROR,
              SETUP_DONE } setupState = SETUP_PROGRESS;

void setup() {
  Serial.begin(115200);

  uint32_t randomNumber = getRandomNumber(0, 5);
  uint8_t pickUnitSynthVoice = TinkleBell;

  switch (randomNumber) {
    case 0:
      pickUnitSynthVoice = TinkleBell;
      break;
    case 1:
      pickUnitSynthVoice = ElPiano1;
      break;
    case 2:
      pickUnitSynthVoice = Glockenspiel;
      break;
    case 3:
      pickUnitSynthVoice = MusicBox;
      break;
    case 4:
      pickUnitSynthVoice = GrandPiano_1;
      break;
    case 5:
      pickUnitSynthVoice = ElPiano1;
      break;
    default:
      break;
  }

  delay(1000);

  Serial.println("arduino-xmas-883-unitSynth");
  synth.begin(&Serial2, UNIT_SYNTH_BAUD, 5, UNIT_SYNTH_PIN_IN);
  synth.setMasterVolume(0x7f);
  synth.setInstrument(0, 0, pickUnitSynthVoice);

  pinMode(LED_PIN, OUTPUT);
  pinMode(WS2812_PIN, OUTPUT);

  if (ENABLE_FASTLED) {
    Serial.println("Starting FastLED!");
    // Shuffle the FastLED patterns:
    fillArrayAscendingNumbers(patternNumberList);
    shuffleArray(patternNumberList);
    setupXmasLights();
  }

  // Initialize LittleFS:
  if (!LittleFS.begin()) {
    Serial.println("LittleFS init fail!");
    state = S_IDLE;
    rp2040.fifo.push(SETUP_DONE_WITH_ERROR);
    return;
  }

  shuffleSongs = new PlayList("/", true);

  // shuffleSongs->showFilesInList();

  // Initialize MIDIFile:
  SMF.begin(&LittleFS);
  SMF.setMidiHandler(midiCallback);

  // Load a MIDI file from there:
  int err = SMF.load(shuffleSongs->getCurrentPlayingSong().c_str());

  if (err != MD_MIDIFile::E_OK) {
    Serial.printf("SMF load error: %d", err);
    state = S_IDLE;
    rp2040.fifo.push(SETUP_DONE_WITH_ERROR);
    return;
  }

  Serial.printf("Filename: %s\n", SMF.getFilename());
  Serial.printf("Format: %d\n", SMF.getFormat());

  state = S_PLAYING;

  rp2040.fifo.push(SETUP_DONE);
}

void loop() {
  if (ENABLE_FASTLED)
    xmasLightsLoop();

  if (millis() - previousLedToggleTime >= LedToggleTime) {
    LedState = LedState ^ 1;
    digitalWrite(LED_PIN, LedState);
    previousLedToggleTime = millis();
  }
}

void setup1() {

  // put your setup code here, to run once:
  while (!rp2040.fifo.available())
    ;

  if (rp2040.fifo.pop() == SETUP_DONE) {
    Serial.println("Setup done! :D");
    LedToggleTime = LED_TOGGLE_TIME_SETUP_DONE;
  } else {
    Serial.println("Setup error! Please check setup!");
    LedToggleTime = LED_TOGGLE_TIME_SETUP_ERROR;
  }
}

void loop1() {
  if (DEBUG_MODE) {
    if (Serial.available() > 0) {
      char incomingChar = Serial.read();
      if (incomingChar == 's')
        state = S_NEXT;
      else if (incomingChar == 'r')
        state = S_RESTART;
    }
  }
  switch (state) {
    case S_IDLE:
      if (IS_BATTERY_OPERATED && !ENABLE_FASTLED) {
        rp2040.idleOtherCore();
        //set_sys_clock_pll(756000000, 7, 6);
        set_sys_clock_48mhz();
        pll_deinit(pll_usb);
        vreg_set_voltage(VREG_VOLTAGE_0_90);
      } else
        delay(500);
      break;
    case S_PLAYING:
      if (!SMF.isEOF()) {
        SMF.getNextEvent();
      } else {
        // Reached end of file.
        // Play next file, and if no next file, go idle mode!
        SMF.close();
        SMF.pause(true);
        midiSilence();

        String midiFileName = shuffleSongs->next();

        delay(1000);

        if (midiFileName == "EndOfDirectory") {
          Serial.println("Playing done! Idle mode now...");
          state = S_IDLE;
        } else {
          Serial.println("Play song done! Next song...");
          nextSong(midiFileName);
          state = S_PLAYING;
          SMF.pause(false);
        }
      }
      break;
    case S_NEXT:
      {
        Serial.println("Skip to next song...");

        SMF.close();
        SMF.pause(true);
        midiSilence();

        delay(500);

        String midiFileName = shuffleSongs->next();

        Serial.printf("Playing song location: %d\n", shuffleSongs->getPlayingSongLocation());

        if (midiFileName == "EndOfDirectory") {
          Serial.println("Reached to the end of the directory!");
          state = S_IDLE;
        } else {
          nextSong(midiFileName);
          state = S_PLAYING;
          SMF.pause(false);
        }
      }
      break;
    case S_RESTART:
      Serial.println("Restarting song...");
      SMF.restart();
      delay(100);
      state = S_PLAYING;
      break;
    default:
      state = S_IDLE;
      break;
  }
}