# Edge Impulse : Motion Recognization Model with Raspberry Pico 2040

This builds an exported impulse to a .uf2 file that can be dragged and ropped to Raspberry Pico. 

# Training your own model
1. Install the uf2 file in drectory Data_collection_image to your pico, make sure the accelerometer (MMA8451) is connected to I2C_0 port at pin number 16,17.
2. Create your project on [EdgeImpulse](https://studio.edgeimpulse.com/studio/select-project) (You may need to create an account first)
3. Install the [EdgeImpulse CLI](https://docs.edgeimpulse.com/docs/edge-impulse-cli/cli-overview), which will directly upload the data collected from your device to your EdgeImpulse project. 
4. Train your ML model on EdgeImpulse following the [instructions](https://docs.edgeimpulse.com/docs/edge-impulse-studio/dashboard).
5. Deploy the model by selecting the C++ library option.
6. You will get a zip file with your trained model, follow the next section for implementation details.

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
