/*******************************************************************************
 Copyright (c) 2016 Lumen Labs (HK) Limited. All Rights Reserved.
*******************************************************************************/

#include "LL_Platform.h"
#include "LL_Para.h"

#define CONFIG_FILE     (0x8010)
#define CONFIG_REC_KEY  (0x7010)

extern T_LL_HardwareID gtHwID;
/* A dummy structure to save in flash. */

static T_Para gtStorePara = 
{
	.para_version = LL_PARA_VERSION,
	.tRemote.ulRoundRobin = 0,
	.tRemote.ulIng        = 0,
	.tRemote.HardwareID[0] = 0x00,
	.tRemote.HardwareID[1] = 0x00,
	.tRemote.bits_isCheapRemote = 0x00,
	.eModeOfWarningLight =  (E_ModeOfWarningLight)0,
	.ulBrakeFunction = 1,
	.eBeepMode = BEEP_EVERY_4_TURNING_FLASH,
	.ulNeedCharge = 0,
	.ssKxyz[0] = 100,
	.ssKxyz[1] = 0,
	.ssKxyz[2] = 0,
	.ssThreshhold = -8700,
	.publicNetworkEnabled = 1,
	.netID_exist = 0,
	.syncPowerOff = 1,
	.glFactoryMode = false,
};

/* A record containing dummy configuration data. */
static fds_record_t const gtPara_record =
{
    .file_id           = CONFIG_FILE,
    .key               = CONFIG_REC_KEY,
    .data.p_data       = &gtStorePara,
    /* The length of a record is always expressed in 4-byte units (words). */
    .data.length_words = (sizeof(gtStorePara) + sizeof(uint32_t) - 1) / sizeof(uint32_t),
};

/* Keep track of the progress of a delete_all operation. 
static struct
{
    bool delete_next;   //!< Delete next record.
    bool pending;       //!< Waiting for an fds FDS_EVT_DEL_RECORD event, to delete the next record.
} m_delete_all;
*/
/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;
static bool volatile m_fds_writeflag;
static bool volatile m_fds_gc_done;
static bool volatile m_fds_del_done;

static void fds_evt_handler(fds_evt_t const * p_evt)
{

    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_evt->result == NRF_SUCCESS)
            {
                m_fds_initialized = true;
            }
            break;

        case FDS_EVT_WRITE:
        case FDS_EVT_UPDATE:
        {
            if (p_evt->result == NRF_SUCCESS)
            {
				m_fds_writeflag = true;
            }                
        }break;
        case FDS_EVT_DEL_RECORD:
        {
            if (p_evt->result == NRF_SUCCESS)
            {

            }
            //m_delete_all.pending = false;
        } break;
        case FDS_EVT_GC:
            if (p_evt->result == NRF_SUCCESS)
            {
                m_fds_gc_done = true;
            }
        default:
            break;
    }
}

