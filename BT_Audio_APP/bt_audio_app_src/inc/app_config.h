/******************************************************************************
 * @file    app_config.h
 * @author
 * @version V_NEW
 * @date    2019-04-15
 * @maintainer
 * @brief
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "type.h"
#include "i2c_host.h"
#include "flash_boot.h"

//************************************************************************************************************
//    固件版本FIRMWARE_VERSION 在flash_config.h配置
// ***********************************************************************************************************

//************************************************************************************************************
//    FLASH_BOOT功能配置使能 请在flash_boot.h中选择配置。
// 1、flash boot占用最开始的64K Byte */
// 2、默认启用flash boot功能，FLASH_BOOT_EN = 1 */
// 3、flashboot实施mva包升级，必须确保void stub(void)指定的0xB8~0xBB版本号不同，同版本不升级*/
// 4、使用pctool mva升级usercode功能，需开启usbdevice模式（检测）*/
//************************************************************************************************************

//************************************************************************************************************
//    本系统默认开启2个系统全局宏，在IDE工程配置(Build Settings-Compiler-Symbols)，此处用于提醒
//*CFG_APP_CONFIG 和 FUNC_OS_EN*/
//************************************************************************************************************

//************************************************************************************************************
//    功能开关说明：
// *CFG_APP_*  : 软件系统应用模式开启，比如USB U盘播放歌曲应用模式的选择
// *CFG_FUNC_* : 软件功能开关
// *CFG_PARA_* : 系统软件参数设置
// *CFG_RES_*  : 系统硬件资源配置
// ************************************************************************************************************

//****************************************************************************************
//       B0,B1口作为DEBUG仿真功能选择配置
// 说明:
//    1.若用到linein4(B0,B1),为了防止转到linein4通道后无法仿真，需要开启此宏；
//    2.若用到linein4(B0,B1),量产时需要关闭此宏，否则转到此通道时会有POP声；
//****************************************************************************************
#define CFG_FUNC_SW_DEBUG_EN  

//****************************************************************************************
//       芯片型号选择配置
// 应用开发时，一定要选对型号,防止上位机显示的mic和linein输入和硬件接口不一致
//****************************************************************************************
#define  CFG_CHIP_BP10128      //128pin,开发板上用
//#define  CFG_CHIP_BP1064A2     //64pin,专业版本型号，主频最高:320Mhz,内置flash容量:16Mbit
//#define  CFG_CHIP_BP1064L2     //64pin,普通版本型号，主频最高:288Mhz,内置flash容量:16Mbit,支持低功耗RTC
//#define  CFG_CHIP_BP1048A2     //48pin,普通版本型号，主频最高:288Mhz,内置flash容量:16Mbit
//#define  CFG_CHIP_BP1048B2     //48pin,普通版本型号，主频最高:288Mhz,内置flash容量:16Mbit
//#define  CFG_CHIP_BP1048P2     //48pin,专业版本型号，主频最高:320Mhz,内置flash容量:16Mbit
//#define  CFG_CHIP_BP1048P4       //48pin,专业版本型号，主频最高:320Mhz,内置flash容量:32Mbit
//#define  CFG_CHIP_BP1032       //32pin,普通版本型号，主频最高:288Mhz,内置flash容量:16Mbit

	#if  defined(CFG_CHIP_BP1048A2) || defined(CFG_CHIP_BP1048B2) || defined(CFG_CHIP_BP1048P2) || defined(CFG_CHIP_BP1048P4) ||defined(CFG_CHIP_BP1064A2) || defined(CFG_CHIP_BP1064L2) || defined(CFG_CHIP_BP10128)
	//0=NO MIC, 1= MIC1, 2= MIC2, 3 = MCI1+MIC2
	#define CFG_RES_MIC_SELECT      (3)
	#endif

	#ifdef CFG_CHIP_BP1032
	//0=NO MIC, 1= MIC1, 2= MIC2, 3 = MCI1+MIC2
	#define CFG_RES_MIC_SELECT      (1)
	#endif	

//****************************************************************************************
// 系统App功能模式选择
//****************************************************************************************

/**UDisk播放模式**/
#define	CFG_APP_USB_PLAY_MODE_EN

/**SD Card播放模式**/
#define	CFG_APP_CARD_PLAY_MODE_EN

/**线路输入模式**/
#define	CFG_APP_LINEIN_MODE_EN

#ifdef CFG_APP_LINEIN_MODE_EN
    #if defined(CFG_CHIP_BP1064A2) || defined(CFG_CHIP_BP1064L2) || defined(CFG_CHIP_BP10128) 
    #define LINEIN_INPUT_CHANNEL                (ANA_INPUT_CH_LINEIN1)
	#elif defined(CFG_CHIP_BP1032)
	#define LINEIN_INPUT_CHANNEL                (ANA_INPUT_CH_LINEIN4)
	#else
	#define LINEIN_INPUT_CHANNEL                (ANA_INPUT_CH_LINEIN5)
	#endif
    //#define CFG_LINEIN_DET_EN
    #ifdef	CFG_LINEIN_DET_EN
		#define LINEIN_DET_GPIO					GPIOA9
		#define LINEIN_DET_GPIO_IN 				GPIO_A_IN
		#define LINEIN_DET_BIT_MASK				GPIO_INDEX9
		#define LINEIN_DET_GPIO_IE 				GPIO_A_IE
		#define LINEIN_DET_GPIO_OE 				GPIO_A_OE
		#define LINEIN_DET_GPIO_PU 				GPIO_A_PU
		#define LINEIN_DET_GPIO_PD 				GPIO_A_PD
    #endif
#endif

/**HDMI IN模式**/
#define CFG_APP_HDMIIN_MODE_EN

/**蓝牙功能**/
#define CFG_APP_BT_MODE_EN
#ifdef CFG_APP_BT_MODE_EN
	/*某些需求:在非蓝牙模式下,尽可能释放协议栈/EM/BB相关的资源*/
	/*如开启了HFP,最好开启蓝牙后台运行功能*/
	#define CFG_BT_BACKGROUND_RUN_EN /*蓝牙后台运行开关*/
	#include "bt_config.h"
#endif

/**收音机功能**/
#define CFG_APP_RADIOIN_MODE_EN
#ifdef CFG_APP_RADIOIN_MODE_EN
	#define RADIO_INPUT_CHANNEL                ANA_INPUT_CH_LINEIN4
	
	#define	CFG_RADIO_CLK_M12  //sdk 默认配置A29口输出12M clk
	//#define CFG_RADIO_I2C_SD_SAME_PIN_EN//fm和sd复用data/clk
	
	#define CFG_RADIO_IIC_ENABLE()				GPIO_PortBModeSet(GPIOB4, 0x0),GPIO_PortBModeSet(GPIOB5, 0x0)
	#define CFG_RADIO_IIC_HANDLER()				I2cMasterCreate(PORT_B, 5, PORT_B, 4)

    #define FUNC_RADIO_RDA5807_EN
    //#define FUNC_RADIO_QN8035_EN  //芯片io时钟只支持12M，qn8035必须外挂晶振

