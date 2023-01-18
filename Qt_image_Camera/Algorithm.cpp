#include "Micro_Plastic_Analyzer.h"

void Micro_Plastic_Analyzer::Algorithm_Algebra()
{
	double alpha = pow(2, sliderVal / 100.);  // Contrast, -100일때 x0.5, 100일때 x2
	int beta = sliderVal2;  // Brightness
	m_processImg.convertTo(m_image, -1, alpha, beta);
}


void Micro_Plastic_Analyzer::Algorithm_Blurring()
{
	int filterSize = sliderVal;
	if (filterSize % 2 == 0) // Size의 x, y 값은 홀수만 가능
		filterSize += 1;
	GaussianBlur(m_processImg, m_image, Size(filterSize, filterSize), 0);
}


void Micro_Plastic_Analyzer::Algorithm_Sharpening()
{
	double alpha = (double)sliderVal / 100;
	GaussianBlur(m_processImg, m_image, Size(0, 0), 3);
	addWeighted(m_processImg, 1 + alpha, m_image, -alpha, 0, m_image);
}


void Micro_Plastic_Analyzer::Algorithm_Canny()
{
	int alpha = sliderVal;
	int beta = sliderVal2;
	Mat grayimg, canny;
	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지라면
	{
		cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
		Canny(grayimg, canny, alpha, beta);
		m_image = Scalar::all(0);
		m_processImg.copyTo(m_image, canny);
	}
	else
		Canny(m_processImg, m_image, alpha, beta);
}


void Micro_Plastic_Analyzer::Algorithm_Threshold()
{
	int alpha = sliderVal, ostuvalue = 0, averagevalue = 0;

	Mat grayimg;
	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
		cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
	else
		grayimg = m_processImg.clone();

	if (ui.OstucheckBox->isChecked()) // 체크 됐을 때
	{
		ui.outputSlider->setEnabled(false);
		ui.sliderSpinBox->setEnabled(false);
		ostuvalue = threshold(grayimg, m_image, 0, 255, THRESH_BINARY | THRESH_OTSU); // alpha 이하면 0, 이상이면 255로 threshold처리, 반환값이 ostu value
		ui.outputSlider->setValue(ostuvalue);
	}
	else if (ui.AveragecheckBox->isChecked())
	{
		ui.outputSlider->setEnabled(false);
		ui.sliderSpinBox->setEnabled(false);
		Scalar sc = mean(grayimg);

		averagevalue = sc[0];
		threshold(grayimg, m_image, averagevalue, 255, THRESH_BINARY); // alpha 이하면 0, 이상이면 255로 threshold처리, 반환값이 ostu value
		ui.outputSlider->setValue(averagevalue);
	}
	else
	{
		EnableUI();
		threshold(grayimg, m_image, alpha, 255, THRESH_BINARY); // alpha 이하면 0, 이상이면 255로 threshold처리
	}
}


void Micro_Plastic_Analyzer::Algorithm_Adaptive_Threshold()
{
	int filterSize = sliderVal;
	if (filterSize % 2 == 0)
		filterSize++;

	Mat grayimg;
	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
		cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
	else
		grayimg = m_processImg.clone();

	adaptiveThreshold(grayimg, m_image, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, filterSize, 20);
}


void Micro_Plastic_Analyzer::Algorithm_Histogram_Equalization()
{
	Mat hsvImage, Value;  //hsv 좌표계로 변환할 이미지, 명도를 저장할 이미지
	Mat HSV[3];  // Split저장 할 배열
	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지라면
	{
		cvtColor(m_processImg, hsvImage, COLOR_RGB2HSV);
		split(hsvImage, HSV);
		equalizeHist(HSV[2], Value);
		Value.copyTo(HSV[2]);
		merge(HSV, 3, hsvImage);  // Split한 배열, 채널수, 옮길 위치
		cvtColor(hsvImage, m_image, COLOR_HSV2RGB);
	}
	else
		equalizeHist(m_processImg, m_image);

}


void Micro_Plastic_Analyzer::Algorithm_Morphology(int operation)
{
	int morph_size = sliderVal;
	int morph_elem = 0;  // 커널 종류 지정
	if (ui.radioRect->isChecked())  //Rect
		morph_elem = MORPH_RECT;
	else if (ui.radioCross->isChecked())  //Cross
		morph_elem = MORPH_CROSS;
	else if (ui.radioEllipse->isChecked()) // Ellipse
		morph_elem = MORPH_ELLIPSE;
	else
		ui.radioRect->setChecked(true);
	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	morphologyEx(m_processImg, m_image, operation, element);
}



