#include "Micro_Plastic_Analyzer.h"


Micro_Plastic_Analyzer::Micro_Plastic_Analyzer(QWidget* parent)
	: QMainWindow(parent), camWidth(640), camHeight(480), sliderVal(0), sliderVal2(0), passmaxValue(0), passminValue(0), videoIndex(0), camNum(0)
{

	camTimer = new QTimer();
	videoTimer = new QTimer();

	ui.setupUi(this);
	//setWindowFlags(Qt::MSWindowsFixedSizeDialogHint); //사이즈 고정
	//ui.videoSlider->setStyleSheet("QSlider::groove:horizontal {border: 1px solid #999999;height: 18px;border-radius: 9px;}QSlider::handle:horizontal{width: 18px;}QSlider::add-page:qlineargradient{background: lightgrey;border-top-right-radius: 9px;border-bottom-right-radius: 9px;border-top-left-radius: 0px;border-bottom-left-radius: 0px;}QSlider::sub-page:qlineargradient{background: blue;border-top-right-radius: 0px;border-bottom-right-radius: 0px;border-top-left-radius: 9px;border-bottom-left-radius: 9px;}");

	// Sender: camTimer, Signal: QTimer::timeout, Slot: Update_Cam -> Sender camTimer에서 Signal timeout이 발생하면 Slot Update_Cam 함수를 실행 함
	connect(camTimer, &QTimer::timeout, this, &Micro_Plastic_Analyzer::Update_Cam);
	// Video
	connect(videoTimer, &QTimer::timeout, this, &Micro_Plastic_Analyzer::Update_Video);

	// 콤보박스에서 선택된 해상도로 카메라 오픈
	connect(ui.InputCamCombo, SIGNAL(activated(int)), this, SLOT(Cam_Resolution(int)));
	// 선택된 카메라로 카메라 오픈
	connect(ui.CameraCombo, SIGNAL(activated(int)), this, SLOT(SelectCamera(int)));
	// 콤보박스 선택 시 해당 인덱스에 대한 영상처리를 수행함
	connect(ui.outputCombo, SIGNAL(activated(int)), this, SLOT(Change_Index(int)));
	// 슬라이더 값 변경 시 setSlider_Value 함수 실행
	connect(ui.outputSlider, SIGNAL(valueChanged(int)), this, SLOT(setSlider_Value(int)));
	// SpinBox 값 변경 시 setSpinBox_Value 함수 실행
	connect(ui.sliderSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setSpinBox_Value(int)));
	// radioRect 클릭 시 clicked_radioRect 함수 실행
	connect(ui.radioRect, SIGNAL(clicked()), this, SLOT(clicked_radioButton()));
	// radioCross 클릭 시 clicked_radioCross 함수 실행
	connect(ui.radioCross, SIGNAL(clicked()), this, SLOT(clicked_radioButton()));
	// radioEllipse 클릭 시 clicked_radioEllipse 함수 실행
	connect(ui.radioEllipse, SIGNAL(clicked()), this, SLOT(clicked_radioButton()));
	// checkBox 클릭 시 clicked_CheckBox 함수 실행
	connect(ui.OstucheckBox, SIGNAL(clicked()), this, SLOT(clicked_CheckBox_Ostu()));
	connect(ui.AveragecheckBox, SIGNAL(clicked()), this, SLOT(clicked_CheckBox_Average()));


	//Brightness&Contrast
	// 슬라이더 값 변경 시 setSlider_Value 함수 실행
	connect(ui.outputSlider_2, SIGNAL(valueChanged(int)), this, SLOT(setSlider_Value_beta(int)));
	// SpinBox 값 변경 시 setSpinBox_Value 함수 실행
	connect(ui.sliderSpinBox_2, SIGNAL(valueChanged(int)), this, SLOT(setSpinBox_Value_beta(int)));
	// video 슬라이더
	connect(ui.videoSlider, SIGNAL(valueChanged(int)), this, SLOT(setVideoSlider_Value(int)));
	connect(ui.videoSlider, SIGNAL(sliderPressed()), this, SLOT(pressedVideoSlider()));

	connect(ui.passMaxspinBox, SIGNAL(valueChanged(int)), this, SLOT(setpassmaxValue(int)));
	connect(ui.passMinspinBox, SIGNAL(valueChanged(int)), this, SLOT(setpassminValue(int)));

	connect(ui.passFiltercheckBox, SIGNAL(clicked()), this, SLOT(clicked_CheckBox_2Pass()));
	connect(ui.passRectcheckBox, SIGNAL(clicked()), this, SLOT(clicked_CheckBox_2Pass()));
	connect(ui.passOrigincheckBox, SIGNAL(clicked()), this, SLOT(clicked_CheckBox_2Pass()));


	DisableUI();
	imgFile.clear();

	FindAllCamera();
}