#if defined(FUNC_RADIO_RDA5807_EN) && defined(FUNC_RADIO_QN8035_EN)
   #error Conflict: radio type error //不能同时选择两种显示模式
#endif
#endif

/**SPDIF 光纤模式**/
#define	CFG_APP_OPTICAL_MODE_EN
#ifdef CFG_APP_OPTICAL_MODE_EN
	#define SPDIF_OPTICAL_PORT_ANA_INPUT	SPDIF_ANA_INPUT_A30
#endif

/**SPDIF 同轴模式**/
#define CFG_APP_COAXIAL_MODE_EN

/**USB声卡，读卡器，一线通功能 **/
#define CFG_APP_USB_AUDIO_MODE_EN
#define CFG_PARA_USB_MODE	AUDIO_MIC
#ifdef  CFG_APP_USB_AUDIO_MODE_EN
	#define CFG_RES_AUDIO_USB_IN_EN
	#define CFG_RES_AUDIO_USB_OUT_EN
	#define CFG_RES_AUDIO_USB_SRC_EN
	#define CFG_RES_AUDIO_USB_VOL_SET_EN
#endif

/**I2SIN输入模式**/
#define CFG_APP_I2SIN_MODE_EN

//待机模式使能 ，禁止mic和主音源，禁止后插先播，录音，回放等
//注意不要与CFG_FUNC_REMIND_DEEPSLEEP同时使用
//#define CFG_APP_REST_MODE_EN
//#define CFG_FUNC_REMIND_WAKEUP		//deepsleep后可以出现开机提示音。

//****************************************************************************************
//                 音频输出通道相关配置
//说明:
//    如下输出源可同时输出；
//****************************************************************************************
/**DAC-0通道配置选择**/
#define CFG_RES_AUDIO_DAC0_EN

/**DAC-X通道配置选择**/
#define CFG_RES_AUDIO_DACX_EN

/**I2S音频输出通道配置选择**/
//#define CFG_RES_AUDIO_I2SOUT_EN

//****************************************************************************************
//     I2S相关配置选择
//说明:
//    1.I2S输出也使能时端口选择和模式需要注意保持一致;
//    2.I2s如果作为slave，需要注意MCLK选择和采样率问题，以及异步时钟问题;
//	  3.CFG_FUNC_AUDIO_EFFECT_EN使能时，I2S参数上电后跟随音效文件和在线调音动态调整。
//****************************************************************************************
#if defined(CFG_APP_I2SIN_MODE_EN) || defined(CFG_RES_AUDIO_I2SOUT_EN)
	#define	CFG_RES_I2S_PORT  					1//1:I2S1总线; 0:I2S0总线
	#define	CFG_RES_I2S_MODE					0//0:master mode ;1:slave mode
	#if (CFG_RES_I2S_PORT==0)
		#define	CFG_RES_I2S_IO_PORT				0//  0:i2S0-A0-1-2-3-4; 1:I2S1-A7-8-9-10-11
	#else
		#define	CFG_RES_I2S_IO_PORT				1//  0:i2S0-A0-1-2-3-4; 1:I2S1-A7-8-9-10-11
	#endif
#endif

//****************************************************************************************
//                 解码器类型选择
//说明:
//    如下解码器类型选择会影响code size;
//****************************************************************************************
#define USE_MP3_DECODER
#define USE_WMA_DECODER
#define USE_SBC_DECODER
#define USE_WAV_DECODER
//#define USE_FLAC_DECODER	//24bit 1.5Mbps高码率时，需要扩大DECODER_FIFO_SIZE_FOR_PLAYER 输出fifo，或扩大输入：FLAC_INPUT_BUFFER_CAPACITY
//#define USE_AAC_DECODER
//#define USE_AIF_DECODER
//#define USE_AMR_DECODER
//#define USE_APE_DECODER

//****************************************************************************************
//                 总音效功能配置
//****************************************************************************************
//高低音调节功能配置说明:
//    1.此功能是基于MIC OUT EQ进行手动设置的，需要在调音参数中使能此EQ；
//    2.默认f5对应bass gain,f6对应treb gain,若调音界面上修改此EQ filter数目，需要对应修改BassTrebAjust()中对应序号；
//EQ模式功能配置说明:
//    1.此功能是基于MUSIC EQ进行手动设置的，需要在调音参数中使能此EQ；
//    2.可在flat/classic/pop/jazz/pop/vocal boost之间通过KEY来切换   
#define CFG_FUNC_AUDIO_EFFECT_EN //总音效使能开关
#ifdef CFG_FUNC_AUDIO_EFFECT_EN

    #define CFG_FUNC_ECHO_DENOISE          //消除快速调节delay时的杂音，
 	//#define CFG_FUNC_MUSIC_EQ_MODE_EN     //Music EQ模式功能配置               
	#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN	    
 		#define CFG_FUNC_EQMODE_FADIN_FADOUT_EN    //EQ模式切换时fade in/fade out处理功能选择,调节EQ模式中有POP声时，建议打开 		
    #endif
	//#define CFG_FUNC_MUSIC_TREB_BASS_EN    		//Music高低音调节功能配置
    //#define CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN     //无信号自动关机功能，
    #ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN      
		#define  SILENCE_THRESHOLD                 120        //设置信号检测门限，小于这个值认为无信号
		#define  SILENCE_POWER_OFF_DELAY_TIME      60*1000     //无信号关机延时时间，单位：ms
    #endif

	#if CFG_RES_MIC_SELECT
	#define	CFG_FUNC_MIC_KARAOKE_EN      //MIC karaoke功能选择
	#endif

	#ifdef CFG_FUNC_MIC_KARAOKE_EN
		//#define CFG_FUNC_GUITAR_EN        //吉他音效功能选择
		//#define CFG_FUNC_DETECT_MIC_EN 	//mic插拔检测功能
        #define CFG_FUNC_MIC_TREB_BASS_EN   //mic高低音调节功能配置            
		//闪避功能选择设置
		//注:若需要完善移频开启后的啾啾干扰声问题，需要开启此功能(利用MIC信号检测接口处理)
		#define  CFG_FUNC_SHUNNING_EN                                
			#define SHNNIN_VALID_DATA                          	 500  ////MIC音量阈值
			#define SHNNIN_STEP                                  256  /////单次调节的步数
			#define SHNNIN_THRESHOLD                             SHNNIN_STEP*2  ////threshold
			#define SHNNIN_VOL_RECOVER_TIME                      50////伴奏音量恢复时长：100*20ms = 2s
			#define SHNNIN_UP_DLY                                3/////音量上升时间
			#define SHNNIN_DOWN_DLY                              1/////音量下降时间
	#endif	

	//**音频SDK版本号,不要修改**/
	#define  CFG_EFFECT_MAJOR_VERSION						(CFG_SDK_MAJOR_VERSION)
	#define  CFG_EFFECT_MINOR_VERSION						(CFG_SDK_MINOR_VERSION)
	#define  CFG_EFFECT_USER_VERSION						(CFG_SDK_PATCH_VERSION)		

	/**在线调音硬件接口有USB HID接口，或者UART接口*/
	/**建议使用USB HID接口，收发buf共512Byte*/
	/**UART接口占用2路DMA，收发Buf共2k Byte*/
	/**建议使用USB HID作为调音接口，DMA资源紧张*/
	/**如果使用UART作为调音接口请设置DMA接口并保证DMA资源充足*/
	#define  CFG_COMMUNICATION_BY_USB		// usb or uart 					
	//#define  CFG_COMMUNICATION_BY_UART		// usb or uart 				
	
	#define	 CFG_UART_COMMUNICATION_TX_PIN					GPIOA10
	#define  CFG_UART_COMMUNICATION_TX_PIN_MUX_SEL			(3)
	#define  CFG_UART_COMMUNICATION_RX_PIN					GPIOA9
	#define  CFG_UART_COMMUNICATION_RX_PIN_MUX_SEL			(1)
	
	#define  CFG_COMMUNICATION_CRYPTO						(0)////调音通讯加密=1 调音通讯不加密=0
	#define  CFG_COMMUNICATION_PASSWORD                     0x11223344//////四字节的长度密码
