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

typedef struct swap_validated_s {
    bool initialized;
    uint8_t decimals;
    char ticker[MAX_SWAP_TOKEN_LENGTH];
    uint64_t amount;
    char recipient[ADDRESS_LEN];
} swap_validated_t;

static swap_validated_t G_swap_validated;

// Save the BSS address where we will write the return value when finished
static uint8_t* G_swap_sign_return_value_address;

/**
 * Copies both the transaction parameters and address into a static location
 * for posterior comparision against G_context, to avoid any injection attacks
 * through the Exchange App.
 *
 * @param[in] create_transaction_parameters_t
 * The transacttion parameters to be saved.
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
        PRINTF("destination_address_extra_id is not empy: '%s'\n",
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

    // Save amount
    if (!swap_str_to_u64(params->amount, params->amount_length, &swap_validated.amount)) {
        return false;
    }
    char printable_amount[MAX_PRINTABLE_AMOUNT_SIZE];
    if (print_amount(swap_validated.amount,
                     printable_amount,
                     MAX_PRINTABLE_AMOUNT_SIZE,
                     swap_validated.decimals) == 0) {
        PRINTF("print_amount failed\n");
    } else {
        PRINTF("print_amount %s\n", printable_amount);
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

static bool validate_swap_amount(uint64_t amount) {
    char validated_amount_str[MAX_PRINTABLE_AMOUNT_SIZE];
    if (print_amount(G_swap_validated.amount,
                     validated_amount_str,
                     sizeof(validated_amount_str),
                     G_swap_validated.decimals) == 0) {
        PRINTF("Conversion failed\n");
        return false;
    }
    char amount_str[MAX_PRINTABLE_AMOUNT_SIZE];
    if (print_amount(amount,
                     amount_str,
                     sizeof(amount_str),
                     G_swap_validated.decimals) == 0) {
        PRINTF("Conversion failed\n");
        return false;
    }

    if (strcmp(amount_str, validated_amount_str) != 0) {
        PRINTF("Amount requested in this transaction = %s\n", amount_str);
        PRINTF("Amount validated in swap = %s\n", validated_amount_str);
        return false;
    }
    PRINTF("Amounts match: validated(%s) = input(%s) \n", validated_amount_str, amount_str);
    return true;
}

/**
 * Validates that the transaction parameters saved in the app memory
 * are the same as the ones in the Exchange App current G_context.
 * 
 * @return true if success,false otherwise.
 *
 */
bool swap_check_validity(uint64_t amount) {
    PRINTF("Inside Aptos swap_check_validity\n");
    validate_swap_amount(amount);
    return true;
}

void __attribute__((noreturn)) swap_finalize_exchange_sign_transaction(bool is_success) {
    PRINTF("Returning to Exchange with status %d\n", is_success);
    *G_swap_sign_return_value_address = is_success;
    os_lib_end();
}

#endif