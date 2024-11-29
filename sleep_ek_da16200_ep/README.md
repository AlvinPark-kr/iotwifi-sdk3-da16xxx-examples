# Sleep on RTOS Demo Example

The example project demonstrates the functionalities of Sleep 2 and Sleep 3 running on DA16200 EVB.
Two GPIOs are defined as interrupt PINs. When the interrupt for GPIOA_6 or GPIOA_7 is triggerned, 
then it will be handled. As a result of handing the interrupt, DA16200 will enter Sleep 2 (or Sleep 3)
by calling the slepp API and it will be in sleep for the defined time.
When the time is up, then DA16200 will be woken up. The project also demonstrates behaviors of DA16200
during wake-up from Sleep 2 and Sleep 3. 

The supported features are as the following:
- Entering Sleep 2 by pushing the WPS button connected to the GPIOA_6.
- Entering Sleep 3 by pushing the Factory reset button connected to the GPIOA_7.

H/W setup:
1. SW4 on DA16200 EVB should be ON.
2. DIP switch 3 and 4 of SW1 on DA16200 should be OFF.

## Example usage

This assume you went through the getting started and you have successfully built, loaded and ran a get_started or example application on the DA16xxx.

- In the e2studio, select **a DA16200/DA16600 SDK directory** that you want to use as **workspace**.
- From the workspace you have setup go to **File->Import**.
- Select **General->Dialog SDK Project** and click Next.
- In the `Select SDK root directory` input the path where this project is located.
- Select this project to import into the workspace and click Next.
- Make sure that the toolchain version in the dialog is 10.3.1.20210824 and select DA16200 as the target device.
- click Finish.

Note:
- The __SUPPORT_WPS_BTN__ should be undefined to use GPIOA_6 for this project.
- The __SUPPORT_FACTORY_RESET_BTN__ should be undefined to use GPIOA_7 for this project.

## Run example 

1. Build the sleep_ek_da16200_ep project.
2. Download built image to DA16200.
3. Push the WPS or Factory Reset button.
4. Monitor the logs from UART0.

## limitation

None

## Compatibility

- SDK v3.2.8