#endif

//****************************************************************************************
//     音频相关配置参数
//****************************************************************************************
#define	CFG_PARA_SAMPLE_RATE				(44100)//(48000)//
#define CFG_BTHF_PARA_SAMPLE_RATE			(16000)//蓝牙模式下统一采样率为16K
#define	CFG_PARA_SAMPLES_PER_FRAME          (256)//(512)	//系统默认的帧数大小 //注:关闭总音效开关时,必须配置为256,和通话sample配置参数保持一致
#define	CFG_BTHF_PARA_SAMPLES_PER_FRAME     (256)			//蓝牙通话模式下帧数大小
#define CFG_PARA_MIN_SAMPLES_PER_FRAME		(128)//         //系统最小帧数大小，保证mic delay最小
#define CFG_PARA_MAX_SAMPLES_PER_FRAME		(512)//(512)

#if (BT_AVRCP_VOLUME_SYNC == ENABLE) && defined(CFG_APP_BT_MODE_EN)
#define CFG_PARA_MAX_VOLUME_NUM		        (16)	//SDK 16 级音量,针对iphone手机蓝牙音量同步功能定制，音量表16级能一一对应手机端音量级别
#define CFG_PARA_SYS_VOLUME_DEFAULT			(12)	//SDK默认音量
#else
#define CFG_PARA_MAX_VOLUME_NUM		        (32)	//SDK 32 级音量
#define CFG_PARA_SYS_VOLUME_DEFAULT			(25)	//SDK默认音量
#endif

//****************************************************************************************
//     转采样功能选择
//说明:
//    1.使能该宏表示系统会将会以统一的采样率输出，默认使用44.1KHz;
//    2.此版本默认打开，请勿关闭!!!!!!!!!!
//****************************************************************************************	
#define	CFG_FUNC_MIXER_SRC_EN

//****************************************************************************************
//     字符编码类型转换
//说明:
//    1.目前支持Unicode     ==> Simplified Chinese (DBCS)
//    2.字符转换库由fatfs提供，故需要包含文件系统
//    3.如果支持转换其他语言，需要修改fatfs配置表
//****************************************************************************************	
#if(defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN) || BT_AVRCP_SONG_TRACK_INFOR)
#define CFG_FUNC_STRING_CONVERT_EN	// 支持字符编码转换
#endif

//****************************************************************************************
//     采样率微调功能选择
//说明:
//    监测异步音源混音缓冲区水位，跟随音源(时钟)
//	     此微调检测只有一路，同一时刻只可使能开启一个微调。
//****************************************************************************************	
#define	CFG_FUNC_FREQ_ADJUST
#ifdef CFG_FUNC_FREQ_ADJUST
/**	#define CFG_FUNC_APLL_ADJUST_IN **/			//系统异步时钟源根据输入源来微调, 缺省 硬件微调
	//#define CFG_FUNC_SOFT_ADJUST_IN				//系统异步时钟源根据输入源来微调, 使用软件微调
	#ifdef CFG_FUNC_SOFT_ADJUST_IN
		#define CFG_FUNC_SOFT_ADJUST_IN_I2SIN  //I2SIN有多种情况，增加一个宏开关，用于独立控制，如果有MCLK情况下，无需微调，关闭改宏
		#define CFG_FUNC_SOFT_ADJUST_OUT_USBAUDIO //USB 输出使用软件微调
	#endif
#endif

//****************************************************************************************
//                 录音功能配置
//说明:
//    1.录音支持2种情况，一是写入到SD卡/U盘；二是写入到Flash；
//    2.以上2种情况不能同时支持，开启宏时注意一下
//****************************************************************************************
#define CFG_FUNC_RECORDER_EN
#ifdef CFG_FUNC_RECORDER_EN
	#define CFG_FUNC_RECORD_SD_UDISK	//录音到SD卡或者U盘
	//#define CFG_FUNC_RECORD_FLASHFS 	//不可同时开启 CFG_FUNC_RECORD_SD_UDISK
	
	#ifdef CFG_FUNC_RECORD_SD_UDISK
		#define CFG_FUNC_RECORD_UDISK_FIRST				//U盘和卡同时存在时，录音设备优先选择U盘，否则优先选择录音到SD卡。
		#define CFG_PARA_RECORDS_FOLDER 		"REC"	//录卡录U盘时根目录文件夹。注意ffpresearch_init 使用回调函数过滤字符串。
		#define CFG_FUNC_RECORDS_MIN_TIME		1000	//单位ms，开启此宏后，小于这个长度的自动删除。
		#define CFG_PARA_REC_MAX_FILE_NUM       256     //录音文件最大数目

		#define MEDIAPLAYER_SUPPORT_REC_FILE            // U盘或TF卡模式下，打开此功能，则支持播放录音文件；否则，只能在录音回放模式下播放录音文件
        //#define AUTO_DEL_REC_FILE_FUNCTION            //录音文件达到最大数后，自动删除全部录音文件的功能选项
	#endif
	
	#define CFG_PARA_REC_GAIN		        (8191)	    //输入录音增益   8191:+6db;7284:+5db;6492:+4db;5786:+3db;5157:+2db;4596:+1db;4095:0db;

	#define DEL_REC_FILE_EN
	

	/*注意flash空间，避免冲突   middleware/flashfs/file.h FLASH_BASE*/
	#ifdef CFG_FUNC_RECORD_FLASHFS
		#define CFG_PARA_FLASHFS_FILE_NAME		"REC.MP3"//RECORDER_FILE_NAME
	#endif

	//N >= 2 ；考虑128系统帧以及加音效MIPS较高，优先级为3的编码进程处理数据较慢，推荐值为 6。提高系统帧，mips低时可以调小N,节约ram。
	#define MEDIA_RECORDER_FIFO_N				6
	#define MEDIA_RECORDER_FIFO_LEN				(CFG_PARA_MAX_SAMPLES_PER_FRAME * MEDIA_RECORDER_CHANNEL * MEDIA_RECORDER_FIFO_N)
	//调整下列参数后，录音介质可能需要重做兼容性测试 适配FILE_WRITE_FIFO_LEN。
	#define MEDIA_RECORDER_CHANNEL				2
	#define MEDIA_RECORDER_BITRATE				96 //Kbps
	#define MEDIA_RECODER_IO_BLOCK_TIME			1000//ms
	//FIFO_Len=(码率(96) / 8 * 缓冲时间ms(1000) （码率单位Kbps,等效毫秒）
	//根据SDIO协议，写卡阻塞存在250*2ms阻塞 可能，实测部分U盘存在785ms周期性写入阻塞，要求编码数据fifo空间 确保超过这个长度的两倍(含同步)。
	#define FILE_WRITE_FIFO_LEN					((((MEDIA_RECORDER_BITRATE / 8) * MEDIA_RECODER_IO_BLOCK_TIME ) / 512) * 512)//(依据U盘/Card兼容性需求和RAM资源可选400~1500ms。按扇区512对齐
