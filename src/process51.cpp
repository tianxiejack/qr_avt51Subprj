
#include <glut.h>
#include "process51.hpp"
#include "vmath.h"
#include "msgDriv.h"
#include"app_ctrl.h"

#include "osd_cv.h"
#include "app_status.h"
#include "configable.h"
#include "Ipcctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

OSDSTATUS gConfig_Osd_param = {0};
UTCTRKSTATUS gConfig_Alg_param = {0};
extern int ScalerLarge,ScalerMid,ScalerSmall;
CProcess * CProcess::sThis = NULL;
static bool DrawMoveDetect = 0;
CProcess* plat = NULL;

SENDST trkmsg={0};

void inputtmp(unsigned char cmdid)
{
	plat->OnKeyDwn(cmdid);
}

void getMmtTg(unsigned char index,int *x,int *y)
{
	int cur_x = 
	*x = (int)plat->m_mtd[0]->tg[index].cur_x%VIDEO_IMAGE_WIDTH_0;
	*y = (int)plat->m_mtd[0]->tg[index].cur_y%VIDEO_IMAGE_HEIGHT_0;
	return ;
}

CProcess::CProcess()
{
	memset(rcTrackBak, 0, sizeof(rcTrackBak));
	memset(tgBak, 0, sizeof(tgBak));
	memset(&extOutAck, 0, sizeof(ACK_EXT));
	prisensorstatus=0;//tv
	m_castTm=0;
	m_bCast=false;
	rememflag=false;
	rememtime=0;
	// default cmd value
	extInCtrl = (CMD_EXT*)ipc_getimgstatus_p();
	memset(extInCtrl,0,sizeof(CMD_EXT));
	CMD_EXT *pIStuts = extInCtrl;
	
	pIStuts->opticAxisPosX[0]   = VIDEO_IMAGE_WIDTH_0/2;
	pIStuts->opticAxisPosY[0]   = VIDEO_IMAGE_HEIGHT_0/2;
	pIStuts->opticAxisPosX[1]   = VIDEO_IMAGE_WIDTH_1/2;
	pIStuts->opticAxisPosY[1]   = VIDEO_IMAGE_HEIGHT_1/2;
	
	pIStuts->unitAimW 		= 	AIM_WIDTH;
	pIStuts->unitAimH 		= 	AIM_HEIGHT;
	pIStuts->unitAimX		=	VIDEO_IMAGE_WIDTH_0/2;
	pIStuts->unitAimY		=	VIDEO_IMAGE_HEIGHT_0/2;

	pIStuts->SensorStat 	=   MAIN_CHID;
	pIStuts->SensorStatpri  =   pIStuts->SensorStat;
	pIStuts->PicpSensorStatpri	=	pIStuts->PicpSensorStat = 0xFF;
	
	pIStuts->changeSensorFlag = 0;
	crossBak.x = pIStuts->opticAxisPosX[pIStuts->SensorStat ];
	crossBak.y = pIStuts->opticAxisPosY[pIStuts->SensorStat ];
	pIStuts->AvtTrkAimSize= AVT_TRK_AIM_SIZE;

	pIStuts->AvtPosX[0]		=VIDEO_IMAGE_WIDTH_0/2;
	pIStuts->AvtPosY[0]		=VIDEO_IMAGE_HEIGHT_0/2;
	pIStuts->AvtPosX[1]		=VIDEO_IMAGE_WIDTH_1/2;
	pIStuts->AvtPosY[1]		=VIDEO_IMAGE_HEIGHT_1/2;

	pIStuts->AxisPosX[0]	=VIDEO_IMAGE_WIDTH_0/2;
	pIStuts->AxisPosY[0]	=VIDEO_IMAGE_HEIGHT_0/2;
	pIStuts->AxisPosX[1]	=VIDEO_IMAGE_WIDTH_1/2;
	pIStuts->AxisPosY[1]	=VIDEO_IMAGE_HEIGHT_1/2;
	
	pIStuts->PicpPosStat = 0;
	pIStuts->validChId = pIStuts->SensorStatBegin;
	pIStuts->FovStat=1;

	pIStuts->FrCollimation=2;
	pIStuts->PicpSensorStatpri=2;
	pIStuts->axisMoveStepX = 0;
	pIStuts->axisMoveStepY = 0;

	memset(secBak,0,sizeof(secBak));
	memset(Osdflag,0,sizeof(Osdflag));
	
	Mmtsendtime=0;

	rendpos[0].x=vdisWH[0][0]*2/3;
	rendpos[0].y=vdisWH[0][1]*2/3;
	rendpos[0].w=vdisWH[0][0]/3;
	rendpos[0].h=vdisWH[0][1]/3;

	rendpos[1].x=vdisWH[0][0]*2/3;
	rendpos[1].y=0;
	rendpos[1].w=vdisWH[0][0]/3;
	rendpos[1].h=vdisWH[0][1]/3;

	rendpos[2].x=0;
	rendpos[2].y=0;
	rendpos[2].w=vdisWH[0][0]/3;
	rendpos[2].h=vdisWH[0][1]/3;

	rendpos[3].x=0;
	rendpos[3].y=vdisWH[0][1]*2/3;
	rendpos[3].w=vdisWH[0][0]/3;
	rendpos[3].h=vdisWH[0][1]/3;

	msgextInCtrl = extInCtrl;
	sThis = this;
	plat = this;

	update_param_osd();

	pIStuts->DispGrp[0] = 1;
	pIStuts->DispGrp[1] = 1;
	pIStuts->DispColor[0]=2;
	pIStuts->DispColor[1]=2;
	
}

CProcess::~CProcess()
{
	sThis=NULL;
}

int  CProcess::WindowstoPiexlx(int x,int channel)
{
	int ret=0;
	ret= cvRound(x*1.0/vdisWH[0][0]*vcapWH[channel][0]);
	 if(ret<0)
	 	{
			ret=0;
	 	}
	 else if(ret>=vcapWH[channel][0])
	 	{
			ret=vcapWH[channel][0];
	 	}


	  return ret;
}


int  CProcess::WindowstoPiexly(int y,int channel)
{
	 int ret=0;
	 ret= cvRound(y*1.0/vdisWH[0][1]*vcapWH[channel][1]);

	  if(ret<0)
	 	{
			ret=0;
	 	}
	 else if(ret>=vcapWH[channel][1])
	 	{
			ret=vcapWH[channel][1];
	 	}
	return  ret;
}



float  CProcess::PiexltoWindowsx(int x,int channel)
{
	 float ret=0;
	 ret= cvRound(x*1.0/vcapWH[channel][0]*vdisWH[channel][0]);
	 if(ret<0)
 	 {
		ret=0;
 	 }
	 else if(ret>=vdisWH[channel][0])
 	 {
		ret=vdisWH[channel][0];
 	 }
	 if(extInCtrl->ImgMmtshow[extInCtrl->SensorStat])
 	 {
		ret =ret*2/3;
 	 }

	 return ret;
}

float  CProcess::PiexltoWindowsy(int y,int channel)
{
	 float ret=0;
	 ret= cvRound(y*1.0/vcapWH[channel][1]*vdisWH[channel][1]);

	 if(ret<0)
 	 {
		ret=0;
 	 }
	 else if(ret>=vdisWH[channel][1])
 	 {
		ret=vdisWH[channel][1];
 	 }

	  if(extInCtrl->ImgMmtshow[extInCtrl->SensorStat])
 	  {
		ret =ret*2/3;
 	  }
	
	return  ret;
}

float  CProcess::PiexltoWindowsxf(float x,int channel)
{
	float ret=0;
	 ret= (x*1.0/vcapWH[channel][0]*vdisWH[channel][0]);
	 if(ret<0)
 	{
		ret=0;
 	}
	 else if(ret>=vdisWH[channel][0])
 	{
		ret=vdisWH[channel][0];
 	}

	  return ret;
}

float  CProcess::PiexltoWindowsyf(float y,int channel)
{
	 float ret=0;
	 ret= (y*1.0/vcapWH[channel][1]*vdisWH[channel][1]);

	  if(ret<0)
 	{
		ret=0;
 	}
	 else if(ret>=vdisWH[channel][1])
 	{
		ret=vdisWH[channel][1];
 	}
	
	return  ret;
}


int  CProcess::PiexltoWindowsxzoom(int x,int channel)
{
	int ret=0;
	 ret= cvRound(x*1.0/vcapWH[channel][0]*vdisWH[channel][0]);
	 if(ret<0)
 	{
		ret=0;
 	}
	 else if(ret>=vdisWH[channel][0])
 	{
		ret=vdisWH[channel][0];
 	}

	if(extInCtrl->ImgMmtshow[extInCtrl->SensorStat])
 	{
		ret =ret*2/3;
 	}

	if(extInCtrl->FovCtrl==5&&extInCtrl->SensorStat==0)
 	{
 		ret=ret-320;
		ret=2*ret;
 	}
	return ret;
}

int  CProcess::PiexltoWindowsyzoom(int y,int channel)
{
	 int ret=0;
	 ret= cvRound(y*1.0/vcapWH[channel][1]*vdisWH[channel][1]);

	  if(ret<0)
 	{
		ret=0;
 	}
	 else if(ret>=vdisWH[channel][1])
 	{
		ret=vdisWH[channel][1];
 	}

	  if(extInCtrl->ImgMmtshow[extInCtrl->SensorStat])
 	{
		ret =ret*2/3;
 	}

	 if(extInCtrl->FovCtrl==5&&extInCtrl->SensorStat==0)
 	{
 		ret=ret-256;
		ret=2*ret;
 	}
	return  ret;
}

int  CProcess::PiexltoWindowsxzoom_TrkRect(int x,int channel)
{
	int ret=0;

	ret= cvRound(x*1.0/vcapWH[channel][0]*vdisWH[channel][0]);
	
	if(ret<0)
	{
		ret=0;
	}
	else if(ret>=vdisWH[channel][0])
	{
		ret=vdisWH[channel][0];
	}

	//result to even
	if((ret%2)==0)
		ret = ret;
	else
		ret = ret+1;
	
	return ret;
}

int  CProcess::PiexltoWindowsyzoom_TrkRect(int y,int channel)
{
	 int ret=0;

	 ret= cvRound(y*1.0/vcapWH[channel][1]*vdisWH[channel][1]);

	if(ret<0)
 	{
		ret=0;
 	}
	 else if(ret>=vdisWH[channel][1])
 	{
		ret=vdisWH[channel][1];
 	}

	if((ret%2)==0)
		ret = ret;
	else
		ret = ret+1;

	return  ret;
}
void CProcess::OnCreate()
{
	MSGAPI_initial();
};
void CProcess::OnDestroy(){};
void CProcess::OnInit()
{
	extInCtrl->SysMode = 1;
}
void CProcess::OnConfig(){};
void CProcess::OnRun()
{
	update_param_alg();
};
void CProcess::OnStop(){};
void CProcess::Ontimer(){

	//msgdriv_event(MSGID_EXT_INPUT_VIDEOEN,NULL);
};
bool CProcess::OnPreProcess(int chId, Mat &frame)
{
	if(m_bCast){
		Uint32 curTm = OSA_getCurTimeInMsec();
		Uint32 elapse = curTm - m_castTm;

		if(elapse < 2000){
			return false;
		}
		else
		{
			m_bCast=false;
		}
	}
	return true;
}


int onece=0;

void CProcess::osd_mtd_show(TARGET tg[], bool bShow)
{
	int i;
	
	int frcolor=extInCtrl->DispColor[extInCtrl->SensorStat];
	unsigned char Alpha = (bShow) ? frcolor : 0;
	CvScalar colour=GetcvColour(Alpha);

	for(i=0;i<MAX_TARGET_NUMBER;i++)
	{
		if(tg[i].valid)
		{
			cv::Rect result;
			result.width = 32;
			result.height = 32;
			result.x = ((int)tg[i].cur_x) % VIDEO_IMAGE_WIDTH_0;
			result.y = ((int)tg[i].cur_y ) % VIDEO_IMAGE_HEIGHT_0;
			result.x = result.x - result.width/2;
			result.y = result.y - result.height/2;
			rectangle(m_display.m_imgOsd[extInCtrl->SensorStat],
				Point( result.x, result.y ),
				Point( result.x+result.width, result.y+result.height),
				colour, 1, 8);
		}
	}
}


void CProcess::DrawCross(cv::Rect rec,int fcolour ,int sensor,bool bShow /*= true*/)
{
	unsigned char colour = (bShow) ?fcolour : 0;
	Line_Param_fb lineparm;
	lineparm.x		=	rec.x;
	lineparm.y		=	rec.y;
	lineparm.width	=	rec.width;
	lineparm.height	=	rec.height;
	lineparm.frcolor	=	colour;
	if(sensor>1)
		sensor = 1;
	Drawcvcrossaim(m_display.m_imgOsd[sensor],&lineparm);
}

void CProcess::DrawAcqRect(cv::Mat frame,cv::Rect rec,int frcolor,bool bshow)
{
	int color = (bshow)?frcolor:0;
	int leftBottomx 	= rec.x;
	int leftBottomy 	= rec.y;
	int leftTopx 		= leftBottomx ;
	int leftTopy 		= leftBottomy - rec.height;
	int rightTopx 	= leftBottomx + rec.width;
	int rightTopy 		= leftTopy;
	int rightBottomx 	= rightTopx;
	int rightBottomy 	= leftBottomy;

	int cornorx = rec.width/4;
	int cornory = rec.height/4;
	
	Osd_cvPoint start;
	Osd_cvPoint end;

	//leftBottom
	start.x 	= leftBottomx;
	start.y 	= leftBottomy;
	end.x	= leftBottomx + cornorx;
	end.y 	= leftBottomy;
	DrawcvLine(frame,&start,&end,color,1);
	start.x 	= leftBottomx;
	start.y 	= leftBottomy;
	end.x	= leftBottomx;
	end.y 	= leftBottomy - cornory;
	DrawcvLine(frame,&start,&end,color,1);	
	//leftTop
	start.x 	= leftTopx;
	start.y 	= leftTopy;
	end.x	= leftTopx + cornorx;
	end.y 	= leftTopy;
	DrawcvLine(frame,&start,&end,color,1);
	start.x 	= leftTopx;
	start.y 	= leftTopy;
	end.x	= leftTopx;
	end.y 	= leftTopy + cornory;
	DrawcvLine(frame,&start,&end,color,1);	
	//rightTop
	start.x 	= rightTopx;
	start.y 	= rightTopy;
	end.x	= rightTopx - cornorx;
	end.y 	= rightTopy;
	DrawcvLine(frame,&start,&end,color,1);
	start.x 	= rightTopx;
	start.y 	= rightTopy;
	end.x	= rightTopx;
	end.y 	= rightTopy + cornory;
	DrawcvLine(frame,&start,&end,color,1);
	//rightBottom
	start.x 	= rightBottomx;
	start.y 	= rightBottomy;
	end.x	= rightBottomx - cornorx;
	end.y 	= rightBottomy;
	DrawcvLine(frame,&start,&end,color,1);
	start.x 	= rightBottomx;
	start.y 	= rightBottomy;
	end.x	= rightBottomx;
	end.y 	= rightBottomy - cornory;
	DrawcvLine(frame,&start,&end,color,1);	

	return ;
}

void CProcess::DrawRect(Mat frame,cv::Rect rec,int frcolor)
{
	int x = rec.x,y = rec.y;
	int width = rec.width;
	int height = rec.height;
	drawcvrect(frame,x,y,width,height,frcolor);
	return ;
}



int majormmtid=0;
int primajormmtid=0;

