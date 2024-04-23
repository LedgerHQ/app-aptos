#pragma once

#include "nbgl_use_case.h"

extern nbgl_layoutTagValue_t pairs[6];
extern nbgl_layoutTagValueList_t pairList;
extern nbgl_pageInfoLongPress_t infoLongPress;

typedef struct use_case_review_ctx_s {
    const nbgl_icon_details_t *icon;
    union {
        const char *review_title;
        const char *long_press_title;
    };
    union {
        const char *review_sub_title;
        const char *long_press_button_text;
    };
    const char *reject_text;
    nbgl_callback_t continue_callback;
    nbgl_callback_t reject_callback;
    nbgl_choiceCallback_t choice_callback;
} use_case_review_ctx_t;

void nbgl_useCaseReviewVerify(const nbgl_icon_details_t *icon,
                              const char *review_title,
                              const char *review_sub_title,
                              const char *reject_text,
                              nbgl_callback_t continue_callback,
                              nbgl_callback_t reject_callback);

void nbgl_useCaseStaticReviewVerify(const nbgl_icon_details_t *icon,
                                    const char *long_press_title,
                                    const char *long_press_button_text,
                                    const char *reject_text,
                                    nbgl_choiceCallback_t choice_callback,
                                    nbgl_callback_t reject_callback);