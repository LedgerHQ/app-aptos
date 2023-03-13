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

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "../globals.h"
#include "menu.h"
#include "settings.h"
#include "display.h"

UX_STEP_NOCB(ux_menu_ready_step, pnn, {&C_aptos_logo, "Aptos", "is ready"});
UX_STEP_CB(ux_menu_settings_step, pb, ui_menu_settings(), {&C_icon_coggle, "Settings"});
UX_STEP_CB(ux_menu_about_step, pb, ui_menu_about(), {&C_icon_certificate, "About"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

// FLOW for the main menu:
// #1 screen: ready
// #2 screen: settings
// #3 screen: about submenu
// #4 screen: quit
UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_settings_step,
        &ux_menu_about_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

void ui_menu_main() {
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ui_flow_display(ux_menu_main_flow);
}

void ui_menu_settings() {
    ux_menulist_init(0, settings_submenu_getter, settings_submenu_selector);
}

UX_STEP_NOCB(ux_menu_info_step, bn, {"Aptos App", "(c) 2022 Pontem"});
UX_STEP_NOCB(ux_menu_version_step, bn, {"Version", APPVERSION});
UX_STEP_CB(ux_menu_back_step, pb, ui_menu_main(), {&C_icon_back, "Back"});

// FLOW for the about submenu:
// #1 screen: app info
// #2 screen: version of the app
// #3 screen: back button to main menu
UX_FLOW(ux_menu_about_flow,
        &ux_menu_info_step,
        &ux_menu_version_step,
        &ux_menu_back_step,
        FLOW_LOOP);

void ui_menu_about() {
    ui_flow_display(ux_menu_about_flow);
}
