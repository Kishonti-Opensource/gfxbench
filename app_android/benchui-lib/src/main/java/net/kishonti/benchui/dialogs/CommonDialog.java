/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.dialogs;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import net.kishonti.benchui.R;

public abstract class CommonDialog extends DialogFragment implements View.OnClickListener {

	@Override
	public void onClick(View view) {
		boolean dismiss = false;
		if (view.getId() == R.id.btPositive) {
			dismiss = true;
			if (handler != null) dismiss = handler.onDialogClicked(this, true);
		}
		if (view.getId() == R.id.btNegative) {
			dismiss = true;
			if (handler != null) dismiss = handler.onDialogClicked(this, false);
		}
		if (dismiss) dismiss();
	}

	public static interface OnDialogHandler {
		boolean onDialogClicked(DialogFragment sender, boolean result);
	}

	OnDialogHandler handler = null;

	public CommonDialog() {
		super();
		setCancelable(false);
	}

	abstract protected String getTitle();

	abstract protected String getMessage();

	abstract protected String getPositiveText();

	abstract protected String getNegativeText();

	public void setOnDialogHandler(OnDialogHandler handler) {
		this.handler = handler;
	}

	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState) {
		final View v = getActivity().getLayoutInflater().inflate(R.layout.dialog_common, null);
		final Button btPositive;
		final Button btNegative;
		final TextView tvStatusMessage;
		final TextView tvDialogTitle;

		tvStatusMessage = (TextView) v.findViewById(R.id.tvDialogBody);
		tvDialogTitle = (TextView) v.findViewById(R.id.tvDialogTitle);
		btPositive = (Button) v.findViewById(R.id.btPositive);
		btNegative = (Button) v.findViewById(R.id.btNegative);

		tvDialogTitle.setText(getTitle());
		tvStatusMessage.setText(getMessage());

		btPositive.setText(getPositiveText());
		btPositive.setOnClickListener(this);

		final String negativeText = getNegativeText();

		if (negativeText != null) {
			btNegative.setText(negativeText);
			btNegative.setOnClickListener(this);
			btNegative.setVisibility(View.VISIBLE);
		} else {
			btNegative.setVisibility(View.GONE);
		}

		return new AlertDialog.Builder(getActivity()).setView(v).create();
	}

}
