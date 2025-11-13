/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import android.os.Bundle;
import net.kishonti.swig.Descriptor;

public interface DescriptorFactory {
	Descriptor getDescriptorForId(String test_id, Bundle args);
}
