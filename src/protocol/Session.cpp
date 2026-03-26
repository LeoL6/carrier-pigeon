#include "Session.h"

static Session session;

Session& SessionManager::get()
{
  return session;
}