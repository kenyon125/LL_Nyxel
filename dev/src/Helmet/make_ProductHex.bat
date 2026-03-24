nrfutil settings generate --family NRF52 --application ..\..\HEX/Helmet/Nyx_application.hex --application-version 1 --bootloader-version 1 --bl-settings-version 2 settings.hex

mergehex -m nRF5_SDK_17.1.0_ddde560\components\softdevice\s132\hex\s132_nrf52_7.2.0_softdevice.hex nRF5_SDK_17.1.0_ddde560\examples\dfu\secure_bootloader\pca10040_s132_ble\arm5_no_packs\_build\bootloader.hex settings.hex -o temp.hex
		 
mergehex -m  temp.hex ..\..\HEX\Helmet\Nyx_application.hex  -o  ..\..\HEX\Helmet\Nyx_AllInOne.hex

del /F/Q temp.hex