static void set_default(void) {

   // ensure the data point is valid.
    if(DEVICE_NUM_OF_XXX <= gtPara.tRemote.ulRoundRobin) { gtPara.tRemote.ulRoundRobin = 0; }
    if(DEVICE_NUM_OF_XXX <= gtPara.tRemote.ulIng       ) { gtPara.tRemote.ulIng        = 0; }
    if(0xFC == (gtPara.tRemote.bits_isCheapRemote & 0xFC)) { gtPara.tRemote.bits_isCheapRemote = 0x00; } // if the high 6-bit are all 1, there must be an old FW which not support Cheap Remote before.

    if(E_MODE_OF_WARNING_LIGHT__NUM <= gtPara.eModeOfWarningLight) { gtPara.eModeOfWarningLight = (E_ModeOfWarningLight)0; }//E_MODE_OF_WARNING_LIGHT__Triangle; }
    if(1 != gtPara.ulBrakeFunction) {gtPara.ulBrakeFunction = 1;}
    if(BEEP_MODE_MAX <= gtPara.eBeepMode) {gtPara.eBeepMode = BEEP_WHEN_TURNING_ONOFF;}
    if(2 <= gtPara.ulNeedCharge) {gtPara.ulNeedCharge = 0;}
    if( 100 != ( gtPara.ssKxyz[0]
                +gtPara.ssKxyz[1]
                +gtPara.ssKxyz[2]) ) {
                      gtPara.ssKxyz[0] = 100;
                      gtPara.ssKxyz[1] =   0;
                      gtPara.ssKxyz[2] =   0;
                      gtPara.ssThreshhold = -8700;
    }
    /*
    if( (LL_ADC_VALUE_BATTERY_FULL_min > gtPara.usAdcValueOfFullCharge)
    ||  (LL_ADC_VALUE_BATTERY_FULL     < gtPara.usAdcValueOfFullCharge) ) {
                gtPara.usAdcValueOfFullCharge = LL_ADC_VALUE_BATTERY_FULL;
    }
    */

    //front
    gtPara.ePatternOfFrontLight[0] = LED_ANIMATION_SOLID_MODE;
    gtPara.ePatternOfFrontLight[1] = LED_ANIMATION_SLOWFLASH_MODE;
    gtPara.ePatternOfFrontLight[2] = LED_ANIMATION_FASTFLASH_MODE;    

    //rear
    gtPara.ePatternOfRearLight[0] = LED_ANIMATION_SOLID_MODE;
    gtPara.ePatternOfRearLight[1] = LED_ANIMATION_SLOWFLASH_MODE;
    gtPara.ePatternOfRearLight[2] = LED_ANIMATION_FASTFLASH_MODE;
     
    // update color table anyway
    //LL_LED_Panel_UpdateScrollingColor(gtPara.ucScrollColor[0], gtPara.ucScrollColor[1], gtPara.ucScrollColor[2]);

    // beep mode of power on/off
    if( (gtPara.beep_mode_of_power_on_off <= E_BEEP_MODE_OF_POWER_ON_OFF__MIN)
    ||  (gtPara.beep_mode_of_power_on_off >= E_BEEP_MODE_OF_POWER_ON_OFF__MAX) ) { 
            gtPara.beep_mode_of_power_on_off = E_BEEP_MODE_OF_POWER_ON_OFF__BEEP; 
    }

    unsigned long default_front_brightness[   E_MODE_OF_WARNING_LIGHT__NUM ] = {WS2812_COLOR_WHITE, WS2812_COLOR_WHITE, WS2812_COLOR_WHITE};
    unsigned long default_rear_brightness[   E_MODE_OF_WARNING_LIGHT__NUM ] = {WS2812_COLOR_RED_80, WS2812_COLOR_RED_80, WS2812_COLOR_RED_80};
    unsigned long default_flashing[     E_MODE_OF_WARNING_LIGHT__NUM ] = {0x00FFFFFF, 0x00AA0AA0, 0x00001001};
    for(int mode = 0; mode < E_MODE_OF_WARNING_LIGHT__NUM; mode++) {
        gtPara.customer_flashing_timeslot_states[mode][E_LL_LED_FRONT] = default_flashing[mode];
        gtPara.brightness_individual[mode][E_LL_LED_FRONT] = default_front_brightness[mode];
        
        gtPara.customer_flashing_timeslot_states[mode][E_LL_LED_REAR] = default_flashing[mode];  
        gtPara.brightness_individual[mode][E_LL_LED_REAR] = default_rear_brightness[mode];        
    }
    // paras use "flash_erase_flag"
    //unsigned long para_inited = 0xACAC0000 | 0x3B08; // 0x3B08: LL_VER_DATE
    // beep of turn L/R
    T_BUZZER_BEEP tDefaultBeepTypeOfTurningLBegin = BUZZER_BEEP__PER_2S;
    T_BUZZER_BEEP tDefaultBeepTypeOfTurningRBegin = BUZZER_BEEP__TWICE_PER_2S;
    T_BUZZER_BEEP tDefaultBeepTypeOfTurningEnd    = BUZZER_BEEP__NONE;
    T_BUZZER_BEEP *ptBeep; for(int index = 0; index < E_BEEP_OF_TURNING__TOTAL; index++) { // for each beep setting
        if(E_BEEP_OF_TURNING__L_BEGIN == index) {
            ptBeep = &tDefaultBeepTypeOfTurningLBegin;
        } else if(E_BEEP_OF_TURNING__R_BEGIN == index) {
            ptBeep = &tDefaultBeepTypeOfTurningRBegin;                
        } else {
            ptBeep = &tDefaultBeepTypeOfTurningEnd;
        }
        memcpy(&(gtPara.beep_of_turning[index]), ptBeep, sizeof(T_BUZZER_BEEP));
        gulFlashStoreNeeded = 1;
    }
    
    if(0 == gtPara.publicNetworkEnabled) { // if the 1st time access flash 
        //
        gtPara.publicNetworkEnabled = 1; // 0: disabled
        //
        extern T_LL_HardwareID gtHwID;
        memcpy(&(gtPara.netID), &(gtHwID.auc[0]), LL_ESB_BASE_ADDR_LEN);
        //
        gtPara.netID_exist = 0;
				gtPara.syncPowerOff = 1;
				gtPara.glFactoryMode = true;
    gulFlashStoreNeeded = 1; }      
}

