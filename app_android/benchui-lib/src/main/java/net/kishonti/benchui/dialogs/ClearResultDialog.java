/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.dialogs;

import net.kishonti.theme.Localizator;

public class ClearResultDialog extends CommonDialog {
	@Override
	protected String getTitle() {
		return Localizator.getString(getActivity(), "ClearHistoryDialogTitle");
	}

	@Override
	protected String getMessage() {
		return Localizator.getString(getActivity(), "ClearHistoryDialogBody");
	}

	@Override
	protected String getPositiveText() {
		return Localizator.getString(getActivity(), "Yes");
	}

	@Override
	protected String getNegativeText() {
		return Localizator.getString(getActivity(), "Cancel");
	}
}
