/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui.corporate;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import net.kishonti.benchui.MainActivity;
import net.kishonti.benchui.BenchmarkApplication;
import net.kishonti.benchui.DescriptorFactory;
import net.kishonti.benchui.TestSession;
//import net.kishonti.benchui.initialization.RWDatabaseTask;
import net.kishonti.swig.Descriptor;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class CommandLineSession extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {

        BenchmarkApplication.getMinimalProps(context, true);

//		RWDatabaseTask.openCopyBuildDB(context);

        DescriptorFactory factory = ((BenchmarkApplication) BenchmarkApplication.instance)
                .getDescriptorFactory();

        String ids = intent.getExtras().getString("test_ids");
        if (ids == null)
            return;

        List<String> idl = Arrays.asList(ids.split(","));
        if (idl == null)
            return;

        ArrayList<String> selectedTests = new ArrayList<String>(idl);
        if (selectedTests.size() == 0)
            return;

        ArrayList<Descriptor> testdescs = new ArrayList<Descriptor>();
        for (String testId : selectedTests) {
            Descriptor desc = factory.getDescriptorForId(testId,
                    intent.getExtras());
            if (testId.contains("composite")){
                desc.setTestId(testId);
            }
            if (desc == null) {
                Log.v("Error", "Descriptor not found to testid:" + testId);
                return;
            }

            testdescs.add(desc);
        }

        if (testdescs.size() == 0)
            return;

        try {
            MainActivity.testAreStarted = true;
            TestSession.start(context, testdescs, true);
        } catch (IOException e) {
            Log.v("Error", "Failed to start tests!");
        }
    }

}
