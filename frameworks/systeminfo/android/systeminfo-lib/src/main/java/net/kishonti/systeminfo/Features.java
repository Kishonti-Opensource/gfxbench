/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.systeminfo;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import android.content.Context;
import android.content.pm.FeatureInfo;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.telephony.TelephonyManager;

public class Features extends InfoProvider implements Runnable {

	private static List<Sensor> mSensors;
	private static boolean pedometer;
	private static boolean thermometer;
	private static boolean altimeter;
	private static SensorManager mSensorManager;

	private static boolean proximitySensor;
	private static boolean lightSensor;
	private static boolean gyroscopeSensor;
	private static boolean compassSensor;
	private static boolean barometerSensor;

	private static boolean accelerometerSensor;
	private static boolean wifi;
	private static boolean nfc;
	private static boolean gps;
	private static boolean bluetooth;

	private static boolean frontCamera;
	private static boolean backCamera;

	private int simCardNumber;
	private ArrayList<String> sysProps;
	private Map<String, String[]> sensorsMap;
	private Map<String, String> avaiableFeatures;
	private Map<String, String> getprops;

	private int newtworkType;
	private int phoneType;
	private int simState;
	private String newtworkTypeString = "";
	private String phoneTypeString = "";
	private String simStateString = "";
	private List<String> mFeatures;

	public Features(Context context) {
		super(context);
	}

	@Override
	public void run() {
		PackageManager packageManager = context.getPackageManager();
		FeatureInfo[] featureList = packageManager.getSystemAvailableFeatures();
		mFeatures = new ArrayList<String>();
		for (int i = 0; (featureList != null) && (i < featureList.length); i++) {
			FeatureInfo info = featureList[i];
			if (info.name != null) {
				mFeatures.add(info.name);
			}
		}

		sysProps = new ArrayList<String>();
		getprops = new HashMap<String, String>();
		avaiableFeatures = new LinkedHashMap<String, String>();
		sensorsMap = new HashMap<String, String[]>();

		if (context != null) {

			try {
				Process proc = Runtime.getRuntime().exec(new String[] { "/system/bin/getprop" });
				BufferedReader reader = new BufferedReader(new InputStreamReader(proc.getInputStream()));
				String line = null;
				while ((line = reader.readLine()) != null) {
					line = line.trim();
					int keyEndsAt = line.indexOf(": ");
					String key = line.substring(0, keyEndsAt);
					String value = line.substring(keyEndsAt + 2);
					getprops.put(key, value);
				}
				reader.close();
			} catch (Throwable t) {
				t.printStackTrace();
			}

			for (int i = 0; i < mFeatures.size(); i++) {
				String key = "" + i;
				String value = mFeatures.get(i);
				avaiableFeatures.put(key, value);
			}

			mSensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
			mSensors = mSensorManager.getSensorList(Sensor.TYPE_ALL);

			for (int i = 0; i < mSensors.size(); i++) {
				Sensor s = mSensors.get(i);
				String key = "" + i;
				String[] values = new String[2];
				values[0] = s.getName();
				values[1] = "" + s.getType();
				sensorsMap.put(key, values);
			}

			accelerometerSensor = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_SENSOR_ACCELEROMETER);
			gyroscopeSensor = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_SENSOR_GYROSCOPE);
			proximitySensor = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_SENSOR_PROXIMITY);
			lightSensor = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_SENSOR_LIGHT);
			compassSensor = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_SENSOR_COMPASS);
			barometerSensor = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_SENSOR_BAROMETER);

			wifi = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_WIFI);
			nfc = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_NFC);
			gps = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_LOCATION_GPS);
			bluetooth = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH);

			backCamera = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA);
			frontCamera = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA_FRONT);