void Micro_Plastic_Analyzer::Algorithm_Contour()
{
	vector<vector<Point>> contours;  // 컨투어 좌표들을 저장할 벡터
	Mat grayimg;
	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
		cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
	else
		grayimg = m_processImg.clone();
	findContours(grayimg, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	Scalar color(255, 0, 0);  // 컨투어 나타낼 색상

	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
		m_image = m_processImg.clone();
	else  // 흑백 이미지일때 컬러로 변경
		cvtColor(m_processImg, m_image, COLOR_GRAY2RGB);

	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(m_image, contours, i, color);  // 그릴 이미지, 컨투어 좌표, 인덱스, 색상, 두께(-1)일 때 모두 채우기
	}
}


void Micro_Plastic_Analyzer::Algorithm_Watershed()
{
	Mat markers = Mat::zeros(m_processImg.size(), CV_32S);
	Mat grayimg;
	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지라면
		cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY);
	else
		grayimg = m_processImg.clone();

	vector<vector<Point>> contours;  //컨투어 이용해서 윤곽 검출
	findContours(grayimg, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++)
		drawContours(markers, contours, i, i + 1, -1);

	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지라면
		m_image = m_processImg.clone();
	else
	{
		m_image.create(m_processImg.size(), CV_8UC3);
		Mat ary[3] = { m_processImg,m_processImg, m_processImg };
		merge(ary, 3, m_image);
	}
	int ch = m_image.channels();
	watershed(m_image, markers);

	// 컬러 테이블 랜덤 생성
	Mat colorTable(contours.size() + 1, 1, CV_8UC3);
	Vec3b color;
	for (int i = 0; i < contours.size(); i++)
	{
		for (int j = 0; j < 3; j++)
			color[j] = theRNG().uniform(0, 180) + 50;
		colorTable.at<Vec3b>(i, 0) = color;
	}

	for (int i = 0; i < markers.rows; i++)
	{
		for (int j = 0; j < markers.cols; j++)
		{
			int k = markers.at<int>(i, j);

			if (k == -1)  // boundry
				color = { 255,255,255 };
			else if (k <= 0 || k > contours.size() + 1)
				color = { 0,0,0 };
			else
			{
				color = colorTable.at<Vec3b>(k - 1, 0);
				m_image.at<Vec3b>(i, j) = color;
			}
		}
	}
}


void Micro_Plastic_Analyzer::Algorithm_Integral()
{
	Mat grayimg, sumimg;
	sumimg.create(m_processImg.size(), CV_32S);

	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
		cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
	else
		grayimg = m_processImg.clone();

	integral(grayimg, sumimg);
	normalize(sumimg, m_image, 0, 255, NORM_MINMAX, CV_8U);
}


void Micro_Plastic_Analyzer::Algorithm_Distance_Transform()
{
	Mat grayimg, dist;
	m_image = m_processImg.clone();
	if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지라면
		cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY);
	else
		m_processImg.copyTo(grayimg);

	distanceTransform(grayimg, dist, sliderVal, 0);
	normalize(dist, m_image, 0, 255, NORM_MINMAX, CV_8U);
}


