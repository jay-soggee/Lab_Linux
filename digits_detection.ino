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

#include <TensorFlowLite.h>

#include "main_functions.h"

#include "detection_responder.h"
#include "image_provider.h"
#include "model_settings.h"
#include "model.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "7Seg.h"
#define  SW  10

//Keypad define
// Built-in LED
#define SIZE 3
int RED = 24;

// Init_Keypad
int ROWS[SIZE] = {A0,A1,A2};
int COLS[SIZE] = {A3,A6,A7};
int i,j;
int NUM = 0;
volatile int predict = -1;
volatile int ResultNum=-1;
volatile int button = 0;

#define DEBOUNCE_TIME 20

/*void debounce(){
  unsigned long now = millis();
  do{
    if(digitalRead(SW)==LOW)
      now = millis();
  }
  while(digitalRead(SW)== LOW || (millis() - now <=DEBOUNCE_TIME));
}*/

//////////////////
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

constexpr size_t kTensorArenaSize = 136 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];
}  // namespace


void setup() {

    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;


    model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
      TF_LITE_REPORT_ERROR(error_reporter,
                          "Model provided is schema version %d not equal "
                          "to supported version %d.",
                          model->version(), TFLITE_SCHEMA_VERSION);
      return;
    }

    static tflite::MicroMutableOpResolver<6> micro_op_resolver;
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddFullyConnected();
    micro_op_resolver.AddMaxPool2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax();

    static tflite::MicroInterpreter static_interpreter(
        model, 
        micro_op_resolver, 
        tensor_arena, 
        kTensorArenaSize, 
        error_reporter);
    interpreter = &static_interpreter;


    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
      TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
      return;
    }


    input = interpreter->input(0);
    
    //EXINT Code : Kihyeon
    //pinMode(SW1, INPUT_PULLUP);
   // attachInterrupt(digitalPinToInterrupt(SW1), run_model_EXTINT, FALLING);
    //EXINT Code
    //7Seg Code : Kihyeon
    FND_Init();

    // KEYPAD Setting
    pinMode(RED, OUTPUT);
    digitalWrite(RED, HIGH);
    for(i = ROWS[0];i<=ROWS[2];i++){
      // ROWS 포트 설정
      pinMode(i,OUTPUT);
      digitalWrite(i,HIGH);
      // COLS 포트 설정
      pinMode(i+SIZE,INPUT_PULLUP);
  } 

  pinMode(SW, INPUT);
}


int run_model() {
    int result;
    if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels, input->data.int8)) {
        TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
    }

    if (kTfLiteOk != interpreter->Invoke()) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
    }
    
    TfLiteTensor* output = interpreter->output(0);

    for (int i; i < 10; i++) 
        if (output->data.int8[result] < output->data.int8[i])
            result = i;

    return result;
}

int RetRunModel(){
  int cnt=0;
  int res[10]={0,};
  int predictArr[10]={0,};
  int result;
  for(cnt=0; cnt<10; cnt++){
    predict = run_model();
    predictArr[cnt]=predict;
  }
  for(int i=0; i<10; i++){
    for(int j=0; j<10; j++){
      if(predictArr[i]==predictArr[j]){
        res[i]++;
      }
    }
  }
  for(int k=0; k<10; k++){
    for(int p=0; p<10; p++){
      if(res[k]<res[p]) result = predictArr[p];
    }
  }
  button = 0;
  return result;
}

int Get_NUM_By_Keypad(){
  for(i=0;i<SIZE;i++)
  {
    digitalWrite(ROWS[i],LOW);
    for(j=0;j<SIZE;j++){
      if(!digitalRead(COLS[j])){
        if(!digitalRead(ROWS[i])) 
           delay(200);
           NUM = 1 + (i*SIZE+j);
       } 
    }
  digitalWrite(ROWS[i],HIGH);
 }
 return NUM;
}

void loop() {
  
   
    if(!digitalRead(SW)){
      delay(200);
      button=1;
    }
    
    if(button==1){
      predict=RetRunModel();
      TF_LITE_REPORT_ERROR(error_reporter, "predict as : %d", predict);
    }
    else if(!button){
      delay(300);
      GetImage(error_reporter, kNumCols, kNumRows, kNumChannels, input->data.int8);
    }
    OUT_SEG(predict);
    
    
    ResultNum=Get_NUM_By_Keypad();
    if(ResultNum==predict){
      digitalWrite(RED, LOW); 
      delay(1000);
    }
    else if(ResultNum!=predict) digitalWrite(RED, HIGH);
    //7Seg 출력 : Kihyeon
}
