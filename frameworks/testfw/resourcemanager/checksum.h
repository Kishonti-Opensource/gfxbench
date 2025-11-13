/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <set>
#include <string>



namespace tfw {
    std::string calculateChecksum(
            const std::string& root,
            const std::set<std::string>& textExtensions);
}



#endif // CHECKSUM_H