// 흰색에 대해서 객체로 인식하고 레이블링함
void Micro_Plastic_Analyzer::Algorithm_2pass(int connectivity)
{
	// 라벨 레이어 변수
	Mat labels;
	Mat stats; // 라벨링 된 이미지 정보, 객체의 수(배경 포함) == 행의 개수, 열 : 바운딩 박스 정보(x, y, w, h), 면적
	Mat centroids; // 라벨링 된 이미지의 중심 좌표
	int label;  // 라벨의 개수
	Mat two_pass = m_processImg.clone();

	// 연산에 사용되는 변수
	int g_minPix = INT_MAX, minLabel = 0;  // 최소 픽셀 사이즈, 해당하는 최소 라벨
	int l_minPix = passmaxValue, l_maxPix = passminValue;

	// 그레이 스케일이 아니라면  NULL로 변경
	if (m_processImg.channels() == 3)
	{

		ui.outputCombo->setCurrentIndex(0);
		DisableUI();
		m_image = m_processImg.clone();

		QMessageBox::warning(this, "Not Thresholding Image", QString::fromLocal8Bit("2-Pass 알고리즘은 그레이스케일 영상에 대해서만 적용할 수 있습니다"), QMessageBox::Yes);
		return;
	}

	label = connectedComponentsWithStats(m_processImg, labels, stats, centroids, connectivity, CV_32S); // 4, 8방향 적용


	// 최소 픽셀 값 찾기
	vector<int> minLabels;  // 최소 픽셀 라벨들
	vector<int> currentLabels;  // 현재 선택된 사이즈 이하인 픽셀 라벨들
	vector<int> currentMinLabels;  // 현재 선택된 사이즈 이하의 픽셀 중 가장 작은 라벨들

	for (int i = 0; i < stats.rows; i++)
	{
		//경계라면 pass
		if (stats.at<int>(i, CC_STAT_LEFT) < 1 || stats.at<int>(i, CC_STAT_TOP) < 1 || stats.at<int>(i, CC_STAT_LEFT) + stats.at<int>(i, CC_STAT_WIDTH) > m_processImg.cols - 2 || stats.at<int>(i, CC_STAT_TOP) + stats.at<int>(i, CC_STAT_HEIGHT) > m_processImg.rows - 2)
			continue;

		if (stats.at<int>(i, CC_STAT_AREA) <= passmaxValue && stats.at<int>(i, CC_STAT_AREA) >= passminValue)
		{
			l_minPix = min(l_minPix, stats.at<int>(i, CC_STAT_AREA));
			l_maxPix = max(l_maxPix, stats.at<int>(i, CC_STAT_AREA));
			currentLabels.push_back(i);
		}

		g_minPix = min(g_minPix, stats.at<int>(i, CC_STAT_AREA));

	}
	if (g_minPix == INT_MAX)
		g_minPix = 0;  //아무것도 없다면 0
	for (int i = 0; i < stats.rows; i++)
	{
		if (g_minPix == stats.at<int>(i, CC_STAT_AREA))
			minLabels.push_back(i);
		if (l_minPix == stats.at<int>(i, CC_STAT_AREA))
			currentMinLabels.push_back(i);
	}


	// 필터링 과정
	if (ui.passFiltercheckBox->isChecked())
	{
		for (int i = 0; i < two_pass.rows; i++)
		{
			for (int j = 0; j < two_pass.cols; j++)
			{
				bool isexist = false;
				for (int k = 0; k < currentLabels.size(); k++)
				{
					if (labels.at<int>(i, j) == currentLabels[k])
					{
						isexist = true;
						break;
					}
				}
				if (isexist)
					two_pass.at<uchar>(i, j) = 255;
				else
					two_pass.at<uchar>(i, j) = 0;
			}
		}
	}

	cvtColor(two_pass, m_image, COLOR_GRAY2RGB);

	if (ui.passOrigincheckBox->isChecked())
	{
		m_image = m_originImg.clone();
		if (m_originImg.channels() == 1)
			cvtColor(m_image, m_image, COLOR_GRAY2RGB);
	}

	// rect 체크됐을 때 사각형으로 표시
	if (ui.passRectcheckBox->isChecked())
	{
		//최대값으로 설정한 픽셀보다 같거나 작은 라벨들 : 노란색
		for (int i = 0; i < currentLabels.size(); i++)
		{
			rectangle(m_image, Rect(stats.at<int>(currentLabels[i], CC_STAT_LEFT), stats.at<int>(currentLabels[i], CC_STAT_TOP), stats.at<int>(currentLabels[i], CC_STAT_WIDTH), stats.at<int>(currentLabels[i], CC_STAT_HEIGHT)), Scalar(0, 0, 255), 1);
		}
		//최소값에 해당하는 라벨들 : 초록색
		for (int i = 0; i < currentMinLabels.size(); i++)
		{
			rectangle(m_image, Rect(stats.at<int>(currentMinLabels[i], CC_STAT_LEFT), stats.at<int>(currentMinLabels[i], CC_STAT_TOP), stats.at<int>(currentMinLabels[i], CC_STAT_WIDTH), stats.at<int>(currentMinLabels[i], CC_STAT_HEIGHT)), Scalar(255, 255, 0), 1);
		}
	}

	ui.passMinInfolabel->setText(QString::fromLocal8Bit("pixel size : ") + QString::number(g_minPix) + "px       " + QString::fromLocal8Bit("count : ") + QString::number(minLabels.size()));
	ui.passInfolabel->setText(QString::fromLocal8Bit("count : ") + QString::number(currentLabels.size()) + "\n" + QString::fromLocal8Bit("Min pixel size : ") + QString::number(l_minPix) + "px\n" + QString::fromLocal8Bit("Max pixel size : ") + QString::number(l_maxPix) + "px");

}



