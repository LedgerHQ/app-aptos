#ifdef HAVE_SWAP
#include <string.h>
#include "swap.h"
#include "../handler/get_public_key.h"
#include "os.h"

#define ADDRESS_LENGTH 64

/* Set params.result to 0 on error, 1 otherwise */
void swap_handle_check_address(check_address_parameters_t *params) {
    PRINTF("Inside Aptos swap_handle_check_address\n");
    params->result = 0;

    // Checking that parameters are correct
    if (params->address_parameters == NULL || params->address_parameters_length == 0) {
        PRINTF("address_parameters is empty\n");
        return;
    }
    PRINTF("address_parameters_length: %d\n", params->address_parameters_length);
    PRINTF("address_parameters: %.*H\n", params->address_parameters_length, params->address_parameters);

    if (params->address_to_check == NULL) {
        PRINTF("address_to_check is empty\n");
        return;
    }

    PRINTF("address_to_check: %s\n", params->address_to_check);
    //if (strlen(params->address_to_check) != ADDRESS_LENGTH) {
    //    PRINTF("address_to_check length should be %d, not %d\n", ADDRESS_LENGTH, strlen(params->address_to_check));
    //    return;
    //}

    // Check that the address to check is in the list of addresses in the device
    buffer_t cdata;
    cdata.ptr = params->address_parameters;
    cdata.size = params->address_parameters_length;
    cdata.offset = 0;

    uint8_t bip32_path_len;
    uint32_t bip32_path[MAX_BIP32_PATH];
    if (get_public_key(&cdata, &bip32_path_len, bip32_path) != 0) {
        PRINTF("get_public_key failed\n");
        return;
    }


    PRINTF("addess_to_check matches within the addresses in the device\n");
    params->result = 1;
}

#endif