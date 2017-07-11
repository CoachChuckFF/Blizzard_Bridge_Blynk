#ifndef BLIZZARD_ETH_H
#define BLIZZARD_ETH_H

#ifdef __cplusplus
extern "C" {
#endif

#define PIN_PHY_POWER 17
#define PIN_SMI_MDC   23
#define PIN_SMI_MDIO  18

static void phy_device_power_enable_via_gpio(bool enable);
static void eth_gpio_config_rmii(void);
void eth_task(void *pvParameter);
void initialise_blizzard_ethernet(void);

#ifdef __cplusplus
}
#endif

#endif
