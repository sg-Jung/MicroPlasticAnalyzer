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

	void OutPut_ImgProcess(int index);  //영상처리에 대해서만 수행

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
	Mat m_image; // Image와 Cam 둘 다 m_image로 영상을 입력 받음, QImage에 출력되는 이미지
	Mat m_processImg;  // 인덱스 바꿀 때 이전까지의 수행했던 영상처리 저장하는 용도, m_image에서 그 다음 영상처리를 수행
	Mat m_originImg; // 처음 읽어들인 원본 이미지
	Mat m_bufferImg; // 이미지 색 변경 시 원본이미지 교체를 위한 버퍼로 사용

	// Image
	QImage q_image;
	QString imgUrl; // 이미지 경로 및 최신 경로
	string imgFile; // 이미지파일 경로

	// Video
	VideoCapture video;
	QTimer* videoTimer;
	vector<Mat> videoFrame;
	int videoIndex;
	int fps;
	int currentFrame;

	// Cam
	VideoCapture cap;
	QTimer* camTimer; // QTimer를 사용해 매 프레임마다 영상 출력
	int camNum; // 카메라 번호
	double camWidth;
	double camHeight;

	vector<vector<pair<int, int>>> camResList;

	int sliderVal; // slider값
	int sliderVal2; // slider값 (선형 연산 처리)


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