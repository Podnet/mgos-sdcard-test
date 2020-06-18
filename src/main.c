#include "mgos.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 2

// Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
// Please check its source code and implement error recovery when developing
// production applications.

/**> Function to send data to sdcard */
void write_to_sdcard_cb(void *user_data)
{
    sdmmc_card_t *card = (sdmmc_card_t *)user_data;
    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Create a file then open it
    LOG(LL_INFO, ("%s", "Opening file"));
    FILE *f = fopen("/sdcard/gps.txt", "w");
    if (f == NULL)
    {
        LOG(LL_INFO, ("%s", "Failed to open file for writing"));
        return;
    }
    // Write "Hello" to sdcard in gps.txt or to any file for that matter
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    LOG(LL_INFO, ("%s", "File written"));
}

// <----------------------------------SD card code end----------------------------------------------->

enum mgos_app_init_result mgos_app_init(void)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck = PIN_NUM_CLK;
    slot_config.gpio_cs = PIN_NUM_CS;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    sdmmc_card_t *card;
    // add mounting point
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            LOG(LL_INFO, ("%s", "Failed to mount filesystem.  If you want the card to be formatted, set format_if_mount_failed = true."));
        }
        else
        {
            LOG(LL_INFO, ("Failed to initialize the card (%s). ", esp_err_to_name(ret)));
        }
        return MGOS_INIT_APP_INIT_FAILED;
    }
    mgos_set_timer(1000, MGOS_TIMER_REPEAT, write_to_sdcard_cb, card);
    return MGOS_APP_INIT_SUCCESS;
}
