#include "Messages.h"

namespace Messages 
{
  // <========================>
  //   Message Buffer
  // <========================>
  static constexpr int MAX_MESSAGES = 16;

  static Message messages[MAX_MESSAGES];
  static int head = 0;
  static int tail = 0;
  static int messagesLength = 0;

  bool push(const Message& msg)
  {
    if (messagesLength == MAX_MESSAGES)
        return false;

    messages[head] = msg;

    head = (head + 1) % MAX_MESSAGES;

    messagesLength++;

    return true;
  }

  bool pop(Message& msg)
  {
    if (messagesLength == 0)
        return false;

    msg = messages[tail];

    tail = (tail + 1) % MAX_MESSAGES;

    messagesLength--;

    return true;
  }

  bool isMessageBufferEmpty()
  {
    return messagesLength == 0;
  }

  // <========================>
  //   Input Buffer
  // <========================>
  static constexpr int MAX_CHAR = 128;

  static char inputBuffer[128];
  static int inputLength = 0;
  static bool inputDirty = false;

  void appendChar(char c)
  {
    if (inputLength >= MAX_CHAR - 1)
        return;

    inputBuffer[inputLength] = c;

    inputLength++;

    inputDirty = true;

    inputBuffer[inputLength] = '\0';
  }

  void backspace()
  {
    if (inputLength == 0)
        return;

    inputLength--;

    inputBuffer[inputLength] = '\0';

    inputDirty = true;
  }

  int getInputLength()
  {
    return inputLength;
  }

  bool isInputBufferEmpty()
  {
    return getInputLength() <= 0;
  }

  const char* getInputBuffer()
  {
    return inputBuffer;
  }

  void clearInputBuffer()
  {
    inputLength = 0;
    inputBuffer[0] = '\0';

    inputDirty = true;
  }

  bool isInputDirty()
  {
    return inputDirty;
  }

  void clearInputDirty()
  {
    inputDirty = false;
  }
}