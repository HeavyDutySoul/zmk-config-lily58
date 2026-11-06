/*
 * Copyright (c) 2018 Nordic Semiconductor ASA.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <hal/nrf_power.h>

static int board_nrf52840dongle_nrf52840_init(void)
{

	/* if the nrf52840dongle_nrf52840 board is powered from USB
	 * (high voltage mode), GPIO output voltage is set to 1.8 volts by
	 * default and that is not enough to turn the green and blue LEDs on.
	 * Increase GPIO voltage to 3.0 volts.
	 */
	if ((nrf_power_mainregstatus_get(NRF_POWER) ==
	     NRF_POWER_MAINREGSTATUS_HIGH) &&
	    ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
	     (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))) {

		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
			;
		}

		NRF_UICR->REGOUT0 =
		    (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
		    (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
			;
		}

		/* a reset is required for changes to take effect */
		NVIC_SystemReset();
	}

	return 0;
}

SYS_INIT(board_nrf52840dongle_nrf52840_init, PRE_KERNEL_1,
	 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);

#if IS_ENABLED(CONFIG_PM_DEVICE)
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>

static int dongle_pm_action(const struct device *dev, enum pm_device_action action)
{
	ARG_UNUSED(dev);
	switch (action) {
	case PM_DEVICE_ACTION_SUSPEND:
		/* Prevent the USB dongle from entering System OFF.
		 * The dongle must stay powered on to maintain USB connection
		 * and receive BLE reconnections when peripherals wake up.
		 */
		return -EBUSY;
	default:
		return 0;
	}
}

static int dongle_pm_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

PM_DEVICE_DEFINE(dongle_pm, dongle_pm_action);
DEVICE_DEFINE(dongle_pm_dev, "dongle_pm", dongle_pm_init, PM_DEVICE_GET(dongle_pm),
	      NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, NULL);
#endif

