
// yolo_MFC_deployDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "yolo_MFC_deploy.h"
#include "yolo_MFC_deployDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "detector.h"


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CyoloMFCdeployDlg 对话框



CyoloMFCdeployDlg::CyoloMFCdeployDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_YOLO_MFC_DEPLOY_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CyoloMFCdeployDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CyoloMFCdeployDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CyoloMFCdeployDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CyoloMFCdeployDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CyoloMFCdeployDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &CyoloMFCdeployDlg::OnBnClickedRadio1)
END_MESSAGE_MAP()


// CyoloMFCdeployDlg 消息处理程序

BOOL CyoloMFCdeployDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CyoloMFCdeployDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CyoloMFCdeployDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CyoloMFCdeployDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CyoloMFCdeployDlg::OnBnClickedButton1()
{
	// TODO: 配置文件

	CString strFile;
	
	CFileDialog FileDialog(FALSE, _T("par"), NULL, OFN_READONLY | OFN_CREATEPROMPT, _T("Model File (*.*)|*.*||"), this);
	
	CString strPath = _T("DpModule_Param");
	FileDialog.m_ofn.lpstrInitialDir = strPath;
	if (IDOK == FileDialog.DoModal())
	{
		strFile = FileDialog.GetPathName();

		SetDlgItemText(IDC_EDIT1, strFile);
		onnx_path = strFile;
	}


	


	
}


void CyoloMFCdeployDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码

	Invalidate();    //刷新窗口,以便清除上一次显示的图片
	cv::namedWindow("view", cv::WindowFlags::WINDOW_AUTOSIZE);
	HWND hWnd = static_cast<HWND>(cvGetWindowHandle("view"));
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_STATIC1)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);

	int width, height;
	CImage  image;
	CString picPath;   //定义图片路径变量  
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY |
		OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, NULL, this);   //选择文件对话框  

	if (dlg.DoModal() == IDOK)
	{
		picPath = dlg.GetPathName();  //获取图片路径  
	}
	
	
	
	const char* charStr = CW2A(picPath.GetString(), CP_UTF8);


	org_mat = cv::imread(charStr);
 
	CRect rect;
	cv::Mat imagedst;
	//使用GetDlgItem函数来访问对话框中的控件
	GetDlgItem(IDC_STATIC1)->GetClientRect(&rect);
	//Rect dst(rect.left,rect.top,rect.right,rect.bottom);  
	resize(org_mat, imagedst, cv::Size(rect.Width(), rect.Height()));
	imshow("view", imagedst);
	


}


void CyoloMFCdeployDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	Invalidate();
	onnx_path.Replace(_T("/"), _T("\\"));
	CButton* pCheckBox = (CButton*)GetDlgItem(IDC_CHECK1);

	// 检查控件是否有效
	if (pCheckBox)
	{
		// 获取 Check Box 的状态
		int nCheckState = pCheckBox->GetCheck();

		// 判断是否被选中
		if (nCheckState == BST_CHECKED)
		{
			use_gpu = false;
		}
		else
		{
			// Check Box 未被选中
			// 执行相应的代码
			
		}
	}
	else {
		// RadioButton未被选中
		use_gpu = false;
	}

	int len = WideCharToMultiByte(CP_ACP, 0, onnx_path, -1, NULL, 0, NULL, NULL);
	char* modelPath = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, onnx_path, -1, modelPath, len, NULL, NULL);
	if (len == 0)
		return;





	YOLODetector detector(modelPath, use_gpu, cv::Size(640, 640), 1);

	cv::Mat image;
	cv::namedWindow("result", cv::WindowFlags::WINDOW_AUTOSIZE);
	HWND hWnd = static_cast<HWND>(cvGetWindowHandle("result"));
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_STATIC2)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);


	std::vector<std::string> classNames = detector.GetdeteClassName();
	std::vector<Detection> result;
	org_mat.copyTo(image);

	if (image.channels() == 1)
	{
		cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
	}

	CRect rect;
	cv::Mat imagedst;

	result=detector.detect(image, 0.25, 0.45);

	
	Detectutils::visualizeDetection(image, result, classNames);
	GetDlgItem(IDC_STATIC2)->GetClientRect(&rect);
	resize(image, imagedst, cv::Size(rect.Width(), rect.Height()));
	cv::imshow("result", imagedst);
	AfxMessageBox(_T("检测成功"));
	//CDialogEx::OnOK();
}


void CyoloMFCdeployDlg::MatCImage(cv::Mat& mat, CImage& cimage)
{
	if (0 == mat.total())
	{
		return;
	}


	int nChannels = mat.channels();
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = mat.cols;
	int nHeight = mat.rows;


	//重建cimage  
	cimage.Destroy();  //这一步是防止重复利用造成内存问题
	cimage.Create(nWidth, nHeight, 8 * nChannels);  //默认图像像素单通道占用1个字节


	//拷贝数据  


	uchar* pucRow;                                  //指向数据区的行指针  
	uchar* pucImage = (uchar*)cimage.GetBits();     //指向数据区的指针  
	int nStep = cimage.GetPitch();                  //每行的字节数,注意这个返回值有正有负  

	// 如果是1个通道的图像(灰度图像) DIB格式才需要对调色板设置    
	// CImage中内置了调色板，我们要对他进行赋值：  
	if (1 == nChannels)                             //对于单通道的图像需要初始化调色板  
	{
		RGBQUAD* rgbquadColorTable;
		int nMaxColors = 256;
		rgbquadColorTable = new RGBQUAD[nMaxColors];
		//这里可以通过CI.GetMaxColorTableEntries()得到大小(如果你是CI.Load读入图像的话)    
		cimage.GetColorTable(0, nMaxColors, rgbquadColorTable);  //这里是取得指针  
		for (int nColor = 0; nColor < nMaxColors; nColor++)
		{
			//BYTE和uchar一回事，但MFC中都用它  
			rgbquadColorTable[nColor].rgbBlue = (uchar)nColor;  // (BYTE)nColor
			rgbquadColorTable[nColor].rgbGreen = (uchar)nColor;
			rgbquadColorTable[nColor].rgbRed = (uchar)nColor;
		}
		cimage.SetColorTable(0, nMaxColors, rgbquadColorTable);
		delete[]rgbquadColorTable;
	}


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				*(pucImage + nRow * nStep + nCol) = pucRow[nCol];
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					*(pucImage + nRow * nStep + nCol * 3 + nCha) = pucRow[nCol * 3 + nCha];
				}
			}
		}
	}
}

void CyoloMFCdeployDlg::CImageToMat(CImage& cimage, cv::Mat& mat)
{
	if (true == cimage.IsNull())
	{
		return;
	}


	int nChannels = cimage.GetBPP() / 8;
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = cimage.GetWidth();
	int nHeight = cimage.GetHeight();


	//重建mat  
	if (1 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC1);
	}
	else if (3 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC3);
	}


	//拷贝数据  


	uchar* pucRow;                                  //指向数据区的行指针  
	uchar* pucImage = (uchar*)cimage.GetBits();     //指向数据区的指针  
	int nStep = cimage.GetPitch();                  //每行的字节数,注意这个返回值有正有负  


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				pucRow[nCol] = *(pucImage + nRow * nStep + nCol);
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					pucRow[nCol * 3 + nCha] = *(pucImage + nRow * nStep + nCol * 3 + nCha);
				}
			}
		}
	}
}

void CyoloMFCdeployDlg::OnBnClickedRadio1()
{
	// TODO: 在此添加控件通知处理程序代码


}