#endif //CFG_FUNC_RECORDER_EN

//****************************************************************************************
//                 U盘或SD卡模式相关功能配置
//    
//****************************************************************************************
#if(defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_APP_CARD_PLAY_MODE_EN))
/**LRC歌词功能 **/
#define CFG_FUNC_LRC_EN			 	// LRC歌词文件解析

/*------browser function------*/
//#define FUNC_BROWSER_PARALLEL_EN  		//browser Parallel
//#define FUNC_BROWSER_TREE_EN  			//browser tree
//#define FUNC_SPECIFY_FOLDER_PLAY_EN    //APP :story machine
#if	defined(FUNC_BROWSER_PARALLEL_EN)||defined(FUNC_BROWSER_TREE_EN)||defined(FUNC_SPECIFY_FOLDER_PLAY_EN)
#define FUNCTION_FILE_SYSTEM_REENTRY
#if defined(FUNC_BROWSER_TREE_EN)||defined(FUNC_BROWSER_PARALLEL_EN)
#define GUI_ROW_CNT_MAX		5		//最多显示多少行
#else
#define GUI_ROW_CNT_MAX		1		//最多显示多少行
#endif
#endif
/*------browser function------*/


#define PREVIEW_PLAY_TIME	10


/**后插先播功能（开启后关闭后插先播）**/
//#define CFG_FUNC_APP_MODE_AUTO

/**取消AA55判断**/
/*fatfs内磁盘系统MBR和DBR扇区结尾有此标记检测，为提高非标类型盘兼容性，可开启此项, 为有效鉴定无效盘，此项默认关闭*/
//#define	CANCEL_COMMON_SIGNATURE_JUDGMENT

//#define FUNC_UPDATE_CONTROL   //升级交互过程控制(通过按键确认升级)
#endif

//****************************************************************************************
//           TF卡，USB设备检测功能
//****************************************************************************************
#if (defined(CFG_APP_CARD_PLAY_MODE_EN) )|| defined(CFG_FUNC_RECORD_SD_UDISK) || defined(CFG_APP_USB_AUDIO_MODE_EN)
#define	CFG_RES_CARD_USE  //启用SD电路
#ifdef CFG_RES_CARD_USE
#include "sd_card.h"

#ifdef CFG_CHIP_BP10128
	#define	CFG_RES_CARD_GPIO				SDIO_A20_A21_A22 //注意其他文件参数修改。
#else
	#define	CFG_RES_CARD_GPIO				SDIO_A15_A16_A17 //注意其他文件参数修改。
#endif
#define	CFG_FUNC_CARD_DETECT //检测sd卡插入弹出，电路配合

#if CFG_RES_CARD_GPIO == SDIO_A15_A16_A17
	#define SDIO_Clk_Disable				SDIO_ClkDisable
	#define SDIO_Clk_Eable					SDIO_ClkEnable
	#define CARD_DETECT_GPIO				GPIOA16
	#define CARD_DETECT_GPIO_IN				GPIO_A_IN
	#define CARD_DETECT_BIT_MASK			GPIOA16
	#define CARD_DETECT_GPIO_IE				GPIO_A_IE
	#define CARD_DETECT_GPIO_OE				GPIO_A_OE
	#define CARD_DETECT_GPIO_PU				GPIO_A_PU
	#define CARD_DETECT_GPIO_PD				GPIO_A_PD
	#define CARD_DETECT_GPIO_OUT			GPIO_A_OUT
#elif CFG_RES_CARD_GPIO == SDIO_A20_A21_A22
	#define SDIO_Clk_Disable				SDIO_ClkDisable
	#define SDIO_Clk_Eable					SDIO_ClkEnable
	#define CARD_DETECT_GPIO				GPIOA21
	#define CARD_DETECT_GPIO_IN				GPIO_A_IN
	#define CARD_DETECT_BIT_MASK			GPIOA21
	#define CARD_DETECT_GPIO_IE				GPIO_A_IE
	#define CARD_DETECT_GPIO_OE				GPIO_A_OE
	#define CARD_DETECT_GPIO_PU				GPIO_A_PU
	#define CARD_DETECT_GPIO_PD				GPIO_A_PD
#endif

#endif //CFG_RES_CARD_USE
#endif

/**USB Host检测功能**/
#if(defined(CFG_APP_USB_PLAY_MODE_EN) || defined(CFG_FUNC_RECORD_SD_UDISK))
	#define CFG_RES_UDISK_USE
	#define CFG_FUNC_UDISK_DETECT
#endif

/**USB Device检测功能**/
#if (defined (CFG_APP_USB_AUDIO_MODE_EN)) || (defined(CFG_COMMUNICATION_BY_USB))
	#define CFG_FUNC_USB_DEVICE_EN
#endif

//****************************************************************************************
//                 BT功能配置
//说明:
//    1.蓝牙相关功能配置细节请在bt_config.h中定制!!!!!!!!!!
//    
//****************************************************************************************
#ifdef CFG_APP_BT_MODE_EN

//来电报号和铃声
//#define CFG_BT_NUMBER_REMIND
#define CFG_BT_RING_REMIND
#define CFG_BT_RING_LOCAL	//打开该宏后，如果手机铃声没传过来就播放本地铃声
#define CFG_BT_RING_TIME	800	//设置本地铃声播放间隔时间

//-------------------------------//
//       AI功能选择
//说明:目前支持两种APP 小Q同学  小爱同学
//************小Q同学*************//
//1，需要下载小Q同学
//2. 删除 xiaomi_ai和libopus文件，因为是重复的头文件，工程中忽略目录也会报错。
//3. CFG_RES_MIC_SELECT不能为0
//4. 以下信息需要向小Q同学服务商提供
//     const char *APP_ID = "";
//     const char *SECRET = "";
//5. AI需要同时开启BLE和SPP协议
//   BT_SPP_SUPPORT == ENABLE && BLE_SUPPORT ==  ENABLE
//6. BT名称必须为"Hifier-mic"	(#define BT_NAME "Hifier-mic")
//7. 同时打开	CFG_AI_ENCODE_EN & CFG_FUNC_AI_EN


//************小爱同学*************//
//1，需要下载小爱同学，与小米开发人员联系
//2. 删除 ai 文件，因为是重复的头文件，工程中忽略目录也会报错。
//3. CFG_RES_MIC_SELECT不能为0
//4. AI需要同时开启BLE和SPP协议
//   BT_SPP_SUPPORT == ENABLE && BLE_SUPPORT ==  ENABLE
//5. 同时打开  CFG_AI_ENCODE_EN & CFG_XIAOAI_AI_EN
//-------------------------------//