void CProcess::erassdrawmmt(TARGET tg[],bool bShow)
{
			int startx=0;
			int starty=0;
			int endx=0;
			int endy=0;
			Mat frame=m_display.m_imgOsd[extInCtrl->SensorStat];
			int i=0,j=0;
			cv::Rect result;
			short tempmmtx=0;
			short tempmmty=0;
			int tempdata=0;
			int testid=0;
			extInCtrl->Mmttargetnum=0;
			char numbuf[3];
			int frcolor=extInCtrl->DispColor[extInCtrl->SensorStat];
			unsigned char Alpha = (bShow) ? frcolor : 0;
			CvScalar colour=GetcvColour(Alpha);

			tempdata=primajormmtid;
			for(i=0;i<MAX_TARGET_NUMBER;i++)
				{

						//if(m_mtd[chId]->tg[i].valid)
						
						if((tg[primajormmtid].valid)&&(i==0))
						{
							//majormmtid=i;
							result.width = 32;
							result.height = 32;
							tempmmtx=result.x = ((int)tg[primajormmtid].cur_x) % VIDEO_IMAGE_WIDTH_0;
							tempmmty=result.y = ((int)tg[primajormmtid].cur_y ) % VIDEO_IMAGE_HEIGHT_0;


							extInCtrl->MmtPixelX=result.x;
							extInCtrl->MmtPixelY=result.y;
							extInCtrl->MmtValid=1;
							result.x = result.x - result.width/2;
							result.y = result.y - result.height/2;

							
							 startx=PiexltoWindowsx(result.x,prisensorstatus);
							 starty=PiexltoWindowsy(result.y,prisensorstatus);
							 endx=PiexltoWindowsx(result.x+result.width,prisensorstatus);
						 	 endy=PiexltoWindowsy(result.y+result.height,prisensorstatus);

							rectangle( frame,
								Point( startx, starty ),
								Point( endx, endy),
								colour, 1, 8);
							
						}
						
						else if(tg[tempdata].valid)
							{
								testid++;
								result.width = 32;
								result.height = 32;
								tempmmtx=result.x = ((int)tg[tempdata].cur_x) % VIDEO_IMAGE_WIDTH_0;
								tempmmty=result.y = ((int)tg[tempdata].cur_y ) % VIDEO_IMAGE_HEIGHT_0;

								 startx=PiexltoWindowsx(result.x,prisensorstatus);
								 starty=PiexltoWindowsy(result.y,prisensorstatus);
								line(frame, cvPoint(startx-16,starty), cvPoint(startx+16,starty), colour, 1, 8, 0 ); 
								line(frame, cvPoint(startx,starty-16), cvPoint(startx,starty+16), colour, 1, 8, 0 ); 
								//OSA_printf("******************the num  majormmtid=%d\n",majormmtid);
								sprintf(numbuf,"%d",(tempdata+MAX_TARGET_NUMBER-primajormmtid)%MAX_TARGET_NUMBER);
								putText(frame,numbuf,cvPoint(startx+14,starty+14),CV_FONT_HERSHEY_SIMPLEX,1,colour);
								
								
							}
				
				
						tempdata=(tempdata+1)%MAX_TARGET_NUMBER;

					}


}


void CProcess::drawmmt(TARGET tg[],bool bShow)
{
	int startx=0;
	int starty=0;
	int endx=0;
	int endy=0;
	Mat frame=m_display.m_imgOsd[extInCtrl->SensorStat];
	int i=0,j=0;
	cv::Rect result;
	short tempmmtx=0;
	short tempmmty=0;
	int tempdata=0;
	int testid=0;
	extInCtrl->Mmttargetnum=0;
	char numbuf[3];
	int frcolor=extInCtrl->DispColor[extInCtrl->SensorStat];
	unsigned char Alpha = (bShow) ? frcolor : 0;
	CvScalar colour=GetcvColour(Alpha);
	
	for(i=0;i<20;i++)
	{
		extInCtrl->MmtOffsetXY[i]=0;
	}
	for(i=0;i<MAX_TARGET_NUMBER;i++)
	{

		if(tg[majormmtid].valid==0)
		{
			//majormmtid++;
			majormmtid=(majormmtid+1)%MAX_TARGET_NUMBER;
		}
		if(tg[i].valid==1)
		{
			extInCtrl->Mmttargetnum++;
		}
	}

	primajormmtid=tempdata=majormmtid;
	for(i=0;i<MAX_TARGET_NUMBER;i++)
	{
		if((tg[majormmtid].valid)&&(i==0))
		{
			//majormmtid=i;
			result.width = 32;
			result.height = 32;
			tempmmtx=result.x = ((int)tg[majormmtid].cur_x) % VIDEO_IMAGE_WIDTH_0;
			tempmmty=result.y = ((int)tg[majormmtid].cur_y ) % VIDEO_IMAGE_HEIGHT_0;


			extInCtrl->MmtPixelX=result.x;
			extInCtrl->MmtPixelY=result.y;
			extInCtrl->MmtValid=1;
			
			//OSA_printf("the num  majormmtid=%d\n",majormmtid);
			result.x = result.x - result.width/2;
			result.y = result.y - result.height/2;

			 startx=PiexltoWindowsx(result.x,extInCtrl->SensorStat);
			 starty=PiexltoWindowsy(result.y,extInCtrl->SensorStat);
			 endx=PiexltoWindowsx(result.x+result.width,extInCtrl->SensorStat);
		 	 endy=PiexltoWindowsy(result.y+result.height,extInCtrl->SensorStat);


			if((((extInCtrl->AvtTrkStat == eTrk_mode_mtd)||(extInCtrl->AvtTrkStat == eTrk_mode_acq)))&&(extInCtrl->DispGrp[extInCtrl->SensorStat]<3))
			{
				rectangle( frame,
					Point( startx, starty ),
					Point( endx, endy),
					colour, 1, 8);
			}
			//OSA_printf("******************the num  majormmtid=%d x=%d y=%d w=%d h=%d\n",majormmtid,
			//	result.x,result.y,result.width,result.height);
			extInCtrl->MmtOffsetXY[j]		=	tempmmtx&0xff;
			extInCtrl->MmtOffsetXY[j+1]	=	(tempmmtx>>8)&0xff;
			extInCtrl->MmtOffsetXY[j+2]	=	tempmmty&0xff;
			extInCtrl->MmtOffsetXY[j+3]	=	(tempmmty>>8)&0xff;
		}	
		else if(tg[tempdata].valid)
		{
			testid++;
			result.width = 32;
			result.height = 32;
			tempmmtx=result.x = ((int)tg[tempdata].cur_x) % VIDEO_IMAGE_WIDTH_0;
			tempmmty=result.y = ((int)tg[tempdata].cur_y ) % VIDEO_IMAGE_HEIGHT_0;

			 startx=PiexltoWindowsx(result.x,extInCtrl->SensorStat);
			 starty=PiexltoWindowsy(result.y,extInCtrl->SensorStat);
			if((((extInCtrl->AvtTrkStat == eTrk_mode_mtd)||(extInCtrl->AvtTrkStat == eTrk_mode_acq)))&&(extInCtrl->DispGrp[extInCtrl->SensorStat]<3))
			{
				line(frame, cvPoint(startx-16,starty), cvPoint(startx+16,starty), colour, 1, 8, 0 ); 
				line(frame, cvPoint(startx,starty-16), cvPoint(startx,starty+16), colour, 1, 8, 0 ); 
				//OSA_printf("******************the num  majormmtid=%d\n",majormmtid);
				sprintf(numbuf,"%d",(tempdata+MAX_TARGET_NUMBER-majormmtid)%MAX_TARGET_NUMBER);
				putText(frame,numbuf,cvPoint(startx+14,starty+14),CV_FONT_HERSHEY_SIMPLEX,1,colour);
			}
			extInCtrl->MmtOffsetXY[j+testid*4]=tempmmtx&0xff;
			extInCtrl->MmtOffsetXY[j+1+testid*4]=(tempmmtx>>8)&0xff;
			extInCtrl->MmtOffsetXY[j+2+testid*4]=tempmmty&0xff;
			extInCtrl->MmtOffsetXY[j+3+testid*4]=(tempmmty>>8)&0xff;	
		}
		tempdata=(tempdata+1)%MAX_TARGET_NUMBER;
	}

	if(Mmtsendtime==0)
		;
	Mmtsendtime++;
	if(Mmtsendtime==1)
	{
		Mmtsendtime=0;
	}
}


void CProcess::erassdrawmmtnew(TARGETDRAW tg[],bool bShow)
{
	int startx=0;
	int starty=0;
	int endx=0;
	int endy=0;
	Mat frame=m_display.m_imgOsd[extInCtrl->SensorStat];
	int i=0,j=0;
	cv::Rect result;
	short tempmmtx=0;
	short tempmmty=0;
	int tempdata=0;
	int testid=0;
	extInCtrl->Mmttargetnum=0;
	char numbuf[3];
	int frcolor=extInCtrl->DispColor[extInCtrl->SensorStat];
	unsigned char Alpha = (bShow) ? frcolor : 0;
	CvScalar colour=GetcvColour(Alpha);

	primajormmtid;
	for(i=0;i<MAX_TARGET_NUMBER;i++)
	{
		if((tg[primajormmtid].valid)&&(i==primajormmtid))
		{	
			 startx=tg[primajormmtid].startx;//PiexltoWindowsx(result.x,prisensorstatus);
			 starty=tg[primajormmtid].starty;//PiexltoWindowsy(result.y,prisensorstatus);
			 endx=tg[primajormmtid].endx;//PiexltoWindowsx(result.x+result.width,prisensorstatus);
		 	 endy=tg[primajormmtid].endy;//PiexltoWindowsy(result.y+result.height,prisensorstatus);

			rectangle( frame,
				Point( startx, starty ),
				Point( endx, endy),
				colour, 1, 8);
			rectangle( frame,
				Point( startx-1, starty-1 ),
				Point( endx+1, endy+1),
				colour, 1, 8);
			sprintf(numbuf,"%d",primajormmtid+1);
			putText(frame,numbuf,cvPoint(startx,starty-2),CV_FONT_HERSHEY_SIMPLEX,0.8,colour);	
		}

		if((tg[i].valid)&&(i!=primajormmtid))
		{
			 startx=tg[i].startx;//PiexltoWindowsx(result.x,prisensorstatus);
			 starty=tg[i].starty;//PiexltoWindowsy(result.y,prisensorstatus);
			 endx=tg[i].endx;
			 endy=tg[i].endy;

			rectangle( frame,
			Point( startx, starty ),
			Point( endx, endy),
			colour, 1, 8);

			//OSA_printf("******************the num  majormmtid=%d\n",majormmtid);
			sprintf(numbuf,"%d",i+1);
			putText(frame,numbuf,cvPoint(startx,starty-2),CV_FONT_HERSHEY_SIMPLEX,0.8,colour);
		}
	}
}


void CProcess::drawmmtnew(TARGET tg[],bool bShow)
{
	int startx=0;
	int starty=0;
	int endx=0;
	int endy=0;
	Mat frame=m_display.m_imgOsd[extInCtrl->SensorStat];
	int i=0,j=0;
	cv::Rect result;
	short tempmmtx=0;
	short tempmmty=0;
	int tempdata=0;
	int testid=0;
	extInCtrl->Mmttargetnum=0;
	char numbuf[3];
	int frcolor=extInCtrl->DispColor[extInCtrl->SensorStat];
	unsigned char Alpha = (bShow) ? frcolor : 0;
	CvScalar colour=GetcvColour(Alpha);
	
	//memset(extInCtrl->MmtOffsetXY,0,20);
	for(i=0;i<20;i++)
	{
		extInCtrl->MmtOffsetXY[i]=0;
	}
	for(i=0;i<MAX_TARGET_NUMBER;i++)
	{

		if(tg[majormmtid].valid==0)
		{
			//majormmtid++;
			//find mmt major target;
			if(extInCtrl->MMTTempStat==3)
				majormmtid=(majormmtid+1)%MAX_TARGET_NUMBER;
			else if(extInCtrl->MMTTempStat==4)
				majormmtid=(majormmtid-1+MAX_TARGET_NUMBER)%MAX_TARGET_NUMBER;
			else
				majormmtid=(majormmtid+1)%MAX_TARGET_NUMBER;

		}
		if(tg[i].valid==1)
		{
			//valid mmt num;
			extInCtrl->Mmttargetnum++;
		}
		Mdrawbak[i].valid=0;//reset

	}
	
	primajormmtid=tempdata=majormmtid;
	for(i=0;i<MAX_TARGET_NUMBER;i++)
	{
		if((tg[majormmtid].valid)&&(i==majormmtid))
		{

			if(extInCtrl->SensorStat==0)
			{
				if(extInCtrl->FovCtrl!=5)
				{
					result.width 	= 32;
					result.height 	= 32;
				}
				else
				{
					result.width 	= 16;
					result.height 	= 16;
				}
			}
			else
			{
				result.width 	= 16;
				result.height 	= 16;
			}
			tempmmtx=result.x = ((int)tg[majormmtid].cur_x) % VIDEO_IMAGE_WIDTH_0;
			tempmmty=result.y = ((int)tg[majormmtid].cur_y ) % VIDEO_IMAGE_HEIGHT_0;

			
			//mmt track target set
			extInCtrl->MmtPixelX=result.x;
			extInCtrl->MmtPixelY=result.y;
			extInCtrl->MmtValid=1;

			
		
			result.x = result.x - result.width/2;
			result.y = result.y - result.height/2;
			
			 startx=PiexltoWindowsxzoom(result.x,extInCtrl->SensorStat);
			 starty=PiexltoWindowsyzoom(result.y,extInCtrl->SensorStat);
			 endx=PiexltoWindowsxzoom(result.x+result.width,extInCtrl->SensorStat);
		 	 endy=PiexltoWindowsyzoom(result.y+result.height,extInCtrl->SensorStat);
			 //erase param
			 Mdrawbak[i].startx=startx;
			 Mdrawbak[i].starty=starty;
			 Mdrawbak[i].endx=endx;
			 Mdrawbak[i].endy=endy;
			 Mdrawbak[i].valid=1;

			if((((extInCtrl->AvtTrkStat == eTrk_mode_mtd)||(extInCtrl->AvtTrkStat == eTrk_mode_acq)))&&(extInCtrl->DispGrp[extInCtrl->SensorStat]<=3))
			{
				rectangle( frame,
				Point( startx, starty ),
				Point( endx, endy),
				colour, 1, 8);
				Osdflag[osdindex]=1;

				rectangle( frame,
				Point( startx-1, starty-1 ),
				Point( endx+1, endy+1),
				colour, 1, 8);

				sprintf(numbuf,"%d",majormmtid+1);
				putText(frame,numbuf,cvPoint(startx,starty-2),CV_FONT_HERSHEY_SIMPLEX,0.8,colour);

			}
			//OSA_printf("******************the num  majormmtid=%d x=%d y=%d w=%d h=%d\n",majormmtid,
			//	result.x,result.y,result.width,result.height);
			tempmmtx  =PiexltoWindowsx(tempmmtx,extInCtrl->SensorStat);
			tempmmty  =PiexltoWindowsy(tempmmty,extInCtrl->SensorStat);
			extInCtrl->MmtOffsetXY[j]=tempmmtx&0xff;
			extInCtrl->MmtOffsetXY[j+1]=(tempmmtx>>8)&0xff;
			extInCtrl->MmtOffsetXY[j+2]=tempmmty&0xff;
			extInCtrl->MmtOffsetXY[j+3]=(tempmmty>>8)&0xff;
			
		}
		
		if((tg[i].valid)&&(i!=majormmtid))
		{
			testid++;
			if(extInCtrl->SensorStat==0)
			{
				if(extInCtrl->FovCtrl!=5)
				{
					result.width = 32;
					result.height = 32;
				}
				else
				{
					result.width = 16;
					result.height = 16;
				}
			}
			else
			{
				result.width = 16;
				result.height = 16;

			}
			
			tempmmtx=result.x = ((int)tg[i].cur_x) % VIDEO_IMAGE_WIDTH_0;
			tempmmty=result.y = ((int)tg[i].cur_y ) % VIDEO_IMAGE_HEIGHT_0;		

			//OSA_printf("+++++++++++++++the num  majormmtid=%d x=%d y=%d w=%d h=%d\n",majormmtid,
			//result.x,result.y,result.width,result.height);
			result.x = result.x - result.width/2;
			result.y = result.y - result.height/2;
			//OSA_printf("the num  majormmtid=%d\n",tempdata);

			startx=PiexltoWindowsxzoom(result.x,extInCtrl->SensorStat);
			starty=PiexltoWindowsyzoom(result.y,extInCtrl->SensorStat);
			endx=PiexltoWindowsxzoom(result.x+result.width,extInCtrl->SensorStat);
			endy=PiexltoWindowsyzoom(result.y+result.height,extInCtrl->SensorStat);

			Mdrawbak[i].startx=startx;
			Mdrawbak[i].starty=starty;
			Mdrawbak[i].endx=endx;
			Mdrawbak[i].endy=endy;
			Mdrawbak[i].valid=1;
			if((((extInCtrl->AvtTrkStat == eTrk_mode_mtd)||(extInCtrl->AvtTrkStat == eTrk_mode_acq)))&&(extInCtrl->DispGrp[extInCtrl->SensorStat]<=3))
			{
				//DrawCross(result.x,result.y,frcolor,bShow);
				//trkimgcross(frame,result.x,result.y,16);
				#if 1
				rectangle( frame,
				Point( startx, starty ),
				Point( endx, endy),
				colour, 1, 8);
				#endif
				//OSA_printf("******************the num  majormmtid=%d\n",majormmtid);
				sprintf(numbuf,"%d",i+1);
				putText(frame,numbuf,cvPoint(startx,starty-2),CV_FONT_HERSHEY_SIMPLEX,0.8,colour);
			}
			
			extInCtrl->MmtOffsetXY[j+testid*4]=tempmmtx&0xff;
			extInCtrl->MmtOffsetXY[j+1+testid*4]=(tempmmtx>>8)&0xff;
			extInCtrl->MmtOffsetXY[j+2+testid*4]=tempmmty&0xff;
			extInCtrl->MmtOffsetXY[j+3+testid*4]=(tempmmty>>8)&0xff;

			extInCtrl->MmtOffsetXY[j+testid*4]    =PiexltoWindowsx(extInCtrl->MmtOffsetXY[j+testid*4],extInCtrl->SensorStat);
			extInCtrl->MmtOffsetXY[j+1+testid*4]=PiexltoWindowsx(extInCtrl->MmtOffsetXY[j+1+testid*4],extInCtrl->SensorStat);
			extInCtrl->MmtOffsetXY[j+2+testid*4]=PiexltoWindowsy(extInCtrl->MmtOffsetXY[j+2+testid*4],extInCtrl->SensorStat);
			extInCtrl->MmtOffsetXY[j+3+testid*4]=PiexltoWindowsy(extInCtrl->MmtOffsetXY[j+3+testid*4],extInCtrl->SensorStat);
			//j++;
			
		}

		//mmt show
		tempmmtx=result.x = ((int)tg[i].cur_x) % VIDEO_IMAGE_WIDTH_0;
		tempmmty=result.y = ((int)tg[i].cur_y ) % VIDEO_IMAGE_HEIGHT_0;
		Mmtpos[i].x=tempmmtx-result.width/2;
		Mmtpos[i].y=tempmmty-result.height/2;
		Mmtpos[i].w=result.width;
		Mmtpos[i].h=result.height;
		Mmtpos[i].valid=tg[i].valid;

	}	

	if(Mmtsendtime==0)
		;//MSGAPI_AckSnd( AckMtdInfo);
	Mmtsendtime++;
	if(Mmtsendtime==1)
	{
		Mmtsendtime=0;
	}
	
	msgdriv_event(MSGID_EXT_INPUT_MMTSHOWUPDATE, NULL);

}



