#ifdef HAVE_SWAP

#include <string.h>  // memset, explicit_bzero
#include "handle_swap_sign_transaction.h"
#include "swap.h"
#include "os.h"
#include "constants.h"

/* Set empty printable_amount on error, printable amount otherwise */
void swap_handle_get_printable_amount(get_printable_amount_parameters_t* params) {
    PRINTF("TODO: swap_handle_get_printable_amount\n");
}

#endif