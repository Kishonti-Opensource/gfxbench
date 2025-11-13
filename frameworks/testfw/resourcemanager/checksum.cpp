/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "checksum.h"

#include <Poco/FileStream.h>
#include <Poco/Path.h>
#include <Poco/RecursiveDirectoryIterator.h>
#include <Poco/SHA1Engine.h>



namespace tfw {

    std::string calculateChecksum(
        const std::string& root,
        const std::set<std::string>& textExtensions)
    {
        std::istreambuf_iterator<char> eos;
        Poco::SHA1Engine digestEngine;
        Poco::SimpleRecursiveDirectoryIterator iterator =
                Poco::SimpleRecursiveDirectoryIterator(Poco::Path(root));
        Poco::SimpleRecursiveDirectoryIterator end;
        while (iterator != end) {
            if (iterator->isFile()) {
                if (textExtensions.find(iterator.path().getExtension()) == textExtensions.end()) {
                    long long size = iterator->getSize();
                    digestEngine.update(&size, sizeof(size));
                } else {
                    Poco::FileStream stream(iterator->path());
                    std::string buffer(std::istreambuf_iterator<char>(stream), eos);
                    digestEngine.update(buffer);
                 }
            }
            ++iterator;
        }

        return Poco::DigestEngine::digestToHex(digestEngine.digest());
    }

}
