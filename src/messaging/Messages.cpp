#include "Messages.h"

namespace Display {
  static constexpr int MAX_MESSAGES = 32;

  Message messages[MAX_MESSAGES];
  int messageCount = 0;

  char inputBuffer[128];
  int inputLength = 0;

  // void addIncoming(const char* text)
  // {
  //   if (messageCount >= MAX_MESSAGES)
  //       return;

  //   Message& msg = messages[messageCount++];

  //   msg.outgoing = false;
  //   strncpy(msg.text, text, sizeof(msg.text));
  // }

  // void appendInputChar(char c)
  // {
  //   if (inputLength >= sizeof(inputBuffer) - 1)
  //       return;

  //   inputBuffer[inputLength++] = c;
  //   inputBuffer[inputLength] = '\0';
  // }

  // void backspace()
  // {
  //   if (inputLength == 0)
  //       return;

  //   inputLength--;
  //   inputBuffer[inputLength] = '\0';
  // }
}