/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.benchui;

import org.apache.commons.io.IOUtils;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.lang.reflect.Type;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.List;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import net.kishonti.benchui.model.BenchmarkTestModel;
import net.kishonti.benchui.model.MinimalProps;
import net.kishonti.swig.Descriptor;
import net.kishonti.swig.Result;
import net.kishonti.swig.ResultGroup;
import net.kishonti.testfw.TestUtils;
import net.kishonti.testfw.TfwActivity;
import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonParseException;
import com.google.gson.JsonPrimitive;
import com.google.gson.JsonSerializationContext;
import com.google.gson.JsonSerializer;
import com.google.gson.Gson;
import com.google.gson.stream.JsonReader;
import com.google.gson.stream.JsonWriter;

public class TestSession extends BroadcastReceiver {

	private static final byte[] KEY = {
		0x31, 0x14, (byte) 0xd5, (byte) 0xe2, 0x42, (byte) 0xd4, (byte) 0x83, (byte) 0xd5,
		0x44, (byte) 0xdc, 0x20, 0x65, 0x64, (byte) 0x9b, 0x6f, 0x3d
	};
	//	private static final String FILE_TEST_QUEUE = "test_queue.json";
	//	private static final String FILE_CURRENT_TEST = "test_current.json";
	private static final String FILE_TEST_SESSION = "test_session.json";

	private static boolean cmdsession;

	private static class DescriptorSerializer implements JsonSerializer<Descriptor> {
		public JsonElement serialize(Descriptor src, Type typeOfSrc, JsonSerializationContext context) {
			String s = src.toJsonString();
			return new JsonPrimitive(s);
		}
	}

	private static class DescriptorDeserializer implements JsonDeserializer<Descriptor> {
		public Descriptor deserialize(JsonElement json, Type typeOfT, JsonDeserializationContext context)
				throws JsonParseException {
			Descriptor desc = new Descriptor();
			String s = json.getAsJsonPrimitive().getAsString();
			try {
				if(!desc.fromJsonString(s)) {
					return null;
				}
			} catch (Exception e)
			{
				e.printStackTrace();
				return null;
			}
			return desc;
		}
	}

	private static class SessionState {

		public long session_id;
		public Descriptor current_test;
		public List<Descriptor> remaining_tests;

		public SessionState(List<Descriptor> tests) {
			session_id = System.currentTimeMillis();
			current_test = null;
			remaining_tests = tests;
		}

		public boolean hasRemainingTests() {
			return !(remaining_tests == null || remaining_tests.size() == 0);
		}

		public Descriptor getNextTest() {
			current_test = null;
			if (!remaining_tests.isEmpty()) {
				current_test = remaining_tests.get(0);
				remaining_tests.remove(0);
			}
			return current_test;
		}

	}

	// We should continue if app was force closed in low resource situation
	public static boolean closeBrokenSession(Context context) {
		try {
			SessionState state = loadSessionState(context);
			if (state.current_test != null) {
				Log.i("XXX", "marking result out of memory");
				// mark current_test as failed with Out of memory
				ResultGroup failure = TestUtils.createSingleResultList(TestUtils.createFailedResult(state.current_test.testId(), state.current_test.testId(), "OUT_OF_MEMORY"));
				Utils.saveResultsToFile(context, state.session_id, failure);
				BenchmarkTestModel model = BenchmarkApplication.getModel(context);
				model.newResults(state.session_id, failure);
				state.current_test = null;


				cleanup(context);
				model.endSession(state.session_id);
				BenchmarkApplication.getModel(context).loadUserResults(context);
				return true;
			}
		} catch (IOException e) {
		}
		return false;
	}

	public static void start(Context context, List<Descriptor> tests, boolean cmd) throws IOException {
		cleanup(context);
		createSession(context, tests, cmd);
		startNext(context);
	}

