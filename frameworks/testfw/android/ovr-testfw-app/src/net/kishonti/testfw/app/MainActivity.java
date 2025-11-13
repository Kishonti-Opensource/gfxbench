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

        try {
            String config = FileUtils.readFileToString(new File(TestApplication.BASE_PATH + "/config/gl_5_ovr_2.json"));
            Intent tfw = new Intent();
            tfw.setClass(this, TfwActivity.class);
            tfw.putExtra("test_id", "gl_5_ovr_2");
            tfw.putExtra("config", config);
            startActivityForResult(tfw, RUN_TEST_REQUEST);
        } catch (IOException e) {
            e.printStackTrace();
        }

        // mDetailView = (TextView) findViewById(R.id.detailView);
        // mDetailView.setMovementMethod(new ScrollingMovementMethod());
        // mSpinner = (Spinner) findViewById((R.id.spinner));

        // File configDir = new File(TestApplication.BASE_PATH, "config");
        // try {
        //     Collection<File> files = FileUtils.listFiles(configDir, new String[] { "json" }, false);
        //     ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item);
        //     for (File f : files) {
        //         adapter.add(FilenameUtils.getBaseName(f.getName()));
        //     }
        //     mSpinner.setAdapter(adapter);
        // } catch (IllegalArgumentException e) {
        //     Log.e(TAG, "failed to read config files in: " + configDir.getAbsolutePath());
        // }
    }

    @Override
    protected void onActivityResult (int requestCode, int resultCode, Intent data) {
        if (requestCode == RUN_TEST_REQUEST) {
            // mResult = "[Error] No result";
            // if (data != null && data.hasExtra("result")) {
            //     mResult = data.getStringExtra("result");
            // }
            // mDetailView.setText(mResult);

            finish();
            System.exit(0);
        }
    }

    @Override
    protected void onRestoreInstanceState (Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        // mResult = savedInstanceState.getString("mainactivity.result");
        // mDetailView.setText(mResult);
        // int pos = savedInstanceState.getInt("mainactivity.selected_position", -1);
        // if (-1 < pos && pos < mSpinner.getAdapter().getCount()) {
        //     mSpinner.setSelection(pos);
        // }
    }

    @Override
    protected void onSaveInstanceState (Bundle outState) {
        // outState.putString("mainactivity.result", mResult);
        // outState.putInt("mainactivity.selected_position", mSpinner.getSelectedItemPosition());
    }

    public void onStartClicked(View v) {
        Spinner spinner = (Spinner) findViewById((R.id.spinner));
        String test_id = spinner.getSelectedItem().toString();
        try {
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
