//AD初始化参数
#ifndef _USB9825_PARA_INIT
typedef struct _USB9825_PARA_INIT    
{
	LONG    lSelADClk;           //时钟选择
	LONG    lChCnt;              //通道选择
	LONG    ClkDeci;             //分频因子
	LONG	TriggerMode;         //触发模式
	LONG	TriggerSource;	     //触发源 
	LONG    TriggerDelay;        //触发延时
	LONG    TriggerLength;       //触发长度
	LONG    TriggerLevel;        //模拟触发电平
	LONG    lADFmt;              //AD输出数据格式   
} USB9825_PARA_INIT,*PUSB9825_PARA_INIT;
#endif

//时钟选择
typedef enum EmADClkSel
{
	ADCLK_INT        = 0, //板上时钟
	ADCLK_EXT        = 1  //板外时钟
} ADCLK_SEL;

//触发模式
typedef enum EmTriggerMode
{
	TRIG_MODE_CONTINUE        = 0, //连续采集
	TRIG_MODE_POST            = 1, //后触发		
	TRIG_MODE_DELAY           = 2, //延时触发
	TRIG_MODE_PRE			  = 3, //前触发，USB9825不支持		
	TRIG_MODE_MIDDLE          = 4  //中触发，USB9825不支持		
} TRIGGER_MODE;

//触发源
typedef enum EmTriggerSource
{
	TRIG_SRC_EXT_RISING      = 0,  //外正沿触发
	TRIG_SRC_EXT_FALLING     = 1,  //外负沿触发	
	TRIG_SRC_SOFT_RISING     = 2,   //软件正沿触发
	TRIG_SRC_SOFT_FALLING    = 3,   //软件负沿触发
	TRIG_SRC_CH1_RISING      = 4,   //CH1正沿触发	
	TRIG_SRC_CH1_FALLING     = 5,   //CH1负沿触发
	TRIG_SRC_CH2_RISING      = 6,   //CH2正沿触发
	TRIG_SRC_CH2_FALLING     = 7,   //CH2负沿触发
	TRIG_SRC_CH3_RISING      = 8,   //CH3正沿触发	
	TRIG_SRC_CH3_FALLING     = 9,   //CH3负沿触发
	TRIG_SRC_CH4_RISING      = 10,   //CH4正沿触发
	TRIG_SRC_CH_FALLING      = 11,   //CH4负沿触发
	TRIG_SRC_INT_RISING      = 12,   //PWM正沿触发
	TRIG_SRC_INT_FALLING     = 13    //PWM负沿触发	
} TRIGGER_SOURCE;

//AD数据输出格式
typedef enum EmADFormat
{
	ADFMT_STBIN    = 0, //直接二进制输出
	ADFMT_2SBIN    = 1  //二进制补码输出
} AD_FORMAT;

//读/写零偏
#define WRITEOFFSET 0 //写零偏
#define READOFFSET  1 //读零偏

//触发长度单位
#define   TRIG_UNIT   32

#define READ_MAX_LEN   1572864L     //最大读取长度

/***********************************************************/
#ifndef DEFINING
#define DEVAPI __declspec(dllimport)
#else
#define DEVAPI __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	//判断是否为高速USB设备
	DEVAPI BOOL FAR PASCAL USB9825_IsHighDevice(HANDLE hDevice,PUCHAR pDat);
	//根据设备号打开设备
	DEVAPI HANDLE FAR PASCAL USB9825_Link(UCHAR DeviceNO);
	//断开设备
	DEVAPI BOOL FAR PASCAL USB9825_UnLink(HANDLE hDevice);
	//初始化参数并开始采集
	DEVAPI BOOL FAR PASCAL USB9825_InitAD(HANDLE hDevice, PUSB9825_PARA_INIT para_init);
	//读取AD数据
	DEVAPI BOOL FAR PASCAL USB9825_ReadAD(HANDLE hDevice,PUSHORT pBuf, ULONG nCount);
	//结束采集
	DEVAPI BOOL FAR PASCAL USB9825_StopAD(HANDLE hDevice, UCHAR devNum);
	//设置DO
	DEVAPI BOOL FAR PASCAL USB9825_SetDO(HANDLE hDevice, LONG byDO);
	//读取DI
	DEVAPI BOOL FAR PASCAL USB9825_GetDI(HANDLE hDevice, PLONG pDI);
	//设置AD零偏
	DEVAPI BOOL FAR PASCAL USB9825_ADoffset(HANDLE hDevice, LONG lselAD,
		BOOL bWtRd,PLONG plADoffset);
	//读取硬件FIFO溢出位
	DEVAPI BOOL FAR PASCAL USB9825_GetBufOver(HANDLE hDevice, PLONG pBufOver);
	//软件触发
	DEVAPI BOOL FAR PASCAL USB9825_ExeSoftTrig(HANDLE hDevice);
	//读取设备信息
	DEVAPI BOOL FAR PASCAL USB9825_GetDevInfo(HANDLE hdl,LONG* devFifoSize,LONG* devADbit);
	//读写用户存储器
	DEVAPI BOOL FAR PASCAL USB9825_WrRdUsrMem(HANDLE hDevice, BOOL bWtRd, PLONG pMem);
	//设置PWM脉冲发生器
	DEVAPI BOOL FAR PASCAL USB9825_SetPulGen(HANDLE hDevice, ULONG lAllcnt, ULONG lHighCnt, BOOL bEnable);

#ifdef __cplusplus
}
#endif


