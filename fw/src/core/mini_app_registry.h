#ifndef MINI_APP_REGISTRY_H
#define MINI_APP_REGISTRY_H

#include "mini_app_defines.h"

#include "status_bar_app.h"

typedef enum {
    MINI_APP_ID_STATUS_BAR,
    MINI_APP_ID_DESKTOP
} mini_app_id_t;


extern mini_app_t* mini_app_registry[];

mini_app_t* mini_app_registry_find_by_id(uint32_t id);

#endif