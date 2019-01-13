/*
The MIT License (MIT)
Copyright (c) 2012 Mike Sims <msims04@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include <conio.h>
#include <signal.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>

const double version = 1.0;

bool running   = false;
int  transpose = 0;

void onExit(int param) {
  running = false; }

void CALLBACK midiCallback(HMIDIIN handle,
                           UINT    message,
                           DWORD   instance,
                           DWORD   param1,
                           DWORD   param2) {
  switch(message) {
    case MIM_DATA: {
      if((param1 & 0xFF) != 176) { // don't tranpose the pedal
        param1 += (transpose << 8); }

      DWORD result = midiOutShortMsg((HMIDIOUT)instance, param1);

      if(result != MMSYSERR_NOERROR) {
        fprintf(stderr, "Error: midiOutShortMsg returned %d.\n", result); }

      break; }

    case MIM_OPEN:
    case MIM_CLOSE:
    case MIM_MOREDATA:
    case MIM_ERROR:
    default: {
      break; } }; }

int main(int argc, char **argv) {
  fprintf(stdout, "transposeMIDI v%.2f\n"
                  "Copyright 2012-2013 Mike Sims (msims04@gmail.com)\n\n",
                  version);

  signal(SIGABRT, onExit);
  signal(SIGINT,  onExit);
  signal(SIGTERM, onExit);

  int input  = -1;
  int output = -1;

  if(argc == 3) {
    input  = atoi(argv[1]);
    output = atoi(argv[2]); }

  // Input device
  fprintf(stdout, "ID  Input Device Name\n"
                  "==  =================\n");

  for(int i = 0; i < midiInGetNumDevs(); ++i) {
    MIDIINCAPS caps;
    memset(&caps, 0, sizeof(caps));
    midiInGetDevCaps(i, &caps, sizeof(caps));
    fprintf(stdout, "%2d  %s\n", i, caps.szPname); }
  fprintf(stdout, "\n");

  fprintf(stdout, "Select input device ID: ");
  if(input == -1) {
    scanf("%d", &input); }
  else {
    fprintf(stdout, "%d\n", input); }
  fprintf(stdout, "\n");

  if(input < 0 || input >= midiInGetNumDevs()) {
    fprintf(stderr, "Error: Input device (%d) is not valid.\n", input);
    return 0; }

  // Output device
  fprintf(stdout, "ID  Output Device Name\n"
                  "==  ==================\n");

  for(int i = 0; i < midiOutGetNumDevs(); ++i) {
    MIDIOUTCAPS caps;
    memset(&caps, 0, sizeof(caps));
    midiOutGetDevCaps(i, &caps, sizeof(caps));
    fprintf(stdout, "%2d  %s\n", i, caps.szPname); }
  fprintf(stdout, "\n");

  fprintf(stdout, "Select output device ID: ");
  if(output == -1) {
    scanf("%d", &output); }
  else {
    fprintf(stdout, "%d\n", output); }
  fprintf(stdout, "\n");

  if(output < 0 || output >= midiOutGetNumDevs()) {
    fprintf(stderr, "Error: Output device (%d) is not valid.\n", output);
    return 0; }

  // Connect devices
  DWORD    result = 0;
  HMIDIIN  hInput;
  HMIDIOUT hOutput;

  result = midiOutOpen(&hOutput, output, 0, 0, CALLBACK_NULL);
  if(result != MMSYSERR_NOERROR) {
    fprintf(stderr, "Error: Could not open the output device.\n");
    return 0; }

  result = midiInOpen(&hInput, input, (DWORD_PTR)midiCallback,
                      (DWORD_PTR)hOutput, CALLBACK_FUNCTION);
  if(result != MMSYSERR_NOERROR) {
    fprintf(stderr, "Error: Could not open the input device.\n");
    return -1; }

  midiInStart(hInput);

  // User input loop
  printf("Press 'h' to display help.\n");
  printf("Press 'q' to quit.\n");

  running = true;
  while(running) {
    if(kbhit()) {
      char c = getch();

      if(c == 'q') {
        break; }

      else if(c == 'h') {
        printf("Press 'h' to display help.\n");
        printf("Press 'q' to quit.\n");
        printf("Press '-' or '=' to adjust the transpose amount.\n");
        printf("Press '1', '2', or '3' to switch to presets (-12, 0, 12).\n"); }

      else if(c == '1') {
        transpose = -12;
        printf("Transpose: %d\n", transpose); }
      else if(c == '2') {
        transpose = 0;
        printf("Transpose: %d\n", transpose); }
      else if(c == '3') {
        transpose = 12;
        printf("Transpose: %d\n", transpose); }

      else if(c == '-') {
        transpose -= 1;
        if(transpose < -12) {
          transpose += 1; }
        printf("Transpose: %d\n", transpose); }
      else if(c == '=') {
        transpose += 1;
        if(transpose > 12) {
          transpose  -= 1; }
        printf("Transpose: %d\n", transpose); } }

    Sleep(1); }

  midiInStop (hInput);
  midiInReset(hInput);
  midiInClose(hInput);

  midiOutReset(hOutput);
  midiOutClose(hOutput);

  return 1; }