void CProcess::DrawMeanuCross(int lenx,int leny,int fcolour , bool bShow ,int centerx,int centery)
{
	int templenx=lenx;
	int templeny=leny;
	int lenw=35;
	unsigned char colour = (bShow) ?fcolour : 0;
	Osd_cvPoint start;
	Osd_cvPoint end;

	////v
	start.x=centerx-templenx;
	start.y=centery-templeny;
	end.x=centerx-templenx+lenw;
	end.y=centery-templeny;
	DrawcvLine(m_display.m_imgOsd[extInCtrl->SensorStat],&start,&end,colour,1);

	start.x=centerx+templenx-lenw;
	start.y=centery-templeny;
	end.x=centerx+templenx;
	end.y=centery-templeny;
	DrawcvLine(m_display.m_imgOsd[extInCtrl->SensorStat],&start,&end,colour,1);


	start.x=centerx-templenx;
	start.y=centery+templeny;
	end.x=centerx-templenx+lenw;
	end.y=centery+templeny;
	DrawcvLine(m_display.m_imgOsd[extInCtrl->SensorStat],&start,&end,colour,1);

	start.x=centerx+templenx-lenw;
	start.y=centery+templeny;
	end.x=centerx+templenx;
	end.y=centery+templeny;
	DrawcvLine(m_display.m_imgOsd[extInCtrl->SensorStat],&start,&end,colour,1);

	//h
	start.x=centerx-templenx;
	start.y=centery-templeny;
	end.x=centerx-templenx;
	end.y=centery-templeny+lenw;
	DrawcvLine(m_display.m_imgOsd[extInCtrl->SensorStat],&start,&end,colour,1);

	start.x=centerx+templenx;
	start.y=centery-templeny;
	end.x=centerx+templenx;
	end.y=centery-templeny+lenw;
	DrawcvLine(m_display.m_imgOsd[extInCtrl->SensorStat],&start,&end,colour,1);


	start.x=centerx-templenx;
	start.y=centery+templeny-lenw;
	end.x=centerx-templenx;
	end.y=centery+templeny;
	DrawcvLine(m_display.m_imgOsd[extInCtrl->SensorStat],&start,&end,colour,1);

	start.x=centerx+templenx;
	start.y=centery+templeny-lenw;
	end.x=centerx+templenx;
	end.y=centery+templeny;
	DrawcvLine(m_display.m_imgOsd[extInCtrl->SensorStat],&start,&end,colour,1);

}

void CProcess::DrawdashCross(int x,int y,int fcolour ,bool bShow /*= true*/)
{

	int startx=0;
	int starty=0;
	int endx=0;
	int endy=0;
	unsigned char colour = (bShow) ?fcolour : 0;
	Line_Param_fb lineparm;

	startx=WindowstoPiexlx(extInCtrl->AvtPosX[extInCtrl->SensorStat],extInCtrl->SensorStat);
	starty=WindowstoPiexly(extInCtrl->AvtPosY[extInCtrl->SensorStat],extInCtrl->SensorStat);
	
	lineparm.x=startx;
	lineparm.y=starty;
	lineparm.width=50;
	lineparm.height=50;
	lineparm.frcolor=colour;

	int dashlen=2;

	Point start,end;

	if(!bShow)
	{
		lineparm.x=secBak[1].x;
		lineparm.y=secBak[1].y;
		if(extInCtrl->SensorStat==video_gaoqing)
			DrawcvDashcross(m_dccv,&lineparm,dashlen,dashlen);
		else if(extInCtrl->SensorStat==video_pal)
			DrawcvDashcross(m_dc,&lineparm,dashlen,dashlen);
		startx=secBak[0].x;
		starty=secBak[0].y;
		endx=secBak[1].x;
		endy=secBak[1].y;
		
		drawdashlinepri(m_display.m_imgOsd[extInCtrl->SensorStat],startx,starty,endx,endy,dashlen,dashlen,colour);
	}

	else if(extInCtrl->DispGrp[extInCtrl->SensorStat]<3)
	{
		if(extInCtrl->SensorStat==video_gaoqing)
			DrawcvDashcross(m_dccv,&lineparm,dashlen,dashlen);
		else if(extInCtrl->SensorStat==video_pal)
			DrawcvDashcross(m_dc,&lineparm,dashlen,dashlen);
		startx=PiexltoWindowsxzoom(extInCtrl->AvtPosX[extInCtrl->SensorStat ],extInCtrl->SensorStat);
		starty=PiexltoWindowsyzoom(extInCtrl->AvtPosY[extInCtrl->SensorStat ],extInCtrl->SensorStat);
		endx=lineparm.x;
		endy=lineparm.y;
		if(extInCtrl->SensorStat==video_gaoqing)
			drawdashlinepri(m_dccv,startx,starty,endx,endy,dashlen,dashlen,colour);
		else
			drawdashlinepri(m_dc,startx,starty,endx,endy,dashlen,dashlen,colour);

		secBak[0].x=startx;
		secBak[0].y=starty;
		secBak[1].x=endx;
		secBak[1].y=endy;
		
		Osdflag[osdindex]=1;	
	}
}


void CProcess::DrawdashRect(int startx,int starty,int endx,int endy,int colour)
{
	int dashlen=3;
	if(extInCtrl->SensorStat==video_gaoqing)
	{
		drawdashlinepri(m_dccv,startx,starty,endx,starty,dashlen,dashlen,colour);
		drawdashlinepri(m_dccv,startx,endy,endx,endy,dashlen,dashlen,colour);
		drawdashlinepri(m_dccv,endx,starty,endx,endy,dashlen,dashlen,colour);
		drawdashlinepri(m_dccv,startx,starty,startx,endy,dashlen,dashlen,colour);
	}
	else if(extInCtrl->SensorStat==video_pal)
	{
		drawdashlinepri(m_dc,startx,starty,endx,starty,dashlen,dashlen,colour);
		drawdashlinepri(m_dc,startx,endy,endx,endy,dashlen,dashlen,colour);
		drawdashlinepri(m_dc,endx,starty,endx,endy,dashlen,dashlen,colour);
		drawdashlinepri(m_dc,startx,starty,startx,endy,dashlen,dashlen,colour);
	}
}

