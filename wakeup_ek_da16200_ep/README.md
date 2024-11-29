# FreeRTOS demo example for waking up from sleep

This is an example application for waking up from Sleep.

The example project demonstrates the functionalities of waking up from Sleep 2/3 on DA16200 EVB.
Two GPIOs are defined as interrupt PINs. When the interrupt for GPIOA_6 or GPIOA_7 is triggerned, 
then it will be handled. As a result of handing the interrupt, DA16200 will enter Sleep 2/3
by calling the sleep API and it will be waked up by external wake up signal(RTC_WAKUP_PIN1).

The supported features are as the following:
- Entering Sleep 2 by pushing the WPS button(S1) connected to the GPIOA_6.
- Entering Sleep 3 by pushing the Factory reset button(S2) connected to the GPIOA_7.
- Waking up by switching RTC_WAKE_UP(SW5) from sleep 2 or sleep3.

H/W setup:
1. SW4 on DA16200 EVB should be ON.
2. DIP switch 3 and 4 of SW1 on DA16200 should be OFF.
3. RTC_WAKE_UP Switch(SW5) in EVK.

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

1. Build the wakeup_ek_da16200_ep project.
2. Download built image to DA16200.
3. Push the WPS(S1) or Factory Reset(S2) button.
4. Wake up from sleep by switching RTC_WAKE_UP(SW5).
5. Monitor the logs from UART0.

##limitation

None

## Compatibility

- SDK v3.2.8