Micro_Plastic_Analyzer::~Micro_Plastic_Analyzer()
{
}

void Micro_Plastic_Analyzer::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);

	Mat2QImage_DisplayImage();
}

void Micro_Plastic_Analyzer::Mat2QImage_DisplayImage()
{
	/* Mat->QImage, Mat자료형으로 읽을 때 BGR->RGB형식으로 바꿔주고, 다음과 같이 QImage로 변환해주면 됨(m_image.step 필수) */

	if (m_image.channels() == 3) // 컬러 이미지
		q_image = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_RGB888);
	else // 흑백 이미지
		q_image = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_Grayscale8);

	// 이미지 크기를 imgLabel의 크기에 맞춘다 | SmoothTransformation: FastTransformation보다 이미지를 부드럽게 출력해 줌
	q_image = q_image.scaledToWidth(ui.imgLabel->width(), Qt::SmoothTransformation);
	q_image = q_image.scaledToHeight(ui.imgLabel->height(), Qt::SmoothTransformation);

	// 처리한 이미지를 imgLabel에 출력
	ui.imgLabel->setPixmap(QPixmap::fromImage(q_image).scaled(ui.imgLabel->width(), ui.imgLabel->height(), Qt::KeepAspectRatio));
}

void Micro_Plastic_Analyzer::ImageCvt()
{
	if (ui.btnConColor->text() == "GrayScale") // 이미지를 흑백으로 바꾸기 전
		cvtColor(m_originImg, m_originImg, COLOR_BGR2RGB); // m_image을 BGR->RGB형식으로 변환
	else if (ui.btnConColor->text() == "Return") // 이미지를 흑백으로 바꾼 후
		cvtColor(m_originImg, m_originImg, COLOR_BGR2GRAY); // m_image를 BGR->GRAY형식으로 변환

	if (m_image.empty())
		m_image = m_originImg.clone();
	if (m_processImg.empty())
		m_processImg = m_originImg.clone();
	if (m_bufferImg.empty())
		m_bufferImg = m_originImg.clone();
}

void Micro_Plastic_Analyzer::on_btnImg_clicked()
{
	// 이미지를 불러오는 다이얼로그 창을 제공해 이미지 파일의 경로를 저장한다. | QString::fromLocal8Bit: 한글이 깨지는 것을 방지하기 위해 사용
	imgUrl = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("이미지 선택"), imgUrl, QString::fromLocal8Bit("Images (*.png *.xpm *.jpg *.gif *.bmp);; Videos(*.avi *.mp4 *.wmv"));

	// 이미지 경로에 한글이 있는 경우 한글도 읽을 수 있도록 인코딩 해준 후 imgFile에 경로 저장
	imgFile = imgUrl.toLocal8Bit().constData();

	// =================================================================================================================================================================================

	if (imgUrl == NULL) // 파일 경로가 NULL이라면 함수 종료
		return;

	if (camTimer->isActive() && imgUrl != NULL) // camTimer가 실행 중이라면 Cam 종료
		Cam_Stop();

	if (video.isOpened() || videoTimer->isActive())
	{
		Video_Stop();
	}

	// 이전에 저장했던 부분들 전부 초기화
	ui.outputCombo->setCurrentIndex(0);
	total_log.clear();
	current_log.clear();
	DisableUI();


	string fileExt = imgFile; // 파일 확장자 추출
	fileExt = fileExt.substr(fileExt.length() - 3);

	if (fileExt == "png" || fileExt == "xpm" || fileExt == "jpg" || fileExt == "gif" || fileExt == "bmp")
	{
		// 불러온 이미지를 QLabel로 출력한다.
		// 이미지를 Mat자료형으로 읽어 들인다.
		m_image = imread(imgFile);

		// 이미지 읽기
		cvtColor(m_image, m_image, COLOR_BGR2RGB); // m_image을 BGR->RGB형식으로 변환

		// 원본이미지와 버퍼이미지에 불러온 이미지 저장
		m_originImg = m_image.clone();
		m_bufferImg = m_image.clone();
		m_processImg = m_image.clone();

		// 새로 이미지를 불러들이면 NULL상태로 변경
		ui.outputCombo->setCurrentIndex(0); // setCurrentIndex()로 콤보박스 변경 시 signal로 연결 된 함수는 동작하지 않음
		OutPut_ImgProcess(0);
	}
	else if (fileExt == "avi" || fileExt == "mp4" || fileExt == "wmv")
	{
		if (!load_Video()) // 동영상을 제대로 불러오면 true 반환
			return;

		videoIndex = 0;

		ui.outputCombo->setCurrentIndex(0);
		OutPut_ImgProcess(0);

		VideoUse(true);
		ui.btnStartPause->setText(QString::fromLocal8Bit("||"));

		videoTimer->start(1000 / fps);
	}

	if (ui.btnConColor->text() == "Return") // 색상 변환 버튼이 Return인 경우 이미지를 새로 불러왔으므로 GrayScale로 변경
		ui.btnConColor->setText("GrayScale");
}


