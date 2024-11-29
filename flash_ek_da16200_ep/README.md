# FreeRTOS demo example for reading/writing data to/from flash memory

This is an example application for reading/writing data to/from flash memory on the FreeRTOS.

The supported functions are as follows:
- Write data to the flash memory.
- Read and verify data from the flash memory.

H/W setup:
1. SW4 : Turn on FTR_RST.

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

1. Build the flash_ek_da16200_ep project.
2. Download the built image to the DA16200.
3. Press the FTR_RST(S2) switch on the EVK.
4. View the logs from UART0.

##limitation

None

## Compatibility

- SDK v3.2.8.1