void Micro_Plastic_Analyzer::Algorithm_Micro_Plastic_Detection()
{
	Mat step1_grayimg, step2_blur, step3_binary, step4_2pass;
	Mat element;

	Mat labels;
	Mat stats; // 라벨링 된 이미지 정보, 객체의 수(배경 포함) == 행의 개수, 열 : 바운딩 박스 정보(x, y, w, h), 면적
	Mat centroids; // 라벨링 된 이미지의 중심 좌표
	int label;  // 라벨의 개수

	// 연산에 사용되는 변수
	//int g_minPix = INT_MAX, minLabel = 0;  // 최소 픽셀 사이즈, 해당하는 최소 라벨
	int minimumPix = 0, maximumPix = 100000;
	int l_minPix = maximumPix, l_maxPix = minimumPix;

	// 최소 픽셀 값 찾기
	vector<int> currentLabels;  // 현재 선택된 사이즈 이하인 픽셀 라벨들
	vector<int> currentMinLabels;  // 현재 선택된 사이즈 이하의 픽셀 중 가장 작은 라벨들



	//Step1 RGB -> Gray
	if (m_originImg.channels() == 3)
		cvtColor(m_originImg, step1_grayimg, COLOR_RGB2GRAY);
	else
		m_originImg.copyTo(step1_grayimg);



	//Step2 Blur
	GaussianBlur(step1_grayimg, step2_blur, Size(5, 5), 0);



	//Step3_1
	Mat step3_1_1_adaptive_threshold, step3_1_2_opening, step3_1_3_fill, step3_1_4_closing, step3_1_5_inverse;
	//Adaptive threshold
	adaptiveThreshold(step2_blur, step3_1_1_adaptive_threshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 351, 20);

	//Opening
	element = getStructuringElement(MORPH_ELLIPSE, Size(2 * 2 + 1, 2 * 2 + 1), Point(2, 2));
	morphologyEx(step3_1_1_adaptive_threshold, step3_1_2_opening, MORPH_OPEN, element);

	//Contour
	vector<vector<Point>> step3_1_3_contours;  // 컨투어 좌표들을 저장할 벡터
	vector<Vec4i> step3_1_3_hierarchy;

	findContours(step3_1_2_opening, step3_1_3_contours, step3_1_3_hierarchy, RETR_CCOMP, CHAIN_APPROX_NONE, Point(0, 0));
	step3_1_2_opening.copyTo(step3_1_3_fill);
	for (int i = 0; i < step3_1_3_contours.size(); i++)
		if (step3_1_3_hierarchy[i][3] == 1)
			drawContours(step3_1_3_fill, step3_1_3_contours, i, 0, -1, 8, step3_1_3_hierarchy, 1);

	//Closing
	element = getStructuringElement(MORPH_ELLIPSE, Size(2 * 1 + 1, 2 * 1 + 1), Point(1, 1));
	morphologyEx(step3_1_3_fill, step3_1_4_closing, MORPH_CLOSE, element);

	//inverse
	bitwise_not(step3_1_4_closing, step3_1_5_inverse);
	



	//Step3_2
	Mat step3_2_1_blackhat, step3_2_2_ostu,step3_2_3_closing;

	//Blackhat
	element = getStructuringElement(MORPH_ELLIPSE, Size(2 * 50 + 1, 2 * 50 + 1), Point(50, 50));
	morphologyEx(step2_blur, step3_2_1_blackhat, MORPH_BLACKHAT, element);


	threshold(step3_2_1_blackhat, step3_2_2_ostu, 0, 255, THRESH_BINARY | THRESH_OTSU);

	element = getStructuringElement(MORPH_ELLIPSE, Size(2 * 2 + 1, 2 *2 + 1), Point(2, 2));
	morphologyEx(step3_2_2_ostu, step3_2_3_closing, MORPH_CLOSE, element);


	bitwise_and(step3_1_5_inverse, step3_2_3_closing, step3_binary);

	label = connectedComponentsWithStats(step3_binary, labels, stats, centroids, 8, CV_32S); // 8방향 적용
	step3_binary.copyTo(step4_2pass);


	for (int i = 0; i < stats.rows; i++)
	{
		// 경계 처리
		if (stats.at<int>(i, CC_STAT_LEFT) < 1 || stats.at<int>(i, CC_STAT_TOP) < 1 || stats.at<int>(i, CC_STAT_LEFT) + stats.at<int>(i, CC_STAT_WIDTH) > m_processImg.cols - 2 || stats.at<int>(i, CC_STAT_TOP) + stats.at<int>(i, CC_STAT_HEIGHT) > m_processImg.rows - 2)
			continue;

		if (stats.at<int>(i, CC_STAT_AREA) <= maximumPix && stats.at<int>(i, CC_STAT_AREA) >= minimumPix)
		{
			l_minPix = min(l_minPix, stats.at<int>(i, CC_STAT_AREA));
			l_maxPix = max(l_maxPix, stats.at<int>(i, CC_STAT_AREA));
			currentLabels.push_back(i);
		}

	}

	if (l_minPix == maximumPix)
		l_minPix = 0;

	for (int i = 0; i < stats.rows; i++)
	{
		if (l_minPix == stats.at<int>(i, CC_STAT_AREA))
			currentMinLabels.push_back(i);
	}

	// 필터링 과정
	for (int i = 0; i < step4_2pass.rows; i++)
	{
		for (int j = 0; j < step4_2pass.cols; j++)
		{
			bool isexist = false;
			for (int k = 0; k < currentLabels.size(); k++)
			{
				if (labels.at<int>(i, j) == currentLabels[k])
				{
					isexist = true;
					break;
				}
			}
			if (isexist)
				step4_2pass.at<uchar>(i, j) = 255;
			else
				step4_2pass.at<uchar>(i, j) = 0;
		}
	}

		m_image = m_originImg.clone();
		if (m_originImg.channels() == 1)
			cvtColor(m_image, m_image, COLOR_GRAY2RGB);



	vector<vector<Point>> contours;  // 컨투어 좌표들을 저장할 벡터
	findContours(step4_2pass, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	Scalar color(0, 255, 0);  // 컨투어 나타낼 색상

	for (int i = 0; i < contours.size(); i++)
	{
		drawContours(m_image, contours, i, color, 2);  // 그릴 이미지, 컨투어 좌표, 인덱스, 색상, 두께(-1)일 때 모두 채우기
	}
	
	// 텍스트 추가
	for (int i = 0; i < currentLabels.size(); i++)
	{
		int px_s=stats.at<int>(currentLabels[i], CC_STAT_AREA);  // 픽셀의 크기
		string px_d = to_string(sqrt(px_s) / sqrt(l_minPix));
		px_d=px_d.substr(0, px_d.find('.') + 3);
		putText(m_image, to_string(i), Point(stats.at<int>(currentLabels[i], CC_STAT_LEFT) + stats.at<int>(currentLabels[i], CC_STAT_WIDTH) + 5, stats.at<int>(currentLabels[i], CC_STAT_TOP) + 10), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0), 3);
		putText(m_image, px_d +"um", Point(stats.at<int>(currentLabels[i], CC_STAT_LEFT) + stats.at<int>(currentLabels[i], CC_STAT_WIDTH) + 5, stats.at<int>(currentLabels[i], CC_STAT_TOP) + 40), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0), 3);
	}

	string px_d_min = to_string(sqrt(l_minPix) / sqrt(l_minPix));
	px_d_min = px_d_min.substr(0, px_d_min.find('.') + 3);

	string px_d_max = to_string(sqrt(l_maxPix) / sqrt(l_minPix));
	px_d_max = px_d_max.substr(0, px_d_max.find('.') + 3);

	ui.passInfolabel->setText(QString::fromLocal8Bit("count : ") + QString::number(currentLabels.size()) + "\n" + QString::fromLocal8Bit("Min diameter : ") + QString::fromStdString(px_d_min) + "um\n" + QString::fromLocal8Bit("Max diameter : ") + QString::fromStdString(px_d_max)  + "um");

}