//请不要同时打开小爱和小Q的宏

//#define   CFG_FUNC_AI_EN		//小Q同学_enable
//#define 	CFG_XIAOAI_AI_EN	//小爱同学_enable

#ifdef	CFG_FUNC_AI_EN
	#ifndef	CFG_AI_ENCODE_EN
		#define   CFG_AI_ENCODE_EN
	#endif
#endif

#ifdef	CFG_XIAOAI_AI_EN
	#ifndef	CFG_AI_ENCODE_EN
		#define   CFG_AI_ENCODE_EN
	#endif
#endif

#ifdef	CFG_FUNC_AI_EN
	#ifdef	CFG_XIAOAI_AI_EN
		#error	"Please do not enable ( CFG_FUNC_AI_EN & CFG_XIAOAI_AI_EN )!!!"
	#endif
#endif

/**BB EM参数配置 **/
#define BB_EM_MAP_ADDR			0x80000000
#define BB_EM_SIZE				(16*1024)		//(12*1024)//当不需要BLE和HFP功能时,可以使用该EM大小,同时需要更新定制lib
#define BB_EM_START_PARAMS		((320*1024-BB_EM_SIZE)/1024)
#define BB_MPU_START_ADDR		(0x20050000 - BB_EM_SIZE)

#else
#define BB_EM_MAP_ADDR			0x80000000
#define BB_EM_START_PARAMS		0
#define BB_EM_SIZE				0
#define BB_MPU_START_ADDR		(0x20050000 - BB_EM_SIZE)
#endif

//****************************************************************************************
//                            HDMI功能相关配置参数
//						CEC管脚对ARC管脚有影响，需要注意PCB走线
//****************************************************************************************
#ifdef  CFG_APP_HDMIIN_MODE_EN
	/**ARC**/
	#define HDMI_ARC_RECV_IO_OE				GPIO_A_OE
	#define HDMI_ARC_RECV_IO_IE				GPIO_A_IE
	#define HDMI_ARC_RECV_IO_ANA			GPIO_A_ANA_EN
	#define HDMI_ARC_RECV_IO_PIN			GPIOA29
	#define HDMI_ARC_RECV_ANA_PIN           SPDIF_ANA_INPUT_A29

	/**CEC**/
	//timer3可以使用rc时钟或者系统时钟，
	//PWC可以复用所有GPIO口，而PWM只有指定GPIO口可以用，所以二者接口参数用法不一样，CEC接收IO无需设置RECV_IO和RECV_IO_PIN
	#define HDMI_CEC_CLK_MODE				RC_CLK_MODE//SYSTEM_CLK_MODE//RC_CLK_MODE /*启用cec唤醒功能时，只能用rc*/
	#define HDMI_CEC_IO_TYPE				IO_TYPE_A
	#define HDMI_CEC_IO_INDEX				27
	//#define HDMI_CEC_RECV_IO	            28
	//#define HDMI_CEC_RECV_IO_PIN	    	28
	#define HDMI_CEC_SEND_IO	            TIMER3_PWM_A0_A8_A22_A27 /*dma table配合*/
	#define HDMI_CEC_SEND_IO_PIN			(3)
    #define HDMI_CEC_RECV_DATA_ADDR	    	TIMER3_PWC_DATA_ADDR
    #define HDMI_CEC_SEND_DATA_ADDR         TIMER3_PWM_DUTY_ADDR

	#define CFG_PARA_WAKEUP_GPIO_CEC		WAKEUP_GPIOA27

	#define	TIMER3_PWC_DATA_ADDR			(0x4002C034)
	#define TIMER3_PWM_DUTY_ADDR			(0x4002C024)

	#define	TIMER4_PWC_DATA_ADDR			(0x4002C834)
	#define TIMER4_PWM_DUTY_ADDR			(0x4002C824)

	#define	TIMER5_PWC_DATA_ADDR			(0x4002F034)
	#define TIMER5_PWM_DUTY_ADDR			(0x4002F024)

	#define	TIMER6_PWC_DATA_ADDR			(0x4002F834)
	#define TIMER6_PWM_DUTY_ADDR			(0x4002F824)

	#define HDMI_CEC_RECV_TIMER_ID	        TIMER3
	#define HDMI_CEC_RECV_DMA_ID	        PERIPHERAL_ID_TIMER3
	#define HDMI_CEC_RECV_DMA_ADDR	        TIMER3_PWC_DATA_ADDR

	#define HDMI_CEC_SEND_TIMER_ID	        TIMER3
	#define HDMI_CEC_SEND_DMA_ID	        PERIPHERAL_ID_TIMER3
	#define HDMI_CEC_SEND_DMA_ADDR	        TIMER3_PWM_DUTY_ADDR

	#define HDMI_CEC_ARBITRATION_TIMER_ID   TIMER5
	#define HDMI_CEC_ARBITRATION_TIMER_IRQ  Timer5_IRQn
	#define HDMI_CEC_ARBITRATION_TIMER_FUNC Timer5Interrupt

	/**HPD**/
	#define HDMI_HPD_CHECK_DETECT_EN        //HDMI后插先播功能选项
	
	#define HDMI_HPD_CHECK_STATUS_IO	   	GPIO_A_IN
	#define HDMI_HPD_CHECK_INI_IO		   	GPIO_A_INT
	#define HDMI_HPD_CHECK_STATUS_IO_PIN   	GPIOA24
	#define HDMI_HPD_CHECK_IO_INIT()	   	GPIO_RegOneBitClear(GPIO_A_PU, HDMI_HPD_CHECK_STATUS_IO_PIN),\
										   	GPIO_RegOneBitSet(GPIO_A_PD, HDMI_HPD_CHECK_STATUS_IO_PIN),\
										   	GPIO_RegOneBitClear(GPIO_A_OE, HDMI_HPD_CHECK_STATUS_IO_PIN),\
										   	GPIO_RegOneBitSet(GPIO_A_IE, HDMI_HPD_CHECK_STATUS_IO_PIN),\
										   	GPIO_INTEnable(GPIO_A_INT, HDMI_HPD_CHECK_STATUS_IO_PIN, GPIO_EDGE_TRIGGER),\
										   	GPIO_INTFlagClear(GPIO_A_INT, HDMI_HPD_CHECK_STATUS_IO_PIN)

	/**DDC**/
	//#define DDC_USE_SW_I2C
	#ifdef DDC_USE_SW_I2C
		#define DDCSclPortSel               PORT_B
		#define SclIndex                    (5)
		#define DDCSdaPortSel               PORT_B
		#define SdaIndex                    (4)
	#else
	   #define HDMI_DDC_DATA_IO_INIT()		GPIO_RegOneBitSet(GPIO_B_PU, GPIOB3),\
										    GPIO_RegOneBitClear(GPIO_B_PD, GPIOB3)
	   #define HDMI_DDC_CLK_IO_INIT()		GPIO_RegOneBitSet(GPIO_B_PU, GPIOB2),\
										    GPIO_RegOneBitClear(GPIO_B_PD, GPIOB2)
	   #define HDMI_DDC_IO_PIN				I2C_PORT_B4_B5
    #endif
