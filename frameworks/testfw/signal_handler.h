/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SIGNAL_HANDLER_
#define SIGNAL_HANDLER_

#include <signal.h>

class SignalHandler
{
public:
	SignalHandler();
	static void my_sigaction(int signal, siginfo_t *info, void *reserved);
	struct sigaction old_sa_[NSIG];
};

#endif // SIGNAL_HANDLER_
