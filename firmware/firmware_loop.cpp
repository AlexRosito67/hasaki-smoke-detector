#include "smoke-detector-model-float"

#define ALARM_PIN        3
#define TRIGGER_THRESHOLD 3

// Input normalization ranges (from training data)
const float INPUT_MIN[12] = {-22.01f, 10.74f, 0.0f, 400.0f, 10668.0f, 15317.0f, 930.852f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
const float INPUT_MAX[12] = { 59.93f, 75.2f, 60000.0f, 60000.0f, 13803.0f, 21410.0f, 939.861f, 14333.69f, 45432.26f, 61482.03f, 51914.68f, 30026.438f};

void loop() {
    // 1. Capture sensor readings
    float input[12] = {
        read_temp(), read_hum(),      read_tvoc(),   read_eco2(),
        read_rawh2(), read_ethanol(), read_pressure(),
        read_pm1(),  read_pm25(),    read_nc05(),   read_nc1(), read_nc25()
    };

    // 2. Normalize inputs to [0.0, 1.0] using training ranges
    for (int i = 0; i < 12; i++)
        input[i] = (input[i] - INPUT_MIN[i]) / (INPUT_MAX[i] - INPUT_MIN[i]);

    // 3. Run inference — predict() writes result into output array
    float output[1];
    predict(input, output);

    // 4. State accumulator — requires TRIGGER_THRESHOLD consecutive positives
    static int accumulator = 0;
    if (output[0] >= 0.5f) {
        if (accumulator < TRIGGER_THRESHOLD) accumulator++;
    } else {
        if (accumulator > 0) accumulator--;
    }

    // 5. Drive alarm pin
    digitalWrite(ALARM_PIN, accumulator >= TRIGGER_THRESHOLD ? HIGH : LOW);

    delay(1000); // 1 Hz — matches dataset sample rate
}
