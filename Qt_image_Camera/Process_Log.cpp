#include "Micro_Plastic_Analyzer.h"


// 저장한 전체 로그를 순차적으로 적용하는 함수
void Micro_Plastic_Analyzer::readTotallog()
{
	m_processImg = m_originImg.clone();
	Mat temp = m_originImg.clone();  //임시 상태 저장
	for (int i = 0; i < total_log.size(); i++)
	{
		if (total_log[i][0] == OUTPUT_SELECT_ALGEBRA)
		{
			m_processImg.convertTo(temp, -1, pow(2, total_log[i][1] / 100.), total_log[i][2]);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_INVERSION)
		{
			bitwise_not(m_processImg, temp);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_BLURRING)
		{
			if (total_log[i][1] % 2 == 0) // Size의 x, y 값은 홀수만 가능
				total_log[i][1] += 1;
			GaussianBlur(m_processImg, temp, Size(total_log[i][1], total_log[i][1]), 0);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_SHARPENING)
		{
			Mat blur;
			double alpha = (double)total_log[i][1] / 100;
			GaussianBlur(m_processImg, blur, Size(0, 0), 3);
			addWeighted(m_processImg, 1 + alpha, blur, -alpha, 0, temp);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_CANNY)
		{
			Mat grayimg, canny;
			if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지라면
			{
				cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
				Canny(grayimg, canny, total_log[i][1], total_log[i][2]);
				temp = Scalar::all(0);
				m_processImg.copyTo(temp, canny);
			}
			else
				Canny(m_processImg, temp, total_log[i][1], total_log[i][2]);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_THRESHOLD)
		{
			Mat grayimg;
			if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
				cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
			else
				grayimg = m_processImg.clone();

			if (total_log[i][2]) // ostu 체크 됐을 때
			{
				threshold(grayimg, temp, 0, 255, THRESH_BINARY | THRESH_OTSU); // alpha 이하면 0, 이상이면 255로 threshold처리, 반환값이 ostu value
			}
			else if (total_log[i][3])  //average 체크 됐을 때
			{
				Scalar sc = mean(grayimg);
				threshold(grayimg, temp, sc[0], 255, THRESH_BINARY); // alpha 이하면 0, 이상이면 255로 threshold처리, 반환값이 ostu value
			}
			else
			{
				threshold(grayimg, temp, total_log[i][1], 255, THRESH_BINARY); // alpha 이하면 0, 이상이면 255로 threshold처리
			}
		}
		else if (total_log[i][0] == OUTPUT_SELECT_ADAPTIVE_THRESHOLD)
		{
			if (total_log[i][1] % 2 == 0) // Size의 x, y 값은 홀수만 가능
				total_log[i][1] += 1;

			Mat grayimg;
			if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
				cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
			else
				grayimg = m_processImg.clone();
			adaptiveThreshold(grayimg, temp, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, total_log[i][1], 20);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_HISTOGRAM_EQUALIZATION)
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
				cvtColor(hsvImage, temp, COLOR_HSV2RGB);
			}
			else
				equalizeHist(m_processImg, temp);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_OPENING)
		{
			Mat element = getStructuringElement(total_log[i][2], Size(2 * total_log[i][1] + 1, 2 * total_log[i][1] + 1), Point(total_log[i][1], total_log[i][1]));
			morphologyEx(m_processImg, temp, MORPH_OPEN, element);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_CLOSING)
		{
			Mat element = getStructuringElement(total_log[i][2], Size(2 * total_log[i][1] + 1, 2 * total_log[i][1] + 1), Point(total_log[i][1], total_log[i][1]));
			morphologyEx(m_processImg, temp, MORPH_CLOSE, element);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_TOPHAT)
		{
			Mat element = getStructuringElement(total_log[i][2], Size(2 * total_log[i][1] + 1, 2 * total_log[i][1] + 1), Point(total_log[i][1], total_log[i][1]));
			morphologyEx(m_processImg, temp, MORPH_TOPHAT, element);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_BLACKHAT)
		{
			Mat element = getStructuringElement(total_log[i][2], Size(2 * total_log[i][1] + 1, 2 * total_log[i][1] + 1), Point(total_log[i][1], total_log[i][1]));
			morphologyEx(m_processImg, temp, MORPH_BLACKHAT, element);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_CONTOUR)
		{
			vector<vector<Point>> contours;  // 컨투어 좌표들을 저장할 벡터
			Mat grayimg;
			if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
				cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
			else
				grayimg = m_processImg.clone();
			findContours(grayimg, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

			Scalar color(0, 0, 255);  // 컨투어 나타낼 색상

			temp = m_processImg.clone();
			for (int i = 0; i < contours.size(); i++)
			{
				drawContours(temp, contours, i, color, 1);  // 그릴 이미지, 컨투어 좌표, 인덱스, 색상, 두께(-1)일 때 모두 채우기
			}
		}
		else if (total_log[i][0] == OUTPUT_SELECT_WATERSHED)
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
				temp = m_processImg.clone();
			else
			{
				temp.create(m_processImg.size(), CV_8UC3);
				Mat ary[3] = { m_processImg,m_processImg, m_processImg };
				merge(ary, 3, temp);
			}
			int ch = temp.channels();
			watershed(temp, markers);

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
						temp.at<Vec3b>(i, j) = color;
					}
				}
			}

		}
		else if (total_log[i][0] == OUTPUT_SELECT_INTEGRAL)
		{
			Mat grayimg, sumimg;
			sumimg.create(m_processImg.size(), CV_32S);

			if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지
				cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY); // 흑백 이미지로 변경
			else
				grayimg = m_processImg.clone();

			integral(grayimg, sumimg);
			normalize(sumimg, temp, 0, 255, NORM_MINMAX, CV_8U);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_DISTANCE)
		{
			Mat grayimg, dist;
			m_image = m_processImg.clone();
			if (m_processImg.channels() == 3) // 원본 이미지가 컬러 이미지라면
				cvtColor(m_processImg, grayimg, COLOR_RGB2GRAY);
			else
				m_processImg.copyTo(grayimg);

			distanceTransform(grayimg, dist, total_log[i][1], 0);
			normalize(dist, temp, 0, 255, NORM_MINMAX, CV_8U);
		}
		else if (total_log[i][0] == OUTPUT_SELECT_2PASS_4CONNECTIVITY || total_log[i][0] == OUTPUT_SELECT_2PASS_8CONNECTIVITY)
		{
			Mat labels;
			Mat stats; // 라벨링 된 이미지 정보, 객체의 수(배경 포함) == 행의 개수, 열 : 바운딩 박스 정보(x, y, w, h), 면적
			Mat centroids; // 라벨링 된 이미지의 중심 좌표
			int label;  // 라벨의 개수
			temp = m_processImg.clone();

			// 연산에 사용되는 변수

			int l_minPix = total_log[i][1], l_maxPix = total_log[i][2];

			// 그레이 스케일이 아니라면  NULL로 변경
			if (m_processImg.channels() == 3)
			{

				ui.outputCombo->setCurrentIndex(0);
				DisableUI();
				m_image = m_processImg.clone();

				QMessageBox::warning(this, "Not Thresholding Image", QString::fromLocal8Bit("2-Pass 알고리즘은 그레이스케일 영상에 대해서만 적용할 수 있습니다"), QMessageBox::Yes);
				return;
			}
			if(total_log[i][0] == OUTPUT_SELECT_2PASS_4CONNECTIVITY)
				label = connectedComponentsWithStats(m_processImg, labels, stats, centroids, 4, CV_32S); // 4방향 적용
			else
				label = connectedComponentsWithStats(m_processImg, labels, stats, centroids, 8, CV_32S); // 8방향 적용

			vector<int> currentLabels;  // 현재 선택된 사이즈 이하인 픽셀 라벨들
			for (int i = 0; i < stats.rows; i++)
			{
				if (stats.at<int>(i, CC_STAT_AREA) <= passmaxValue && stats.at<int>(i, CC_STAT_AREA) >= passminValue)
				{
					l_minPix = min(l_minPix, stats.at<int>(i, CC_STAT_AREA));
					l_maxPix = max(l_maxPix, stats.at<int>(i, CC_STAT_AREA));
					currentLabels.push_back(i);
				}
			}

			// 필터링 과정

			for (int i = 0; i < temp.rows; i++)
			{
				for (int j = 0; j < temp.cols; j++)
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
						temp.at<uchar>(i, j) = 255;
					else
						temp.at<uchar>(i, j) = 0;
				}
			}

		}


		m_processImg = temp.clone();
	}
}

