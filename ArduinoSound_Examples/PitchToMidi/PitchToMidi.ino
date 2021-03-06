/*
  This example reads audio data from an I2S mic. It calculates the pitch range
  of an incoming sound. Peaks at about 4200 Hz, which is just above
  note value C8.  Then it calculates the nearest MIDI note number
  using the PitchToFrequency chart from the MIDIUSB library.

  Circuit:
   Arduino/Genuino Zero, MKRZero or MKR1000 board
   I2S mic:
     GND connected GND
     3.3V connected 3.3V (Zero) or VCC (MKR1000, MKRZero)
     WS connected to pin 0 (Zero) or pin 3 (MKR1000, MKRZero)
     CLK connected to pin 1 (Zero) or pin 2 (MKR1000, MKRZero)
     SD connected to pin 9 (Zero) or pin A6 (MKR1000, MKRZero)

  created 7 April 2019
  by Tom Igoe
  based on Sandeep Mistry's ArduinoSound examples
*/

#include <ArduinoSound.h>
#include <MIDIUSB.h>
#include <pitchToFrequency.h>

// sample rate for the input, in Hz.
// to hear a max frequency, sample at 2 * the frequency.
// Check your mic to see what the top of its dynamic range is
const int sampleRate = 8400;

// size of the FFT to compute
const int fftSize = 128;

// size of the spectrum output, which is half of FFT size
const int spectrumSize = fftSize / 2;

// array to store spectrum output
int spectrum[spectrumSize];

// create an FFT analyzer to be used with the I2S input
FFTAnalyzer fftAnalyzer(fftSize);

// arbitrary threshold for loudness. Range is 0 - 2^32 (4,294,967,296)
int threshold = 150000;

void setup() {
  // setup the serial
  Serial.begin(9600);
  while (!Serial);
  // setup the I2S audio input for the sample rate with 32-bits per sample
  if (!AudioInI2S.begin(sampleRate, 32)) {
    Serial.println("Failed to initialize I2S input");
    while (true);
  }

  // configure the I2S input as the input for the FFT analyzer
  if (!fftAnalyzer.input(AudioInI2S)) {
    Serial.println("Failed to set FFT analyzer input");
    while (true);
  }

  // print the frequency bands:
  Serial.println("Frequency bands: ");
  for (int b = 0; b < spectrumSize; b++) {
    // if this sample is louder than your loudest so far:
    // get the low frequency for this band:
    float freqLow = (b * float(sampleRate)) / fftSize;
    float freqHigh = ((b + 1) * float(sampleRate)) / fftSize;
    Serial.print(freqLow);
    Serial.print(" - ");
    Serial.println(freqHigh);
  }
  Serial.println();
  delay(5000);
}

void loop() {
  if (fftAnalyzer.available()) {
    // analysis available, read in the spectrum
    fftAnalyzer.read(spectrum, spectrumSize);

    // analyze the bands of the FFT spectrum:
    int loudestSample = 0;  // the loudest sample in the FFT spectrum
    float freqLow = 0;        // low frequency of the loudest band
    float freqHigh = 0;       // high frequency of the loudest band
    // iterate over the spectrum:
    for (int b = 0; b < spectrumSize; b++) {
      // if this sample is louder than your loudest so far:
      if (spectrum[b] >= loudestSample) {
        // then this is the current loudest:
        loudestSample = spectrum[b];
        // get the low frequency for this band:
        freqLow = (b * float(sampleRate)) / fftSize;
        freqHigh = ((b + 1) * float(sampleRate)) / fftSize;
      }
    }

    // you're done. If your loudest was above a threshold,
    // print the results
    if (loudestSample > threshold) {
      Serial.print(freqLow);
      Serial.print(" ");
      Serial.print(freqHigh);
      Serial.print("    ");
      // print out the amplititude to the serial monitor
      Serial.print(loudestSample);
      Serial.print("    ");
      // print out the closest MIDI note:
      /  int closestNote = findMidiNoteFromPitches(freqLow, freqHigh);
      Serial.println(closestNote);
    }
  }
}

// This function finds the closest MIDI note value to a given pitch.
// It works by finding the difference between the given pitch and
// each MIDI note's pitch. The MIDI note whose pitch is closest to
// the given pitch is the right one.
int findMidiNoteFromPitches(int lowPitch, int highPitch) {
  // last difference; ultimately you want it small, so initialize it big:
  int lastDiff = 32000;
  // Your desired midi note; if it comes back -1, you know you had an error:
  int desiredMidiNote = -1;
  // iterate over all the known frequencies of MIDI notes:
  for (int x = 0; x < 127; x++) {
    if (pitchFrequency[x] >= lowPitch && pitchFrequency[x] <= highPitch) {
      Serial.println(pitchFrequency[x]);
    }
    // find the absolute value of difference between this pitch and your pitch:
    int diff = abs(lowPitch - pitchFrequency[x]);
    // if this is the smallest difference so far, it's the desired midi note:
    if (diff < lastDiff) {
      desiredMidiNote = x;
    }
    // save this difference for comparison next time through the for loop:
    lastDiff = diff;
  }
  // return the desired midi note:
  return desiredMidiNote;
}