void LL_Flash_init(void)
{
   ret_code_t err_code;
    
    //flag init
    m_fds_writeflag = true;
    m_fds_gc_done = true;
    m_fds_del_done = true;
    
   (void) fds_register(fds_evt_handler);//FDS register
    err_code = fds_init();//fds init
	
   //APP_ERROR_CHECK(err_code);
	if(err_code == NRF_SUCCESS){
        //
    }
    
	while (!m_fds_initialized)//wait init
    {
        sd_app_evt_wait();//standby
    }		
		
	fds_stat_t stat = {0};
    err_code = fds_stat(&stat);//Set statistics
    //APP_ERROR_CHECK(err_code);   
	if(err_code == NRF_SUCCESS){
        //
    }    
	
	//record_delete_next();//clear all records
			
	fds_record_desc_t desc = {0};//The descriptor structure used to manipulate the record is zeroed out
    fds_find_token_t  tok  = {0};//The token that saves the key is cleared
		
    err_code = fds_record_find(CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok);//Find data, file ID, record KEY
				
    if (err_code == NRF_SUCCESS)//if find successful
    {
        /* A config file is in flash. Let's update it. */
        fds_flash_record_t config = {0};//res

        /* Open the record and read its contents. */
        err_code = fds_record_open(&desc, &config);//open record to read data

        //APP_ERROR_CHECK(err_code);
        if(err_code == NRF_SUCCESS){
            //
        }        
        /* Copy the configuration from flash into m_dummy_cfg*/
        memcpy(&gtPara, config.p_data, sizeof(gtPara));

        //data = (uint32_t *)config.p_data;
                
        /* Close the record when done reading. */
        err_code = fds_record_close(&desc);//close record
        //APP_ERROR_CHECK(err_code);
        if(err_code == NRF_SUCCESS){
            //
        }
     }else{
        //flash load.
        set_default();
        memcpy(&gtStorePara, &gtPara, sizeof(gtPara));
        m_fds_writeflag = false;
        err_code = fds_record_write(&desc, &gtPara_record);//Write records and data. Descriptor, used later to access the record ID
        //APP_ERROR_CHECK(err_code);
        if(err_code == NRF_SUCCESS){
            //
        }         
	 }
     
     //detect valid records
     if(stat.valid_records > 0x01){ //if exist more than 1 valid record
        m_fds_del_done = false;
        err_code = fds_record_delete(&desc);
        //APP_ERROR_CHECK(err_code);
        if(err_code == NRF_SUCCESS){
            //
        }          

        for(uint16_t i=0; i<LL_FLASH_GC_WAITING_TIME; i++){
            if(m_fds_del_done){break;}
            nrf_delay_ms(1);
        }

        m_fds_gc_done = false;        
        err_code = fds_gc();
        //APP_ERROR_CHECK(err_code);  
        if(err_code == NRF_SUCCESS){
            //
        } 
        
        for(uint16_t i=0; i<LL_FLASH_GC_WAITING_TIME; i++){
            if(LL_Flash_gc_finish()){break;}
            nrf_delay_ms(1);
        }
      }
//        //if check err,set para anew
//        if(err_code == FDS_ERR_CRC_CHECK_FAILED){
//            // Handle CRC check failed
//            fds_record_delete(&desc); // delete the corrupted record

//            err_code = fds_gc(); // run garbage collection to free up space
//            APP_ERROR_CHECK(err_code);
//            
//            set_default();
//            
//            // Check if there is enough space after gc
//            fds_stat_t stat = {0};
//            err_code = fds_stat(&stat);
//            APP_ERROR_CHECK(err_code);

//            gulFlashStoreNeeded = 1; //wait 2s to store data
////            if(stat.freeable_words > gtPara_record.data.length_words){
////                err_code = fds_record_write(&desc, &gtPara_record); // write default data
////                record_para = err_code;
////                APP_ERROR_CHECK(err_code);                
////            } else{
////                
////            }
//        }else{
//            APP_ERROR_CHECK(err_code); 
//            /* Copy the configuration from flash into m_dummy_cfg*/
//            memcpy(&gtPara, config.p_data, sizeof(gtPara));
     
//            //data = (uint32_t *)config.p_data;
//                    
//            /* Close the record when done reading. */
//            err_code = fds_record_close(&desc);//close record
//            APP_ERROR_CHECK(err_code);
//        }            
//        
//    }else{ //not found record, first write		
//        //record_delete_next();//clear all records
//        set_default();
//        err_code = fds_record_write(&desc, &gtPara_record);//Write records and data. Descriptor, used later to access the record ID
//        APP_ERROR_CHECK(err_code);	
//        //while(!m_fds_writeflag);     
}

