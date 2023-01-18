#pragma once
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "Strmbase.lib")

#include <QtWidgets/QMainWindow>
#include "ui_Micro_Plastic_Analyzer.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QString>
#include <QFileDialog>
#include <QTextCodec>
#include <QTimer>
#include <QMessageBox>
#include <vector>
#include <string>
#include <dshow.h>

//select index
#define OUTPUT_SELECT_NULL 0
#define OUTPUT_SELECT_INVERSION 1
#define OUTPUT_SELECT_ALGEBRA 2
#define OUTPUT_SELECT_BLURRING 3
#define OUTPUT_SELECT_SHARPENING 4
#define OUTPUT_SELECT_CANNY 5
#define OUTPUT_SELECT_THRESHOLD 6
#define OUTPUT_SELECT_ADAPTIVE_THRESHOLD 7
#define OUTPUT_SELECT_HISTOGRAM_EQUALIZATION 8
#define OUTPUT_SELECT_OPENING 9
#define OUTPUT_SELECT_CLOSING 10
#define OUTPUT_SELECT_TOPHAT 11
#define OUTPUT_SELECT_BLACKHAT 12
#define OUTPUT_SELECT_CONTOUR 13
#define OUTPUT_SELECT_WATERSHED 14
#define OUTPUT_SELECT_INTEGRAL 15
#define OUTPUT_SELECT_DISTANCE 16
#define OUTPUT_SELECT_2PASS_4CONNECTIVITY 17
#define OUTPUT_SELECT_2PASS_8CONNECTIVITY 18
#define OUTPUT_SELECT_MICRO_PLASTIC_DETECTION 19



using namespace cv;
using namespace std;

class Micro_Plastic_Analyzer : public QMainWindow
{
	Q_OBJECT

public:
	Micro_Plastic_Analyzer(QWidget* parent = nullptr);
	~Micro_Plastic_Analyzer();

private:
	void Cam_Stop();
	void Video_Stop();
	void Mat2QImage_DisplayImage();
	void EnableUI();
	void DisableUI();
	void Slider_Setting(int minValue, int maxValue);
	void MorphologyUse(bool);
	void CannyUse(bool);
	void ThresholdUse(bool);
	void AlgebraUse(bool);
	void TwoPassUse(bool);
	void PlasticDetectorUse(bool);
	void VideoUse(bool);

	void FindAllCamera();
	void ImageCvt();

	void resizeEvent(QResizeEvent* event);

	void OutPut_ImgProcess(int index);  //����ó���� ���ؼ��� ����

	bool load_Video();
	void VideoSlider_Setting(int minValue, int maxValue);

private:
	Ui::Micro_PlasticAnalyzerClass ui;


private slots:
	void on_btnImg_clicked();
	void on_btnCam_clicked();
	void on_btnConColor_clicked();
	void on_btnSave_clicked();
	void on_btnReset_clicked();
	void on_btnSet_clicked();
	void on_btnStartPause_clicked();

	void Update_Video();
	void pressedVideoSlider();

	void Update_Cam();
	void Cam_Resolution(int index);
	void SelectCamera(int value);

	void Change_Index(int index);
	void setSlider_Value(int value);
	void setSpinBox_Value(int value);

	void setSlider_Value_beta(int value);
	void setSpinBox_Value_beta(int value);

	void setVideoSlider_Value(int value);

	void clicked_radioButton();

	void clicked_CheckBox_Ostu();
	void clicked_CheckBox_Average();
	
	void setpassminValue(int);
	void setpassmaxValue(int);
	void clicked_CheckBox_2Pass();


private:
	Mat m_image; // Image�� Cam �� �� m_image�� ������ �Է� ����, QImage�� ��µǴ� �̹���
	Mat m_processImg;  // �ε��� �ٲ� �� ���������� �����ߴ� ����ó�� �����ϴ� �뵵, m_image���� �� ���� ����ó���� ����
	Mat m_originImg; // ó�� �о���� ���� �̹���
	Mat m_bufferImg; // �̹��� �� ���� �� �����̹��� ��ü�� ���� ���۷� ���

	// Image
	QImage q_image;
	QString imgUrl; // �̹��� ��� �� �ֽ� ���
	string imgFile; // �̹������� ���

	// Video
	VideoCapture video;
	QTimer* videoTimer;
	vector<Mat> videoFrame;
	int videoIndex;
	int fps;
	int currentFrame;

	// Cam
	VideoCapture cap;
	QTimer* camTimer; // QTimer�� ����� �� �����Ӹ��� ���� ���
	int camNum; // ī�޶� ��ȣ
	double camWidth;
	double camHeight;

	vector<vector<pair<int, int>>> camResList;

	int sliderVal; // slider��
	int sliderVal2; // slider�� (���� ���� ó��)


	int passmaxValue;
	int passminValue;

	//log
private:
	vector<int> current_log;
	vector<vector<int>> total_log;

	void readTotallog();
	void writeCurrentlog(int index);

	//algorithm
private:
	void Algorithm_Algebra();
	void Algorithm_Blurring();
	void Algorithm_Sharpening();
	void Algorithm_Canny();
	void Algorithm_Threshold();
	void Algorithm_Adaptive_Threshold();
	void Algorithm_Histogram_Equalization();
	void Algorithm_Morphology(int operation);

	void Algorithm_Contour();
	void Algorithm_Watershed();
	void Algorithm_Integral();
	void Algorithm_Distance_Transform();

	void Algorithm_2pass(int connectivity);
	void Algorithm_Micro_Plastic_Detection();
};