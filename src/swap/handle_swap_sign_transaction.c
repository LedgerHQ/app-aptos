#ifdef HAVE_SWAP

#include "handle_swap_sign_transaction.h"
#include "display.h"
#include "swap.h"
#include "string.h"
#include "os_lib.h"
#include "constants.h"
#include "os_utils.h"
#include "globals.h"
#include "sw.h"
#include "os.h"
#include "../globals.h"
#include "common/parse.h"

typedef struct swap_validated_s {
    bool initialized;
    uint8_t decimals;
    // NOTE(jmartins): only needed for tokens
    char ticker[MAX_SWAP_TOKEN_LENGTH];
    uint64_t amount;
    char recipient[ADDRESS_LEN];
} swap_validated_t;

static swap_validated_t G_swap_validated;

// Save the BSS address where we will write the return value when finished
static uint8_t* G_swap_sign_return_value_address;

/**
 * Copies both the transaction parameters and address into a static location
 * for posterior comparison against G_context, to avoid any injection attacks
 * through the Exchange App.
 *
 * @param[in] create_transaction_parameters_t
 * The transaction parameters to be saved.
 *
 * @return true if success,false otherwise.
 *
 */
bool swap_copy_transaction_parameters(create_transaction_parameters_t* params) {
    PRINTF("Inside Aptos swap_copy_transaction_parameters\n");

    // Ensure no extraid
    if (params->destination_address_extra_id == NULL) {
        PRINTF("destination_address_extra_id memory not reserved\n");
        return false;
    } else if (params->destination_address_extra_id[0] != '\0') {
        PRINTF("destination_address_extra_id is not empty: '%s'\n",
               params->destination_address_extra_id);
        return false;
    }

    if (params->destination_address == NULL) {
        PRINTF("Destination address expected\n");
        return false;
    }

    if (params->amount == NULL) {
        PRINTF("Amount expected\n");
        return false;
    }
    // first copy parameters to stack, and then to global data.
    // We need this "trick" as the input data position can overlap with app globals
    // and also because we want to memset the whole bss segment as it is not done
    // when an app is called as a lib.
    // This is necessary as many part of the code expect bss variables to
    // initialized at 0.
    swap_validated_t swap_validated;
    memset(&swap_validated, 0, sizeof(swap_validated));

    // Parse config and save decimals and ticker
    // If there is no coin_configuration, consider that we are doing a TRX swap
    if (params->coin_configuration == NULL) {
        memcpy(swap_validated.ticker, "APT", sizeof("APT"));
        swap_validated.decimals = APT_DECIMAL_PRECISION;
    } else {
        if (!swap_parse_config(params->coin_configuration,
                               params->coin_configuration_length,
                               swap_validated.ticker,
                               sizeof(swap_validated.ticker),
                               &swap_validated.decimals)) {
            PRINTF("Fail to parse coin_configuration\n");
            return false;
        }
    }

    // Save recipient
    strlcpy(swap_validated.recipient,
            params->destination_address,
            sizeof(swap_validated.recipient));
    if (swap_validated.recipient[sizeof(swap_validated.recipient) - 1] != '\0') {
        PRINTF("Address copy error\n");
        return false;
    }
    PRINTF("Recipient: %s\n", swap_validated.recipient);
    // Save amount
    if (!swap_str_to_u64(params->amount, params->amount_length, &swap_validated.amount)) {
        return false;
    }

    swap_validated.initialized = true;

    // Full reset the global variables
    os_explicit_zero_BSS_segment();

    // Keep the address at which we'll reply the signing status
    G_swap_sign_return_value_address = &params->result;

    // Commit from stack to global data, params becomes tainted but we won't access it anymore
    memcpy(&G_swap_validated, &swap_validated, sizeof(swap_validated));

    PRINTF("Exiting Aptos swap_copy_transaction_parameters\n");
    return true;
}

/**
 * Validates that amount is the same as the one saved in the app memory.
 * Does a string comparison as well as a uint64_t comparison.
 *
 * param[in] amount
 * The amount to be validated.
 *
 * @return true if success,false otherwise.
 *
 */
static bool validate_swap_amount(uint64_t amount) {
    if (amount != G_swap_validated.amount) {
        return false;
    }
    // NOTE: in other Nano Apps the validation is done in string type. We're keeping it as well.
    char validated_amount_str[MAX_PRINTABLE_AMOUNT_SIZE];
    if (print_amount(G_swap_validated.amount,
                     validated_amount_str,
                     sizeof(validated_amount_str),
                     G_swap_validated.decimals) == 0) {
        PRINTF("Conversion failed\n");
        return false;
    }
    char amount_str[MAX_PRINTABLE_AMOUNT_SIZE];
    if (print_amount(amount, amount_str, sizeof(amount_str), G_swap_validated.decimals) == 0) {
        PRINTF("Conversion failed\n");
        return false;
    }

    if (strcmp(amount_str, validated_amount_str) != 0) {
        PRINTF("Amount requested in this transaction = %s\n", amount_str);
        PRINTF("Amount validated in swap = %s\n", validated_amount_str);
        return false;
    }
    return true;
}

/**
 * Validates that the transaction parameters saved in the app memory
 * are the same as the ones in the Exchange App current G_context.
 *
 * @return true if success,false otherwise.
 *
 */
bool swap_check_validity() {
    PRINTF("Inside Aptos swap_check_validity\n");
    if (!G_swap_validated.initialized) {
        PRINTF("Swap Validated data not initialized.\n");
        return false;
    }
    // Validate it's and actual coin transfer type
    if (G_context.tx_info.transaction.tx_variant != TX_RAW) {
        PRINTF("TX variant different from TX_RAW is not compatible with Swap.\n");
        return false;
    }

    if (G_context.tx_info.transaction.payload_variant != PAYLOAD_ENTRY_FUNCTION) {
        PRINTF("Payload variant incompatible with Swap.\n");
        return false;
    }

    // Differentiate between the different types of transactions
    uint64_t amount = 0;
    uint8_t* receiver;
    switch (G_context.tx_info.transaction.payload.entry_function.known_type) {
        case FUNC_APTOS_ACCOUNT_TRANSFER:
            amount = G_context.tx_info.transaction.payload.entry_function.args.transfer.amount;
            receiver = G_context.tx_info.transaction.payload.entry_function.args.transfer.receiver;
            break;
        case FUNC_COIN_TRANSFER:
        case FUNC_APTOS_ACCOUNT_TRANSFER_COINS:
            amount = G_context.tx_info.transaction.payload.entry_function.args.coin_transfer.amount;
            receiver =
                G_context.tx_info.transaction.payload.entry_function.args.coin_transfer.receiver;
            break;
        default:
            PRINTF("Unknown function type\n");
            return false;
    }

    if (!validate_swap_amount(amount)) {
        PRINTF("Amount on Transaction is different from validated package.\n");
        return false;
    }

    // Validate recipient
    if (memcmp(receiver, G_swap_validated.recipient, ADDRESS_LEN) != 0) {
        PRINTF("Recipient on Transaction is different from validated package.\n");
        return false;
    }

    // Validate recipient
    return true;
}

void __attribute__((noreturn)) swap_finalize_exchange_sign_transaction(bool is_success) {
    PRINTF("Returning to Exchange with status %d\n", is_success);
    *G_swap_sign_return_value_address = is_success;
    os_lib_end();
}

#endif