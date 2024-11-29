# Blinky on RTOS Demo Example

This is an example application for LED blinky on RTOS.

The supported features are as the following:
- Blink LED periodically.

H/W setup:
1. GPIO : GPIOC8 connected to external LED
2. External LED is required

## Example usage

This assume you went through the getting started and you have successfully built, loaded and ran a get_started or example application on the DA16xxx.

- In the e2studio, select **a SDK directory that you want to use** as workspace.
- From the workspace you have setup go to **File->Import**
- Select **General->Dialog SDK Project** and click Next.
- In the `Select root directory` input the path where this project is located.
- Select this project to import into the workspace and click Next.
- Make sure that the toolchain version in the dialog is 10.3.1.20210824 and select DA16200 as the target device.
- click Finish.

## Run example 

1. Build blinky_rtos_ek_da16200_ep project.
2. Download built image to DA16200.
3. Monitor the LED.

## limitation

- None

## Compatibility

- SDK v3.2.8
