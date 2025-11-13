/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.testfw.app;

import java.io.File;
import java.io.IOException;
import java.util.Collection;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.FilenameUtils;

import net.kishonti.testfw.TfwActivity;
import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;
import android.Manifest;
import android.content.pm.PackageManager;

public class MainActivity extends Activity {

    private static final String TAG = "TestRunner";
    private static final int RUN_TEST_REQUEST = 1;
    private TextView mDetailView;
    private String mResult;
    private Spinner mSpinner;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        requestPermissions(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, 2);

        mDetailView = (TextView) findViewById(R.id.detailView);
        mDetailView.setMovementMethod(new ScrollingMovementMethod());
        mSpinner = (Spinner) findViewById((R.id.spinner));

        try {
            File configDir = new File("/sdcard/kishonti/tfw/config");
            Collection<File> files = FileUtils.listFiles(configDir, null, true);
            Log.i(TAG, "Read config files from: " + configDir.getAbsolutePath());
            Log.i(TAG, "Read config files num: " + files.size());
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item);
            for (File f : files) {
                Log.e(TAG, "Read config files in: " + f.getName());
                adapter.add(FilenameUtils.getBaseName(f.getName()));
            }
            mSpinner.setAdapter(adapter);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "failed to read config files in: " );
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case 2:
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // Permission is granted. Continue the action or workflow
                    // in your app.
                }  else {
                    // Explain to the user that the feature is unavailable because
                    // the features requires a permission that the user has denied.
                    // At the same time, respect the user's decision. Don't link to
                    // system settings in an effort to convince the user to change
                    // their decision.
                }
                return;
            }
    }

    @Override
    protected void onActivityResult (int requestCode, int resultCode, Intent data) {
        if (requestCode == RUN_TEST_REQUEST) {
            mResult = "[Error] No result";
            if (data != null && data.hasExtra("result")) {
                mResult = data.getStringExtra("result");
            }
            mDetailView.setText(mResult);
        }
    }

    @Override
    protected void onRestoreInstanceState (Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mResult = savedInstanceState.getString("mainactivity.result");
        mDetailView.setText(mResult);
        int pos = savedInstanceState.getInt("mainactivity.selected_position", -1);
        if (-1 < pos && pos < mSpinner.getAdapter().getCount()) {
            mSpinner.setSelection(pos);
        }
    }

    @Override
    protected void onSaveInstanceState (Bundle outState) {
        outState.putString("mainactivity.result", mResult);
        outState.putInt("mainactivity.selected_position", mSpinner.getSelectedItemPosition());
    }

    public void onStartClicked(View v) {
        Spinner spinner = (Spinner) findViewById((R.id.spinner));
        String test_id = spinner.getSelectedItem().toString();
        try {
            Log.e(TAG, "onStartClicked: " + test_id);
            String config = FileUtils.readFileToString(new File(TestApplication.BASE_PATH + "/config/" + test_id + ".json"));
            Intent tfw = new Intent();
            tfw.setClass(this, TfwActivity.class);
            tfw.putExtra("test_id", test_id);
            tfw.putExtra("config", config);
            startActivityForResult(tfw, RUN_TEST_REQUEST);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