bool CProcess::OnProcess(int chId, Mat &frame)
{
	int frcolor= extInCtrl->osdDrawColor;
	int startx=0;
	int starty=0;
	int endx=0;
	int endy=0;
	int detect_num = 0,i;
	cv::Rect recIn;
	static int coastCnt = 1;
	static int bDraw = 0;
	//algOsdRect = 0;

	static int changesensorCnt = 0;

	if(extInCtrl->changeSensorFlag == 1)
		++changesensorCnt;
	if(changesensorCnt == 3){
		extInCtrl->changeSensorFlag =  0; 
		changesensorCnt = 0;
	}
	
	
	if(((++coastCnt)%10) == 0)
	{
		bDraw = !bDraw;	
		coastCnt = 0;
	}
	
	CvScalar colour=GetcvColour(frcolor);
	static unsigned int countnofresh = 0;
	if((countnofresh ) >= 5)
	{
		countnofresh=0;
	}
	
	countnofresh++;

	osdindex=0;	

#if __TRACK__
	osdindex++;
	{
		 UTC_RECT_float rcResult = m_rcTrack;
		 UTC_RECT_float rcResult_algRect = m_rcTrack;
	 
		 trackinfo_obj->trackrect=m_rcTrack;
		 trackinfo_obj->TrkStat = extInCtrl->AvtTrkStat;
		 extInCtrl->TrkStat = extInCtrl->AvtTrkStat;
		 m_SensorStat = extInCtrl->SensorStat;
		  
		 int aimw = extInCtrl->AimW[extInCtrl->SensorStat];
		 int aimh = extInCtrl->AimH[extInCtrl->SensorStat];

		if(changesensorCnt){
			if(extInCtrl->SensorStatpri == video_gaoqing){
				rectangle( m_dccv,
					Point( rcTrackBak[extInCtrl->SensorStatpri].x, rcTrackBak[extInCtrl->SensorStatpri].y ),
					Point( rcTrackBak[extInCtrl->SensorStatpri].x+rcTrackBak[extInCtrl->SensorStatpri].width, 
						rcTrackBak[extInCtrl->SensorStatpri].y+rcTrackBak[extInCtrl->SensorStatpri].height),
					cvScalar(0,0,0, 0), 1, 8 );
 			}
			else if(extInCtrl->SensorStatpri == video_pal){
				rectangle( m_dc,
					Point( rcTrackBak[extInCtrl->SensorStatpri].x, rcTrackBak[extInCtrl->SensorStatpri].y ),
					Point( rcTrackBak[extInCtrl->SensorStatpri].x+rcTrackBak[extInCtrl->SensorStatpri].width, 
						rcTrackBak[extInCtrl->SensorStatpri].y+rcTrackBak[extInCtrl->SensorStatpri].height),
					cvScalar(0,0,0, 0), 1, 8 );
 			}
		}

		if(Osdflag[osdindex]==1)
 		{			
				if(extInCtrl->SensorStat==video_gaoqing)
				{
					rectangle( m_dccv,
						Point( resultTrackBak.x, resultTrackBak.y ),
						Point( resultTrackBak.x+resultTrackBak.width, resultTrackBak.y+resultTrackBak.height),
						cvScalar(0,0,0,0), 1, 8 );
				}
				else if(extInCtrl->SensorStat==video_pal)
				{
					rectangle( m_dc,
						Point( resultTrackBak.x, resultTrackBak.y ),
						Point( resultTrackBak.x+resultTrackBak.width, resultTrackBak.y+resultTrackBak.height),
						cvScalar(0,0,0,0), 1, 8 );
				}
	 			if(extInCtrl->SensorStat==video_gaoqing)
	 			{
					rectangle( m_dccv,
						Point( rcTrackBak[extInCtrl->SensorStat].x, rcTrackBak[extInCtrl->SensorStat].y ),
						Point( rcTrackBak[extInCtrl->SensorStat].x+rcTrackBak[extInCtrl->SensorStat].width, 
							rcTrackBak[extInCtrl->SensorStat].y+rcTrackBak[extInCtrl->SensorStat].height),
						cvScalar(0,0,0, 0), 1, 8 );
	 			}
				else if(extInCtrl->SensorStat==video_pal)
	 			{
					rectangle( m_dc,
						Point( rcTrackBak[extInCtrl->SensorStat].x, rcTrackBak[extInCtrl->SensorStat].y ),
						Point( rcTrackBak[extInCtrl->SensorStat].x+rcTrackBak[extInCtrl->SensorStat].width, 
							rcTrackBak[extInCtrl->SensorStat].y+rcTrackBak[extInCtrl->SensorStat].height),
						cvScalar(0,0,0, 0), 1, 8 );
	 			}
			Osdflag[osdindex]=0;
		}
		 if(m_bTrack)
		 {
			extInCtrl->TrkXtmp = rcResult.x + rcResult.width/2;
			extInCtrl->TrkYtmp = rcResult.y + rcResult.height/2;
			
			if(chId == video_gaoqing)
			{
				 startx=PiexltoWindowsxzoom_TrkRect(rcResult.x+rcResult.width/2-aimw/2,extInCtrl->SensorStat);			
				 starty=PiexltoWindowsyzoom_TrkRect(rcResult.y+rcResult.height/2-aimh/2 ,extInCtrl->SensorStat);
				 endx  =PiexltoWindowsxzoom_TrkRect(rcResult.x+rcResult.width/2+aimw/2,extInCtrl->SensorStat);
			 	 endy  =PiexltoWindowsyzoom_TrkRect(rcResult.y+rcResult.height/2+aimh/2 ,extInCtrl->SensorStat);
			}
			else if(chId == video_pal)
			{
				startx=PiexltoWindowsxzoom_TrkRect(rcResult.x+rcResult.width/2-aimw/2,extInCtrl->SensorStat);			
			 	starty=PiexltoWindowsyzoom_TrkRect(rcResult.y+rcResult.height/2-aimh/2 ,extInCtrl->SensorStat);
			 	endx  =PiexltoWindowsxzoom_TrkRect(rcResult.x+rcResult.width/2+aimw/2,extInCtrl->SensorStat);
		 	 	endy  =PiexltoWindowsyzoom_TrkRect(rcResult.y+rcResult.height/2+aimh/2 ,extInCtrl->SensorStat);
			}

			if(algOsdRect == true)
			{
				rcResult_algRect.x = PiexltoWindowsx(rcResult_algRect.x,extInCtrl->SensorStat);
				rcResult_algRect.y = PiexltoWindowsy(rcResult_algRect.y,extInCtrl->SensorStat);
				rcResult_algRect.width = PiexltoWindowsx(rcResult_algRect.width,extInCtrl->SensorStat);
				rcResult_algRect.height = PiexltoWindowsy(rcResult_algRect.height,extInCtrl->SensorStat);
			}

			if( m_iTrackStat == 1 && !changesensorCnt)
			{		
				if(algOsdRect == true)
				{
					rectangle( m_display.m_imgOsd[extInCtrl->SensorStat],
						Point( rcResult_algRect.x, rcResult_algRect.y ),
						Point( rcResult_algRect.x+rcResult_algRect.width, rcResult_algRect.y+rcResult_algRect.height),
						cvScalar(0,255,0,255), 1, 8 );
				}
				else
				{
					rectangle( m_display.m_imgOsd[extInCtrl->SensorStat],
						Point( startx, starty ),
						Point( endx, endy),
						colour, 1, 8 );
				}
			}
			else
			{
				if(chId == video_gaoqing)
				{
					startx=PiexltoWindowsxzoom(extInCtrl->TrkXtmp-aimw/2,extInCtrl->SensorStat);			
					starty=PiexltoWindowsyzoom(extInCtrl->TrkYtmp-aimh/2 ,extInCtrl->SensorStat);
					endx=PiexltoWindowsxzoom(extInCtrl->TrkXtmp+aimw/2,extInCtrl->SensorStat);
					endy=PiexltoWindowsyzoom(extInCtrl->TrkYtmp+aimh/2 ,extInCtrl->SensorStat);
				}
				else if(chId == video_pal)
				{
					startx=PiexltoWindowsxzoom(extInCtrl->TrkXtmp-aimw/2,extInCtrl->SensorStat);			
					starty=PiexltoWindowsyzoom(extInCtrl->TrkYtmp-aimh/2 ,extInCtrl->SensorStat);
					endx=PiexltoWindowsxzoom(extInCtrl->TrkXtmp+aimw/2,extInCtrl->SensorStat);
					endy=PiexltoWindowsyzoom(extInCtrl->TrkYtmp+aimh/2 ,extInCtrl->SensorStat);
				}

				#if 0
				if(chId == video_gaoqing)
				{
					startx=PiexltoWindowsxzoom(extInCtrl->AxisPosX[extInCtrl->SensorStat]-aimw/2,extInCtrl->SensorStat);			
					starty=PiexltoWindowsyzoom(extInCtrl->AxisPosY[extInCtrl->SensorStat]-aimh/2 ,extInCtrl->SensorStat);
					endx=PiexltoWindowsxzoom(extInCtrl->AxisPosX[extInCtrl->SensorStat]+aimw/2,extInCtrl->SensorStat);
					endy=PiexltoWindowsyzoom(extInCtrl->AxisPosY[extInCtrl->SensorStat]+aimh/2 ,extInCtrl->SensorStat);
				}
				else if(chId == video_pal)
				{
					startx=PiexltoWindowsxzoom(extInCtrl->AxisPosX[extInCtrl->SensorStat]-aimw/2,extInCtrl->SensorStat);			
					starty=PiexltoWindowsyzoom(extInCtrl->AxisPosY[extInCtrl->SensorStat]-aimh/2 ,extInCtrl->SensorStat);
					endx=PiexltoWindowsxzoom(extInCtrl->AxisPosX[extInCtrl->SensorStat]+aimw/2,extInCtrl->SensorStat);
					endy=PiexltoWindowsyzoom(extInCtrl->AxisPosY[extInCtrl->SensorStat]+aimh/2 ,extInCtrl->SensorStat);
				}
				#endif				

				if(bDraw != 0 && !changesensorCnt)
				{
					if(algOsdRect == true)
						DrawdashRect(rcResult_algRect.x,rcResult_algRect.y,rcResult_algRect.x+rcResult_algRect.width,rcResult_algRect.y+rcResult_algRect.height,frcolor);
					else
						DrawdashRect(startx,starty,endx,endy,frcolor);	// track lost DashRect				
				}
			}
			if(!changesensorCnt){
				Osdflag[osdindex]=1;
				rcTrackBak[extInCtrl->SensorStat].x=startx;
				rcTrackBak[extInCtrl->SensorStat].y=starty;
				rcTrackBak[extInCtrl->SensorStat].width=endx-startx;
				rcTrackBak[extInCtrl->SensorStat].height=endy-starty;
			}
			if(algOsdRect == true)
			{
				resultTrackBak.x = rcResult_algRect.x;
				resultTrackBak.y = rcResult_algRect.y;
				resultTrackBak.width = rcResult_algRect.width;
				resultTrackBak.height = rcResult_algRect.height;
			}
			
			extInCtrl->unitAimX=rcResult.x+rcResult.width/2;
			extInCtrl->unitAimY=rcResult.y+rcResult.height/2;
			extInCtrl->unitAimW=rcResult.width;
			extInCtrl->unitAimH=rcResult.height;
		 }
		 
		 if(m_bTrack && !changesensorCnt)
		 {
			set_trktype(extInCtrl,m_iTrackStat);
			if(m_iTrackStat == 1)
			{
				rememflag=false;
			}
			else if(m_iTrackStat == 2)
			{
				if(!rememflag)
				{
					rememflag=true;
					rememtime=OSA_getCurTimeInMsec();
				}
				
				if((OSA_getCurTimeInMsec()-rememtime) > 3000)
				{		
					set_trktype(extInCtrl,3);
				}
				else
				{
					set_trktype(extInCtrl,2);
				}
			}
		 	 if((extInCtrl->TrkStat == 1)||(extInCtrl->TrkStat == 2))
		 	 {
				extInCtrl->TrkX =rcResult.x+rcResult.width/2;
				extInCtrl->TrkY = rcResult.y+rcResult.height/2;
						
				extInCtrl->trkerrx=(PiexltoWindowsxf(extInCtrl->TrkX ,extInCtrl->SensorStat));
				extInCtrl->trkerry=(PiexltoWindowsyf(extInCtrl->TrkY ,extInCtrl->SensorStat));
				
				if(extInCtrl->TrkStat == 2)
				{
					extInCtrl->trkerrx=(PiexltoWindowsx(extInCtrl->AxisPosX[extInCtrl->SensorStat] ,extInCtrl->SensorStat));
					extInCtrl->trkerry=(PiexltoWindowsy(extInCtrl->AxisPosY[extInCtrl->SensorStat] ,extInCtrl->SensorStat));
				}

				extInCtrl->TrkErrFeedback = 1;
		 	 }
			 else
			 	extInCtrl->TrkErrFeedback = 0;
			 
			if(extInCtrl->TrkStat!=extInCtrl->TrkStatpri)
			{
				extInCtrl->TrkStatpri=extInCtrl->TrkStat;
			}

			#if __IPC__
					if(extInCtrl->TrkStat != 3)
					{
						if(extInCtrl->SensorStat == video_gaoqing)
						{
							extInCtrl->trkerrx = extInCtrl->trkerrx - extInCtrl->opticAxisPosX[extInCtrl->SensorStat];
							extInCtrl->trkerry = extInCtrl->trkerry - extInCtrl->opticAxisPosY[extInCtrl->SensorStat];
						}
						else if(extInCtrl->SensorStat == video_pal)
						{
							extInCtrl->trkerrx = extInCtrl->trkerrx - extInCtrl->opticAxisPosX[extInCtrl->SensorStat];
							extInCtrl->trkerry = extInCtrl->trkerry - extInCtrl->opticAxisPosX[extInCtrl->SensorStat];
						}
					}
					else
					{
						extInCtrl->trkerrx = 0;
						extInCtrl->trkerry = 0;
					}
					
					ipc_settrack(extInCtrl->TrkStat, extInCtrl->trkerrx, extInCtrl->trkerry);
					trkmsg.cmd_ID = read_shm_trkpos;
					//printf("ack the trackerr to mainThr\n");
					ipc_sendmsg(&trkmsg, IPC_FRIMG_MSG);

				if(m_display.disptimeEnable == 1){
				//test zhou qi  time
				int64 disptime = 0;
				disptime = getTickCount();
				double curtime = (disptime/getTickFrequency())*1000;
				static double pretime = 0.0;
				double time = curtime - pretime;
				pretime = curtime;

				if(m_display.disptimeEnable == 1)
				{
					putText(m_display.m_imgOsd[1],trkFPSDisplay,
							Point( m_display.m_imgOsd[1].cols-350, 25),
							FONT_HERSHEY_TRIPLEX,0.8,
							cvScalar(0,0,0,0), 1
							);
					sprintf(trkFPSDisplay, "trkerr time = %0.3fFPS", 1000.0/time);
					putText(m_display.m_imgOsd[1],trkFPSDisplay,
							Point(m_display.m_imgOsd[1].cols-350, 25),
							FONT_HERSHEY_TRIPLEX,0.8,
							cvScalar(255,255,0,255), 1
							);
				}

			}
			#endif	
				
		 }
		 else
	 	{
			rememflag=false;
			extInCtrl->TrkErrFeedback = 0;
	 	}
	}
#endif

/*
	//mtd
osdindex++;
	{
		if(Osdflag[osdindex]==1)
		{
			erassdrawmmtnew(Mdrawbak, false);
			Osdflag[osdindex]=0;
		}
		if(m_bMtd && changesensorCnt)
		{
			drawmmtnew(m_mtd[chId]->tg, true);		
		}
	}
*/

osdindex++;	//cross aim
	{
		if(changesensorCnt){
			recIn.x=crossBak.x;
	 		recIn.y=crossBak.y;
			recIn.width = crossWHBak.x;
			recIn.height = crossWHBak.y;
			DrawCross(recIn,frcolor,extInCtrl->SensorStatpri,false);
		}
		
	 	if(Osdflag[osdindex]==1){
			recIn.x=crossBak.x;
	 		recIn.y=crossBak.y;
			recIn.width = crossWHBak.x;
			recIn.height = crossWHBak.y;
			DrawCross(recIn,frcolor,extInCtrl->SensorStat,false);
			Osdflag[osdindex]=0;
 		}

		if(extInCtrl->DispGrp[extInCtrl->SensorStat] <= 3  &&  !changesensorCnt)
		{
			recIn.x=PiexltoWindowsx(extInCtrl->AxisPosX[extInCtrl->SensorStat],extInCtrl->SensorStat);
	 		recIn.y=PiexltoWindowsy(extInCtrl->AxisPosY[extInCtrl->SensorStat],extInCtrl->SensorStat);
			recIn.width = extInCtrl->crossAxisWidth[extInCtrl->SensorStat];
			recIn.height= extInCtrl->crossAxisHeight[extInCtrl->SensorStat];		
			crossBak.x = recIn.x;
			crossBak.y = recIn.y;
			crossWHBak.x = recIn.width;
			crossWHBak.y = recIn.height;

			if(extInCtrl->AvtTrkStat == eTrk_mode_acq)
			{
				if(m_display.m_crossOsd)
					DrawCross(recIn,frcolor,extInCtrl->SensorStat,true);
				Osdflag[osdindex]=1;
			}
			else if(extInCtrl->AvtTrkStat == eTrk_mode_search)
			{
				frcolor = 3;
				DrawCross(recIn,frcolor,extInCtrl->SensorStat,true);
				Osdflag[osdindex]=1;
			}
		}
	}


osdindex++;	//acqRect
	{
		if(changesensorCnt){
			recIn = acqRectBak;
			if(extInCtrl->SensorStatpri>1)
				extInCtrl->SensorStatpri = 1;
			DrawAcqRect(m_display.m_imgOsd[extInCtrl->SensorStatpri],recIn,frcolor,false);
		}
		
	 	if(Osdflag[osdindex]==1){
			recIn = acqRectBak;
			if(extInCtrl->SensorStat>1)
				extInCtrl->SensorStat = 1;
			DrawAcqRect(m_display.m_imgOsd[extInCtrl->SensorStat],recIn,frcolor,false);
			Osdflag[osdindex]=0;
 		}
		if(extInCtrl->AvtTrkStat == eTrk_mode_acq  && !changesensorCnt){
			recIn.x  = PiexltoWindowsx(extInCtrl->AxisPosX[extInCtrl->SensorStat],extInCtrl->SensorStat);
	 		recIn.y  = PiexltoWindowsy(extInCtrl->AxisPosY[extInCtrl->SensorStat],extInCtrl->SensorStat);
			recIn.width  = extInCtrl->AcqRectW[extInCtrl->SensorStat];
			recIn.height = extInCtrl->AcqRectH[extInCtrl->SensorStat]; 
			if(recIn.width%2 == 1)
				recIn.width++;
			if(recIn.height%2 == 1)
				recIn.height++;
			recIn.x = recIn.x  - recIn.width/2;
			recIn.y = recIn.y  + recIn.height/2;
			if(extInCtrl->SensorStat == video_gaoqing)
				DrawAcqRect(m_display.m_imgOsd[extInCtrl->SensorStat],recIn,frcolor,true);
			else if(extInCtrl->SensorStat == video_pal)
			{
				DrawAcqRect(m_display.m_imgOsd[extInCtrl->SensorStat],recIn,frcolor,true);
			}
			acqRectBak = recIn;
			Osdflag[osdindex]=1;

		}
	}

	
#if __MOVE_DETECT__
	osdindex++;
	{
		if(Osdflag[osdindex]==1)
		{
			detect_num = detect_bak.size();
			for(i=0;i<detect_num;i++)
			{	
				DrawRect(m_display.m_imgOsd[extInCtrl->SensorStat], detect_bak[i].targetRect,0);
			}			
			Osdflag[osdindex]=0;
		}
		if(m_bMoveDetect)
		{
			detect_num = detect_vect.size();		
			DrawMoveDetect = 1;
			for(i =0;i<detect_num;i++)
			{
				DrawRect(m_display.m_imgOsd[extInCtrl->SensorStat], detect_vect[i].targetRect,2);
				random.x = detect_vect[i].targetRect.x;
				random.y = detect_vect[i].targetRect.y;
				random.h = detect_vect[i].targetRect.height;
				random.w =detect_vect[i].targetRect.width;					
			}		
			detect_bak = detect_vect;
			Osdflag[osdindex]=1;
		}
		else
			DrawMoveDetect = 0 ;
	}
#endif
	prisensorstatus=extInCtrl->SensorStat;

	static unsigned int count = 0;
	if((count & 1) == 1)
		OSA_semSignal(&(sThis->m_display.tskdisSemmain));
	count++;
	return true;
}

static inline void my_rotate(GLfloat result[16], float theta)
{
	float rads = float(theta/180.0f) * CV_PI;
	const float c = cosf(rads);
	const float s = sinf(rads);

	memset(result, 0, sizeof(GLfloat)*16);

	result[0] = c;
	result[1] = -s;
	result[4] = s;
	result[5] = c;
	result[10] = 1.0f;
	result[15] = 1.0f;
}

void CProcess::OnMouseLeftDwn(int x, int y){};
void CProcess::OnMouseLeftUp(int x, int y){};
void CProcess::OnMouseRightDwn(int x, int y){};
void CProcess::OnMouseRightUp(int x, int y){};

