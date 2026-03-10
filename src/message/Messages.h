#pragma once

#include "Message.h"

namespace Messages
{
  // <========================>
  //   Message Buffer
  // <========================>
  bool push(const Message& msg);
  bool pop(Message& msg);

  bool isMessageBufferEmpty();

  // <========================>
  //   Input Buffer
  // <========================>
  void appendChar(char c);
  void backspace();

  int getInputLength();
  bool isInputBufferEmpty();
  const char* getInputBuffer();
  void clearInputBuffer();

  bool isInputDirty();
  void clearInputDirty();
};