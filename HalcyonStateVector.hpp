///
/// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MANUALLY EDIT.
///

#ifndef HalcyonStateVector_HPP
#define HalcyonStateVector_HPP

#include "sf/core/StateVector.hpp"

namespace HalcyonStateVector
{

///
/// @brief Initializes a state vector from the autocoded config.
///
/// @note The config is static. This function should only be called once.
///
/// @param[out] kSv  State vector to initialize.
///
/// @retval SUCCESS  Successfully initialized state vector.
/// @retval [other]  Initialization failed.
///
static Result getConfig(StateVector::Config& kSvConfig)
{
    // State vector backing
    #pragma pack(push, 1)
    static struct
    {
        struct
        {
            U8 DV_ELEM_FC_STARTUP_CTRL_MODE;
            U8 DV_ELEM_FC_HEALTH_CTRL_MODE;
            U8 DV_ELEM_FLUIDS_CTRL_MODE;
            U8 DV_ELEM_DN0_DESYNC_CTRL_MODE;
            U8 DV_ELEM_DN1_DESYNC_CTRL_MODE;
            U8 DV_ELEM_DN2_DESYNC_CTRL_MODE;
            U8 DV_ELEM_ABORT_CTRL_MODE;
            bool DV_ELEM_PROCEED_TO_FILL;
            bool DV_ELEM_ABORT;
            U8 DV_ELEM_CN_WATCHDOG_CTRL_MODE;
            U8 DV_ELEM_DN0_WATCHDOG_CTRL_MODE;
            U8 DV_ELEM_DN1_WATCHDOG_CTRL_MODE;
            U8 DV_ELEM_DN2_WATCHDOG_CTRL_MODE;
            U8 DV_ELEM_GNC_CTRL_MODE;
            bool DV_ELEM_PROCEED_TO_TERM_CNT;
            bool DV_ELEM_BURN_BABY_BURN;
            U8 DV_ELEM_CN_LOGGING_CTRL_MODE;
            U8 DV_ELEM_DN0_LOGGING_CTRL_MODE;
            U8 DV_ELEM_DN1_LOGGING_CTRL_MODE;
            U8 DV_ELEM_DN2_LOGGING_CTRL_MODE;
            U8 DV_ELEM_ENGINE_CTRL_MODE;
            U8 DV_ELEM_CMD;
            bool DV_ELEM_PROCEED_TO_LAUNCH;
            bool DV_ELEM_LIFTOFF_CONFIRMED;
            U8 DV_ELEM_NCONE_CTRL_MODE_PRI;
            U8 DV_ELEM_DROGP_DEP_CTRL_MODE_PRI;
            U8 DV_ELEM_MAINP_CTRL_MODE_PRI;
            U8 DV_ELEM_NCONE_CTRL_MODE_BAK;
            U8 DV_ELEM_DROGP_DEP_CTRL_MODE_BAK;
            U8 DV_ELEM_MAINP_CTRL_MODE_BAK;
            U8 DV_ELEM_MTIME_CTRL_MODE;
            bool DV_ELEM_APOGEE;
            F64 DV_ELEM_ROCKET_POS_X;
            bool DV_ELEM_TOUCHDOWN;
        } All;
    } backing;
    #pragma pack(pop)

    // Elements
    static Element<U8> elemDV_ELEM_FC_STARTUP_CTRL_MODE(backing.All.DV_ELEM_FC_STARTUP_CTRL_MODE);
    static Element<U8> elemDV_ELEM_FC_HEALTH_CTRL_MODE(backing.All.DV_ELEM_FC_HEALTH_CTRL_MODE);
    static Element<U8> elemDV_ELEM_FLUIDS_CTRL_MODE(backing.All.DV_ELEM_FLUIDS_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN0_DESYNC_CTRL_MODE(backing.All.DV_ELEM_DN0_DESYNC_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN1_DESYNC_CTRL_MODE(backing.All.DV_ELEM_DN1_DESYNC_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN2_DESYNC_CTRL_MODE(backing.All.DV_ELEM_DN2_DESYNC_CTRL_MODE);
    static Element<U8> elemDV_ELEM_ABORT_CTRL_MODE(backing.All.DV_ELEM_ABORT_CTRL_MODE);
    static Element<bool> elemDV_ELEM_PROCEED_TO_FILL(backing.All.DV_ELEM_PROCEED_TO_FILL);
    static Element<bool> elemDV_ELEM_ABORT(backing.All.DV_ELEM_ABORT);
    static Element<U8> elemDV_ELEM_CN_WATCHDOG_CTRL_MODE(backing.All.DV_ELEM_CN_WATCHDOG_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN0_WATCHDOG_CTRL_MODE(backing.All.DV_ELEM_DN0_WATCHDOG_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN1_WATCHDOG_CTRL_MODE(backing.All.DV_ELEM_DN1_WATCHDOG_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN2_WATCHDOG_CTRL_MODE(backing.All.DV_ELEM_DN2_WATCHDOG_CTRL_MODE);
    static Element<U8> elemDV_ELEM_GNC_CTRL_MODE(backing.All.DV_ELEM_GNC_CTRL_MODE);
    static Element<bool> elemDV_ELEM_PROCEED_TO_TERM_CNT(backing.All.DV_ELEM_PROCEED_TO_TERM_CNT);
    static Element<bool> elemDV_ELEM_BURN_BABY_BURN(backing.All.DV_ELEM_BURN_BABY_BURN);
    static Element<U8> elemDV_ELEM_CN_LOGGING_CTRL_MODE(backing.All.DV_ELEM_CN_LOGGING_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN0_LOGGING_CTRL_MODE(backing.All.DV_ELEM_DN0_LOGGING_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN1_LOGGING_CTRL_MODE(backing.All.DV_ELEM_DN1_LOGGING_CTRL_MODE);
    static Element<U8> elemDV_ELEM_DN2_LOGGING_CTRL_MODE(backing.All.DV_ELEM_DN2_LOGGING_CTRL_MODE);
    static Element<U8> elemDV_ELEM_ENGINE_CTRL_MODE(backing.All.DV_ELEM_ENGINE_CTRL_MODE);
    static Element<U8> elemDV_ELEM_CMD(backing.All.DV_ELEM_CMD);
    static Element<bool> elemDV_ELEM_PROCEED_TO_LAUNCH(backing.All.DV_ELEM_PROCEED_TO_LAUNCH);
    static Element<bool> elemDV_ELEM_LIFTOFF_CONFIRMED(backing.All.DV_ELEM_LIFTOFF_CONFIRMED);
    static Element<U8> elemDV_ELEM_NCONE_CTRL_MODE_PRI(backing.All.DV_ELEM_NCONE_CTRL_MODE_PRI);
    static Element<U8> elemDV_ELEM_DROGP_DEP_CTRL_MODE_PRI(backing.All.DV_ELEM_DROGP_DEP_CTRL_MODE_PRI);
    static Element<U8> elemDV_ELEM_MAINP_CTRL_MODE_PRI(backing.All.DV_ELEM_MAINP_CTRL_MODE_PRI);
    static Element<U8> elemDV_ELEM_NCONE_CTRL_MODE_BAK(backing.All.DV_ELEM_NCONE_CTRL_MODE_BAK);
    static Element<U8> elemDV_ELEM_DROGP_DEP_CTRL_MODE_BAK(backing.All.DV_ELEM_DROGP_DEP_CTRL_MODE_BAK);
    static Element<U8> elemDV_ELEM_MAINP_CTRL_MODE_BAK(backing.All.DV_ELEM_MAINP_CTRL_MODE_BAK);
    static Element<U8> elemDV_ELEM_MTIME_CTRL_MODE(backing.All.DV_ELEM_MTIME_CTRL_MODE);
    static Element<bool> elemDV_ELEM_APOGEE(backing.All.DV_ELEM_APOGEE);
    static Element<F64> elemDV_ELEM_ROCKET_POS_X(backing.All.DV_ELEM_ROCKET_POS_X);
    static Element<bool> elemDV_ELEM_TOUCHDOWN(backing.All.DV_ELEM_TOUCHDOWN);

    // Regions
    static Region regionAll(&backing.All, sizeof(backing.All));

    // Element configs
    static StateVector::ElementConfig elemConfigs[] =
    {
        {"DV_ELEM_FC_STARTUP_CTRL_MODE", &elemDV_ELEM_FC_STARTUP_CTRL_MODE},
        {"DV_ELEM_FC_HEALTH_CTRL_MODE", &elemDV_ELEM_FC_HEALTH_CTRL_MODE},
        {"DV_ELEM_FLUIDS_CTRL_MODE", &elemDV_ELEM_FLUIDS_CTRL_MODE},
        {"DV_ELEM_DN0_DESYNC_CTRL_MODE", &elemDV_ELEM_DN0_DESYNC_CTRL_MODE},
        {"DV_ELEM_DN1_DESYNC_CTRL_MODE", &elemDV_ELEM_DN1_DESYNC_CTRL_MODE},
        {"DV_ELEM_DN2_DESYNC_CTRL_MODE", &elemDV_ELEM_DN2_DESYNC_CTRL_MODE},
        {"DV_ELEM_ABORT_CTRL_MODE", &elemDV_ELEM_ABORT_CTRL_MODE},
        {"DV_ELEM_PROCEED_TO_FILL", &elemDV_ELEM_PROCEED_TO_FILL},
        {"DV_ELEM_ABORT", &elemDV_ELEM_ABORT},
        {"DV_ELEM_CN_WATCHDOG_CTRL_MODE", &elemDV_ELEM_CN_WATCHDOG_CTRL_MODE},
        {"DV_ELEM_DN0_WATCHDOG_CTRL_MODE", &elemDV_ELEM_DN0_WATCHDOG_CTRL_MODE},
        {"DV_ELEM_DN1_WATCHDOG_CTRL_MODE", &elemDV_ELEM_DN1_WATCHDOG_CTRL_MODE},
        {"DV_ELEM_DN2_WATCHDOG_CTRL_MODE", &elemDV_ELEM_DN2_WATCHDOG_CTRL_MODE},
        {"DV_ELEM_GNC_CTRL_MODE", &elemDV_ELEM_GNC_CTRL_MODE},
        {"DV_ELEM_PROCEED_TO_TERM_CNT", &elemDV_ELEM_PROCEED_TO_TERM_CNT},
        {"DV_ELEM_BURN_BABY_BURN", &elemDV_ELEM_BURN_BABY_BURN},
        {"DV_ELEM_CN_LOGGING_CTRL_MODE", &elemDV_ELEM_CN_LOGGING_CTRL_MODE},
        {"DV_ELEM_DN0_LOGGING_CTRL_MODE", &elemDV_ELEM_DN0_LOGGING_CTRL_MODE},
        {"DV_ELEM_DN1_LOGGING_CTRL_MODE", &elemDV_ELEM_DN1_LOGGING_CTRL_MODE},
        {"DV_ELEM_DN2_LOGGING_CTRL_MODE", &elemDV_ELEM_DN2_LOGGING_CTRL_MODE},
        {"DV_ELEM_ENGINE_CTRL_MODE", &elemDV_ELEM_ENGINE_CTRL_MODE},
        {"DV_ELEM_CMD", &elemDV_ELEM_CMD},
        {"DV_ELEM_PROCEED_TO_LAUNCH", &elemDV_ELEM_PROCEED_TO_LAUNCH},
        {"DV_ELEM_LIFTOFF_CONFIRMED", &elemDV_ELEM_LIFTOFF_CONFIRMED},
        {"DV_ELEM_NCONE_CTRL_MODE_PRI", &elemDV_ELEM_NCONE_CTRL_MODE_PRI},
        {"DV_ELEM_DROGP_DEP_CTRL_MODE_PRI", &elemDV_ELEM_DROGP_DEP_CTRL_MODE_PRI},
        {"DV_ELEM_MAINP_CTRL_MODE_PRI", &elemDV_ELEM_MAINP_CTRL_MODE_PRI},
        {"DV_ELEM_NCONE_CTRL_MODE_BAK", &elemDV_ELEM_NCONE_CTRL_MODE_BAK},
        {"DV_ELEM_DROGP_DEP_CTRL_MODE_BAK", &elemDV_ELEM_DROGP_DEP_CTRL_MODE_BAK},
        {"DV_ELEM_MAINP_CTRL_MODE_BAK", &elemDV_ELEM_MAINP_CTRL_MODE_BAK},
        {"DV_ELEM_MTIME_CTRL_MODE", &elemDV_ELEM_MTIME_CTRL_MODE},
        {"DV_ELEM_APOGEE", &elemDV_ELEM_APOGEE},
        {"DV_ELEM_ROCKET_POS_X", &elemDV_ELEM_ROCKET_POS_X},
        {"DV_ELEM_TOUCHDOWN", &elemDV_ELEM_TOUCHDOWN},
        {nullptr, nullptr}
    };

    // Region configs
    static StateVector::RegionConfig regionConfigs[] =
    {
        {"All", &regionAll},
        {nullptr, nullptr}
    };

    kSvConfig = {elemConfigs, regionConfigs};

    return SUCCESS;
}

} // namespace HalcyonStateVector

#endif