void CProcess::OnKeyDwn(unsigned char key)
{
	CMD_EXT *pIStuts = extInCtrl;
	CMD_EXT tmpCmd = {0};

	if(key == 'a' || key == 'A')
	{
		tmpCmd.SensorStat = (pIStuts->SensorStat + 1)%eSen_Max;
		app_ctrl_setSensor(&tmpCmd);		
	}

	if(key == 'b' || key == 'B')
	{
		//pIStuts->PicpSensorStat = (pIStuts->PicpSensorStat + 1) % (eSen_Max+1);
		if(pIStuts->PicpSensorStat==0xff)
			pIStuts->PicpSensorStat=1;
		else 
			pIStuts->PicpSensorStat=0xff;
		
		msgdriv_event(MSGID_EXT_INPUT_ENPICP, NULL);
	}

	if(key == 'c'|| key == 'C')
	{
		if(pIStuts->AvtTrkStat)
			pIStuts->AvtTrkStat = eTrk_mode_acq;
		else
			pIStuts->AvtTrkStat = eTrk_mode_target;
		msgdriv_event(MSGID_EXT_INPUT_TRACK, NULL);
	}

	if(key == 'd'|| key == 'D')
	{
	
		if(pIStuts->MmtStat[pIStuts->SensorStat])
			pIStuts->MmtStat[pIStuts->SensorStat] = eImgAlg_Disable;
		else
			pIStuts->MmtStat[pIStuts->SensorStat] = eImgAlg_Enable;
		msgdriv_event(MSGID_EXT_INPUT_ENMTD, NULL);
	}

	if (key == 'e' || key == 'E')
	{
		if(pIStuts->ImgEnhStat[pIStuts->SensorStat])
			pIStuts->ImgEnhStat[pIStuts->SensorStat] = eImgAlg_Disable;
		else
			pIStuts->ImgEnhStat[pIStuts->SensorStat] = eImgAlg_Enable;
		msgdriv_event(MSGID_EXT_INPUT_ENENHAN, NULL);
	}

	if (key == 'k' || key == 'K')
	{
		
		if(pIStuts->MtdState[pIStuts->SensorStat])
			pIStuts->MtdState[pIStuts->SensorStat] = eImgAlg_Disable;
		else
			pIStuts->MtdState[pIStuts->SensorStat] = eImgAlg_Enable;
		msgdriv_event(MSGID_EXT_MVDETECT, NULL);
		printf("pIStuts->MtdState[pIStuts->SensorStat]  = %d\n",pIStuts->MtdState[pIStuts->SensorStat] );
	}

	if (key == 't' || key == 'T')
		{
			if(pIStuts->ImgVideoTrans[pIStuts->SensorStat])
				pIStuts->ImgVideoTrans[pIStuts->SensorStat] = eImgAlg_Disable;
			else
				pIStuts->ImgVideoTrans[pIStuts->SensorStat] = eImgAlg_Enable;
			msgdriv_event(MSGID_EXT_INPUT_RST_THETA, NULL);
		}
	if (key == 'f' || key == 'F')
		{
			if(pIStuts->ImgFrezzStat[pIStuts->SensorStat])
				pIStuts->ImgFrezzStat[pIStuts->SensorStat] = eImgAlg_Disable;
			else
				pIStuts->ImgFrezzStat[pIStuts->SensorStat] = eImgAlg_Enable;
			
			msgdriv_event(MSGID_EXT_INPUT_ENFREZZ, NULL);
		}
	if (key == 'p'|| key == 'P')
		{
			
			//pIStuts->PicpPosStat=(pIStuts->PicpPosStat+1)%4;
			msgdriv_event(MSGID_EXT_INPUT_PICPCROP, NULL);
		}
	if (key == 'g'|| key == 'G')
	{
		/***************posmov**************/
		printf("before set AxisPos\n");
		tmpCmd.axisMoveStepX = 1;
		tmpCmd.axisMoveStepY = 0;
		app_ctrl_setAxisPos(&tmpCmd);

		//msgdriv_event(MSGID_EXT_INPUT_COAST, NULL);
	}

	if (key == 'h'|| key == 'H')
	{
		printf("before set AxisPos\n");
		tmpCmd.axisMoveStepX= eTrk_ref_left;
		app_ctrl_setAxisPos(&tmpCmd);
		
		//pIStuts->AvtTrkAimSize = 3;
		//app_ctrl_setAimSize(pIStuts);
	}

	if(key == 'w'|| key == 'W')
		{
			if(pIStuts->ImgMmtshow[pIStuts->SensorStat])
				pIStuts->ImgMmtshow[pIStuts->SensorStat] = eImgAlg_Disable;
			else
				pIStuts->ImgMmtshow[pIStuts->SensorStat] = eImgAlg_Enable;
			
			msgdriv_event(MSGID_EXT_INPUT_MMTSHOW, NULL);
			OSA_printf("MSGID_EXT_INPUT_MMTSHOW\n");
		}

	if (key == 'y'|| key == 'Y')
	{		
		if(moveDetectRect == true)
		{
			moveDetectRect = false;
			rectangle( m_display.m_imgOsd[extInCtrl->SensorStat],
				Point( preAcpSR.x, preAcpSR.y ),
				Point( preAcpSR.x+preAcpSR.width, preAcpSR.y+preAcpSR.height),
				cvScalar(0,0,0,0), 1, 8 );

			rectangle( m_display.m_imgOsd[extInCtrl->SensorStat],
				Point( preWarnRect.x, preWarnRect.y ),
				Point( preWarnRect.x+preWarnRect.width, preWarnRect.y+preWarnRect.height),
				cvScalar(0,0,0,0), 2, 8 );
		}
		else
			moveDetectRect = true;
		OSA_printf("moveDetectRect = %d\n",moveDetectRect);
	}

	
	if (key == 'z'|| key == 'Z')
	{
		
		pIStuts->ImgZoomStat[0]=(pIStuts->ImgZoomStat[0]+1)%2;
		pIStuts->ImgZoomStat[1]=(pIStuts->ImgZoomStat[1]+1)%2;
		msgdriv_event(MSGID_EXT_INPUT_ENZOOM, NULL);
	}

	
}


void CProcess::msgdriv_event(MSG_PROC_ID msgId, void *prm)
{
	int tempvalue=0;
	CMD_EXT *pIStuts = extInCtrl;
	CMD_EXT *pInCmd = NULL;
	CMD_EXT tmpCmd = {0};
	if(msgId == MSGID_EXT_INPUT_SENSOR || msgId == MSGID_EXT_INPUT_ENPICP)
	{
		if(prm != NULL)
		{
			pInCmd = (CMD_EXT *)prm;
			pIStuts->SensorStat = pInCmd->SensorStat;
			pIStuts->PicpSensorStat = pInCmd->PicpSensorStat;
		}
		int itmp;
		//chage acq;
		m_rcAcq.width		=	pIStuts->AimW[pIStuts->SensorStat];
		m_rcAcq.height	=	pIStuts->AimH[pIStuts->SensorStat];

		m_rcAcq.x=pIStuts->opticAxisPosX[pIStuts->SensorStat]-m_rcAcq.width/2;
		m_rcAcq.y=pIStuts->opticAxisPosY[pIStuts->SensorStat]-m_rcAcq.height/2;

		OSA_printf("recv   the rctrack x=%f y=%f w=%f h=%f  sensor=%d picpsensor=%d\n",m_rcAcq.x,m_rcAcq.y,
			m_rcAcq.width,m_rcAcq.height,pIStuts->SensorStat,pIStuts->PicpSensorStat);
		
		itmp = pIStuts->SensorStat;
		dynamic_config(VP_CFG_MainChId, itmp, NULL);

#if 1//change the sensor picp change too
		if((pIStuts->PicpSensorStat>=eSen_CH0)&&(pIStuts->PicpSensorStat<eSen_Max))
		{
			for(int chi=eSen_CH0;chi<eSen_Max;chi++)
			{
				if(pIStuts->ImgPicp[chi]==1)
					pIStuts->PicpSensorStatpri=pIStuts->PicpSensorStat;
			}
			
		}
#endif

		itmp = pIStuts->PicpSensorStat;//freeze change
		dynamic_config(VP_CFG_SubChId, itmp, NULL);

//sensor 1 rect

		DS_Rect lay_rect;
			lay_rect.w = vcapWH[0][0]/3;
			lay_rect.h = vcapWH[0][1]/3;
			lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] -lay_rect.w/2;
			lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] -lay_rect.h/2;

		if(pIStuts->ImgZoomStat[pIStuts->SensorStat] == 2){
			lay_rect.w = vcapWH[0][0]/12;
			lay_rect.h = vcapWH[0][1]/12;
			lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] -lay_rect.w/2;
			lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] -lay_rect.h/2;
		}
		else if(pIStuts->ImgZoomStat[pIStuts->SensorStat] == 4){
			lay_rect.w = vcapWH[0][0]/18;
			lay_rect.h = vcapWH[0][1]/18;
			lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] -lay_rect.w/2;
			lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] -lay_rect.h/2;
		}
		else if(pIStuts->ImgZoomStat[pIStuts->SensorStat] == 6){
			lay_rect.w = vcapWH[0][0]/24;
			lay_rect.h = vcapWH[0][1]/24;
			lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] -lay_rect.w/2;
			lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] -lay_rect.h/2;
		}
		else if(pIStuts->ImgZoomStat[pIStuts->SensorStat] == 8){
			lay_rect.w = vcapWH[0][0]/30;
			lay_rect.h = vcapWH[0][1]/30;
			lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] - lay_rect.w/2;
			lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] - lay_rect.h/2;
		}

		m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 1, &lay_rect);

//picp position
		lay_rect=rendpos[pIStuts->PicpPosStat];
		
		lay_rect.w = VIDEO_IMAGE_WIDTH_1/3;
		lay_rect.h =VIDEO_IMAGE_HEIGHT_1/3;
		lay_rect.x = VIDEO_IMAGE_WIDTH_1*2/3;
		lay_rect.y = VIDEO_IMAGE_HEIGHT_1*2/3;
		m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 1, &lay_rect);

///sensor zoom

		if(0)//(pIStuts->ImgZoomStat[pIStuts->SensorStat])
		{
			/*
			memset(&lay_rect, 0, sizeof(DS_Rect));
			if(pIStuts->SensorStat==0)//just tv zooom
			{
				lay_rect.w = vcapWH[pIStuts->SensorStat][0]/2;
				lay_rect.h = vcapWH[pIStuts->SensorStat][1]/2;
				lay_rect.x = vcapWH[pIStuts->SensorStat][0]/4;
				lay_rect.y = vcapWH[pIStuts->SensorStat][1]/4;
			}
			*/
			
			//m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 0, &lay_rect);
			if(pIStuts->PicpSensorStat==1)
			{
				lay_rect.w = vcapWH[1][0]/6;
				lay_rect.h = vcapWH[1][1]/6;
				lay_rect.x = vcapWH[1][0]/2-lay_rect.w/2;
				lay_rect.y = vcapWH[1][1]/2-lay_rect.h/2;
				m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 1, &lay_rect);
			}
			if(pIStuts->PicpSensorStat==0)
			{	
				lay_rect.w = vcapWH[0][0]/6;
				lay_rect.h = vcapWH[0][1]/6;
				lay_rect.x = vcapWH[0][0]/2-lay_rect.w/2;
				lay_rect.y = vcapWH[0][1]/2-lay_rect.h/2;
				m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 1, &lay_rect);
			}
		}


