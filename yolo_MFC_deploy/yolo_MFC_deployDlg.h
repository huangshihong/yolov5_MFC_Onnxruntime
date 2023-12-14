
// yolo_MFC_deployDlg.h: 头文件
//

#pragma once


// CyoloMFCdeployDlg 对话框
class CyoloMFCdeployDlg : public CDialogEx
{
// 构造
public:



	CyoloMFCdeployDlg(CWnd* pParent = nullptr);	// 标准构造函数
	CString onnx_path;
	CImage org_cimage;
	CString image_path;
	cv::Mat org_mat;
	bool use_gpu;
	void CyoloMFCdeployDlg::CImageToMat(CImage& cimage, cv::Mat& mat);
	void CyoloMFCdeployDlg::MatCImage(cv::Mat& mat, CImage& cimage);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_YOLO_MFC_DEPLOY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedOk();

	afx_msg void OnBnClickedRadio1();
};
