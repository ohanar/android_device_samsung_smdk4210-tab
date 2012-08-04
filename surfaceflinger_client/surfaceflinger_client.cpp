/*
 * Copyright (C) 2012, The CyanogenMod Project
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

class SkDeviceFactory;
class SkBitmap;
class SkCanvas;
class SkPaint;

class SkTextBox {
public:
    SkTextBox();

    enum Mode {
        kOneLine_Mode,
        kLineBreak_Mode,

        kModeCount
    };

    enum SpacingAlign {
        kStart_SpacingAlign,
        kCenter_SpacingAlign,
        kEnd_SpacingAlign,

        kSpacingAlignCount
    };
};

typedef float SkScalar;

extern "C" void _ZN8SkCanvasC1EP15SkDeviceFactory(SkDeviceFactory* factory) {
}

extern "C" void _ZN8SkCanvas15setBitmapDeviceERK8SkBitmapb(const SkBitmap& bitmap, bool forLayer) {
}

extern "C" void _ZN9SkTextBoxC1Ev() {
}

extern "C" void _ZN9SkTextBox7setModeENS_4ModeE(SkTextBox::Mode) {
}

extern "C" void _ZN9SkTextBox15setSpacingAlignENS_12SpacingAlignE(SkTextBox::SpacingAlign) {
}

extern "C" void _ZN9SkTextBox6setBoxEffff(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom) {
}

extern "C" void _ZN9SkTextBox10setSpacingEff(SkScalar left, SkScalar right) {
}

extern "C" void _ZN9SkTextBox4drawEP8SkCanvasPKcjRK7SkPaint(SkCanvas*, char const* foo1, unsigned int foo2, SkPaint const&) {
}