bool LL_Flash_store_finish(void)
{
    return m_fds_writeflag;
}

bool LL_Flash_gc_finish(void)
{
    return m_fds_gc_done;
}

void LL_Flash_store(void)
{
    ret_code_t err_code;
    fds_record_desc_t desc = {0};//The descriptor structure used to manipulate the record is zeroed out
    fds_find_token_t  tok  = {0};//The token that saves the key is cleared
    //fds_record_t record = {0};

    //gtStorePara = gtPara;
    memcpy(&gtStorePara, &gtPara, sizeof(gtPara));
    
    // Check the record exists
    err_code = fds_record_find( CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok );

    /* Write the updated record to flash.*/
    if( err_code == NRF_SUCCESS){
        //write_flag set
        m_fds_writeflag = false;
        err_code = fds_record_update(&desc, &gtPara_record);
        if(err_code == FDS_ERR_NO_SPACE_IN_FLASH){
            m_fds_gc_done = false;
            err_code = fds_gc();
            //APP_ERROR_CHECK(err_code);
            if(err_code == NRF_SUCCESS){
                //
            } 
            
            for(uint16_t i=0; i<LL_FLASH_GC_WAITING_TIME; i++){
                if(LL_Flash_gc_finish()){break;}
                nrf_delay_ms(1);
            }
            if(LL_Flash_gc_finish()){
                m_fds_writeflag = false;
                err_code = fds_record_update(&desc, &gtPara_record);
                if(err_code == NRF_SUCCESS){
                    //
                } 
                gulFlashStoreNeeded = 0; //interrupt other flash store
            }else{
                m_fds_writeflag = true;
                gulFlashStoreNeeded = 1; // wait next time to store flash
            }
        }               
        
        fds_stat_t stat = {0};
        err_code = fds_stat(&stat);
        //APP_ERROR_CHECK(err_code);	
        if(err_code == NRF_SUCCESS){
            //
        }         
    }		
}

void LL_Flash_store_beforeSleep(void)
{
    ret_code_t err_code;
    fds_record_desc_t desc = {0};//The descriptor structure used to manipulate the record is zeroed out
    fds_find_token_t  tok  = {0};//The token that saves the key is cleared
    //fds_record_t record = {0};
    
    //gtStorePara = gtPara;
    memcpy(&gtStorePara, &gtPara, sizeof(gtPara));
    
    // Check the record exists
    err_code = fds_record_find( CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok );

    /* Write the updated record to flash.*/
    if( err_code == NRF_SUCCESS){
        //write_flag set
        m_fds_writeflag = false;
        err_code = fds_record_update(&desc, &gtPara_record);
        if(err_code == FDS_ERR_NO_SPACE_IN_FLASH){
            m_fds_gc_done = false;
            err_code = fds_gc();
            //APP_ERROR_CHECK(err_code);
            if(err_code == NRF_SUCCESS){
                //
            } 
            
            for(uint16_t i=0; i<LL_FLASH_WAITING_TIME_BEFORE_SLEEP; i++){
                if(LL_Flash_gc_finish()){break;}
                nrf_delay_ms(1);
            }
            
            m_fds_writeflag = false;
            err_code = fds_record_update(&desc, &gtPara_record);
            
            for(uint16_t i=0; i<LL_FLASH_WAITING_TIME_BEFORE_SLEEP; i++){
                if(LL_Flash_store_finish()){break;}
                nrf_delay_ms(1);
            }            
        }          
        
        fds_stat_t stat = {0};
        err_code = fds_stat(&stat);
        //APP_ERROR_CHECK(err_code);	
        if(err_code == NRF_SUCCESS){
            //
        }         
    }		
}

void LL_Para_load(void) {

    // set default or convert to current version
    if(LL_PARA_VERSION == gtPara.para_version) { // if right version
        return;
    } else if(LL_FLASH_ERASE_VALUE_LONG == gtPara.para_version) { // if initial flash
        set_default();
    } else { // else, wrong version, need convert to right version or just set to default
        set_default();        
    }; 
    	
    // mark the right version after set default or convert to current version
    gtPara.para_version = LL_PARA_VERSION; gulFlashStoreNeeded = 1;
  
}