	private static void createSession(Context context, List<Descriptor> tests,boolean cmd) throws IOException {
		SessionState state = new SessionState(tests);

		saveSessionState(context, state);
		cmdsession = cmd;
		BenchmarkApplication.getModel(context).newSession(state.session_id, cmd);
	}

	private static void saveSessionState(Context context, SessionState state) throws IOException {
		GsonBuilder gson = new GsonBuilder();
		gson.registerTypeAdapter(Descriptor.class, new DescriptorDeserializer());
		gson.registerTypeAdapter(Descriptor.class, new DescriptorSerializer());
		JsonWriter writer = openJsonWriter(context, FILE_TEST_SESSION);
		gson.create().toJson(state, SessionState.class, writer);
		writer.close();
	}

	private static SessionState loadSessionState(Context context) throws FileNotFoundException, IOException {
		GsonBuilder gson = new GsonBuilder();
		gson.registerTypeAdapter(Descriptor.class, new DescriptorDeserializer());
		gson.registerTypeAdapter(Descriptor.class, new DescriptorSerializer());
		JsonReader reader = openJsonReader(context, FILE_TEST_SESSION);
		Gson g = gson.create();
		SessionState state = g.fromJson(reader, SessionState.class);
		reader.close();
		Log.i("XXX", "loadSessionState" + g.toJson(state, SessionState.class));
		return state;
	}

	@Override
	public void onReceive(Context context, Intent intent) {
		Bundle b = intent.getExtras();
		boolean closeSession = b.getBoolean("close_session", false);
		BenchmarkTestModel model = BenchmarkApplication.getModel(context);
		Log.i("XXX", "onReceive" + intent);
		try {
			SessionState state = loadSessionState(context);
			ResultGroup results = Utils.getResultsFromIntent(intent);
			Log.i("XXX", "onReceive" + closeSession);
			if (!closeSession) {
				Utils.saveResultsToFile(context, state.session_id, results);

				model.newResults(state.session_id, results);
			} else if(checkResultWasCancelled(results)) {

				model.newResults(state.session_id, results);
			}
			if (state.hasRemainingTests() && !closeSession && results != null && !checkResultWasCancelled(results)) {
				Log.i("XXX", "startNext");
				startNext(context);
			} else {
				Log.i("XXX", "cleanup");
				cleanup(context);

				model.endSession(state.session_id);
				model.loadUserResults(context);
				if(cmdsession) {
					//finishAffinity();
				}
				if (!closeSession) {
					Log.i("XXX", "sendShowResultBroadcast");
					sendShowResultBroadcast(context);
				} else {
					Log.i("XXX", "session closed");
				}
			}
		} catch (IOException e) {
			Log.i("XXX", "no session state");
			sendShowResultBroadcast(context);
		}
	}

	private void sendShowResultBroadcast(Context context) {
		Intent showResult = new Intent("net.kishonti.benchui.ACTION_SHOW_RESULT");
		showResult.addCategory(context.getPackageName());
		showResult.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		context.startActivity(showResult);
		Log.i("XXX", "sending broadcast: " + showResult);
	}

	private static boolean checkResultWasCancelled(ResultGroup results) {
		if(results.results().size() <= 0) return true;
		return results.results().get(0).status() == Result.Status.CANCELLED;
	}

	private static void cleanup(Context context) {
		context.getFileStreamPath(FILE_TEST_SESSION).delete();
	}