//mmt show change
	if(pIStuts->ImgMmtshow[pIStuts->SensorStat^1]==0x01)
		{
			
			int mmtchid=0;
			int chid=pIStuts->SensorStat;
			pIStuts->ImgMmtshow[pIStuts->SensorStat^1]=0;
			pIStuts->ImgMmtshow[pIStuts->SensorStat]=1;
			itmp = chid;
			mmtchid=2;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			//chid++;
			itmp=chid;
			mmtchid=3;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			//chid++;
			itmp=chid;
			mmtchid=4;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			//chid++;
			itmp=chid;
			mmtchid=5;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			//chid++;
			itmp=chid;
			mmtchid=6;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			lay_rect.w = vdisWH[0][0]/3*2;
			lay_rect.h = vdisWH[0][1]/3*2;
			lay_rect.x = 0;
			lay_rect.y = vdisWH[0][1]/3;
			m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 0, &lay_rect);
			//m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 7, NULL);
			



		}

		
		
	
	}

	if(msgId == MSGID_EXT_INPUT_TRACK)
	{
		if(prm != NULL)
		{
			pInCmd = (CMD_EXT *)prm;
			pIStuts->AvtTrkStat = pInCmd->AvtTrkStat;
		}
		char procStr[][10] = {"ACQ", "TARGET", "MTD", "SECTRK", "SEARCH", "ROAM", "SCENE", "IMGTRK"};
		UTC_RECT_float rc;
		if (pIStuts->AvtTrkStat == eTrk_mode_acq)
		{
			OSA_printf(" %d:%s set track to [%s]\n", OSA_getCurTimeInMsec(), __func__,
					   procStr[pIStuts->AvtTrkStat]);

			dynamic_config(VP_CFG_TrkEnable, 0);
			pIStuts->AvtPosX[extInCtrl->SensorStat] = pIStuts->AxisPosX[extInCtrl->SensorStat];
			pIStuts->AvtPosY[extInCtrl->SensorStat] = pIStuts->AxisPosY[extInCtrl->SensorStat];			
			if(DrawMoveDetect)
				pIStuts->unitAimX =  random.x+random.w/2;
			else
				pIStuts->unitAimX = pIStuts->AvtPosX[extInCtrl->SensorStat] ;
			if(pIStuts->unitAimX < 0)
			{
				pIStuts->unitAimX = 0;
			}
			if(DrawMoveDetect)
				pIStuts->unitAimY = random.y+random.h/2;
			else
				pIStuts->unitAimY = pIStuts->AvtPosY[extInCtrl->SensorStat];

			if(pIStuts->unitAimY<0)
			{
				pIStuts->unitAimY=0;
			}
			rc.width	= pIStuts->AimW[pIStuts->SensorStat];
			rc.height	= pIStuts->AimH[pIStuts->SensorStat];
			rc.x=pIStuts->unitAimX-rc.width/2;
			rc.y=pIStuts->unitAimY-rc.height/2;
			dynamic_config(VP_CFG_TrkEnable, 0,&rc);
			return ;
		}

		if (pIStuts->AvtTrkStat == eTrk_mode_sectrk)
		{
			OSA_printf(" %d:%s line:%d set track to [%s]\n", OSA_getCurTimeInMsec(), __func__,
					   __LINE__,procStr[pIStuts->AvtTrkStat]);
			pIStuts->unitAimX = pIStuts->AvtPosX[extInCtrl->SensorStat];
			pIStuts->unitAimY = pIStuts->AvtPosY[extInCtrl->SensorStat] ;
		}
		else if (pIStuts->AvtTrkStat == eTrk_mode_search)
		{
			OSA_printf(" %d:%s line:%d set track to [%s]\n", OSA_getCurTimeInMsec(), __func__,
					   __LINE__,procStr[pIStuts->AvtTrkStat]);

		  	//pIStuts->AvtTrkStat = eTrk_mode_search;
		 	pIStuts->unitAimX = pIStuts->AvtPosX[extInCtrl->SensorStat];
		   	pIStuts->unitAimY = pIStuts->AvtPosY[extInCtrl->SensorStat] ;
		}
		else if (pIStuts->AvtTrkStat == eTrk_mode_mtd)
		{
			pIStuts->unitAimX = pIStuts->AvtPosX[extInCtrl->SensorStat];
		   	pIStuts->unitAimY = pIStuts->AvtPosY[extInCtrl->SensorStat] ;
			dynamic_config(VP_CFG_TrkEnable, 0,NULL);
			return ;
			
			pIStuts->AvtTrkStat = eTrk_mode_target;

			//zoom for mtdTrk change xy 	
			pIStuts->unitAimX=pIStuts->MmtPixelX;
			pIStuts->unitAimY=pIStuts->MmtPixelY;
		 	
			if(pIStuts->MmtValid)
			{
				tempvalue=pIStuts->MmtPixelX;
				
				if(tempvalue<0)
					{
						pIStuts->unitAimX=0;
					}
				else
					{
						pIStuts->unitAimX=tempvalue;

					}
				tempvalue=pIStuts->MmtPixelY ;
				//- pIStuts->unitAimH/2;
				if(tempvalue<0)
					{
						pIStuts->unitAimY=0;
					}
				else
					{
						pIStuts->unitAimY=tempvalue;

					}
				
				OSA_printf(" %d:%s set track to x =%f y=%f  mtdx=%d mtdy=%d  w=%d  h=%d\n", OSA_getCurTimeInMsec(), __func__,
						pIStuts->unitAimX,pIStuts->unitAimY, pIStuts->MmtPixelX,pIStuts->MmtPixelY,pIStuts->unitAimW/2,pIStuts->unitAimH/2);
			}
			else
			{
				pIStuts->unitAimX = pIStuts->opticAxisPosX[extInCtrl->SensorStat ] - pIStuts->unitAimW/2;
				pIStuts->unitAimY = pIStuts->opticAxisPosY[extInCtrl->SensorStat ] - pIStuts->unitAimH/2;
			}
		}

		//printf("%s,line:%d   aimx,aimy=(%d,%d)\n",__func__,__LINE__,pIStuts->AvtPosX[0],pIStuts->AvtPosY[0]);
		if(m_curChId== video_gaoqing)
		{
			rc.width= pIStuts->AimW[pIStuts->SensorStat];
			rc.height=pIStuts->AimW[pIStuts->SensorStat];
			pIStuts->unitAimX = pIStuts->AvtPosX[pIStuts->SensorStat];
			pIStuts->unitAimY = pIStuts->AvtPosY[pIStuts->SensorStat];
			printf("AvtPosX[%d] , AvtPosY[%d] (%d,%d) \n",pIStuts->SensorStat,
				pIStuts->SensorStat,pIStuts->AvtPosX[pIStuts->SensorStat],
				pIStuts->AvtPosY[pIStuts->SensorStat]);

		}
		else if(m_curChId == video_pal)
		{
			rc.width= pIStuts->AcqRectW[pIStuts->SensorStat];
			rc.height=pIStuts->AcqRectH[pIStuts->SensorStat];
			pIStuts->unitAimX = pIStuts->AvtPosX[pIStuts->SensorStat];
			pIStuts->unitAimY = pIStuts->AvtPosY[pIStuts->SensorStat];
		}
		if(pIStuts->AvtTrkStat == eTrk_mode_sectrk || pIStuts->AvtTrkStat ==eTrk_mode_acqmove){
			pIStuts->unitAimX = pIStuts->AvtPosX[pIStuts->SensorStat];
			pIStuts->unitAimY = pIStuts->AvtPosY[pIStuts->SensorStat];
			printf("set set set   x  , y (%d , %d ) \n",pIStuts->unitAimX,pIStuts->unitAimY);
		}

		rc.x=pIStuts->unitAimX-rc.width/2;
		rc.y=pIStuts->unitAimY-rc.height/2;
		
			
		OSA_printf("%s,line:%d   rc. xy(%f,%f),wh(%f,%f)\n",__func__,__LINE__,rc.x,rc.y,rc.width,rc.height);
		dynamic_config(VP_CFG_TrkEnable, 1,&rc);
		if(pIStuts->AvtTrkStat == eTrk_mode_sectrk)
		{
			m_intervalFrame=2;
			m_rcAcq=rc;
			pIStuts->AvtTrkStat = eTrk_mode_target;
			OSA_printf("%s  line:%d		set sec track\n ",__func__,__LINE__);	
		}		
 	}

	if(msgId == MSGID_EXT_INPUT_ENMTD)
	{
		if(prm != NULL)
		{
			pInCmd = (CMD_EXT *)prm;
			pIStuts->MmtStat[0] = pInCmd->MmtStat[0];
			pIStuts->MmtStat[1] = pInCmd->MmtStat[1];
		}
		int MMTStatus = (pIStuts->MmtStat[pIStuts->SensorStat]&0x01) ;
		if(MMTStatus)
			dynamic_config(VP_CFG_MmtEnable, 1);
		else
			dynamic_config(VP_CFG_MmtEnable, 0);
		//FOR DUMP FRAME
		if(MMTStatus)
			dynamic_config(CDisplayer::DS_CFG_MMTEnable, pIStuts->SensorStat, &MMTStatus);
		else
			dynamic_config(CDisplayer::DS_CFG_MMTEnable, pIStuts->SensorStat, &MMTStatus);
	}

	if(msgId == MSGID_EXT_INPUT_ENENHAN)
	{
		if(prm != NULL)
		{
			pInCmd = (CMD_EXT *)prm;
			pIStuts->ImgEnhStat[0] = pInCmd->ImgEnhStat[0];
			pIStuts->ImgEnhStat[1] = pInCmd->ImgEnhStat[1];
		}
		int ENHStatus = (pIStuts->ImgEnhStat[pIStuts->SensorStat]&0x01) ;
		OSA_printf(" %d:%s set mtd enMask %d\n", OSA_getCurTimeInMsec(),__func__,ENHStatus);
		if(ENHStatus)
			dynamic_config(CDisplayer::DS_CFG_EnhEnable, pIStuts->SensorStat, &ENHStatus);
		else
			dynamic_config(CDisplayer::DS_CFG_EnhEnable, pIStuts->SensorStat, &ENHStatus);
	}


	if(msgId == MSGID_EXT_INPUT_AIMPOS || msgId == MSGID_EXT_INPUT_AIMSIZE)
	{
		if(prm != NULL)
		{
			pInCmd = (CMD_EXT *)prm;
			pIStuts->AvtTrkAimSize = pInCmd->AvtTrkAimSize;
			pIStuts->aimRectMoveStepX = pInCmd->aimRectMoveStepX;
			pIStuts->aimRectMoveStepY= pInCmd->aimRectMoveStepY;
		}
		
		if(pIStuts->AvtTrkStat)
		{
			UTC_RECT_float rc;
			if(msgId == MSGID_EXT_INPUT_AIMSIZE)
			{
				pIStuts->unitAimW  =  pIStuts->AimW[pIStuts->SensorStat];
				pIStuts->unitAimH	  =  pIStuts->AimW[pIStuts->SensorStat];

				rc.x	=	pIStuts->unitAimX-pIStuts->unitAimW/2;
				rc.y	=	pIStuts->unitAimY-pIStuts->unitAimH/2;
				rc.width=pIStuts->unitAimW;
				rc.height=pIStuts->unitAimH;
				//OSA_printf("***xy = (%f,%f)  WH(%f,%f)\n",rc.x,rc.y,rc.width,rc.height);
			}
			else
			{
				moveStat = true;
				//printf("----- XY(%d,%d),WH(%d,%d)\n",pIStuts->unitAimX,pIStuts->unitAimY,pIStuts->unitAimW,pIStuts->unitAimH);
				
				printf("111W,H : (%d,%d)\n",pIStuts->unitAimW,pIStuts->unitAimH);
				rc.width=pIStuts->unitAimW;
				rc.height=pIStuts->unitAimH;
				printf("222rc.width,rc.height : (%f,%f)\n",rc.width,rc.height);
				
				rc.x = pIStuts->unitAimX-pIStuts->unitAimW/2 + pIStuts->aimRectMoveStepX;
				rc.y = pIStuts->unitAimY-pIStuts->unitAimH/2  + pIStuts->aimRectMoveStepY;
				printf("333rc.x,rc.y : (%d,%d)\n",rc.x,rc.y);

				pIStuts->aimRectMoveStepX = 0;
				pIStuts->aimRectMoveStepY = 0;
				
			}
			m_intervalFrame=1;
			m_rcAcq=rc;
			OSA_printf(" %d:%s refine move (%d, %d), wh(%f, %f)  aim(%d,%d) rc(%f,%f)\n", OSA_getCurTimeInMsec(), __func__,
						pIStuts->aimRectMoveStepX, pIStuts->aimRectMoveStepY, 
						rc.width, rc.height,pIStuts->unitAimX,pIStuts->unitAimY,rc.x,rc.y);
		}

		return ;
	}

	if(msgId == MSGID_EXT_INPUT_ENZOOM)
	{
		if(prm != NULL)
		{
			pInCmd = (CMD_EXT *)prm;
			pIStuts->ImgZoomStat[0] = pInCmd->ImgZoomStat[0];
			pIStuts->ImgZoomStat[1] = pInCmd->ImgZoomStat[1];
		}

		DS_Rect lay_rect;
		
		if(pIStuts->SensorStat==0)//tv
		{
			memset(&lay_rect, 0, sizeof(DS_Rect));
			if(pIStuts->ImgZoomStat[0] == 2)
			{
				lay_rect.w = vdisWH[0][0]/2;
				lay_rect.h = vdisWH[0][1]/2;
				lay_rect.x = vdisWH[0][0]/4;
				lay_rect.y = vdisWH[0][1]/4;
			}
			else if(pIStuts->ImgZoomStat[0] == 4)
			{
				lay_rect.w = vdisWH[0][0]/4;
				lay_rect.h = vdisWH[0][1]/4;
				lay_rect.x = vdisWH[0][0]/2 - lay_rect.w/2;
				lay_rect.y = vdisWH[0][1]/2 - lay_rect.h/2;
			}
			else if(pIStuts->ImgZoomStat[0] == 8)
			{
				lay_rect.w = vdisWH[0][0]/8;
				lay_rect.h = vdisWH[0][1]/8;
				lay_rect.x = vdisWH[0][0]/2 - lay_rect.w/2;
				lay_rect.y = vdisWH[0][1]/2 - lay_rect.h/2;
			}
			else
			{
				lay_rect.w = vdisWH[0][0];
				lay_rect.h = vdisWH[0][1];
				lay_rect.x = 0;
				lay_rect.y = 0;
			}		
			
			m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 0, &lay_rect);
			memset(&lay_rect, 0, sizeof(DS_Rect));
			
			lay_rect.w = vcapWH[0][0]/6;
			lay_rect.h = vcapWH[0][1]/6;
			lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] - lay_rect.w/2;
			lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] - lay_rect.h/2;
			
			if(pIStuts->ImgZoomStat[pIStuts->SensorStat] == 2){
				lay_rect.w =vdisWH[0][0]/12;
				lay_rect.h = vdisWH[0][1]/12;
				lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] - lay_rect.w/2;
				lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] - lay_rect.h/2;
			}
			if(pIStuts->ImgZoomStat[pIStuts->SensorStat] == 4){
				lay_rect.w = vcapWH[0][0]/24;
				lay_rect.h = vcapWH[0][1]/24;
				lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] -lay_rect.w/2;
				lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] -lay_rect.h/2;
			}
			else if(pIStuts->ImgZoomStat[pIStuts->SensorStat] == 6){
				lay_rect.w = vcapWH[0][0]/48;
				lay_rect.h = vcapWH[0][1]/48;
				lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] -lay_rect.w/2;
				lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] -lay_rect.h/2;
			}
			else if(pIStuts->ImgZoomStat[pIStuts->SensorStat] == 8){
				lay_rect.w = vcapWH[0][0]/64;
				lay_rect.h = vcapWH[0][1]/64;
				lay_rect.x = pIStuts->AxisPosX[pIStuts->SensorStat] - lay_rect.w/2;
				lay_rect.y = pIStuts->AxisPosY[pIStuts->SensorStat] - lay_rect.h/2;
			}
			
			
			m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 1, &lay_rect);			
		}
		else
		{
			memset(&lay_rect, 0, sizeof(DS_Rect));
			memset(&lay_rect, 0, sizeof(DS_Rect));
			if(pIStuts->ImgZoomStat[0]&&(pIStuts->PicpSensorStat==0))
			{
				lay_rect.w = vcapWH[0][0]/6;
				lay_rect.h = vcapWH[0][1]/6;
				lay_rect.x = vcapWH[0][0]/2-lay_rect.w/2;
				lay_rect.y = vcapWH[0][1]/2-lay_rect.h/2;
			}
			else 
			{
				lay_rect.w = vcapWH[0][0]/3;
				lay_rect.h = vcapWH[0][1]/3;
				lay_rect.x = vcapWH[0][0]/2-lay_rect.w/2;
				lay_rect.y = vcapWH[0][1]/2-lay_rect.h/2;		
			}
			m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 1, &lay_rect);
		}

		return ;
	}
	
	if(msgId ==MSGID_EXT_INPUT_PICPCROP)
	{		
		m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 1, &rendpos[pIStuts->PicpPosStat]);
	}

	if(msgId ==MSGID_EXT_INPUT_VIDEOEN)
	{
		int status=pIStuts->unitFaultStat&0x01;
		status^=1;
		m_display.dynamic_config(CDisplayer::DS_CFG_VideodetEnable, 0, &status);
		OSA_printf("MSGID_EXT_INPUT_VIDEOEN status0=%d\n",status);
		 status=(pIStuts->unitFaultStat>1)&0x01;
		 status^=1;
		m_display.dynamic_config(CDisplayer::DS_CFG_VideodetEnable, 1, &status);
		OSA_printf("MSGID_EXT_INPUT_VIDEOEN status1=%d\n",status);
	}
	if(msgId ==MSGID_EXT_INPUT_MMTSHOW)
	{
		int itmp=0;
		int mmtchid=0;
		DS_Rect lay_rect;
		if(pIStuts->ImgMmtshow[pIStuts->SensorStat])
		{
			int chid=pIStuts->SensorStat;
			itmp = chid;
			mmtchid=2;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			//chid++;
			itmp=chid;
			mmtchid=3;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			//chid++;
			itmp=chid;
			mmtchid=4;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			//chid++;
			itmp=chid;
			mmtchid=5;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			//chid++;
			itmp=chid;
			mmtchid=6;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			lay_rect.w = vdisWH[0][0]/3*2;
			lay_rect.h = vdisWH[0][1]/3*2;
			lay_rect.x = 0;
			lay_rect.y = vdisWH[0][1]/3;
			m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 0, &lay_rect);
			//m_display.dynamic_config(CDisplayer::DS_CFG_Rendercount, 7, NULL);

			//m_display.m_renderCount
		}
		else
		{
			itmp = 8;
			mmtchid=2;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			itmp=8;
			mmtchid=3;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			itmp=8;
			mmtchid=4;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			itmp=8;
			mmtchid=5;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			itmp=8;
			mmtchid=6;
			dynamic_config(VP_CFG_SubPicpChId, itmp, &mmtchid);
			lay_rect.w = vdisWH[0][0];
			lay_rect.h = vdisWH[0][1];
			lay_rect.x = 0;
			lay_rect.y = 0;
			m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 0, &lay_rect);
			//m_display.dynamic_config(CDisplayer::DS_CFG_Rendercount, 2, NULL);
		}
		
	#if 1	
	lay_rect.w = 30;
	lay_rect.h = 30;
	lay_rect.x = 0;
	lay_rect.y = 0;
	m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 2, &lay_rect);
	m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 3, &lay_rect);
	m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 4, &lay_rect);
	m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 5, &lay_rect);
	m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, 6, &lay_rect);
	lay_rect.w = vdisWH[0][0]/3;
	lay_rect.h = vdisWH[0][1]/3;
	lay_rect.x = 0;
	lay_rect.y = 0;
	m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 2, &lay_rect);
	lay_rect.w = vdisWH[0][0]/3;
	lay_rect.h = vdisWH[0][1]/3;
	lay_rect.x = vdisWH[0][0]/3;
	lay_rect.y = 0;
	m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 3, &lay_rect);
	lay_rect.w = vdisWH[0][0]/3;
	lay_rect.h = vdisWH[0][1]/3;
	lay_rect.x = vdisWH[0][0]/3;
	lay_rect.x=lay_rect.x*2;
	lay_rect.y = 0;
	m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 4, &lay_rect);
	lay_rect.w = vdisWH[0][0]/3;
	lay_rect.h = vdisWH[0][1]/3;
	lay_rect.x = vdisWH[0][0]/3;
	lay_rect.x=lay_rect.x*2;
	lay_rect.y = vdisWH[0][1]/3;
	m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 5, &lay_rect);
	lay_rect.w = vdisWH[0][0]/3;
	lay_rect.h = vdisWH[0][1]/3;
	lay_rect.x = vdisWH[0][0]/3;
	lay_rect.x=lay_rect.x*2;
	lay_rect.y = vdisWH[0][1]/3;
	lay_rect.y=lay_rect.y*2;
	m_display.dynamic_config(CDisplayer::DS_CFG_RenderPosRect, 6, &lay_rect);
	#endif

	}

	if(msgId ==MSGID_EXT_INPUT_MMTSHOWUPDATE)
	{	
		for(int i=0;i<5;i++)
		{
			if(Mmtpos[i].valid)
			{
				//m_display.m_renders[i+2].videodect=1;
				m_display.dynamic_config(CDisplayer::DS_CFG_VideodetEnable, i+2, &Mmtpos[i].valid);
				m_display.dynamic_config(CDisplayer::DS_CFG_CropRect, i+2, &Mmtpos[i]);
			}
			else
			{
				//m_display.m_renders[i+2].videodect=0;
				//OSA_printf("the id=%d valid =%d\n",i+2,Mmtpos[i].valid);
				m_display.dynamic_config(CDisplayer::DS_CFG_VideodetEnable, i+2, &Mmtpos[i].valid);	
			}
		}
	}

	if(msgId == MSGID_EXT_MVDETECT)
	{	
		int Mtdstatus = (pIStuts->MtdState[pIStuts->validChId]&0x01) ;
		if(Mtdstatus)
		{
			dynamic_config(VP_CFG_MvDetect, 1,NULL);
			tmpCmd.MtdState[pIStuts->SensorStat] = 1;
			app_ctrl_setMtdStat(&tmpCmd);
		}
		else
		{
			dynamic_config(VP_CFG_MvDetect, 0,NULL);
			tmpCmd.MtdState[pIStuts->SensorStat] = 0;
			app_ctrl_setMtdStat(&tmpCmd);
		}
	}
	if(msgId == MSGID_EXT_INPUT_ALGOSDRECT)
	{
	 	algOsdRect=pIStuts->Imgalgosdrect;
	}
	
}