#endif

//****************************************************************************************
//						SPDIF光纤同轴功能相关配置参数
//****************************************************************************************
#if (defined (CFG_APP_OPTICAL_MODE_EN)) || (defined (CFG_APP_COAXIAL_MODE_EN))
	#define	CFG_FUNC_SPDIF_EN

	#ifdef CFG_APP_OPTICAL_MODE_EN
		//#define PORT_B_INPUT_DIGATAL
		#ifndef PORT_B_INPUT_DIGATAL
		#define SPDIF_OPTICAL_INDEX				GPIOA30
		//#define SPDIF_PORT_OE					GPIO_A_OE
		//#define SPDIF_PORT_IE					GPIO_A_IE
		#define SPDIF_OPTICAL_PORT_MODE			8
		#define SPDIF_OPTICAL_PORT_ANA_INPUT	SPDIF_ANA_INPUT_A30
		#else
		#define SPDIF_OPTICAL_INDEX				GPIOB1
		//#define SPDIF_PORT_OE					GPIO_A_OE
		//#define SPDIF_PORT_IE					GPIO_A_IE
		#define SPDIF_OPTICAL_PORT_MODE			1//(digital)
		//#define SPDIF_OPTICAL_PORT_ANA_INPUT	SPDIF_ANA_INPUT_A31
		#endif
	#endif
	#ifdef CFG_APP_COAXIAL_MODE_EN
		#define SPDIF_COAXIAL_INDEX				GPIOA31
		//#define SPDIF_PORT_OE					GPIO_A_OE
		//#define SPDIF_PORT_IE					GPIO_A_IE
		#define SPDIF_COAXIAL_PORT_MODE			8
		#define SPDIF_COAXIAL_PORT_ANA_INPUT	SPDIF_ANA_INPUT_A31
	#endif
#endif

/**OS操作系统进入IDLE时经core进入休眠状态，以达到降低功耗目的**/
/*注意，这是OS调度的IDLE，并非应用层APPMODE，应用层无需关心*/
#define CFG_FUNC_IDLE_TASK_LOW_POWER
#ifdef	CFG_FUNC_IDLE_TASK_LOW_POWER
	#define	CFG_GOTO_SLEEP_USE
#endif

//****************************************************************************************
//                 深度休眠功能配置
//****************************************************************************************
//#define 	CFG_FUNC_MAIN_DEEPSLEEP_EN	//系统启动先睡眠，等唤醒。
#define	CFG_FUNC_DEEPSLEEP_EN //app模式运行时 可反复睡眠/唤醒。
#if defined(CFG_FUNC_DEEPSLEEP_EN) || defined(CFG_FUNC_MAIN_DEEPSLEEP_EN)
	/**至少一个唤醒源**/ //Source(通道) 不可重复,唤醒IO和功能IO配置需一致。 参见:deepsleep.c
		/*红外按键唤醒,注意CFG_PARA_WAKEUP_GPIO_IR和 唤醒键IR_KEY_POWER设置*/
	#define CFG_PARA_WAKEUP_SOURCE_IR		SYSWAKEUP_SOURCE9_IR//固定source9

		/*ADCKey唤醒 配合CFG_PARA_WAKEUP_GPIO_ADCKEY 唤醒键ADCKEYWAKEUP设置及其电平*/
	#define CFG_PARA_WAKEUP_SOURCE_ADCKEY	SYSWAKEUP_SOURCE1_GPIO//如使用ADC唤醒，必须打开CFG_RES_ADC_KEY_SCAN 和CFG_RES_ADC_KEY_USE
	#define CFG_PARA_WAKEUP_SOURCE_POWERKEY	SYSWAKEUP_SOURCE6_POWERKEY
	#define CFG_PARA_WAKEUP_SOURCE_CEC		SYSWAKEUP_SOURCE2_GPIO//HDMI专用，CFG_PARA_WAKEUP_GPIO_CEC
	#define CFG_PARA_WAKEUP_SOURCE_IOKEY1	SYSWAKEUP_SOURCE3_GPIO
	#define CFG_PARA_WAKEUP_SOURCE_IOKEY2	SYSWAKEUP_SOURCE4_GPIO
	
	//#define CFG_PARA_WAKEUP_SOURCE_RTC		SYSWAKEUP_SOURCE7_RTC//
		/*注意:RTC唤醒默认是延时唤醒，影响rtc模式已设置的闹钟；如需指定时刻唤醒功能，请屏蔽下一行，在rtcmode设置闹钟。*/
	//#define CFG_PARA_WAKEUP_TIME_RTC	25//睡眠时长 秒	-
	
//	#define CFG_FUNC_WAKEUP_MCU_RESET		//唤醒后mcu复位
#endif

//****************************************************************************************
//                 SHELL功能配置
//说明:
//    1.shell功能启用以及配置请到shell.c中配置;
//    2.shell功能默认关闭，默认使用UART1;
//****************************************************************************************
//#define CFG_FUNC_SHELL_EN

//****************************************************************************************
//                 UART DEBUG功能配置
//注意：DEBUG打开后，会增大mic通路的delay，不需要DEBUG调试代码时，建议关闭掉！
//****************************************************************************************
#define CFG_FUNC_DEBUG_EN
#ifdef CFG_FUNC_DEBUG_EN
	//#define CFG_USE_SW_UART
	#ifdef CFG_USE_SW_UART
		#define SW_UART_IO_PORT				    SWUART_GPIO_PORT_A//SWUART_GPIO_PORT_B
		#define SW_UART_IO_PORT_PIN_INDEX	    1//bit num
		#define  CFG_SW_UART_BANDRATE   		115200//software uart baud rate select:38400 57600 115200 256000 460800 ,default 460800
	#else
		#ifdef CFG_COMMUNICATION_BY_UART
		#define CFG_UART_TX_PORT  				(1)//tx port  0--A6，1--A10, 2--A25
		#else
		#define CFG_UART_TX_PORT 				(2)//tx port  0--A6，1--A10, 2--A25, 3--A0, 4--A1
		#endif
		#define  CFG_UART_BANDRATE   			115200//hardware uart baud set
	#endif
#endif

//****************************************************************************************
//                 提示音功能配置
//说明:
//    1.烧录工具参见MVs26_SDK\tools\script；
//    2.提示音功能开启，注意flash中const data提示音数据需要预先烧录，否则不会播放;
//    3.const data数据开机检查，影响开机速度，主要用于验证。
//****************************************************************************************
#define CFG_FUNC_REMIND_SOUND_EN
#ifdef CFG_FUNC_REMIND_SOUND_EN
	#define CFG_FUNC_REMIND_CHECK_EN
	//#define CFG_FUNC_REMIND_SBC
	#define CFG_PARAM_FIXED_REMIND_VOL    CFG_PARA_MAX_VOLUME_NUM/2 //固定提示音音量值,0表示受music vol同步控制
#ifdef CFG_BT_NUMBER_REMIND
	#define CFG_PARAM_REMIND_LIST_MAX		14	//提示音阻塞播放最大个数。
