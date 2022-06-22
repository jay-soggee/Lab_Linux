/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "image_provider.h"

/*
 * The sample requires the following third-party libraries to be installed and
 * configured:
 *
 * Arducam
 * -------
 * 1. Download https://github.com/ArduCAM/Arduino and copy its `ArduCAM`
 *    subdirectory into `Arduino/libraries`. Commit #e216049 has been tested
 *    with this code.
 * 2. Edit `Arduino/libraries/ArduCAM/memorysaver.h` and ensure that
 *    "#define OV2640_MINI_2MP_PLUS" is not commented out. Ensure all other
 *    defines in the same section are commented out.
 *
 * JPEGDecoder
 * -----------
 * 1. Install "JPEGDecoder" 1.8.0 from the Arduino library manager.
 * 2. Edit "Arduino/Libraries/JPEGDecoder/src/User_Config.h" and comment out
 *    "#define LOAD_SD_LIBRARY" and "#define LOAD_SDFAT_LIBRARY".
 */

#if defined(ARDUINO) && !defined(ARDUINO_ARDUINO_NANO33BLE)
#define ARDUINO_EXCLUDE_CODE
#endif  // defined(ARDUINO) && !defined(ARDUINO_ARDUINO_NANO33BLE)

#ifndef ARDUINO_EXCLUDE_CODE

// Required by Arducam library
#include <SPI.h>
#include <Wire.h>
#include <memorysaver.h>
// Arducam library
#include <ArduCAM.h>
// JPEGDecoder library
#include <JPEGDecoder.h>

// Checks that the Arducam library has been correctly configured
#if !(defined OV2640_MINI_2MP_PLUS)
#error Please select the hardware platform and camera module in the Arduino/libraries/ArduCAM/memorysaver.h
#endif

// The size of our temporary buffer for holding
// JPEG data received from the Arducam module
#define MAX_JPEG_BYTES 4096
// The pin connected to the Arducam Chip Select
#define CS 7

// Camera library instance
ArduCAM myCAM(OV2640, CS);
// Temporary buffer for holding JPEG data from camera
uint8_t jpeg_buffer[MAX_JPEG_BYTES] = {0};
// Length of the JPEG data currently in the buffer
uint32_t jpeg_length = 0;

// Get the camera module ready
TfLiteStatus InitCamera(tflite::ErrorReporter* error_reporter) {
  TF_LITE_REPORT_ERROR(error_reporter, "Attempting to start Arducam");
  // Enable the Wire library
  Wire.begin();
  // Configure the CS pin
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  // initialize SPI
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  // Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
  // Test whether we can communicate with Arducam via SPI
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  uint8_t test;
  test = myCAM.read_reg(ARDUCHIP_TEST1);
  if (test != 0x55) {
    TF_LITE_REPORT_ERROR(error_reporter, "Can't communicate with Arducam");
    delay(1000);
    return kTfLiteError;
  }
  // Use JPEG capture mode, since it allows us to specify
  // a resolution smaller than the full sensor frame
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  // Specify the smallest possible resolution
  myCAM.OV2640_set_JPEG_size(OV2640_160x120);
  delay(100);
  return kTfLiteOk;
}

// Begin the capture and wait for it to finish
TfLiteStatus PerformCapture(tflite::ErrorReporter* error_reporter) {
  TF_LITE_REPORT_ERROR(error_reporter, "Starting capture");
  // Make sure the buffer is emptied before each capture
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  // Start capture
  myCAM.start_capture();
  // Wait for indication that it is done
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
  }
  TF_LITE_REPORT_ERROR(error_reporter, "Image captured");
  delay(50);
  // Clear the capture done flag
  myCAM.clear_fifo_flag();
  return kTfLiteOk;
}

