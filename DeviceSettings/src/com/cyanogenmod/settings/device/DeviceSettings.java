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

import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;

public class DeviceSettings extends PreferenceActivity  {

    public static final String KEY_SCREEN_CATEGORY = "category_screen";
    public static final String KEY_HAPTIC_CATEGORY = "category_haptic";
    public static final String KEY_RADIO_CATEGORY = "category_radio";
    public static final String KEY_DOCK_CATEGORY = "category_dock";

    public static final String KEY_MDNIE_MODE = "mdnie_mode";
    public static final String KEY_MDNIE_SCENARIO = "mdnie_scenario";
    public static final String KEY_MDNIE_NEGATIVE = "mdnie_negative";
    public static final String KEY_MDNIE_OUTDOOR = "mdnie_outdoor";
    public static final String KEY_TOUCHSCREEN_SENSITIVITY = "touchscreen_sensitivity";
    public static final String KEY_VIBRATOR_INTENSITY = "vibrator_intensity";
    public static final String KEY_HSPA = "hspa";
    public static final String KEY_DOCK_AUDIO = "dock_audio";

    private ListPreference mmDNIeMode;
    private ListPreference mmDNIeScenario;
    private CheckBoxPreference mmDNIeNegative;
    private CheckBoxPreference mmDNIeOutdoor;
    private ListPreference mTouchscreenSensitivity;
    private ListPreference mVibratorIntensity;
    private ListPreference mHspa;
    private CheckBoxPreference mDockAudio;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.main);

        PreferenceCategory category;
        Boolean disableCategory;

        category = (PreferenceCategory) getPreferenceScreen().findPreference(KEY_SCREEN_CATEGORY);
        disableCategory = true;

        mmDNIeMode = (ListPreference) findPreference(KEY_MDNIE_MODE);
        if (mDNIeMode.isSupported()) {
            mmDNIeMode.setOnPreferenceChangeListener(new mDNIeMode());
            disableCategory = false;
        } else
            category.removePreference(mmDNIeMode);

        mmDNIeScenario = (ListPreference) findPreference(KEY_MDNIE_SCENARIO);
        if (mDNIeScenario.isSupported()) {
            mmDNIeScenario.setOnPreferenceChangeListener(new mDNIeScenario());
            disableCategory = false;
        } else
            category.removePreference(mmDNIeScenario);

        mmDNIeNegative = (CheckBoxPreference) findPreference(KEY_MDNIE_NEGATIVE);
        if (mDNIeNegative.isSupported()) {
            mmDNIeNegative.setOnPreferenceChangeListener(new mDNIeNegative());
            disableCategory = false;
        } else
            category.removePreference(mmDNIeNegative);

        mmDNIeOutdoor = (CheckBoxPreference) findPreference(KEY_MDNIE_OUTDOOR);
        if (mDNIeOutdoor.isSupported()) {
            mmDNIeOutdoor.setOnPreferenceChangeListener(new mDNIeOutdoor());
            disableCategory = false;
        } else
            category.removePreference(mmDNIeOutdoor);

        mTouchscreenSensitivity = (ListPreference) findPreference(KEY_TOUCHSCREEN_SENSITIVITY);
        if (TouchscreenSensitivity.isSupported()) {
            mTouchscreenSensitivity.setOnPreferenceChangeListener(new TouchscreenSensitivity());
            disableCategory = false;
        } else
            category.removePreference(mTouchscreenSensitivity);

        if (disableCategory)
            getPreferenceScreen().removePreference(category);

        category = (PreferenceCategory) getPreferenceScreen().findPreference(KEY_HAPTIC_CATEGORY);
        disableCategory = true;

        mVibratorIntensity = (ListPreference) findPreference(KEY_VIBRATOR_INTENSITY);
        if (VibratorIntensity.isSupported()) {
            mVibratorIntensity.setOnPreferenceChangeListener(new VibratorIntensity(this));
            disableCategory = false;
        } else
            category.removePreference(mVibratorIntensity);

        if (disableCategory)
            getPreferenceScreen().removePreference(category);

        category = (PreferenceCategory) getPreferenceScreen().findPreference(KEY_RADIO_CATEGORY);
        disableCategory = true;

        mHspa = (ListPreference) findPreference(KEY_HSPA);
        if (Hspa.isSupported()) {
            mHspa.setOnPreferenceChangeListener(new Hspa(this));
            disableCategory = false;
        } else
            category.removePreference(mHspa);

        if (disableCategory)
            getPreferenceScreen().removePreference(category);

        category = (PreferenceCategory) getPreferenceScreen().findPreference(KEY_DOCK_CATEGORY);
        disableCategory = true;

        mDockAudio = (CheckBoxPreference) findPreference(KEY_DOCK_AUDIO);
        if (DockAudio.isSupported()) {
            mDockAudio.setOnPreferenceChangeListener(new DockAudio());
            disableCategory = false;
        } else
            category.removePreference(mDockAudio);

        if (disableCategory)
            getPreferenceScreen().removePreference(category);
    }
}
