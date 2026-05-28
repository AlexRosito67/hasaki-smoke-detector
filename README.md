<p align="center">
  <img src="./logo_hasaki.jpg" width="600" alt="Hasaki Logo">
</p>

# hasaki-smoke-detector

Companion repository for the article:  
**"The 3.7 kB Alarm: A Zero-Bloat Edge AI Smoke Detector in Pure C"**

A fully trained, INT8-quantized neural network for fire detection via sensor fusion.  
No runtime. No framework. No dependencies. Just a 3.7 kB C header.

---

## What's in this repo

| File | Description |
|------|-------------|
| `alarm_model.h` | Exported model — drop into any C/C++ firmware |
| `train.csv` | 28,596 balanced, normalized training samples |
| `test.csv` | 7,150 held-out validation samples (never seen during training) |
| `parser/main.cpp` | C++ preprocessing pipeline — normalization, CNT removal, class balancing |
| `firmware/firmware_loop.cpp` | Arduino-compatible inference loop with state accumulator |

---

## Architecture

```
12 inputs → 8 (ReLU) → 4 (ReLU) → 1 (Sigmoid)
Quantization: INT8 symmetric
Weights: 96 bytes
Total Flash footprint: ~3.7 kB
```

**Input features** (12 sensors, 4–6 physical pins via I2C/UART):

| # | Feature | Unit |
|---|---------|------|
| 0 | Temperature | °C |
| 1 | Humidity | % |
| 2 | TVOC | ppb |
| 3 | eCO₂ | ppm |
| 4 | Raw H₂ | — |
| 5 | Raw Ethanol | — |
| 6 | Pressure | hPa |
| 7 | PM1.0 | µg/m³ |
| 8 | PM2.5 | µg/m³ |
| 9 | NC0.5 | #/cm³ |
| 10 | NC1.0 | #/cm³ |
| 11 | NC2.5 | #/cm³ |

---

## Validation Results

Evaluated on 7,150 unseen samples:

```
[3547    4]   ← TN  FP
[   1 3598]   ← FN  TP

Accuracy:    99.9301%
Sensitivity: 99.97%
FN:          1
```

One missed fire event out of 3,599. Four false alarms out of 3,551 clean-air readings.

---

## Reproducing the validation

Requires [Hasaki 刃先](https://github.com/AlexRosito67/hasaki).

```bash
hasaki -d 12,8,4,1 -act relu,relu,sigmoid \
  -a validate -f test.csv -m alarm_model.h \
  2>/dev/null | awk '
/Expected:/ {
  split($0, a, "Expected: "); split(a[2], b, " "); expected=int(b[1]+0.5)
  split($0, c, "Got: "); split(c[2], d, " "); got=(d[1]+0>=0.5)?1:0
  if(expected==0 && got==0) TN++
  else if(expected==0 && got==1) FP++
  else if(expected==1 && got==0) FN++
  else TP++
}
END {
  total=TN+FP+FN+TP
  print "["TN" "FP"]"
  print "["FN" "TP"]"
  printf "Accuracy: %.4f%%\n", (TN+TP)/total*100
  print "FN: "FN
}'
```

---

## Firmware integration

```c
#include "alarm_model.h"

#define ALARM_PIN         3
#define TRIGGER_THRESHOLD 3

const float INPUT_MIN[12] = {-22.01f, 10.74f,    0.0f,    400.0f,
                              10668.0f, 15317.0f, 930.852f, 0.0f,
                              0.0f,     0.0f,     0.0f,     0.0f};
const float INPUT_MAX[12] = { 59.93f,  75.2f,  60000.0f, 60000.0f,
                              13803.0f, 21410.0f, 939.861f, 14333.69f,
                              45432.26f, 61482.03f, 51914.68f, 30026.438f};

void loop() {
    float input[12] = {
        read_temp(), read_hum(),      read_tvoc(),     read_eco2(),
        read_rawh2(), read_ethanol(), read_pressure(),
        read_pm1(),  read_pm25(),    read_nc05(), read_nc1(), read_nc25()
    };

    for (int i = 0; i < 12; i++)
        input[i] = (input[i] - INPUT_MIN[i]) / (INPUT_MAX[i] - INPUT_MIN[i]);

    float output[1];
    predict(input, output);

    static int accumulator = 0;
    if (output[0] >= 0.5f) {
        if (accumulator < TRIGGER_THRESHOLD) accumulator++;
    } else {
        if (accumulator > 0) accumulator--;
    }

    digitalWrite(ALARM_PIN, accumulator >= TRIGGER_THRESHOLD ? HIGH : LOW);
    delay(1000);
}
```

---

## Dataset

Original data collected at 1 Hz by Stefan Blattmann —  
*Real-time Smoke Detection with AI-based Sensor Fusion*

 (Datasets Grandmaster):  
[Smoke Detection Dataset](https://www.kaggle.com/datasets/deepcontractor/smoke-detection-dataset)

---

## Trained with

[Hasaki 刃先](https://github.com/AlexRosito67/hasaki) — CLI C++ tool for embedded neural network training.  
Exports standalone C headers. No runtime. No Python. No dependencies.

---

*Alex Rosito — Valley Glen, CA*