#else
	#define CFG_PARAM_REMIND_LIST_MAX		6
#endif
//	#define CFG_FUNC_REMIND_DEEPSLEEP		//Deepsleep 前播放(sys)提示音
//	#define CFG_FUNC_REMIND_WAKEUP
#endif

//****************************************************************************************
//                 断点记忆功能配置        
//****************************************************************************************
#define CFG_FUNC_BREAKPOINT_EN
#ifdef CFG_FUNC_BREAKPOINT_EN
	#define BP_PART_SAVE_TO_NVM			// 断点信息保存到NVM
	#define BP_SAVE_TO_FLASH			// 断电信息保存到Flash
	#define FUNC_MATCH_PLAYER_BP		// 获取FS扫描后与播放模式断点信息匹配的文件。文件夹和ID号
#endif

//****************************************************************************************
//                 PowerKey功能，NVM存储，低功耗RTC等
//PowerKey软开关建议POWERKEY_CNT: 2000；硬开关建议POWERKEY_CNT < 100,可以配置为8
//****************************************************************************************
//#define	CFG_FUNC_BACKUP_EN
#ifdef CFG_FUNC_BACKUP_EN
	#define	CFG_FUNC_POWERKEY_EN
	#define POWERKEY_MODE					POWERKEY_MODE_PUSH_BUTTON
	#if (POWERKEY_MODE == POWERKEY_MODE_SLIDE_SWITCH_LPD) || (POWERKEY_MODE == POWERKEY_MODE_SLIDE_SWITCH_HPD)
	#define POWERKEY_CNT					20
	#else
	#define POWERKEY_CNT					2000
	#endif

	//powerkey复用短按键功能。
	#define USE_POWERKEY_SOFT_PUSH_BUTTON
#endif

//****************************************************************************************
//                 POWER MONITOR功能配置
// 定义电能监视(适用于带电池系统)的功能宏和选项宏
// 电能监视包括，电池电压检测及低电压后的系统行为以及充电指示等
// 电池电压检测，是指LDOIN输入端的电压检测功能
// 该功能宏打开后，默认包含电池电压检测功能
//****************************************************************************************
//#define CFG_FUNC_POWER_MONITOR_EN
	//#define	CFG_FUNC_OPTION_CHARGER_DETECT		//打开该宏定义，支持GPIO检测充电设备插入功能
	//充电检测端口设置
	#define CHARGE_DETECT_PORT_PU			GPIO_A_PU
	#define CHARGE_DETECT_PORT_PD			GPIO_A_PD
	#define CHARGE_DETECT_PORT_IN			GPIO_A_IN
	#define CHARGE_DETECT_PORT_IE			GPIO_A_IE
	#define CHARGE_DETECT_PORT_OE			GPIO_A_OE
	#define CHARGE_DETECT_GPIO				GPIOA31

//****************************************************************************************
//                            Key 按键相关配置
//****************************************************************************************
/**按键beep音功能**/
//#define  CFG_FUNC_BEEP_EN
    #define CFG_PARA_BEEP_DEFAULT_VOLUME    15//注意:若蓝牙音量同步功能开启后，此值最大为16

/**按键双击功能**/
//#define  CFG_FUNC_DBCLICK_MSG_EN
#ifdef CFG_FUNC_DBCLICK_MSG_EN
	#define  CFG_PARA_CLICK_MSG             MSG_MODE //单击执行消息
	#define  CFG_PARA_DBCLICK_MSG           MSG_EQ   //双击执行消息
	#define  CFG_PARA_DBCLICK_DLY_TIME      20       //双击有效间隔时间:4ms*20=80ms
#endif

/**ADC按键**/
#define CFG_RES_ADC_KEY_SCAN				//在device service 中启用Key扫描ADCKEY
#if defined(CFG_RES_ADC_KEY_SCAN) 
	#define CFG_RES_ADC_KEY_USE				//ADC按键功能 启用
#endif
#define CFG_PARA_ADC_KEY_COUNT				11  //key count per adc channel 电路配合
#ifdef CFG_RES_ADC_KEY_USE

	#define CFG_RES_POWERKEY_ADC_EN         //power key脚上adc key功能使能，共5个key

    #define CFG_RES_ADC_KEY_PORT_CH1		ADC_CHANNEL_GPIOA20_A23
	#define CFG_RES_ADC_KEY_CH1_ANA_EN		GPIO_A_ANA_EN
	#define CFG_RES_ADC_KEY_CH1_ANA_MASK	GPIO_INDEX23
	#define CFG_PARA_WAKEUP_GPIO_ADCKEY		WAKEUP_GPIOA23 //同步设置唤醒端口

	#ifdef CFG_CHIP_BP10128
	#define CFG_RES_ADC_KEY_PORT_CH2		ADC_CHANNEL_GPIOA26
	#endif
	#define CFG_RES_ADC_KEY_CH2_ANA_EN		GPIO_A_ANA_EN
	#define CFG_RES_ADC_KEY_CH2_ANA_MASK	GPIO_INDEX26

#endif //CFG_RES_ADC_KEY_USE

/**IR按键**/
#define CFG_RES_IR_KEY_SCAN				//启用device service Key扫描IRKey
#if defined(CFG_RES_IR_KEY_SCAN) 
#define	CFG_RES_IR_KEY_USE

#define CFG_RES_IR_NUMBERKEY //数字键操作功能

	#define CFG_RES_IR_PIN                  IR_GPIOB6//IR_GPIOB6,IR_GPIOB7,IR_GPIOA29
	#define CFG_PARA_IR_SEL					IR_MODE_NEC
	#define CFG_PARA_IR_BIT					IR_NEC_32BITS
	#define IR_MANU_ID						0x7F80//0xFF00//不同遥控器会不同,键值表ir_key.c\gIrVal[]
#if 0
	#define	IR_KEY_TABLE	0xBA,/*POWER*/		0xB9,/*MODE*/		0xB8,/*MUTE*/	\
							0xBB,/*PLAY/PAUSE*/	0xBF,/*PRE*/		0xBC,/*NEXT*/	\
							0xF8,/*EQ*/			0xEA,/*VOL-*/		0xF6,/*VOL+*/	\
							0xE9,/*0*/			0xE6,/*REPEAT*/		0xF2,/*SCN*/	\
							0xF3,/*1*/			0xE7,/*2*/			0xA1,/*3*/		\
							0xF7,/*4*/			0xE3,/*5*/			0xA5,/*6*/		\
							0xBD,/*7*/			0xAD,/*8*/			0xB5/*9*/
#endif
	#define	IR_KEY_TABLE	0xED,/*POWER*/		0xE5,/*MODE*/		0xE1,/*MUTE*/	\
							0xFE,/*PLAY/PAUSE*/	0xFD,/*PRE*/		0xFC,/*NEXT*/	\
							0xFB,/*EQ*/			0xFA,/*VOL-*/		0xF9,/*VOL+*/	\
							0xF8,/*0*/			0xF7,/*REPEAT*/		0xF6,/*SCN*/	\
							0xF5,/*1*/			0xE4,/*2*/			0xE0,/*3*/		\
							0xF3,/*4*/			0xF2,/*5*/			0xF1,/*6*/		\
							0xFF,/*7*/			0xF0,/*8*/			0xE6/*9*/