// 지금 띄워져 있는 상태를 임시 저장하는 함수
void Micro_Plastic_Analyzer::writeCurrentlog(int index)
{
	current_log.clear();
	//current_log.push_back(index);  //옵션 값 먼저 push
	int morph_elem = 0;
	switch (index)
	{
	case OUTPUT_SELECT_HISTOGRAM_EQUALIZATION:
	case OUTPUT_SELECT_CONTOUR:
	case OUTPUT_SELECT_WATERSHED:
	case OUTPUT_SELECT_INTEGRAL:
	case OUTPUT_SELECT_INVERSION:
		current_log = { index };
		break;

	case OUTPUT_SELECT_BLURRING:
	case OUTPUT_SELECT_SHARPENING:

	case OUTPUT_SELECT_DISTANCE:
	case OUTPUT_SELECT_ADAPTIVE_THRESHOLD:
		current_log = { index,ui.outputSlider->value() };
		break;

	case OUTPUT_SELECT_CANNY:
	case OUTPUT_SELECT_ALGEBRA:
		current_log = { index,ui.outputSlider->value(),ui.outputSlider_2->value() };
		break;

	case OUTPUT_SELECT_THRESHOLD:
		current_log = { index,ui.outputSlider->value(), ui.OstucheckBox->isChecked(),ui.AveragecheckBox->isChecked() };
		break;

	case OUTPUT_SELECT_OPENING:
	case OUTPUT_SELECT_CLOSING:
	case OUTPUT_SELECT_TOPHAT:
	case OUTPUT_SELECT_BLACKHAT:
		if (ui.radioRect->isChecked())  //Rect
			morph_elem = MORPH_RECT;
		else if (ui.radioCross->isChecked())  //Cross
			morph_elem = MORPH_CROSS;
		else if (ui.radioEllipse->isChecked()) // Ellipse
			morph_elem = MORPH_ELLIPSE;
		current_log = { index, ui.outputSlider->value(), morph_elem };
		break;

	case OUTPUT_SELECT_2PASS_4CONNECTIVITY:
	case OUTPUT_SELECT_2PASS_8CONNECTIVITY:
		current_log = { index,ui.passMinspinBox->value(),ui.passMaxspinBox->value() };
		break;
	}
}