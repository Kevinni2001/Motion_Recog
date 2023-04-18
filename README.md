# Edge Impulse : Motion Recognization Model with Raspberry Pico 2040

This builds an exported impulse to a .uf2 file that can be dragged and ropped to Raspberry Pico. 

# Compliation Guides:
1. First Extract the trained model from EdgeImpulse, copy and paste following three directories in the directory where CMakeLists.txt and source is located (aka the root directory of this project).
    ~~~
    edge-impulse-sdk
    model-parameters
    tflite-model
    ~~~
2. Then create a build directory (if don't exist), then cd into it.
3. Then type in terminal:
    ~~~
    export PICO_SDK_PATH=../../pico-sdk
    ~~~
4. in terminal, do the following:
    ~~~
    cmake ..
    make -j24
    ~~~
Note that -j24 means that we are creating 24 threads to compile this file.