#endif

/**编码旋钮按键**/
//#define	CFG_RES_CODE_KEY_USE
#ifdef CFG_RES_CODE_KEY_USE
	#define CFG_CODE_KEY1P_BANK				'A'
	#define CFG_CODE_KEY1P_PIN				(8)
	#define CFG_CODE_KEY1N_BANK				'A'
	#define CFG_CODE_KEY1N_PIN				(9)
#endif

/**GPIO按键**/
//#define CFG_RES_IO_KEY_SCAN	
#ifdef CFG_RES_IO_KEY_SCAN
	#define  CFG_SOFT_POWER_KEY_EN                          //外围硬件自锁式软开关宏开关,开机后，做普通GPIO KEY0使用；    
    #define  CFG_GPIO_KEY1_EN                               //GPIO KEY1使能
    #define  CFG_GPIO_KEY2_EN                               //GPIO KEY2使能   

	#define CFG_PARA_WAKEUP_GPIO_IOKEY1		WAKEUP_GPIOA23 //同步设置唤醒端口
	#define CFG_PARA_WAKEUP_GPIO_IOKEY2		WAKEUP_GPIOA26 //同步设置唤醒端口
#endif

/**电位器功能选择**/
//#define CFG_ADC_LEVEL_KEY_EN 
#ifdef CFG_ADC_LEVEL_KEY_EN
    #define  ADCLEVL_CHANNEL_MAP            (ADC_GPIOA20|ADC_GPIOA21|ADC_GPIOA22)//选择GPIOA20，GPIOA21,GPIOA22口做3路ADC电位器

	#define MAX_ADCLEVL_LEVEL_VAL 	        4096//电位器最大电压值:4096对应3.3v
	#define MAX_ADCLEVL_STEP_NUMBER 	    CFG_PARA_MAX_VOLUME_NUM//电位器调节最大步数，范围:0-31
    #define DISTANCE_BETWEEN_STEP 		    5//电位器滤波采样值://5//15//25 

#endif

//***************************************************************************************
//					RTC/闹钟功能配置
//注意:
//   1.RTC时钟源选择32K晶振方案，只有BP1064L2型号支持;
//   2.其他芯片型号，RTC时钟源默认选择rc时钟（精度低）；
//   3.其他芯片型号，推荐12M晶振模式，deepsleep功耗有所抬高
//***************************************************************************************
#ifdef CFG_CHIP_BP1064L2
	#define CFG_FUNC_RTC_EN
#else
	#ifndef CFG_FUNC_BACKUP_EN
		#define CFG_FUNC_RTC_EN
	#endif
#endif

#ifdef CFG_FUNC_RTC_EN
	#define CFG_RES_RTC_EN
	#ifdef CFG_RES_RTC_EN
		#define CFG_PARA_RTC_12M 	//预留配置参数
	#endif
	
	#define CFG_FUNC_ALARM_EN  //闹钟功能,必须开时钟
	#define CFG_FUNC_LUNAR_EN  //万年历,必须开时钟
	#ifdef CFG_FUNC_ALARM_EN
		#define CFG_FUNC_SNOOZE_EN //闹钟贪睡功能
	#endif
#endif

//****************************************************************************************
//                            Display 显示配置
//****************************************************************************************
//#define  CFG_FUNC_DISPLAY_EN
#ifdef CFG_FUNC_DISPLAY_EN

//  #define  DISP_DEV_SLED
  #define  DISP_DEV_7_LED
/**8段LED显示操作**/
/*LED显存刷新需要在Timer1ms中断进行，读写flash操作时会关闭中断*/
/*所以需要做特殊处理，请关注该宏包含的代码段*/
/*注意timer中断服务函数和调用到的API必须进入TCM，含调用的所有api，库函数请咨询支持*/
/*开启此宏，要关注所有使用NVIC_SetPriority 设置为0的代码，避免对应中断调用非TCM代码引起死机复位*/
#ifdef DISP_DEV_7_LED
  #define	CFG_FUNC_LED_REFRESH
#endif
#ifdef DISP_DEV_SLED
	#define DISP_LED_INIT()  	    GPIO_PortAModeSet(GPIOA10, 0),\
                                    GPIO_RegOneBitClear(GPIO_A_PU, GPIOA10),\
							        GPIO_RegOneBitClear(GPIO_A_PD, GPIOA10),\
							        GPIO_RegOneBitSet(GPIO_A_OE, GPIOA10),\
							        GPIO_RegOneBitClear(GPIO_A_IE, GPIOA10),\
									GPIO_RegOneBitSet(GPIO_A_OUT, GPIOA10)
   #define DISP_LED_ON()            GPIO_RegOneBitSet(GPIO_A_OUT, GPIOA10)
   #define DISP_LED_OFF()           GPIO_RegOneBitClear(GPIO_A_OUT, GPIOA10)
#endif

#if defined(DISP_DEV_SLED) && defined(DISP_DEV_7_LED)
   #error Conflict: display setting error //不能同时选择两种显示模式
#endif
#endif

//****************************************************************************************
//				   耳机插拔检测功能选择设置
//****************************************************************************************
//#define  CFG_FUNC_DETECT_PHONE_EN                            

//****************************************************************************************
//				   3线，4线耳机类型检测功能选择设置
//****************************************************************************************
//#define  CFG_FUNC_DETECT_MIC_SEG_EN  


//****************************************************************************************
//                            FLASH相关配置
// 2M大小flash，提示音和flash录音不能同时存在，flash录音的空间大小由flashfs来分配
//****************************************************************************************
/*需关注启动扫描数秒问题。*/
#ifdef CFG_FUNC_RECORD_FLASHFS
#define CFG_RES_FLASHFS_EN
#endif

/*关注：init-defalut.c的stub()、file.h的FLASH_BASE和FLASH_FS_SIZE 参数宏在flash_config.h配置*/

#define REMIND_FLASH_STORE_BASE				CONST_DATA_ADDR //系统flash空间分配决定
/*//提示音数据空间溢出地址 */
#ifdef CFG_RES_FLASHFS_EN
#define REMIND_FLASH_STORE_OVERFLOW			FLASHFS_ADDR //flashfs不是缺省配置。
#else
#define REMIND_FLASH_STORE_OVERFLOW			BP_DATA_ADDR
#endif                        

//****************************************************************************************
//                            配置冲突 编译警告
//****************************************************************************************

#if defined(CFG_FUNC_SHELL_EN) && defined(CFG_USE_SW_UART)
#error	Conflict: shell  X  SW UART No RX!
#endif

#if defined(CFG_FUNC_SHELL_EN) && CFG_UART_TX_PORT == 2 && defined(CFG_APP_HDMIIN_MODE_EN) && HDMI_HPD_CHECK_STATUS_IO_PIN == GPIOA24
#error Conflict: Uart Rx Pin  X  Hdmi HPD Pin!
#endif


/**
 * @brief 自定义添加宏控制
 */
#define TRACE_ENABLE		//使用自己习惯的日志打印格式


#endif /* APP_CONFIG_H_ */
