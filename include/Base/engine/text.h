//---------------------------------------------------------------------------
//	文字串处理
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-31
//---------------------------------------------------------------------------
#ifndef _ENGINE_TEXT_H_
#define _ENGINE_TEXT_H_

//字符串控制码的内部编码
enum KTEXT_CTRL_CODE
{
	KTC_INVALID			=	0,
	KTC_ENTER			=	0x0a,
	KTC_COLOR			=	0x02,	//后面跟随三个BYTE数据分别为RGB三色分量
	KTC_COLOR_RESTORE	=	0x03,	//回复到原来设置的颜色
	KTC_BORDER_COLOR	=	0x04,	//设置边缘色，后面跟随三个BYTE数据分别为RGB三色分量
	KTC_BORDER_RESTORE	=	0x05,	//设置边缘色还原
	KTC_INLINE_PIC		=	0x06,	//嵌入式图片[wxb 2003-6-19]
	KTC_LINK			=	0x07,	//嵌入图形描述控件[ChenWeiMing 2004-11-9]
	KTC_ITEM			=	0x08,	//嵌入式窗口（带物品信息）
	KTC_TAB				=	0x09,	//tab
	KTC_SPACE			=	0x20,	//空格
	KTC_TAIL			=	0xFF,	//字符串结束
};

struct KTP_CTRL
{
	unsigned char	cCtrl;	//此成员取值为Text.h中梅举KTEXT_CTRL_CODE中的一个值
	union
	{
	    struct
	    {
   			unsigned char cParam0;	//此及以下两个成员的取值与含义依据cCtrl的取值而定
		    unsigned char cParam1;
		    unsigned char cParam2;
	    };
	    unsigned short wParam;
   };
};

// TODO: Fanghao_Wu 这两个什么区别？
#define	MAXCONTROL		16
#define MAX_CTROL_NUM	10

//嵌入控件【chenweiming 2004-11-3】
struct KInlineControl
{
	//CtrlType cCtrl;			//控件类型 
	int	nPos;					//控件文字起始位置
	int	nLen;					//控件长度
	char sParam[MAX_PATH];		//控件所带参数
};

//这个是内嵌式窗口控件，它与嵌入式图片不同的是，它可以有自己的响应事件
struct IInlineControl
{
	//获得该控件的大小
	virtual long	GetSize(unsigned short wID, int nFontSize, int& nX, int& nY) = 0;
	//分析该控件所带的数据
	virtual short	AnalyzeData(char* pString, int nLen) = 0;
	//绘制时对该控件的处理（绘制或调整位置）
	virtual long	RepresentAction(unsigned short wID, int x, int y) = 0;
	//插入新的内嵌窗口
	virtual long	InsertInlineCtrl(unsigned short& wDataID, char* pString, int nType = 0) = 0;
	//卸载内嵌窗口
	virtual long	RemoveInlineCtrl(unsigned short wDataID) = 0;
	//标示字符串
	virtual const char*	GetTagString() = 0;
	//标示记号
	virtual const char	GetTagSign() = 0;
};

struct IInlinePicEngineSink
{
	enum INLINE_PIC_ENGINE_SINK_PARAM
	{
		MAX_SYSTEM_INLINE_PICTURES	= 4096,	//系统预留的嵌入式图片个数
	};

	//获取指定嵌入图片的大小,返回布尔值
	virtual int GetPicSize(unsigned short wIndex, int& cx, int& cy) = 0;
	//绘制指定图片,返回布尔值
	virtual int DrawPic(unsigned short wIndex, int x, int y, int nColor, int nStretchPercent = 100) = 0;
	//动态加载图片,获取一个WORD,即图片的索引,返回布尔值
	virtual int AddCustomInlinePic(unsigned short& wIndex, const char* szSprPathName) = 0;
	//动态卸载图片,返回布尔值
	virtual int RemoveCustomInlinePic(unsigned short wIndex) = 0;
	//释放接口对象
	virtual void Release() = 0;
};

class KControlTable
{
public:
	KControlTable()
	{
		m_nCtrlNum = 0;
		ZeroMemory(m_szInlineCtrl, sizeof(m_szInlineCtrl));
	};
	~KControlTable(){};

	BOOL RegisterCtrl(IInlineControl* pCtrl)//注册新控件
	{
		if( m_nCtrlNum >= MAX_CTROL_NUM ||m_szInlineCtrl[m_nCtrlNum] != NULL
			|| NULL == pCtrl)
		{
			//ASSERT(0);
			return FALSE;
		}

		m_szInlineCtrl[m_nCtrlNum] = pCtrl;
		m_nCtrlNum ++;

		return TRUE;
	}

	BOOL RemoveCtrl(IInlineControl* pCtrl)//释放指定控件
	{
		if(NULL == pCtrl)
			return FALSE;

		int i = 0;
		for(; i < m_nCtrlNum; i++)
		{
			if(pCtrl == m_szInlineCtrl[i])
			{
				m_szInlineCtrl[i] = NULL;
				int rmvId = i;

				if(rmvId == MAX_CTROL_NUM - 1)
					break;

				for(; rmvId < m_nCtrlNum; rmvId ++)
				{
					m_szInlineCtrl[rmvId] = m_szInlineCtrl[rmvId + 1];
				}

				break;
			}
		}//end of "for("

		-- m_nCtrlNum;
		return TRUE;
	}