void Micro_Plastic_Analyzer::on_btnCam_clicked()
{
	m_image = NULL;

	if (cap.isOpened()) // 카메라가 실행중이라면 카메라 종료
	{
		cap.release();
	}

	if (videoTimer->isActive() || video.isOpened()) // 동영상이 실행중이라면 동영상 stop, 해제
	{
		Video_Stop();
	}

	cap.open(camNum, CAP_DSHOW); // 카메라 오픈

	if (!cap.isOpened())
	{
		QMessageBox::warning(this, "No Cam", QString::fromLocal8Bit("카메라가 연결되지 않았습니다"), QMessageBox::Yes);
		return;
	}

	// 이전에 저장했던 부분들 전부 초기화
	ui.outputCombo->setCurrentIndex(0);
	total_log.clear();
	current_log.clear();
	DisableUI();


	bool w = cap.set(CAP_PROP_FRAME_WIDTH, camWidth);
	bool h = cap.set(CAP_PROP_FRAME_HEIGHT, camHeight);
	if (!(w && h)) // cam이 camWidth나 camHeight크기를 지원하지 않는 경우
	{
		cerr << "This Camera is not support!" << endl;
		QMessageBox::warning(this, "This Camera is not support!", QString::fromLocal8Bit("해상도가 지원되지 않습니다."), QMessageBox::Yes);
		return;
	}

	// 프레임 설정
	cap.set(CAP_PROP_FPS, 60);
	cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));

	// Cam을 불러오면 콤보박스를 NULL상태로 변경
	OutPut_ImgProcess(0);

	camTimer->start(20); // 20ms마다 camTimer를 반복해서 실행함 (실행->종료(timeout)->실행->종료(timeout)를 20ms마다 반복)
}


void Micro_Plastic_Analyzer::on_btnConColor_clicked()
{

	if (m_image.empty()) // m_image에 영상이 없다면 함수 종료
	{
		QMessageBox::warning(this, "No image", QString::fromLocal8Bit("이미지가 존재하지 않습니다"), QMessageBox::Yes);
		return;
	}
	if (ui.btnConColor->text() == "GrayScale") // 흑백이미지로 변환 후 버튼의 Text를 Color로 변경
	{
		cvtColor(m_originImg, m_processImg, COLOR_RGB2GRAY);
		m_originImg = m_processImg.clone(); // m_originImg를 흑백 이미지로 변경
		Mat2QImage_DisplayImage();

		ui.btnConColor->setText("Return");
	}
	else if (ui.btnConColor->text() == "Return") // 원본이미지로 변환 후 버튼의 Text를 GrayScale로 변경
	{
		if (!camTimer->isActive() && !videoTimer->isActive())
		{
			m_originImg = m_bufferImg.clone();
			m_processImg = m_originImg.clone();
			m_image = m_originImg.clone();
		}

		Mat2QImage_DisplayImage();

		ui.btnConColor->setText("GrayScale");
	}

	OutPut_ImgProcess(ui.outputCombo->currentIndex());
}



