#include <TensorFlowLite.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "image_provider.h"
#include "model_settings.h"
#include "digits_model.h"

namespace {
    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;

    constexpr int kTensorArenaSize = 136 * 1024;
    static uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

void setup() {
    tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(digits_model);
    
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
            "Model provided is schema version %d not equal "
            "to supported version %d.\n",
            model->version(), TFLITE_SCHEMA_VERSION);
    }

    tflite::AllOpsResolver resolver;

    static tflite::MicroInterpreter static_interpreter(
        model, 
        resolver, 
        tensor_arena, 
        kTensorArenaSize, 
        error_reporter
    );
    interpreter = &static_interpreter;
    
    interpreter->AllocateTensors();

    input = interpreter->input(0);
}

void loop() {

    if (kTfLiteOk != GetImage(error_reporter, kNumCols, kNumRows, kNumChannels, input->data.int8)) {
        TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
    }

    if (kTfLiteOk != interpreter->Invoke()) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
    }

    TfLiteTensor* output = interpreter->output(0);

    TF_LITE_REPORT_ERROR(error_reporter, "%d : %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
        output->dims->size,
        output->data.uint8[0],
        output->data.uint8[1],
        output->data.uint8[2],
        output->data.uint8[3],
        output->data.uint8[4],
        output->data.uint8[5],
        output->data.uint8[6],
        output->data.uint8[7],
        output->data.uint8[8],
        output->data.uint8[9]
    );
}
