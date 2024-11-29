# Data on Retention Memory Demo Example

The example project demonstrates the functionalitie about how to read write data to retention memory on DA16200 EVB.

The supported features are as the following:
- Increase index in retnetion memory.
- Go to sleep 3 after 2 sec.
- Wake up from sleep 3 after 2 sec.
- Repeate again.

H/W setup:
None

## Example usage

This assume you went through the getting started and you have successfully built, loaded and ran a get_started or example application on the DA16xxx.

- In the e2studio, select **a DA16200/DA16600 SDK directory** that you want to use as **workspace**.
- From the workspace you have setup go to **File->Import**.
- Select **General->Dialog SDK Project** and click Next.
- In the `Select SDK root directory` input the path where this project is located.
- Select this project to import into the workspace and click Next.
- Make sure that the toolchain version in the dialog is 10.3.1.20210824 and select DA16200 as the target device.
- click Finish.

## Run example 

1. Build rtm_data_ek_da16200_ep project.
2. Download built image to DA16200.
3. Monitor the logs from UART0.

##limitation

These APIs works only at DPM disabled.
Please refer to UM-WI-030_DA16200_DA16600_DPM_User_Manual for usage in DPM mode.

## Compatibility

- SDK v3.2.8