	int GetCtrlNum()
	{
		return m_nCtrlNum;
	}

	IInlineControl* GetCtrlHandle(int nIndex)
	{
		if(nIndex >= MAX_CTROL_NUM || nIndex < 0)
			return NULL;

		return m_szInlineCtrl[nIndex];
	}
private:
	IInlineControl* m_szInlineCtrl[MAX_CTROL_NUM];
	int				m_nCtrlNum;
};

#ifndef ENGINE_EXPORTS

//接口 IInlinePicEngineSink 由应用层实现并挂接进 Engine 模块 [wxb 2003-6-19]
//相关挂接函数:
// AdviseEngine(IInlinePicEngineSink*);
// UnAdviseEngine(IInlinePicEngineSink*);
extern "C"
{
	//本地化初始化
	void	TLocalizationInitialize();
	//本地化结束
	void	TLocalizationTerminate();
	//获取本行的下个显示字符
	const char* TGetSecondVisibleCharacterThisLine(const char* pCharacter, int nPos, int nLen);
	//检测某个字符是否为不许放置行首的字符，不是限制字符则返回0，否则返回字符占的子节数
	int TIsCharacterNotAlowAtLineHead(const char* pCharacter);
	//如果原字符串长度（包括结尾符）超过限定的长度，则截短它并加上..后缀
	const char* TGetLimitLenString(const char* pOrigString, int nOrigLen, char* pLimitLenString, int nLimitLen);
	//如果原(包含控制符)字符串长度（包括结尾符）超过限定的长度，则截短它并加上..后缀
	const char* TGetLimitLenEncodedString(const char* pOrigString, int nOrigLen, int nFontSize,
		int nWrapCharaNum, char* pLimitLenString, int& nShortLen, int nLineLimit, int bPicPackInSingleLine = false);
	//寻找分割字符串的合适位置
	int	TSplitString(const char* pString, int nDesirePos, int bLess);
	//在编码字串寻找分割字符串的合适位置
	int	TSplitEncodedString(const char* pString, int nCount, int nDesirePos, int bLess);
	//获得指定行的开始位置
	int TGetEncodeStringLineHeadPos(const char* pBuffer, int nCount, int nLine, int nWrapCharaNum, int nFontSize, int bPicPackInSingleLine = false);
	//对文本串中的控制标记进行转换，去除无效字符，缩短文本串存储长度
	int	TEncodeText(char* pBuffer, int nCount);
	int TEncodeTextAndPickCtrl(char* pBuffer, int nCount, KInlineControl* pControl, int &nControlNum);
	//对文本串中的控制标记进行转换，去除无效字符，缩短文本串存储长度
	int TFilterEncodedText(char* pBuffer, int nCount);
	//去除编码文本中的控制符号
	int	TRemoveCtrlInEncodedText(char* pBuffer, int nCount);
	//获取编码文本的行数与最大行宽
	int	TGetEncodedTextLineCount(const char* pBuffer, int nCount, int nWrapCharaNum, int& nMaxLineLen, int nFontSize, int nSkipLine = 0, int nLineLimit = 0, int bPicSingleLine = false);
	//对已经编码的文本，从指定位置开始查找指定的控制符号的位置，返回-1表示未找到
	int	TFindSpecialCtrlInEncodedText(const char* pBuffer, int nCount, int nStartPos, char cControl);
	//对已经编码的文本，去除指定类型的控制符
	int	TClearSpecialCtrlInEncodedText(char* pBuffer, int nCount, char cControl);
	//对已经编码的文本，指定输出长度的在缓冲区中位置
	int TGetEncodedTextOutputLenPos(const char* pBuffer, int nCount, int& nLen, int bLess, int nFontSize);
	//对已经编码的文本，指定的前段缓冲区中控制符，对后面的输出产生效果影响
	int TGetEncodedTextEffectCtrls(const char* pBuffer, int nSkipCount, KTP_CTRL& Ctrl0, KTP_CTRL& Ctrl1);
	//返回布尔值
	int	TAdviseEngine(IInlinePicEngineSink*);
	//返回布尔值
	int	TUnAdviseEngine(IInlinePicEngineSink*);
	//根据当前编码获得控件表中对应的控件
	IInlineControl* GetCtrlHandle(int nCtrl);
	//根据当前字符串获得控件列表中对应的控件
	IInlineControl* GetCtrlHandleByStr(char* pBuff, int nCtrlCodeSize);
	//注册内嵌控件
	long RegisterInlineCtrl(IInlineControl*);
	//删除内嵌控件
	long RemoveInlineWndCtrl(IInlineControl*);
}

#endif	//ENGINE2_EXPORTS

#endif  //ifndef _ENGINE_TEXT_H_