// Read data from the camera module into a local buffer
TfLiteStatus ReadData(tflite::ErrorReporter* error_reporter) {
  // This represents the total length of the JPEG data
  jpeg_length = myCAM.read_fifo_length();
  TF_LITE_REPORT_ERROR(error_reporter, "Reading %d bytes from Arducam",
                       jpeg_length);
  // Ensure there's not too much data for our buffer
  if (jpeg_length > MAX_JPEG_BYTES) {
    TF_LITE_REPORT_ERROR(error_reporter, "Too many bytes in FIFO buffer (%d)",
                         MAX_JPEG_BYTES);
    return kTfLiteError;
  }
  if (jpeg_length == 0) {
    TF_LITE_REPORT_ERROR(error_reporter, "No data in Arducam FIFO buffer");
    return kTfLiteError;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  for (int index = 0; index < jpeg_length; index++) {
    jpeg_buffer[index] = SPI.transfer(0x00);
  }
  delayMicroseconds(15);
  TF_LITE_REPORT_ERROR(error_reporter, "Finished reading");
  myCAM.CS_HIGH();
  return kTfLiteOk;
}

// Decode the JPEG image, crop it, and convert it to greyscale
TfLiteStatus DecodeAndProcessImage(tflite::ErrorReporter* error_reporter,
                                   int image_width, int image_height,
                                   int8_t* image_data) {
  TF_LITE_REPORT_ERROR(error_reporter,
                       "Decoding JPEG and converting to greyscale");
  // Parse the JPEG headers. The image will be decoded as a sequence of Minimum
  // Coded Units (MCUs), which are 16x8 blocks of pixels.
  JpegDec.decodeArray(jpeg_buffer, jpeg_length);

  // Crop the image by keeping a certain number of MCUs in each dimension
  const int keep_x_mcus = image_width / JpegDec.MCUWidth;
  const int keep_y_mcus = image_height / JpegDec.MCUHeight;



  image_width = 100;
  image_height = 100;
  int8_t my_image[image_width * image_height];



  // Calculate how many MCUs we will throw away on the x axis
  const int skip_x_mcus = JpegDec.MCUSPerRow - keep_x_mcus;
  // Roughly center the crop by skipping half the throwaway MCUs at the
  // beginning of each row
  const int skip_start_x_mcus = skip_x_mcus / 2;
  // Index where we will start throwing away MCUs after the data
  const int skip_end_x_mcu_index = skip_start_x_mcus + keep_x_mcus;
  // Same approach for the columns
  const int skip_y_mcus = JpegDec.MCUSPerCol - keep_y_mcus;
  const int skip_start_y_mcus = skip_y_mcus / 2;
  const int skip_end_y_mcu_index = skip_start_y_mcus + keep_y_mcus;

  // Pointer to the current pixel
  uint16_t* pImg;
  // Color of the current pixel
  uint16_t color;

  // Loop over the MCUs
  while (JpegDec.read()) {
    // Skip over the initial set of rows
    if (JpegDec.MCUy < (skip_start_y_mcus)) {
      continue;
    }
    // Skip if we're on a column that we don't want
    if (JpegDec.MCUx < (skip_start_x_mcus) ||
        JpegDec.MCUx >= (skip_end_x_mcu_index + 3)) {
      continue;
    }
    // Skip if we've got all the rows we want
    if (JpegDec.MCUy >= skip_end_y_mcu_index + 4) {
      continue;
    }
    // Pointer to the current pixel
    pImg = JpegDec.pImage;

    // The x and y indexes of the current MCU, ignoring the MCUs we skip
    int relative_mcu_x = JpegDec.MCUx - skip_start_x_mcus;
    int relative_mcu_y = JpegDec.MCUy - skip_start_y_mcus;

    // The coordinates of the top left of this MCU when applied to the output
    // image
    int x_origin = relative_mcu_x * JpegDec.MCUWidth;
    int y_origin = relative_mcu_y * JpegDec.MCUHeight;

    // Loop through the MCU's rows and columns
    for (int mcu_row = 0; mcu_row < JpegDec.MCUHeight; mcu_row++) {
      // The y coordinate of this pixel in the output index
      int current_y = y_origin + mcu_row;
      for (int mcu_col = 0; mcu_col < JpegDec.MCUWidth; mcu_col++) {
        // Read the color of the pixel as 16-bit integer
        color = *pImg++;
        // Extract the color values (5 red bits, 6 green, 5 blue)
        uint8_t r, g, b;
        r = ((color & 0xF800) >> 11) * 8;
        g = ((color & 0x07E0) >> 5) * 4;
        b = ((color & 0x001F) >> 0) * 8;
        // Convert to grayscale by calculating luminance
        // See https://en.wikipedia.org/wiki/Grayscale for magic numbers
        float gray_value = (0.2126 * r) + (0.7152 * g) + (0.0722 * b);

        // Convert to signed 8-bit integer by subtracting 128.
        gray_value -= 128;

        // The x coordinate of this pixel in the output image
        int current_x = x_origin + mcu_col;
        // The index of this pixel in our flat output buffer
        int index = (current_y * image_width) + current_x;
        my_image[index] = static_cast<int8_t>(gray_value);
      }
    }
  }

  

  for (int i = 0; i < 56; i += 2) {
    int8_t min;
    for (int j = 0; j < 56; j += 2) {
      min = my_image[i * image_width + j];
      if (min > my_image[i * image_width + j + 1]) min = my_image[i * image_width + j + 1];
      if (min > my_image[(i + 1) * image_width + j]) min = my_image[(i + 1) * image_width + j];
      if (min > my_image[(i + 1) * image_width + j + 1]) min = my_image[(i + 1) * image_width + j + 1];
      image_data[(i / 2) * 28 + j / 2] = min > 50 ? min : -120;
    }
  }
    


  for(int i = 0; i < 28; i++) {
    TF_LITE_REPORT_ERROR(error_reporter, 
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ;", 
      image_data[i * 28 + 0] < 0 ? 1  : 8,
      image_data[i * 28 + 1] < 0 ? 1  : 8,
      image_data[i * 28 + 2] < 0 ? 1  : 8,
      image_data[i * 28 + 3] < 0 ? 1  : 8,
      image_data[i * 28 + 4] < 0 ? 1  : 8,
      image_data[i * 28 + 5] < 0 ? 1  : 8,
      image_data[i * 28 + 6] < 0 ? 1  : 8,
      image_data[i * 28 + 7] < 0 ? 1  : 8,
      image_data[i * 28 + 8] < 0 ? 1  : 8,
      image_data[i * 28 + 9] < 0 ? 1  : 8,
      image_data[i * 28 + 10] < 0 ? 1 : 8,
      image_data[i * 28 + 11] < 0 ? 1 : 8,
      image_data[i * 28 + 12] < 0 ? 1 : 8,
      image_data[i * 28 + 13] < 0 ? 1 : 8,
      image_data[i * 28 + 14] < 0 ? 1 : 8,
      image_data[i * 28 + 15] < 0 ? 1 : 8,
      image_data[i * 28 + 16] < 0 ? 1 : 8,
      image_data[i * 28 + 17] < 0 ? 1 : 8,
      image_data[i * 28 + 18] < 0 ? 1 : 8,
      image_data[i * 28 + 19] < 0 ? 1 : 8,
      image_data[i * 28 + 20] < 0 ? 1 : 8,
      image_data[i * 28 + 21] < 0 ? 1 : 8,
      image_data[i * 28 + 22] < 0 ? 1 : 8,
      image_data[i * 28 + 23] < 0 ? 1 : 8,
      image_data[i * 28 + 24] < 0 ? 1 : 8,
      image_data[i * 28 + 25] < 0 ? 1 : 8,
      image_data[i * 28 + 26] < 0 ? 1 : 8,
      image_data[i * 28 + 27] < 0 ? 1 : 8
      );
  }

  TF_LITE_REPORT_ERROR(error_reporter, "Image decoded and processed");
  return kTfLiteOk;
}

// Get an image from the camera module
TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data) {
  static bool g_is_camera_initialized = false;
  if (!g_is_camera_initialized) {
    TfLiteStatus init_status = InitCamera(error_reporter);
    if (init_status != kTfLiteOk) {
      TF_LITE_REPORT_ERROR(error_reporter, "InitCamera failed");
      return init_status;
    }
    g_is_camera_initialized = true;
  }

  TfLiteStatus capture_status = PerformCapture(error_reporter);
  if (capture_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "PerformCapture failed");
    return capture_status;
  }

  TfLiteStatus read_data_status = ReadData(error_reporter);
  if (read_data_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "ReadData failed");
    return read_data_status;
  }

  TfLiteStatus decode_status = DecodeAndProcessImage(
      error_reporter, image_width, image_height, image_data);
  if (decode_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "DecodeAndProcessImage failed");
    return decode_status;
  }

  return kTfLiteOk;
}

#endif  // ARDUINO_EXCLUDE_CODE