/////////////////////////////////////////////////////
//int majormmtid=0;

 int  CProcess::MSGAPI_initial()
{
   MSGDRIV_Handle handle=&g_MsgDrvObj;
    assert(handle != NULL);
    memset(handle->msgTab, 0, sizeof(MSGTAB_Class) * MAX_MSG_NUM);
//MSGID_EXT_INPUT_MTD_SELECT
    MSGDRIV_attachMsgFun(handle,    MSGID_SYS_INIT,           				MSGAPI_init_device,       		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_SENSOR,           	   	MSGAPI_inputsensor,       		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_PICPCROP,      		MSGAPI_croppicp,       		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_TRACK,          		MSGAPI_inputtrack,     		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_ENMTD,                   MSGAPI_inpumtd,       		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_MTD_SELECT,     	MSGAPI_inpumtdSelect,    		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_AIMPOS,          	 	MSGAPI_setAimRefine,    		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_AIMSIZE,          	       MSGAPI_setAimSize,    		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_ENENHAN,           	MSGAPI_inpuenhance,       	            0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_ENBDT,           		MSGAPI_inputbdt,         		    0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_ENZOOM,           	MSGAPI_inputzoom,                     0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_ENFREZZ,           	MSGAPI_inputfrezz,                      0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_MTD_SELECT,      	MSGAPI_inputmmtselect,              0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_AXISPOS,     	  	MSGAPI_inputpositon,                   0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_COAST,             	MSGAPI_inputcoast,                      0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_FOVSELECT,             MSGAPI_inputfovselect,                 0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_FOVSTAT,                	MSGAPI_inputfovchange,               0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_SEARCHMOD,            MSGAPI_inputsearchmod,              0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_VIDEOEN,            	MSGAPI_inputvideotect,                 0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_MMTSHOW,             	MSGAPI_mmtshow,                 	     0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_FOVCMD,             	MSGAPI_FOVcmd,                 	     0);
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_CFGSAVE,             	MSGAPI_SaveCfgcmd,                 	     0);	
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_MVDETECT,             		MSGAPI_setMtdState,                 	     0);	

    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_UPDATE_ALG,             		MSGAPI_update_alg,                 	     0);	
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_UPDATE_OSD,             		MSGAPI_update_osd,                 	0);	
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_UPDATE_CAMERA,             	MSGAPI_update_camera,              0);	
    MSGDRIV_attachMsgFun(handle,    MSGID_EXT_INPUT_ALGOSDRECT,          MSGAPI_input_algosdrect,              0);	
 
    return 0;
}


void CProcess::MSGAPI_init_device(long lParam )
{
	sThis->msgdriv_event(MSGID_SYS_INIT,NULL);
}

  void CProcess::MSGAPI_inputsensor(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;	
	sThis->msgdriv_event(MSGID_EXT_INPUT_SENSOR,NULL);
}

void CProcess::MSGAPI_picp(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;
		if(pIStuts->PicpSensorStat == 0xFF)
			pIStuts->PicpSensorStat = (pIStuts->SensorStat + 1)%eSen_Max;
		else
			pIStuts->PicpSensorStat = 0xFF;
	
	sThis->msgdriv_event(MSGID_EXT_INPUT_ENPICP,NULL);
}


void CProcess::MSGAPI_croppicp(long lParam )
{
	//sThis->msgdriv_event(MSGID_EXT_INPUT_PICPCROP,NULL);
	sThis->msgdriv_event(MSGID_EXT_INPUT_ENPICP,NULL);
}

void CProcess::MSGAPI_inputtrack(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;
	sThis->msgdriv_event(MSGID_EXT_INPUT_TRACK,NULL);
}


void CProcess::MSGAPI_inpumtd(long lParam )
{
	sThis->msgdriv_event(MSGID_EXT_INPUT_ENMTD,NULL);
}

void CProcess::MSGAPI_inpumtdSelect(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;
	int i;
	if(pIStuts->MMTTempStat==3)
	{
		for(i=0;i<MAX_TARGET_NUMBER;i++)
		{
			if(sThis->m_mtd[pIStuts->SensorStat]->tg[majormmtid].valid==1)
			{
				//majormmtid++;
				majormmtid=(majormmtid+1)%MAX_TARGET_NUMBER;
			}
		}	
	}
	else if(pIStuts->MMTTempStat==4)
	{
		for(i=0;i<MAX_TARGET_NUMBER;i++)
		{
			if(sThis->m_mtd[pIStuts->SensorStat]->tg[majormmtid].valid==1)
			{
				//majormmtid++;
				if(majormmtid>0)
					majormmtid=(majormmtid-1);
				else
				{
					majormmtid=MAX_TARGET_NUMBER-1;
				}
			}
		}
	}
	OSA_printf("MSGAPI_inpumtdSelect\n");
}


void CProcess::MSGAPI_inpuenhance(long lParam )
{
	sThis->msgdriv_event(MSGID_EXT_INPUT_ENENHAN,NULL);
}

void CProcess::MSGAPI_setMtdState(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;
	if(pIStuts->MtdState[pIStuts->validChId] == 0)
	{
		rectangle( sThis->m_display.m_imgOsd[1],
			Point( sThis->preWarnRect.x, sThis->preWarnRect.y ),
			Point( sThis->preWarnRect.x+sThis->preWarnRect.width, sThis->preWarnRect.y+sThis->preWarnRect.height),
			cvScalar(0,0,0,0), 2, 8 );
	}

	sThis->msgdriv_event(MSGID_EXT_MVDETECT,NULL);
}

void CProcess::MSGAPI_setAimRefine(long lParam)
{
	CMD_EXT *pIStuts = sThis->extInCtrl;

	if(pIStuts->aimRectMoveStepX==eTrk_ref_left)
	{
		pIStuts->aimRectMoveStepX=-1;
	}
	else if(pIStuts->aimRectMoveStepX==eTrk_ref_right)
	{
		pIStuts->aimRectMoveStepX=1;
	}
	if(pIStuts->aimRectMoveStepY==eTrk_ref_up)
	{
		pIStuts->aimRectMoveStepY=-1;
	}
	else if(pIStuts->aimRectMoveStepY==eTrk_ref_down)
	{
		pIStuts->aimRectMoveStepY=1;
	}
	sThis->msgdriv_event(MSGID_EXT_INPUT_AIMPOS,NULL);
}


void CProcess::MSGAPI_setAimSize(long lParam)
{
	sThis->msgdriv_event(MSGID_EXT_INPUT_AIMSIZE,NULL);
}

void CProcess::MSGAPI_inputbdt(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;
	if(pIStuts->TvCollimation!=1)
		pIStuts->ImgBlobDetect[pIStuts->SensorStat] = eImgAlg_Disable;
	else
		pIStuts->ImgBlobDetect[pIStuts->SensorStat] = eImgAlg_Enable;
	sThis->msgdriv_event(MSGID_EXT_INPUT_ENBDT,NULL);	
	OSA_printf("fun=%s line=%d \n",__func__,__LINE__);
}


void CProcess::MSGAPI_inputzoom(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;
	sThis->msgdriv_event(MSGID_EXT_INPUT_ENZOOM,NULL);
}


void CProcess::MSGAPI_inputfrezz(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;	
	if( pIStuts->FrCollimation==1)
	{
		pIStuts->PicpSensorStat=0;//tv picp sensor
		sThis->msgdriv_event(MSGID_EXT_INPUT_ENPICP, NULL);
		//dong jie chuang kou
		pIStuts->ImgFrezzStat[pIStuts->SensorStat] = eImgAlg_Enable;
		sThis->msgdriv_event(MSGID_EXT_INPUT_ENFREZZ,NULL);
	}
	else
	{	
		if((pIStuts->PicpSensorStatpri!=0))//tui picp the sensor is tv
		{
			pIStuts->PicpSensorStatpri=pIStuts->PicpSensorStat=2;//tui chu picp
			sThis->msgdriv_event(MSGID_EXT_INPUT_ENPICP, NULL);
			OSA_printf("MSGAPI_inputfrezz*****************************************disable \n");
		}
		else
		{
			pIStuts->PicpSensorStat=0;
		}
		//tui chu dong jie chuang kou
		pIStuts->ImgFrezzStat[pIStuts->SensorStat] = eImgAlg_Disable;
		sThis->msgdriv_event(MSGID_EXT_INPUT_ENFREZZ,NULL);
		
		OSA_printf("the*****************************************disable PicpSensorStatpri=%d\n",pIStuts->PicpSensorStatpri);
	}

			
	
	OSA_printf("%s\n",__func__);
}


void CProcess::MSGAPI_inputmmtselect(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;
	if(pIStuts->MmtSelect[pIStuts->SensorStat]  ==eMMT_Next)
		majormmtid=(majormmtid+1)%MAX_TARGET_NUMBER;
	else if(pIStuts->MmtSelect[pIStuts->SensorStat]  ==  eMMT_Prev)
	{
		majormmtid=(majormmtid-1+MAX_TARGET_NUMBER)%MAX_TARGET_NUMBER;
	}
	OSA_printf("%s\n",__func__);
}



void CProcess::MSGAPI_inputpositon(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;

	if((pIStuts->AxisPosX[pIStuts->SensorStat]>=50)&&(pIStuts->AxisPosX[pIStuts->SensorStat]<=vcapWH[pIStuts->SensorStat][0]-50))
	{
		if(pIStuts->axisMoveStepX != 0)
		{
			pIStuts->AxisPosX[pIStuts->SensorStat] += pIStuts->axisMoveStepX;
			pIStuts->axisMoveStepX = 0;
		}	
		pIStuts->unitAimX = pIStuts->AxisPosX[pIStuts->SensorStat];
	}
	if((pIStuts->AxisPosY[pIStuts->SensorStat]>=50)&&(pIStuts->AxisPosY[pIStuts->SensorStat]<=vcapWH[pIStuts->SensorStat][1]-50))
	{
		if(pIStuts->axisMoveStepY != 0)
		{
			pIStuts->AxisPosY[pIStuts->SensorStat] += pIStuts->axisMoveStepY;
			pIStuts->axisMoveStepY = 0;
		}
		pIStuts->unitAimY = pIStuts->AxisPosY[pIStuts->SensorStat];
	}
	
	OSA_printf("%s   THE=unitAimX=%d unitAxisY=%d\n",__func__,pIStuts->opticAxisPosX[pIStuts->SensorStat],pIStuts->opticAxisPosY[pIStuts->SensorStat]);
}

void CProcess::MSGAPI_inputcoast(long lParam )
{

	sThis->msgdriv_event(MSGID_EXT_INPUT_COAST,NULL);
	
	//printf("%s\n",__func__);
}

void CProcess::MSGAPI_inputfovselect(long lParam )
{

	CMD_EXT *pIStuts = sThis->extInCtrl;

	if(pIStuts->changeSensorFlag == 0)
	{
		//OSA_printf("FovStat = %d SensorStat=%d\n",pIStuts->FovStat,pIStuts->SensorStat);
		if(pIStuts->SensorStat == 0)
		{
		#if __TRACK__
			if(pIStuts->FovStat == 1)
				sThis->Track_fovreacq( 2400,pIStuts->SensorStat,0);
			else if(pIStuts->FovStat == 3)
				sThis->Track_fovreacq( 330,pIStuts->SensorStat,0);
			else if(pIStuts->FovStat == 4)	
				sThis->Track_fovreacq( 110,pIStuts->SensorStat,0);					
			else if(pIStuts->FovStat == 5)
				sThis->Track_fovreacq( 55,pIStuts->SensorStat,0);
			
		#endif
		}
		else if(pIStuts->SensorStat == 1){
		#if __TRACK__
			if(pIStuts->FovStat == 1)
				sThis->Track_fovreacq( 4000,pIStuts->SensorStat,0);
			else if(pIStuts->FovStat == 4)
				sThis->Track_fovreacq( 120,pIStuts->SensorStat,0);
			else if(pIStuts->FovStat == 5)
				sThis->Track_fovreacq( 60,pIStuts->SensorStat,0);
		#endif
		}

		//OSA_printf("fovselectXY(%f,%f),WH(%f,%f)\n",sThis->trackinfo_obj->reAcqRect.x,sThis->trackinfo_obj->reAcqRect.y,sThis->trackinfo_obj->reAcqRect.width,sThis->trackinfo_obj->reAcqRect.height);
		#if __TRACK__
		if(pIStuts->AvtTrkStat){	
			sThis->Track_reacq(sThis->trackinfo_obj->reAcqRect,2);
		}
		#endif
	}
}

void CProcess::MSGAPI_inputfovchange(long lParam )
{

	CMD_EXT *pIStuts = sThis->extInCtrl;

	//OSA_printf("%s:unitFovAngle = %f\n",__func__,pIStuts->unitFovAngle[pIStuts->SensorStat]);
	#if __TRACK__
	sThis->Track_fovreacq( pIStuts->unitFovAngle[pIStuts->SensorStat],pIStuts->SensorStat,0);
	#endif
}


void CProcess::MSGAPI_inputsearchmod(long lParam )
{
}


 void CProcess::MSGAPI_inputvideotect(long lParam )
{
	OSA_printf("MSGAPI_inputvideotect*******************\n");
	sThis->msgdriv_event(MSGID_EXT_INPUT_VIDEOEN,NULL);
}

  void CProcess::MSGAPI_mmtshow(long lParam )
{
	OSA_printf("MSGAPI_mmtshow\n");
}
void CProcess::MSGAPI_FOVcmd(long lParam )
{
	CMD_EXT *pIStuts = sThis->extInCtrl;
	if((pIStuts->FovCtrl==5)&&(pIStuts->SensorStat==0))
		sThis->tvzoomStat=1;
	else
		sThis->tvzoomStat=0;
}
void CProcess::MSGAPI_SaveCfgcmd(long lParam )
{
	sThis->msgdriv_event(MSGID_EXT_INPUT_CFGSAVE,NULL);
}	

void CProcess::initAcqRect()
{	
	CMD_EXT *pIStuts = extInCtrl;
	pIStuts->AcqRectW[0] = gConfig_Osd_param.ch0_acqRect_width;
	pIStuts->AcqRectW[1] = gConfig_Osd_param.ch1_acqRect_width;
	pIStuts->AcqRectW[2] = gConfig_Osd_param.ch2_acqRect_width;
	pIStuts->AcqRectW[3] = gConfig_Osd_param.ch3_acqRect_width;
	pIStuts->AcqRectW[4] = gConfig_Osd_param.ch4_acqRect_width;
	pIStuts->AcqRectH[0]  = gConfig_Osd_param.ch0_acqRect_height;
	pIStuts->AcqRectH[1]  = gConfig_Osd_param.ch1_acqRect_height;
	pIStuts->AcqRectH[2]  = gConfig_Osd_param.ch2_acqRect_height;
	pIStuts->AcqRectH[3]  = gConfig_Osd_param.ch3_acqRect_height;
	pIStuts->AcqRectH[4]  = gConfig_Osd_param.ch4_acqRect_height;
	return ;
}

void CProcess::initAimRect()
{
	CMD_EXT *pIStuts = extInCtrl;
	
	
	return ;
}
void CProcess::MSGAPI_update_osd(long lParam)
{
	plat->update_param_osd();
}

void CProcess::update_param_osd()
{
	CMD_EXT *pIStuts = extInCtrl;
	pIStuts->SensorStatBegin 		= gConfig_Osd_param.MAIN_Sensor;
	pIStuts->osdTextShow 			= gConfig_Osd_param.OSD_text_show;
	pIStuts->osdDrawShow 			= gConfig_Osd_param.OSD_draw_show;
	pIStuts->crossDrawShow 			= gConfig_Osd_param.CROSS_draw_show;
	pIStuts->osdTextColor 			=  gConfig_Osd_param.OSD_text_color;
	pIStuts->osdTextAlpha			=  gConfig_Osd_param.OSD_text_alpha;
	pIStuts->osdTextFont			= gConfig_Osd_param.OSD_text_font;
	pIStuts->osdTextSize			= gConfig_Osd_param.OSD_text_size;
	pIStuts->osdDrawColor 			= gConfig_Osd_param.OSD_draw_color;
	pIStuts->AcqRectW[0] 			= gConfig_Osd_param.ch0_acqRect_width;
	pIStuts->AcqRectW[1] 			= gConfig_Osd_param.ch1_acqRect_width;
	pIStuts->AcqRectW[2] 			= gConfig_Osd_param.ch2_acqRect_width;
	pIStuts->AcqRectW[3] 			= gConfig_Osd_param.ch3_acqRect_width;
	pIStuts->AcqRectW[4] 			= gConfig_Osd_param.ch4_acqRect_width;
	pIStuts->AcqRectW[5] 			= gConfig_Osd_param.ch5_acqRect_width;
	pIStuts->AcqRectH[0] 			= gConfig_Osd_param.ch0_acqRect_height;
	pIStuts->AcqRectH[1] 			= gConfig_Osd_param.ch1_acqRect_height;
	pIStuts->AcqRectH[2] 			= gConfig_Osd_param.ch2_acqRect_height;
	pIStuts->AcqRectH[3] 			= gConfig_Osd_param.ch3_acqRect_height;
	pIStuts->AcqRectH[4] 			= gConfig_Osd_param.ch4_acqRect_height;
	pIStuts->AcqRectH[5] 			= gConfig_Osd_param.ch5_acqRect_height;

	pIStuts->AimW[0] 				= gConfig_Osd_param.ch0_aim_width;
	pIStuts->AimW[1] 				= gConfig_Osd_param.ch1_aim_width;
	pIStuts->AimW[2] 				= gConfig_Osd_param.ch2_aim_width;
	pIStuts->AimW[3] 				= gConfig_Osd_param.ch3_aim_width;
	pIStuts->AimW[4] 				= gConfig_Osd_param.ch4_aim_width;
	pIStuts->AimW[5] 				= gConfig_Osd_param.ch5_aim_width;
	pIStuts->AimH[0] 				= gConfig_Osd_param.ch0_aim_height;
	pIStuts->AimH[1] 				= gConfig_Osd_param.ch1_aim_height;
	pIStuts->AimH[2] 				= gConfig_Osd_param.ch2_aim_height;
	pIStuts->AimH[3] 				= gConfig_Osd_param.ch3_aim_height;
	pIStuts->AimH[4] 				= gConfig_Osd_param.ch4_aim_height;
	pIStuts->AimH[5] 				= gConfig_Osd_param.ch5_aim_height;

	m_acqRectW 	= pIStuts->AimW[pIStuts->SensorStat];
	m_acqRectH  = pIStuts->AimH[pIStuts->SensorStat];
	
	m_display.disptimeEnable = gConfig_Osd_param.Timedisp_9;
	m_display.m_bOsd = pIStuts->osdDrawShow;
	m_display.m_crossOsd = pIStuts->crossDrawShow;


	//pIStuts->crossAxisWidth 		= gConfig_Osd_param.CROSS_AXIS_WIDTH;
	//pIStuts->crossAxisHeight		= gConfig_Osd_param.CROSS_AXIS_HEIGHT;
	//pIStuts->picpCrossAxisWidth		= gConfig_Osd_param.Picp_CROSS_AXIS_WIDTH;
	//pIStuts->picpCrossAxisHeight	= gConfig_Osd_param.Picp_CROSS_AXIS_HEIGHT;
	pIStuts->crossAxisWidth[video_pal] = 40;
	pIStuts->crossAxisHeight[video_pal] = 40;
	pIStuts->crossAxisWidth[video_gaoqing] = 60;
	pIStuts->crossAxisHeight[video_gaoqing] = 60;
	pIStuts->picpCrossAxisWidth = 40;
	pIStuts->picpCrossAxisHeight = 40;

	return;
}

