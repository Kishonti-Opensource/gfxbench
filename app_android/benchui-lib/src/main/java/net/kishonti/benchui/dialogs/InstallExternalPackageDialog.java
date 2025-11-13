/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.dialogs;

public class InstallExternalPackageDialog extends CommonDialog {

	@Override
	protected String getTitle() {
		return "Application not installed!";
	}

	@Override
	protected String getMessage() {
		return "To be able to run this test we need to install an external application.";
	}

	@Override
	protected String getPositiveText() {
		return "Install";
	}

	@Override
	protected String getNegativeText() {
		return "Back";
	}

}