/*
			TelephonyManager tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
			newtworkType = tm.getNetworkType();
			switch (newtworkType) {
				case TelephonyManager.NETWORK_TYPE_UNKNOWN:
					newtworkTypeString = "NETWORK_TYPE_UNKNOWN";
					break;
				case TelephonyManager.NETWORK_TYPE_GPRS:
					newtworkTypeString = "NETWORK_TYPE_GPRS";
					break;
				case TelephonyManager.NETWORK_TYPE_EDGE:
					newtworkTypeString = "NETWORK_TYPE_EDGE";
					break;
				case TelephonyManager.NETWORK_TYPE_UMTS:
					newtworkTypeString = "NETWORK_TYPE_UMTS";
					break;
				case TelephonyManager.NETWORK_TYPE_HSDPA:
					newtworkTypeString = "NETWORK_TYPE_HSDPA";
					break;
				case TelephonyManager.NETWORK_TYPE_HSUPA:
					newtworkTypeString = "NETWORK_TYPE_HSUPA";
					break;
				case TelephonyManager.NETWORK_TYPE_HSPA:
					newtworkTypeString = "NETWORK_TYPE_HSPA";
					break;
				case TelephonyManager.NETWORK_TYPE_CDMA:
					newtworkTypeString = "NETWORK_TYPE_CDMA";
					break;
				case TelephonyManager.NETWORK_TYPE_EVDO_0:
					newtworkTypeString = "NETWORK_TYPE_EVDO_0";
					break;
				case TelephonyManager.NETWORK_TYPE_EVDO_A:
					newtworkTypeString = "NETWORK_TYPE_EVDO_A";
					break;
				case TelephonyManager.NETWORK_TYPE_EVDO_B:
					newtworkTypeString = "NETWORK_TYPE_EVDO_B";
					break;
				case TelephonyManager.NETWORK_TYPE_1xRTT:
					newtworkTypeString = "NETWORK_TYPE_1xRTT";
					break;
				case TelephonyManager.NETWORK_TYPE_IDEN:
					newtworkTypeString = "NETWORK_TYPE_IDEN";
					break;
				case 13:
					newtworkTypeString = "NETWORK_TYPE_LTE";
					break; // api level 11
				case 14:
					newtworkTypeString = "NETWORK_TYPE_EHRPD";
					break; // api level 11
				case 15:
					newtworkTypeString = "NETWORK_TYPE_HSPAP";
					break; // api level 13
				default:
					newtworkTypeString = "undefinied";
			}

			phoneType = tm.getPhoneType();
			switch (phoneType) {
				case TelephonyManager.PHONE_TYPE_NONE:
					phoneTypeString = "PHONE_TYPE_NONE";
					break;
				case TelephonyManager.PHONE_TYPE_GSM:
					phoneTypeString = "PHONE_TYPE_GSM";
					break;
				case TelephonyManager.PHONE_TYPE_CDMA:
					phoneTypeString = "PHONE_TYPE_CDMA";
					break;
				case 3:
					phoneTypeString = "PHONE_TYPE_SIP";
					break; // api level 11
				default:
					phoneTypeString = "undefinied";
			}
			*/
			//temporally solution: getSimState() function cause crash on Android N (Nexus 6, Intel device)
			simStateString = "undefinied";
			boolean validSimState = false;

			//simState = tm.getSimState();
			//boolean validSimState = true; // to avoid forward compatibility issues
			//switch (simState) {
			//	case TelephonyManager.SIM_STATE_UNKNOWN:
			//		simStateString = "SIM_STATE_UNKNOWN";
			//		break; // eg flight mode
			//	case TelephonyManager.SIM_STATE_ABSENT:
			//		simStateString = "SIM_STATE_ABSENT";
			//		break;
			//	case TelephonyManager.SIM_STATE_PIN_REQUIRED:
			//		simStateString = "SIM_STATE_PIN_REQUIRED";
			//		break;
			//	case TelephonyManager.SIM_STATE_PUK_REQUIRED:
			//		simStateString = "SIM_STATE_PUK_REQUIRED";
			//		break;
			//	case TelephonyManager.SIM_STATE_NETWORK_LOCKED:
			//		simStateString = "SIM_STATE_NETWORK_LOCKED";
			//		break;
			//	case TelephonyManager.SIM_STATE_READY:
			//		simStateString = "SIM_STATE_READY";
			//		break;
			//	default:
			//		simStateString = "undefinied";
			//		validSimState = false;
			//}

			if ((validSimState && (simState != TelephonyManager.SIM_STATE_UNKNOWN))
					|| ((phoneType == TelephonyManager.PHONE_TYPE_GSM) || (phoneType == TelephonyManager.PHONE_TYPE_CDMA))) {
				simCardNumber = 1;
				if (SysFileReaderUtil.fetchInfo(sysProps, "[gsm.network.type.2]", ':') != null) {
					simCardNumber = 2;
				}
			}
			// other interesting sysProps
			// [gsm.network.type.2] [gsm.network.type] [ro.carrier] [gsm.sim.state]
		}
	}

	public String getRawData() {
		StringBuilder sb = new StringBuilder();
		sb.append("/** Sensors **/").append("\n");
		sb.append("FEATURE_SENSOR_ACCELEROMETER: ").append(accelerometerSensor).append("\n");
		sb.append("FEATURE_SENSOR_GYROSCOPE: ").append(gyroscopeSensor).append("\n");
		sb.append("FEATURE_SENSOR_PROXIMITY: ").append(proximitySensor).append("\n");
		sb.append("FEATURE_SENSOR_LIGHT: ").append(lightSensor).append("\n");
		sb.append("FEATURE_SENSOR_COMPASS: ").append(compassSensor).append("\n");
		sb.append("FEATURE_SENSOR_BAROMETER: ").append(barometerSensor).append("\n");
		sb.append("FEATURE_WIFI: ").append(wifi).append("\n");
		sb.append("FEATURE_NFC: ").append(nfc).append("\n");
		sb.append("FEATURE_LOCATION_GPS: ").append(gps).append("\n");
		sb.append("FEATURE_BLUETOOTH: ").append(bluetooth).append("\n");
		sb.append("FEATURE_CAMERA: ").append(backCamera).append("\n");
		sb.append("FEATURE_CAMERA_FRONT: ").append(frontCamera).append("\n");
		sb.append("Context.getSystemService(Context.TELEPHONY_SERVICE).getNetworkType(): ").append(newtworkType).append("\n");
		sb.append("Context.getSystemService(Context.TELEPHONY_SERVICE).getNetworkType().asString: ").append(newtworkTypeString).append("\n");
		sb.append("Context.getSystemService(Context.TELEPHONY_SERVICE).getPhoneType(): ").append(phoneType).append("\n");
		sb.append("Context.getSystemService(Context.TELEPHONY_SERVICE).getPhoneType().asString: ").append(phoneTypeString).append("\n");
		sb.append("Context.getSystemService(Context.TELEPHONY_SERVICE).getSimState(): ").append(simState).append("\n");
		sb.append("Context.getSystemService(Context.TELEPHONY_SERVICE).getSimState().asString: ").append(simStateString).append("\n");

		sb.append("-- properties (/system/bin/getprop): ").append("\n");
		for (int i = 0; (sysProps != null) && (i < sysProps.size()); i++) {
			sb.append((String) sysProps.get(i)).append("\n");
		}

		sb.append("/** END **/").append("\n");

		return sb.toString();
	}

	public Map<String, String> getSimProps() {
		final Map<String, String> props = new HashMap<String, String>();

		props.put("telephony/Context.getSystemService(Context.TELEPHONY_SERVICE).getNetworkType()", "" + newtworkType);
		props.put("telephony/Context.getSystemService(Context.TELEPHONY_SERVICE).getNetworkType().asString", "" + newtworkTypeString);
		props.put("telephony/Context.getSystemService(Context.TELEPHONY_SERVICE).getPhoneType()", "" + phoneType);
		props.put("telephony/Context.getSystemService(Context.TELEPHONY_SERVICE).getPhoneType().asString", "" + phoneTypeString);
		props.put("telephony/Context.getSystemService(Context.TELEPHONY_SERVICE).getSimState()", "" + simState);
		props.put("telephony/Context.getSystemService(Context.TELEPHONY_SERVICE).getSimState().asString", "" + simStateString);

		return props;
	}

	public Map<String, String[]> getSensors() {
		return sensorsMap;
	}

	public Map<String, String> runGetprop() {
		return getprops;
	}

	public boolean haveWifi() {
		return wifi;
	}

	public boolean haveGPS() {
		return gps;
	}

	public boolean haveBlueTooth() {
		return bluetooth;
	}

	public boolean haveNFC() {
		return nfc;
	}

	public boolean haveBackCamera() {
		return backCamera;
	}

	public boolean haveFrontCamera() {
		return frontCamera;
	}

	public boolean haveSimCards() {
		return simCardNumber > 0;
	}

	public boolean haveAccelerometer() {
		return accelerometerSensor;
	}

	public boolean havePedometer() {
		return pedometer;
	}

	public boolean haveThermometer() {
		return thermometer;
	}

	public boolean haveAlimeter() {
		return altimeter;
	}

	public boolean haveBarometer() {
		return barometerSensor;
	}

	public boolean haveGyroscope() {
		return gyroscopeSensor;
	}

	public boolean haveCompass() {
		return compassSensor;
	}

	public boolean haveProximity() {
		return proximitySensor;
	}

	public boolean haveLightsensor() {
		return lightSensor;
	}

	public Map<String, String> runGetAvaiableFeatures() {
		return avaiableFeatures;
	}

}
