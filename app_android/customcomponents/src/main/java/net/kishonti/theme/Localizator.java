/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.theme;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import net.kishonti.customcomponents.R;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;
import org.xmlpull.v1.XmlPullParserException;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Locale;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

public class Localizator {

	private volatile static Localizator mInstance;
	private boolean XMLFileIsLoaded;
	private boolean DefaultXMLFileIsLoaded;
	private HashMap<String, String> mDefaultStrings;
	private HashMap<String, String> mStrings;
	private InputStreamReader mInputLanguage;

	public static Localizator getInstance() {
		if (mInstance == null) {
			synchronized (Localizator.class) {
				if (mInstance == null) {
					mInstance = new Localizator();
				}
			}
		}
		return mInstance;
	}

	public void loadXML(Context ctx, boolean loadFallback, boolean reload) throws XmlPullParserException, IOException {
		if (reload) {
			XMLFileIsLoaded = false;
			DefaultXMLFileIsLoaded = false;
			loadXML(ctx, true);
			loadXML(ctx, false);
		} else {
			loadXML(ctx, loadFallback);
		}
	}

	public void loadXML(Context ctx,  boolean loadFallback) throws XmlPullParserException, IOException {
		if ((!XMLFileIsLoaded && !loadFallback) || (!DefaultXMLFileIsLoaded && loadFallback)) {
			String current_language = Locale.getDefault().getLanguage();
			if(loadFallback) current_language = "en";
			

			String workingDir = ctx.getSharedPreferences("mainPreferences", Context.MODE_PRIVATE).getString("bigDataDir", "");
			AssetManager assetManager = ctx.getAssets();
			
			final String filename = "strings_" + current_language + ".xml";
			InputStream in;
			try {
				File f = new File(workingDir + "/localization/" + filename);
				if (f.exists() == true) {
					in = new FileInputStream(f);
				} else {
					in = assetManager.open("localization/" + filename);
				}

				mInputLanguage = new InputStreamReader(in, "UTF-8");
				if(!loadFallback)
					XMLFileIsLoaded = true;
				else
					DefaultXMLFileIsLoaded = true;
				
			} catch (Exception e1) {
				in = assetManager.open("localization/strings_en.xml");
				mInputLanguage = new InputStreamReader(in, "UTF-8");
				if(!loadFallback)
					XMLFileIsLoaded = true;
				else
					DefaultXMLFileIsLoaded = true;
			}
			SAXParserFactory saxPF = SAXParserFactory.newInstance();
			try {
				SAXParser saxP = saxPF.newSAXParser();
				XMLReader xmlR = saxP.getXMLReader();
				Parser myXMLHandler = new Parser(ctx.getString(R.string.app_product_id));
				xmlR.setContentHandler(myXMLHandler);
				InputSource inputSource = new InputSource(mInputLanguage);
				inputSource.setEncoding("UTF-8");
				xmlR.parse(inputSource);

				if(loadFallback) {
					mDefaultStrings = myXMLHandler.getXMLData();
				} else {
					mStrings = myXMLHandler.getXMLData();
				}
			} catch (ParserConfigurationException e) {
				Log.e("Localizator", "", e);
			} catch (SAXException e) {
				Log.e("Localizator", "", e);
			}
		}
	}

	public static String getString(Context ctx, String attribName) {
		try {
			getInstance().loadXML(ctx, true);
			getInstance().loadXML(ctx, false);
		} catch (FileNotFoundException e) {
			Log.e("Localizator", Log.getStackTraceString(e).split("\n")[0]);
		} catch (XmlPullParserException e) {
			Log.e("Localizator", "", e);
		} catch (IOException e) {
			Log.e("Localizator", "", e);
		}

		String localizedString = attribName;
		if (getInstance().mStrings != null) {
			localizedString = getInstance().mStrings.get(attribName);
			if (localizedString == null) {
				localizedString = attribName;
			}
		}
		if (getInstance().mDefaultStrings != null && localizedString.equals(attribName)) {
			localizedString = getInstance().mDefaultStrings.get(attribName);
			if (localizedString == null) {
				localizedString = attribName;
			}
		}
		
		return localizedString;
	}

	public String localize(Context ctx, String attribName) {
		return Localizator.getString(ctx, attribName);
	}


	private static class Parser extends DefaultHandler {

		private StringBuffer translatedString;
		private boolean elementOn;
		private HashMap<String, String> data;
		private String key;
		private String product_id;

		public HashMap<String, String> getXMLData() {
			return data;
		}
		
		public Parser(String product_id) {
			super();
			this.product_id = product_id;
		}

		@Override
		public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
			elementOn = true;
			if (localName.equals("string")) {
				if (data == null) {
					data = new HashMap<String, String>();
				}
				// if product attribute is set only use the key if product_id matches
				String product_attr = attributes.getValue("product");
				String desktop_attr = attributes.getValue("is_desktop");
				
				if ((product_attr == null || product_attr.equals(product_id)) && 
					(desktop_attr == null || desktop_attr.equals("false"))) {
					String attributeValue = attributes.getValue("name");
					key = attributeValue;
				} else {
					// ignore translated string. product_attr set but does not match
					key = null;
				}
			}
		}

		@Override
		public void endElement(String uri, String localName, String qName) throws SAXException {
			elementOn = false;
			if (key != null && localName.equalsIgnoreCase("string")) {
				data.put(key, translatedString.toString());
			}
		}

		@Override
		public void characters(char[] ch, int start, int length) throws SAXException {
			if (elementOn) {
				translatedString = new StringBuffer(new String(ch, start, length));
				elementOn = false;
			} else {
				translatedString.append(new String(ch, start, length));
			}
		}
	}
}
