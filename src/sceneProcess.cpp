/*
 * sceneProcess.cpp
 *
 *  Created on: 2019年1月4日
 *      Author: jet
 */
#include "sceneProcess.hpp"

void CSceneProcess::start()
{
	memset(m_cnt,0,sizeof(m_cnt));
}

void CSceneProcess::detect(const Mat& frame, int chId)
{
	//Rect roi = Rect(100-100*sin((m_cnt[chId]%60)*6*M3D_PI_DIV_180), 100, frame.cols-200, frame.rows-200);
	Rect roi = Rect(100,100,frame.cols-200, frame.rows-200);
	Mat frameROI(frame, roi);

	if(m_cnt[chId] == 0)
		m_obj.InitSceneLock(frameROI);
	else
		m_obj.CalSceneLock(frameROI);

}

void CSceneProcess::getResult(cv::Point2f & result)
{
	result.x = m_obj.m_filterVel.x;
	result.y = m_obj.m_filterVel.y;
	return ;
}


