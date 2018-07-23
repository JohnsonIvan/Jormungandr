#ifndef MESSENGER_H
#define MESSENGER_H
/*
 * Src/messenger.h
 *
 * Sends jobs from the client to the appropriate server
 *
 * Copyright(C) 2018, Ivan Tobias Johnson
 *
 * LICENSE: GPL 2.0
 */
#include <stdbool.h>

#include "server.h"

bool messengerSendJob(int serverfd, struct job job);
bool messengerInitialize(int serverfd);

#endif
