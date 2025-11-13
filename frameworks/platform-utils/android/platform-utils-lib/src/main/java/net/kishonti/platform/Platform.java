/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
package net.kishonti.platform;

public class Platform {
   static {
      System.loadLibrary("c++_shared");
      System.loadLibrary("platform_utils");
   }
   public static void setApplicationContext(android.content.Context context) {
      setApplicationContext(context, context.getAssets());
   }

   private static native void setApplicationContext(android.content.Context context, android.content.res.AssetManager asset_mgr);
}