void Micro_Plastic_Analyzer::on_btnSave_clicked() // 현재 출력중인 이미지 저장
{
	if (q_image.isNull()) // m_image에 영상이 없다면 함수 종료
	{
		QMessageBox::warning(this, "No image", QString::fromLocal8Bit("이미지가 존재하지 않습니다"), QMessageBox::Yes);
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Save Image File"), QString(), QString::fromLocal8Bit("Images (*.png)"));

	if (!fileName.isEmpty()) // fileName에 경로가 잘 저장됐다면 isEmpty() false
	{
		q_image.save(fileName);
	}

}

void Micro_Plastic_Analyzer::on_btnReset_clicked()
{
	if (m_image.empty()) // m_image에 영상이 없다면 함수 종료
	{
		QMessageBox::warning(this, "No image", QString::fromLocal8Bit("이미지가 존재하지 않습니다"), QMessageBox::Yes);
		return;
	}

	// 영상처리 수행했던 이미지들 전부 초기화
	m_processImg = m_originImg.clone();
	m_image = m_originImg.clone();

	ui.outputCombo->setCurrentIndex(0);
	Mat2QImage_DisplayImage();


	total_log.clear();
	current_log.clear();

	DisableUI();
}

void Micro_Plastic_Analyzer::on_btnSet_clicked()
{
	if (m_image.empty()) // m_image에 영상이 없다면 함수 종료
	{
		QMessageBox::warning(this, "No image", QString::fromLocal8Bit("이미지가 존재하지 않습니다"), QMessageBox::Yes);
		return;
	}

	if (!current_log.empty())
		total_log.push_back(current_log);
	current_log.clear();

	OutPut_ImgProcess(ui.outputCombo->currentIndex());
}

void Micro_Plastic_Analyzer::SelectCamera(int value)
{
	camNum = value;
	ui.InputCamCombo->clear();

	for (int i = 0; i < camResList[camNum].size(); i++)
	{
		ui.InputCamCombo->addItem(QString::number(camResList[camNum][i].first) + " * " + QString::number(camResList[camNum][i].second));
	}
}

void Micro_Plastic_Analyzer::clicked_radioButton()
{
	int index = ui.outputCombo->currentIndex();

	if (index == OUTPUT_SELECT_OPENING)
		Algorithm_Morphology(MORPH_OPEN);
	else if (index == OUTPUT_SELECT_CLOSING)
		Algorithm_Morphology(MORPH_CLOSE);
	else if (index == OUTPUT_SELECT_TOPHAT)
		Algorithm_Morphology(MORPH_TOPHAT);
	else if (index == OUTPUT_SELECT_BLACKHAT)
		Algorithm_Morphology(MORPH_BLACKHAT);

	Mat2QImage_DisplayImage();
}

void Micro_Plastic_Analyzer::clicked_CheckBox_Ostu()
{
	if (ui.OstucheckBox->isChecked())
	{
		if (ui.AveragecheckBox->isChecked())
			ui.AveragecheckBox->setChecked(false);
	}
	Algorithm_Threshold();
	Mat2QImage_DisplayImage();
}

void Micro_Plastic_Analyzer::clicked_CheckBox_Average()
{
	if (ui.AveragecheckBox->isChecked())
	{
		if (ui.OstucheckBox->isChecked())
			ui.OstucheckBox->setChecked(false);
	}
	Algorithm_Threshold();
	Mat2QImage_DisplayImage();
}

void Micro_Plastic_Analyzer::setpassminValue(int value)
{
	passminValue = value;

	int index = ui.outputCombo->currentIndex();
	if (index == OUTPUT_SELECT_2PASS_4CONNECTIVITY)
		Algorithm_2pass(4);
	else if (index == OUTPUT_SELECT_2PASS_8CONNECTIVITY)
		Algorithm_2pass(8);
	else if (index == OUTPUT_SELECT_MICRO_PLASTIC_DETECTION)
		Algorithm_Micro_Plastic_Detection();
	Mat2QImage_DisplayImage();
}

void Micro_Plastic_Analyzer::setpassmaxValue(int value)
{
	passmaxValue = value;

	int index = ui.outputCombo->currentIndex();
	if (index == OUTPUT_SELECT_2PASS_4CONNECTIVITY)
		Algorithm_2pass(4);
	else if (index == OUTPUT_SELECT_2PASS_8CONNECTIVITY)
		Algorithm_2pass(8);
	else if (index == OUTPUT_SELECT_MICRO_PLASTIC_DETECTION)
		Algorithm_Micro_Plastic_Detection();
	Mat2QImage_DisplayImage();
}

void Micro_Plastic_Analyzer::clicked_CheckBox_2Pass()
{
	int index = ui.outputCombo->currentIndex();
	if (index == OUTPUT_SELECT_2PASS_4CONNECTIVITY)
		Algorithm_2pass(4);
	else if (index == OUTPUT_SELECT_2PASS_8CONNECTIVITY)
		Algorithm_2pass(8);
	else if (index == OUTPUT_SELECT_MICRO_PLASTIC_DETECTION)
		Algorithm_Micro_Plastic_Detection();
	Mat2QImage_DisplayImage();
}


bool Micro_Plastic_Analyzer::load_Video()
{
	video.open(imgFile);

	if (video.isOpened())
	{
		fps = (int)video.get(CAP_PROP_FPS);

		while (video.read(m_originImg))
		{
			videoFrame.push_back(m_originImg.clone());
		}

		VideoSlider_Setting(0, videoFrame.size());

		return true;
	}
	else
	{
		QMessageBox::warning(this, "This Video is not support!", QString::fromLocal8Bit("재생할 수 없는 동영상 입니다."), QMessageBox::Yes);
		return false;
	}
}


void Micro_Plastic_Analyzer::on_btnStartPause_clicked()
{
	// 재생 모양이면 일시정지, 일시정지 모양이면 재생하도록 구현
	if (ui.btnStartPause->text() == QString::fromLocal8Bit("▶"))
	{
		videoIndex = ui.videoSlider->value();

		if (videoIndex == ui.videoSlider->maximum()) // videoIndex가 슬라이더 최댓값이라면 처음부터 재생
		{
			videoIndex = 0;
			ui.videoSlider->setValue(0);
		}

		videoTimer->start(1000 / fps);
		ui.btnStartPause->setText(QString::fromLocal8Bit("||"));
	}
	else if (ui.btnStartPause->text() == QString::fromLocal8Bit("||"))
	{
		videoTimer->stop();
		ui.btnStartPause->setText(QString::fromLocal8Bit("▶"));

		m_originImg = m_bufferImg.clone();
		m_processImg = m_originImg.clone();
		m_image = m_originImg.clone();
	}
}


void Micro_Plastic_Analyzer::pressedVideoSlider()
{
	if (videoTimer->isActive())
	{
		videoTimer->stop();
		ui.btnStartPause->setText(QString::fromLocal8Bit("▶"));
	}

	m_originImg = m_bufferImg.clone();
	m_processImg = m_originImg.clone();
	m_image = m_originImg.clone();
}


void Micro_Plastic_Analyzer::Update_Video()
{
	if (videoIndex >= videoFrame.size()) // videoIndex가 videoFrame 갯수보다 크거나 같으면 videoTimer멈춤
	{
		videoTimer->stop();
		ui.btnStartPause->setText(QString::fromLocal8Bit("▶"));
		return;
	}

	m_originImg = videoFrame[videoIndex].clone();
	videoIndex++;
	ui.videoSlider->setValue(videoIndex);

	ImageCvt();
	OutPut_ImgProcess(ui.outputCombo->currentIndex());
}


void Micro_Plastic_Analyzer::Update_Cam()
{
	cap.read(m_originImg);
	ImageCvt();
	OutPut_ImgProcess(ui.outputCombo->currentIndex());
}


//{640, 480}, { 320,240 }, { 800,600 },
//{ 960,720 }, { 1280,720 }, { 1600,900 }

void Micro_Plastic_Analyzer::Cam_Resolution(int index)
{
	QStringList list = ui.InputCamCombo->currentText().split("*");

	camWidth = list[0].toInt();
	camHeight = list[1].toInt();
}


void Micro_Plastic_Analyzer::Cam_Stop()
{
	camTimer->stop();
	cap.release(); // Cam 연동해제
}

void Micro_Plastic_Analyzer::Video_Stop()
{
	videoTimer->stop();
	video.release();
	videoFrame.clear();
}


void Micro_Plastic_Analyzer::Change_Index(int index)
{
	if (m_image.empty()) // m_image에 영상이 없다면 함수 종료
	{
		QMessageBox::warning(this, "No image", QString::fromLocal8Bit("이미지가 존재하지 않습니다"), QMessageBox::Yes);
		return;
	}
	if (index == OUTPUT_SELECT_NULL || index == OUTPUT_SELECT_INVERSION || index == OUTPUT_SELECT_HISTOGRAM_EQUALIZATION || index == OUTPUT_SELECT_CONTOUR
		|| index == OUTPUT_SELECT_WATERSHED || index == OUTPUT_SELECT_INTEGRAL || index == OUTPUT_SELECT_2PASS_4CONNECTIVITY || index == OUTPUT_SELECT_2PASS_8CONNECTIVITY || index == OUTPUT_SELECT_MICRO_PLASTIC_DETECTION)
		DisableUI();
	else // NULL상태가 아닌경우 UI활성화
		EnableUI();

	if (index == OUTPUT_SELECT_OPENING || index == OUTPUT_SELECT_CLOSING || index == OUTPUT_SELECT_TOPHAT || index == OUTPUT_SELECT_BLACKHAT || index == OUTPUT_SELECT_DISTANCE)
		ui.outputSlider->setTickInterval(1);  //틱 간격 1로
	else
		ui.outputSlider->setTickInterval(10);  //틱 간격 10으로

	MorphologyUse(index == OUTPUT_SELECT_OPENING || index == OUTPUT_SELECT_CLOSING || index == OUTPUT_SELECT_TOPHAT || index == OUTPUT_SELECT_BLACKHAT);
	ThresholdUse(index == OUTPUT_SELECT_THRESHOLD);

	if (index == OUTPUT_SELECT_ALGEBRA)
	{
		AlgebraUse(true);
	}
	else if (index == OUTPUT_SELECT_CANNY)
	{
		CannyUse(true);
	}
	else
	{
		AlgebraUse(false);
		CannyUse(false);
	}

	//TwoPassUse(index == OUTPUT_SELECT_2PASS_4CONNECTIVITY || index == OUTPUT_SELECT_2PASS_8CONNECTIVITY || index == OUTPUT_SELECT_MICRO_PLASTIC_DETECTION);

	if (index == OUTPUT_SELECT_2PASS_4CONNECTIVITY || index == OUTPUT_SELECT_2PASS_8CONNECTIVITY)
	{
		TwoPassUse(true);
	}
	else if (index == OUTPUT_SELECT_MICRO_PLASTIC_DETECTION)
	{
		PlasticDetectorUse(true);
	}
	else
	{
		TwoPassUse(false);
		PlasticDetectorUse(false);

	}

	OutPut_ImgProcess(index);
}



// Cam이면 매 프레임마다 영상처리가 가능하도록 만들어야함
// Image가 새로 로딩되면 콤보박스를 인덱스0번(NULL) 상태로 변경시켜야 함

void Micro_Plastic_Analyzer::OutPut_ImgProcess(int index) // 영상 알고리즘 처리
{
	if (m_image.empty()) // m_image에 영상이 없다면 함수 종료
	{
		return;
	}
	readTotallog();


	switch (index)
	{
	case OUTPUT_SELECT_NULL:
		m_image = m_processImg.clone(); // 현재까지의 이미지
		break;

	case OUTPUT_SELECT_INVERSION:
		bitwise_not(m_processImg, m_image);
		break;

	case OUTPUT_SELECT_ALGEBRA:
		Slider_Setting(-100, 100);
		Algorithm_Algebra();
		break;

	case OUTPUT_SELECT_BLURRING:
		Slider_Setting(3, 128);
		Algorithm_Blurring();
		break;

	case OUTPUT_SELECT_SHARPENING:
		Slider_Setting(-100, 100);
		Algorithm_Sharpening();
		break;

	case OUTPUT_SELECT_CANNY:
		Slider_Setting(0, 255);
		Algorithm_Canny();
		break;

	case OUTPUT_SELECT_THRESHOLD:
		Slider_Setting(0, 255);
		Algorithm_Threshold();
		break;

	case OUTPUT_SELECT_ADAPTIVE_THRESHOLD:
		Slider_Setting(3, 1000);
		Algorithm_Adaptive_Threshold();
		break;

	case OUTPUT_SELECT_HISTOGRAM_EQUALIZATION:
		Algorithm_Histogram_Equalization();
		break;

	case OUTPUT_SELECT_OPENING:
		Slider_Setting(0, 50);
		Algorithm_Morphology(MORPH_OPEN);
		break;

	case OUTPUT_SELECT_CLOSING:
		Slider_Setting(0, 50);
		Algorithm_Morphology(MORPH_CLOSE);
		break;

	case OUTPUT_SELECT_TOPHAT:
		Slider_Setting(0, 50);
		Algorithm_Morphology(MORPH_TOPHAT);
		break;

	case OUTPUT_SELECT_BLACKHAT:
		Slider_Setting(0, 50);
		Algorithm_Morphology(MORPH_BLACKHAT);
		break;

	case OUTPUT_SELECT_CONTOUR:
		Algorithm_Contour();
		break;

	case OUTPUT_SELECT_WATERSHED:
		Algorithm_Watershed();
		break;

	case OUTPUT_SELECT_INTEGRAL:
		Algorithm_Integral();
		break;

	case OUTPUT_SELECT_DISTANCE:
		Slider_Setting(1, 7);
		Algorithm_Distance_Transform();
		break;
	case OUTPUT_SELECT_2PASS_4CONNECTIVITY:
		Algorithm_2pass(4);
		break;
	case OUTPUT_SELECT_2PASS_8CONNECTIVITY:
		Algorithm_2pass(8);
		break;
	case OUTPUT_SELECT_MICRO_PLASTIC_DETECTION:
		Algorithm_Micro_Plastic_Detection();
		break;
	}
	writeCurrentlog(index);  //현재 로그(m_image)를 임시 저장

	Mat2QImage_DisplayImage();
}


void Micro_Plastic_Analyzer::setSlider_Value(int value)
{
	ui.sliderSpinBox->setValue(value); // 변경된 슬라이더 값으로 SpinBox값도 변경된다
	sliderVal = value;
	OutPut_ImgProcess(ui.outputCombo->currentIndex()); // 변경된 slider값으로 영상을 처리한다
}

void Micro_Plastic_Analyzer::setSpinBox_Value(int value)
{
	ui.outputSlider->setValue(value);
}

void Micro_Plastic_Analyzer::setSlider_Value_beta(int value)
{
	ui.sliderSpinBox_2->setValue(value); // 변경된 슬라이더 값으로 SpinBox값도 변경된다
	sliderVal2 = value;
	OutPut_ImgProcess(ui.outputCombo->currentIndex()); // 변경된 slider값으로 영상을 처리한다
}

void Micro_Plastic_Analyzer::setSpinBox_Value_beta(int value)
{
	ui.outputSlider_2->setValue(value);
}

void Micro_Plastic_Analyzer::setVideoSlider_Value(int value)
{
	if (value >= ui.videoSlider->maximum())
		return;

	videoIndex = value;

	if (ui.btnStartPause->text() == QString::fromLocal8Bit("▶")) // 일시정지 상태일 때 슬라이더를 움직이는 경우
	{
		m_originImg = videoFrame[videoIndex].clone();
		ImageCvt();
		OutPut_ImgProcess(ui.outputCombo->currentIndex());
	}
}

void Micro_Plastic_Analyzer::EnableUI() // 사용자가 제어하는 UI 활성화
{
	ui.outputSlider->setEnabled(true);
	ui.sliderSpinBox->setEnabled(true);
	ui.outputSlider->setVisible(true);
	ui.sliderSpinBox->setVisible(true);
}

void Micro_Plastic_Analyzer::DisableUI() // 사용자가 제어하는 UI 비활성화 & 초기화
{
	Slider_Setting(-128, 127);
	ui.outputSlider->setValue(0);

	ui.outputSlider->setEnabled(false);
	ui.sliderSpinBox->setEnabled(false);
	ui.outputSlider->setVisible(false);
	ui.sliderSpinBox->setVisible(false);

	MorphologyUse(false);
	CannyUse(false);
	ThresholdUse(false);
	AlgebraUse(false);
	TwoPassUse(false);

	if (video.isOpened())
		return;
	VideoUse(false);
}

void Micro_Plastic_Analyzer::VideoSlider_Setting(int minValue, int maxValue)
{
	ui.videoSlider->setRange(minValue, maxValue);
}

void Micro_Plastic_Analyzer::Slider_Setting(int minValue, int maxValue)
{
	ui.outputSlider->setRange(minValue, maxValue);
	ui.sliderSpinBox->setRange(minValue, maxValue);
}

void Micro_Plastic_Analyzer::MorphologyUse(bool b)  //사용시 true 아닐시 false
{
	ui.groupBox->setVisible(b);
	ui.groupBox->setEnabled(b);
}

void Micro_Plastic_Analyzer::ThresholdUse(bool b)
{
	ui.OstucheckBox->setVisible(b);
	ui.OstucheckBox->setEnabled(b);
	ui.AveragecheckBox->setVisible(b);
	ui.AveragecheckBox->setEnabled(b);
}

void Micro_Plastic_Analyzer::AlgebraUse(bool b)
{
	ui.outputSlider_2->setRange(-255, 255);
	ui.sliderSpinBox_2->setRange(-255, 255);

	ui.outputSlider_2->setVisible(b);
	ui.sliderSpinBox_2->setVisible(b);
	ui.outputSlider_2->setEnabled(b);
	ui.sliderSpinBox_2->setEnabled(b);
}


void Micro_Plastic_Analyzer::CannyUse(bool b)
{
	ui.outputSlider_2->setRange(0, 255);
	ui.sliderSpinBox_2->setRange(0, 255);

	ui.outputSlider_2->setVisible(b);
	ui.sliderSpinBox_2->setVisible(b);
	ui.outputSlider_2->setEnabled(b);
	ui.sliderSpinBox_2->setEnabled(b);
}

void  Micro_Plastic_Analyzer::TwoPassUse(bool b)
{
	ui.passMaxlabel->setVisible(b);
	ui.passMaxlabel->setEnabled(b);
	ui.passMaxspinBox->setVisible(b);
	ui.passMaxspinBox->setEnabled(b);


	ui.passMinlabel->setVisible(b);
	ui.passMinlabel->setEnabled(b);
	ui.passMinspinBox->setVisible(b);
	ui.passMinspinBox->setEnabled(b);


	ui.passFiltercheckBox->setVisible(b);
	ui.passFiltercheckBox->setEnabled(b);
	ui.passRectcheckBox->setVisible(b);
	ui.passRectcheckBox->setEnabled(b);
	ui.passOrigincheckBox->setVisible(b);
	ui.passOrigincheckBox->setEnabled(b);
	ui.passRectcheckBox->setText("Rect");

	ui.passInfolabel->setVisible(b);
	ui.passMinInfolabel->setVisible(b);
	ui.passlabel->setVisible(b);
	ui.passlabel2->setVisible(b);

	if (!b)
	{
		ui.passInfolabel->clear();
		ui.passMinInfolabel->clear();

		ui.imgLabel->repaint();  // 제거했으므로 다시 그리기 요청
	}
}

void Micro_Plastic_Analyzer::PlasticDetectorUse(bool b)
{
	ui.passMaxlabel->setVisible(false);
	ui.passMaxlabel->setEnabled(false);
	ui.passMaxspinBox->setVisible(false);
	ui.passMaxspinBox->setEnabled(false);


	ui.passMinlabel->setVisible(false);
	ui.passMinlabel->setEnabled(false);
	ui.passMinspinBox->setVisible(false);
	ui.passMinspinBox->setEnabled(false);


	ui.passFiltercheckBox->setVisible(false);
	ui.passFiltercheckBox->setEnabled(false);
	ui.passRectcheckBox->setVisible(false);
	ui.passRectcheckBox->setEnabled(false);
	ui.passOrigincheckBox->setVisible(false);
	ui.passOrigincheckBox->setEnabled(false);

	ui.passRectcheckBox->setText("Contour");

	ui.passInfolabel->setVisible(b);
	ui.passMinInfolabel->setVisible(false);
	ui.passlabel->setVisible(false);
	ui.passlabel2->setVisible(b);

	if (!b)
	{
		ui.passInfolabel->clear();
		ui.passMinInfolabel->clear();

		ui.imgLabel->repaint();  // 제거했으므로 다시 그리기 요청
	}
}


void Micro_Plastic_Analyzer::VideoUse(bool b)
{
	ui.videoSlider->setEnabled(b);
	ui.videoSlider->setVisible(b);

	ui.btnStartPause->setEnabled(b);
	ui.btnStartPause->setVisible(b);
}


void Micro_Plastic_Analyzer::FindAllCamera()
{
	HRESULT hr;
	ICaptureGraphBuilder2* pBuild;
	IGraphBuilder* pGraph;
	IMoniker* pMoniker;
	IEnumMoniker* pEnum;
	ICreateDevEnum* pDevEnum;
	IBaseFilter* pvCap = NULL;
	IAMStreamConfig* pConfig = NULL;


	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) {}

	hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IFilterGraph, (void**)&pGraph);
	if (FAILED(hr))
	{
		wprintf(L"Can't create GraphFilter\n");
	}

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
	if (FAILED(hr))
	{
		wprintf(L"Can't create CaputreGraph\n");
	}

	pBuild->SetFiltergraph(pGraph);
	if (FAILED(hr))
	{
		wprintf(L"SetFiltergraph Error\n");
	}

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum));
	if (FAILED(hr)) {}

	hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

	if (pEnum == NULL) // PC에 설치된 카메라가 없다면 함수 종료
		return;

	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag* pPropBag;
		AM_MEDIA_TYPE* pMediaType;

		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		VARIANT var;
		VariantInit(&var);

		hr = pPropBag->Read(L"FriendlyName", &var, 0); // 카메라의 이름을 읽음
		if (SUCCEEDED(hr))
		{
			ui.CameraCombo->addItem(QString::fromWCharArray(var.bstrVal)); // 카메라 이름을 CameraCombo에 추가


			// ---------------------------------카메라가 지원하는 해상도를 찾음---------------------------------------------------------

			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)(&pvCap)); // IBaseFilter 값 설정
			if (FAILED(hr))
			{
				wprintf(L"Base filter error\n");
			}

			hr = pBuild->FindInterface(&PIN_CATEGORY_CAPTURE, 0, pvCap, IID_IAMStreamConfig, (void**)&pConfig);
			if (FAILED(hr))
			{
				wprintf(L"Can't get IAMStreamConfig\n");
			}

			hr = pConfig->GetFormat(&pMediaType);
			if (FAILED(hr))
			{
				wprintf(L"Can't get MediaType to IAMStreamConfig\n");
			}

			int iCount = 0, iSize = 0;

			hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
			if (FAILED(hr))
			{
				wprintf(L"GetNumberOFCapabilities error\n");
			}

			if (iSize != sizeof(VIDEO_STREAM_CONFIG_CAPS))
			{
				wprintf(L"VIDEO_STREAM_CONFIG_CAPS error\n");
				return;
			}

			vector<int> pTemp;
			vector<pair<int, int>> pairTemp;
			int index = 0;

			for (int iFormat = 0; iFormat < iCount; iFormat++) // 해당 카메라에서 지원하는 해상도를 전부 추출
			{
				VIDEO_STREAM_CONFIG_CAPS scc;
				AM_MEDIA_TYPE* pmtConfig;

				hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
				if (SUCCEEDED(hr))
				{
					VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					//cout << "FPS: " << pVih->AvgTimePerFrame << endl;

					if (pVih->bmiHeader.biWidth < 100 || pVih->bmiHeader.biHeight < 100)
						continue;

					pTemp.push_back(index);
					pairTemp.push_back(make_pair((int)pVih->bmiHeader.biWidth, (int)pVih->bmiHeader.biHeight));
					index++;
				}

			}
			camResList.push_back(pairTemp);

			VariantClear(&var);
		}

		pPropBag->Release();
		pMoniker->Release();
	}

	// 중복되는 해상도 제거
	for (int i = 0; i < camResList.size(); i++)
	{
		std::sort(camResList[i].begin(), camResList[i].end());

		camResList[i].erase(unique(camResList[i].begin(), camResList[i].end()), camResList[i].end());
	}

	ui.CameraCombo->activated(0);
}

