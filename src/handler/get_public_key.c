/*****************************************************************************
 *   Ledger App Aptos.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"
#include "io.h"
#include "buffer.h"

#include "get_public_key.h"
#include "../globals.h"
#include "../types.h"
#include "../sw.h"
#include "../crypto.h"
#include "../address.h"
#include "../ui/display.h"
#include "../helper/send_response.h"

int handler_get_public_key(buffer_t *cdata, bool display) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;

    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};

    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        G_context.req_type = REQUEST_UNDEFINED;
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    if (!validate_aptos_bip32_path(G_context.bip32_path, G_context.bip32_path_len)) {
        G_context.req_type = REQUEST_UNDEFINED;
        return io_send_sw(SW_GET_PUB_KEY_FAIL);
    }

    // derive private key according to BIP32 path
    cx_err_t error = crypto_derive_private_key(&private_key,
                                               G_context.pk_info.chain_code,
                                               G_context.bip32_path,
                                               G_context.bip32_path_len);
    if (error != CX_OK) {
        explicit_bzero(&private_key, sizeof(private_key));
        PRINTF("crypto_derive_private_key error code: %x.\n", error);
        G_context.req_type = REQUEST_UNDEFINED;
        return io_send_sw(SW_GET_PUB_KEY_FAIL);
    }

    // generate corresponding public key
    error = crypto_init_public_key(&private_key, &public_key, G_context.pk_info.raw_public_key);

    if (error != CX_OK) {
        explicit_bzero(&private_key, sizeof(private_key));
        PRINTF("crypto_init_public_key error code: %x.\n", error);
        G_context.req_type = REQUEST_UNDEFINED;
        return io_send_sw(SW_GET_PUB_KEY_FAIL);
    }

    // reset private key
    explicit_bzero(&private_key, sizeof(private_key));

    if (display) {
        int ui_status = ui_display_address();
        G_context.req_type = REQUEST_UNDEFINED;  // all the work is done, reset the context
        return ui_status;
    }

    G_context.req_type = REQUEST_UNDEFINED;  // all the work is done, reset the context
    return helper_send_response_pubkey();
}