void CProcess::MSGAPI_update_alg(long lParam)
{
	plat->update_param_alg();
}
#define UTCPARM 0
void CProcess::update_param_alg()
{
	UTC_DYN_PARAM dynamicParam;
	if(gConfig_Alg_param.occlusion_thred > 0.0001)
		dynamicParam.occlusion_thred = gConfig_Alg_param.occlusion_thred;
	else
		dynamicParam.occlusion_thred = 0.28;

	//dynamicParam.occlusion_thred = 0.30;
	
	if(gConfig_Alg_param.retry_acq_thred> 0.0001)
		dynamicParam.retry_acq_thred = gConfig_Alg_param.retry_acq_thred;
	else
		dynamicParam.retry_acq_thred = 0.38;

	//dynamicParam.retry_acq_thred = 0.40;
	
	float up_factor;
	if(gConfig_Alg_param.up_factor > 0.0001)
		up_factor = gConfig_Alg_param.up_factor;
	else
		up_factor = 0.0125;
	//up_factor = 0.025;

	TRK_SECH_RESTRAINT resTraint;
	if(gConfig_Alg_param.res_distance > 0)
		resTraint.res_distance = gConfig_Alg_param.res_distance;
	else
		resTraint.res_distance = 80;
	
	if(gConfig_Alg_param.res_area> 0)
		resTraint.res_area = gConfig_Alg_param.res_area;
	else
		resTraint.res_area = 5000;
	//printf("UtcSetRestraint: distance=%d area=%d \n", resTraint.res_distance, resTraint.res_area);

	int gapframe;
	if(gConfig_Alg_param.gapframe> 0)
		gapframe = gConfig_Alg_param.gapframe;
	else
		gapframe = 10;

   	bool enhEnable;
	enhEnable = gConfig_Alg_param.enhEnable;	

	float cliplimit;
	if(gConfig_Alg_param.cliplimit> 0)
		cliplimit = gConfig_Alg_param.cliplimit;
	else
		cliplimit = 4.0;

	bool dictEnable;

	dictEnable = gConfig_Alg_param.dictEnable;

	int moveX,moveY;
	if(gConfig_Alg_param.moveX > 0)
		moveX = gConfig_Alg_param.moveX;
	else
		moveX = 20;

	if(gConfig_Alg_param.moveY>0)
		moveY = gConfig_Alg_param.moveY;
	else
		moveY = 10;

	int moveX2,moveY2;
	if(gConfig_Alg_param.moveX2 > 0)
		moveX2 = gConfig_Alg_param.moveX2;
	else
		moveX2 = 30;

	if(gConfig_Alg_param.moveY2 > 0)
		moveY2 = gConfig_Alg_param.moveY2;
	else
		moveY2 = 20;

	int segPixelX,segPixelY;

	if(gConfig_Alg_param.segPixelX > 0)
		segPixelX = gConfig_Alg_param.segPixelX;
	else
		segPixelX = 600;
	if(gConfig_Alg_param.segPixelY > 0)
		segPixelY = gConfig_Alg_param.segPixelY;
	else
		segPixelY = 450;

	/*
	if(gConfig_Alg_param.algOsdRect_Enable == 1)
		algOsdRect = true;
	else
		algOsdRect = false;
	*/

	if(gConfig_Alg_param.ScalerLarge > 0)
		ScalerLarge = gConfig_Alg_param.ScalerLarge;
	else
		ScalerLarge = 256;
	if(gConfig_Alg_param.ScalerMid > 0)
		ScalerMid = gConfig_Alg_param.ScalerMid;
	else
		ScalerMid = 128;
	if(gConfig_Alg_param.ScalerSmall >0)
		ScalerSmall = gConfig_Alg_param.ScalerSmall;
	else
		ScalerSmall = 64;

	int Scatter;
	if(gConfig_Alg_param.Scatter > 0)
		Scatter = gConfig_Alg_param.Scatter;
	else
		Scatter = 10;

	float ratio;
	if(gConfig_Alg_param.ratio >0.1)
		ratio = gConfig_Alg_param.ratio;
	else
		ratio = 1.0;

	bool FilterEnable;

	FilterEnable = gConfig_Alg_param.FilterEnable;

	bool BigSecEnable;
	BigSecEnable = gConfig_Alg_param.BigSecEnable;

	int SalientThred;
	if(gConfig_Alg_param.SalientThred > 0)
		SalientThred = gConfig_Alg_param.SalientThred;
	else
		SalientThred = 40;
	bool ScalerEnable;
	ScalerEnable = gConfig_Alg_param.ScalerEnable;

	bool DynamicRatioEnable;
	DynamicRatioEnable = ScalerEnable = gConfig_Alg_param.DynamicRatioEnable;

	UTC_SIZE acqSize;
	if(gConfig_Alg_param.acqSize_width > 0)	
		acqSize.width = gConfig_Alg_param.acqSize_width;
	else
		acqSize.width = 8;
	if(gConfig_Alg_param.acqSize_height > 0)
		acqSize.height = gConfig_Alg_param.acqSize_height;
	else
		acqSize.height = 8;
	
	if(gConfig_Alg_param.TrkAim43_Enable == 1)
		TrkAim43 = true;
	else
		TrkAim43 = false;

	bool SceneMVEnable;
	SceneMVEnable = gConfig_Alg_param.SceneMVEnable;

	bool BackTrackEnable;
	BackTrackEnable = gConfig_Alg_param.BackTrackEnable;

	bool  bAveTrkPos;
	bAveTrkPos = gConfig_Alg_param.bAveTrkPos;

	float fTau;
	if(gConfig_Alg_param.fTau > 0.01)
		fTau = gConfig_Alg_param.fTau;
	else
		fTau = 0.5;

	int  buildFrms;
	if(gConfig_Alg_param.buildFrms > 0)
		buildFrms = gConfig_Alg_param.buildFrms;
	else
		buildFrms = 500;
	
	int  LostFrmThred;
	if(gConfig_Alg_param.LostFrmThred > 0)
		LostFrmThred = gConfig_Alg_param.LostFrmThred;
	else
		LostFrmThred = 30;

	float  histMvThred;
	if(gConfig_Alg_param.histMvThred > 0.01)
		histMvThred = gConfig_Alg_param.histMvThred;
	else
		histMvThred = 1.0;

	int  detectFrms;
	if(gConfig_Alg_param.detectFrms > 0)
		detectFrms = gConfig_Alg_param.detectFrms;
	else
		detectFrms = 30;

	int  stillFrms;
	if(gConfig_Alg_param.stillFrms > 0)
		stillFrms = gConfig_Alg_param.stillFrms;
	else
		stillFrms = 50;

	float  stillThred;
	if(gConfig_Alg_param.stillThred> 0.01)
		stillThred = gConfig_Alg_param.stillThred;
	else
		stillThred = 0.1;


	bool  bKalmanFilter;
	bKalmanFilter = gConfig_Alg_param.bKalmanFilter;

	float xMVThred, yMVThred;
	if(gConfig_Alg_param.xMVThred> 0.01)
		xMVThred = gConfig_Alg_param.xMVThred;
	else
		xMVThred = 3.0;
	if(gConfig_Alg_param.yMVThred> 0.01)
		yMVThred = gConfig_Alg_param.yMVThred;
	else
		yMVThred = 2.0;

	float xStillThred, yStillThred;
	if(gConfig_Alg_param.xStillThred> 0.01)
		xStillThred = gConfig_Alg_param.xStillThred;
	else
		xStillThred = 0.5;
	if(gConfig_Alg_param.yStillThred> 0.01)
		yStillThred= gConfig_Alg_param.yStillThred;
	else
		yStillThred = 0.3;

	float slopeThred;
	if(gConfig_Alg_param.slopeThred> 0.01)
		slopeThred = gConfig_Alg_param.slopeThred;
	else
		slopeThred = 0.08;

	float kalmanHistThred;
	if(gConfig_Alg_param.kalmanHistThred> 0.01)
		kalmanHistThred = gConfig_Alg_param.kalmanHistThred;
	else
		kalmanHistThred = 2.5;

	float kalmanCoefQ, kalmanCoefR;
	if(gConfig_Alg_param.kalmanCoefQ> 0.000001)
		kalmanCoefQ = gConfig_Alg_param.kalmanCoefQ;
	else
		kalmanCoefQ = 0.00001;
	if(gConfig_Alg_param.kalmanCoefR> 0.000001)
		kalmanCoefR = gConfig_Alg_param.kalmanCoefR;
	else
		kalmanCoefR = 0.0025;

	bool  bSceneMVRecord;
	bSceneMVRecord = 0;//gConfig_Alg_param.SceneMVEnable;
	
	if(bSceneMVRecord == true)
		wFileFlag = true;
	
	

	UtcSetPLT_BS(m_track, tPLT_TST, BoreSight_Mid);



	//enh
	if(gConfig_Alg_param.Enhmod_0 > 4)
		m_display.enhancemod = gConfig_Alg_param.Enhmod_0;
	else
		m_display.enhancemod = 1;
	
	if((gConfig_Alg_param.Enhparm_1>0.0)&&(gConfig_Alg_param.Enhparm_1<5.0))
		m_display.enhanceparam=gConfig_Alg_param.Enhparm_1;
	else
		m_display.enhanceparam=3.5;

	//mmt
	if((gConfig_Alg_param.Mmtdparm_2<0) || (gConfig_Alg_param.Mmtdparm_2>15))
		DetectGapparm = 10;
	else
		DetectGapparm = 3;
	m_MMTDObj.SetSRDetectGap(DetectGapparm);

	if(gConfig_Alg_param.Mmtdparm_3 > 0)
		MinArea = gConfig_Alg_param.Mmtdparm_3;
	else
		MinArea = 80;
	if(gConfig_Alg_param.Mmtdparm_4 > 0)
		MaxArea = gConfig_Alg_param.Mmtdparm_4;
	else
		MaxArea = 3600;

	m_MMTDObj.SetConRegMinMaxArea(MinArea, MaxArea);

	if(gConfig_Alg_param.Mmtdparm_5 > 0)
		stillPixel = gConfig_Alg_param.Mmtdparm_5;
	else
		stillPixel = 6;
	if(gConfig_Alg_param.Mmtdparm_6 > 0)
		movePixel = gConfig_Alg_param.Mmtdparm_6;
	else
		movePixel = 16;
	m_MMTDObj.SetMoveThred(stillPixel, movePixel);

	if(gConfig_Alg_param.Mmtdparm_7 > 0.001)
		lapScaler = gConfig_Alg_param.Mmtdparm_7;
	else
		lapScaler = 1.25;
	m_MMTDObj.SetLapScaler(lapScaler);

	if(gConfig_Alg_param.Mmtdparm_8 > 0)
		lumThred = gConfig_Alg_param.Mmtdparm_8;
	else
		lumThred = 50;
	m_MMTDObj.SetSRLumThred(lumThred);

	UtcSetDynParam(m_track, dynamicParam);
	UtcSetUpFactor(m_track, up_factor);
	UtcSetUpFactor(m_track, up_factor);
	UtcSetBlurFilter(m_track,FilterEnable);
	UtcSetBigSearch(m_track, BigSecEnable);
	
#if UTCPARM

	UtcSetDynParam(m_track, dynamicParam);
	UtcSetUpFactor(m_track, up_factor);
	UtcSetRestraint(m_track, resTraint);
	UtcSetIntervalFrame(m_track, gapframe);
	UtcSetEnhance(m_track, enhEnable);
	UtcSetEnhfClip(m_track, cliplimit);	
	UtcSetPredict(m_track, dictEnable);
	UtcSetMvPixel(m_track,moveX,moveY);
	UtcSetMvPixel2(m_track,moveX2,moveY2);
	UtcSetSegPixelThred(m_track,segPixelX,segPixelY);
	UtcSetSalientScaler(m_track, ScalerLarge, ScalerMid, ScalerSmall);
	UtcSetSalientScatter(m_track, Scatter);
	UtcSetSRAcqRatio(m_track, ratio);
	UtcSetBlurFilter(m_track,FilterEnable);
	UtcSetBigSearch(m_track, BigSecEnable);
	UtcSetSalientThred(m_track,SalientThred);
	UtcSetMultScaler(m_track, ScalerEnable);
	UtcSetDynamicRatio(m_track, DynamicRatioEnable);
	UtcSetSRMinAcqSize(m_track,acqSize);
	UtcSetSceneMV(m_track, SceneMVEnable);
	UtcSetBackTrack(m_track, BackTrackEnable);
	UtcSetAveTrkPos(m_track, bAveTrkPos);
	UtcSetDetectftau(m_track, fTau);
	UtcSetDetectBuildFrms(m_track, buildFrms);
	UtcSetLostFrmThred(m_track, LostFrmThred);
	UtcSetHistMVThred(m_track, histMvThred);
	UtcSetDetectFrmsThred(m_track, detectFrms);
	UtcSetStillFrmsThred(m_track, stillFrms);
	UtcSetStillPixThred(m_track, stillThred);
	UtcSetKalmanFilter(m_track, bKalmanFilter);
	UtcSetKFMVThred(m_track, xMVThred, yMVThred);
	UtcSetKFStillThred(m_track, xStillThred, yStillThred);
	UtcSetKFSlopeThred(m_track, slopeThred);
	UtcSetKFHistThred(m_track, kalmanHistThred);
	UtcSetKFCoefQR(m_track, kalmanCoefQ, kalmanCoefR);
	UtcSetSceneMVRecord(m_track, bSceneMVRecord);
	UtcSetRoiMaxWidth(m_track, 400);

#endif


	
	return ;
}

void CProcess::MSGAPI_update_camera(long lParam)
{
}
void CProcess::MSGAPI_input_algosdrect(long lParam)
{
	sThis->msgdriv_event(MSGID_EXT_INPUT_ALGOSDRECT,NULL);
}
#if __TRACK__
void CProcess::set_trktype(CMD_EXT *p, unsigned int stat)
{
	
	static int old_stat = -1;
	int flag = 0;
	SENDST test;
	test.cmd_ID = trktype;

	p->TrkStat = stat;

	if(old_stat == eTrk_Lost && stat == eTrk_Assi)
		return;
		
	if(old_stat != stat)
	{
		old_stat = stat;
		flag = 1;
	}
	if(flag&&(extInCtrl->AvtTrkStat == eTrk_mode_target))
		ipc_sendmsg(&test,IPC_FRIMG_MSG);
}
#endif

