/*
 * Copyright (C) 2012 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.cyanogenmod.settings.device;

import android.app.ActivityManagerNative;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.UserHandle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceManager;

public class DockAudio implements OnPreferenceChangeListener {

    public static boolean isSupported() {
        return true;
    }

    /**
     * Restore dock audio setting from SharedPreferences.
     * @param context       The context to read the SharedPreferences from
     */
    public static void restore(Context context) {
        if (!isSupported()) {
            return;
        }

        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);
        Intent i = new Intent("com.cyanogenmod.settings.SamsungDock");
        i.putExtra("data", sharedPrefs.getBoolean(DeviceSettings.KEY_DOCK_AUDIO, false) ? "1" : "0");
        ActivityManagerNative.broadcastStickyIntent(i, null, UserHandle.USER_ALL);
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        Intent i = new Intent("com.cyanogenmod.settings.SamsungDock");
        i.putExtra("data", ((CheckBoxPreference)preference).isChecked() ? "1" : "0");
        ActivityManagerNative.broadcastStickyIntent(i, null, UserHandle.USER_ALL);
        return true;
    }

}
