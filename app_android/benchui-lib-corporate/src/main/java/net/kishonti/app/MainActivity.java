/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.app;

import net.kishonti.theme.Localizator;
import net.kishonti.benchui.R;
import net.kishonti.benchui.fragments.SettingsFragment;

public class MainActivity extends net.kishonti.benchui.MainActivity {
    @Override
    protected void AddAllTabs() {
        super.AddAllTabs();
        AddTab(Localizator.getString(this, "TabSettings"), SettingsFragment.class, R.drawable.profile_icon, R.drawable.profile_icon_sel);
    }
}