	private static void startNext(Context context) throws IOException {
		SessionState state = loadSessionState(context);
		Descriptor test = state.getNextTest();

		String base_path = BenchmarkApplication.instance.getWorkingDir() + "/";
		if (test != null) {
			saveSessionState(context, state);

			if (test.junit() != null && !test.junit().test().equals("")) {
				ComponentName comp = new ComponentName(test.junit().pkg(),
						test.junit().cls());
				Bundle args = new Bundle();
				test.env().setReadPath(base_path + "data/" + test.dataPrefix());
				String config = test.toJsonString();
				args.putString("config", config);
				args.putString("test_name", test.junit().test());
				args.putString("test_id", test.testId());
				boolean started = context
						.startInstrumentation(comp, null, args);
				if (!started) {
					ResultGroup error = TestUtils.createSingleResultList(TestUtils.createFailedResult(test.testId(), test.testId(), "NOT_INSTALLED"));
					Intent intent = Utils.getIntentForResult(error);
					context.sendBroadcast(intent);
				}
			} else {
				String config = test.toJsonString();
				Intent intent = null;
				if (test.testId().contains("composite")){
					intent = new Intent(context, CompositeActivity.class);
				} else {
					intent = new Intent(context, BenchTestActivity.class);
				}

				intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK
							  | Intent.FLAG_ACTIVITY_NO_ANIMATION
							  );

				if (test.testId().contains("composite")){
					config = IOUtils.toString(context.getAssets().open("config/" + test.testId() + ".json"));
				} else {

				}

				intent.putExtra("config", config);
				intent.putExtra("base_path", base_path);

				MinimalProps props = BenchmarkApplication.getMinimalProps(context);

				if(props.appinfo_benchmark_id.equals("gfxbench_gl") && props.appinfo_versioncode >= 40000 && !props.appinfo_packagename.contains("corporate") ) {
					if(props.appinfo_featureset_major == 2) {
						intent.putExtra("preload_libs", "gfxbench40_gl__TO__gfxbench40_gl_es2");
					}
				}


				if(test.testId().contains("battery")) {
					intent.putExtra("brightness", 0.5f);
				}
				if(context.getSharedPreferences("prefs", Context.MODE_PRIVATE).getBoolean("forceBrightness", false)) {
					intent.putExtra("brightness", context.getSharedPreferences("prefs", Context.MODE_PRIVATE).getInt("brightness", 255) / 255.0f);
				}
				context.startActivity(intent);
			}

		}
	}

	private static JsonReader openJsonReader(Context context, String name) throws FileNotFoundException {
		FileInputStream f = context.openFileInput(name);
		InputStream dec = null;//decryptedStream(f); // TODO: enable
		JsonReader reader = new JsonReader(new InputStreamReader(dec != null ? dec : f));
		return reader;
	}

	@SuppressLint("TrulyRandom")
	private static OutputStream encryptedStream(OutputStream os) {
		try {
			Cipher chipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
			SecretKeySpec keySpec = new SecretKeySpec(KEY, "AES");
			IvParameterSpec ivSpec = new IvParameterSpec(KEY);
			chipher.init(Cipher.ENCRYPT_MODE, keySpec, ivSpec);
			CipherOutputStream enc = new CipherOutputStream(os, chipher);
			return enc;
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		} catch (NoSuchPaddingException e) {
			e.printStackTrace();
		} catch (InvalidKeyException e) {
			e.printStackTrace();
		} catch (InvalidAlgorithmParameterException e) {
			e.printStackTrace();
		}
		return null;
	}

	private static JsonWriter openJsonWriter(Context context, String name) throws FileNotFoundException {
		FileOutputStream f = context.openFileOutput(name, Context.MODE_PRIVATE);
		OutputStream enc = null;//encryptedStream(f); // TODO: enable
		JsonWriter writer = new JsonWriter(new OutputStreamWriter(enc != null ? enc : f));
		return writer;
	}

	@SuppressWarnings("unused")
	private static InputStream decryptedStream(InputStream is) {
		try {
			Cipher chipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
			SecretKeySpec keySpec = new SecretKeySpec(KEY, "AES");
			IvParameterSpec ivSpec = new IvParameterSpec(KEY);
			chipher.init(Cipher.DECRYPT_MODE, keySpec, ivSpec);
			CipherInputStream dec = new CipherInputStream(is, chipher);
			return dec;
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		} catch (NoSuchPaddingException e) {
			e.printStackTrace();
		} catch (InvalidKeyException e) {
			e.printStackTrace();
		} catch (InvalidAlgorithmParameterException e) {
			e.printStackTrace();
		}
		return null;
	}


}
