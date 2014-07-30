/*******************************************************************************
*                                                                              *
*   PrimeSense NiTE 2.0 - Hand Viewer Sample                                   *
*   Copyright (C) 2012 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "Viewer.h"
#include <Windows.h>
#include <math.h>



#if (ONI_PLATFORM == ONI_PLATFORM_MACOSX)
        #include <GLUT/glut.h>
#else
        #include <GL/glut.h> 
        #include <GL/GLU.h> 
#endif

#include "HistoryBuffer.h"
#include <NiteSampleUtilities.h>
#include <windows.h>   /* required before including mmsystem.h */
#include <mmsystem.h>  /* multimedia functions (such as MIDI) for Windows */
#include "RtMIDI\RtMidi.h"

using namespace std;

#define GL_WIN_SIZE_X	1280
#define GL_WIN_SIZE_Y	960  //1024
#define TEXTURE_SIZE	512
#define PI 3.141592653589793

#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

SampleViewer* SampleViewer::ms_self = NULL;

std::map<int, HistoryBuffer<20> *> g_histories;

bool g_drawDepth = true;
bool g_smoothing = false;
bool g_drawFrameId = false;

std::vector<unsigned char> midiMessage;
int g_nXRes = 0, g_nYRes = 0;
RtMidiOut *midiout ;
RtMidiOut *midiout2 ;
fstream fp;

int lastkey=0;
////////////////////////////////////////////
int BMPwidth1; 
int BMPheight1; 
unsigned char *BMPimage1;  

/////////////////////////////////////////////////////////
int drum_notes[8]={45,49,70,39,71,36,46,65};
bool note_on[8]={false,false,false,false,false,false,false,false};
bool last_note[8]={false,false,false,false,false,false,false,false};

/////////////////////////////////////////////////////////
openni::VideoStream streamDepth;
nite::HandTrackerFrameRef handFrame;
openni::VideoFrameRef depthFrame;  
void SampleViewer::glutIdle()
{
	glutPostRedisplay();
}
void SampleViewer::glutDisplay()
{
	SampleViewer::ms_self->Display();
}

void SampleViewer::glutKeyboard(unsigned char key, int x, int y)
{
	SampleViewer::ms_self->OnKey(key, x, y);
}

SampleViewer::SampleViewer(const char* strSampleName)
{
	ms_self = this;
	strncpy(m_strSampleName, strSampleName, ONI_MAX_STR);
	m_pHandTracker = new nite::HandTracker;
}
SampleViewer::~SampleViewer()
{
	Finalize();

	delete[] m_pTexMap;

	ms_self = NULL;
}

void SampleViewer::Finalize()
{
	delete m_pHandTracker;
	nite::NiTE::shutdown();
	openni::OpenNI::shutdown();
}

openni::Status SampleViewer::Init(int argc, char **argv)
{
	m_pTexMap = NULL;

	openni::OpenNI::initialize();

	const char* deviceUri = openni::ANY_DEVICE;
	for (int i = 1; i < argc-1; ++i)
	{
		if (strcmp(argv[i], "-device") == 0)
		{
			deviceUri = argv[i+1];
			break;
		}
	}

	openni::Status rc = m_device.open(deviceUri);
	if (rc != openni::STATUS_OK)
	{
		printf("Open Device failed:\n%s\n", openni::OpenNI::getExtendedError());
		return rc;
		
	}

	nite::NiTE::initialize();

	if (m_pHandTracker->create(&m_device) != nite::STATUS_OK)
	{
		return openni::STATUS_ERROR;
	}

	m_pHandTracker->startGestureDetection(nite::GESTURE_WAVE);
	m_pHandTracker->startGestureDetection(nite::GESTURE_CLICK);

	return InitOpenGL(argc, argv);

}
openni::Status SampleViewer::Run()	//Does not return
{
	glutMainLoop();
	return openni::STATUS_OK;
}

float Colors[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
int colorCount = 3;


float Calculate_Distance(float x1, float y1, float x2, float y2)
{
	float X = (x1-x2)*(x1-x2);
 	float Y = (y1-y2)*(y1-y2);
	return sqrt(X+Y);
}
int Calculate_NoteKey(float x1,float y1)
{
	float Hypotenuse = Calculate_Distance(x1 ,y1 , 640, 512);     //斜邊
	float Subtense = 512.0 - y1;                                  //對邊
	float result = asin(Subtense/Hypotenuse)*180.0/PI;
	if(x1 > 640.0)
	   result=result+90;   //return result;
	else
	   result=180.0-result+90; //return 180.0-result;

	if(Hypotenuse>200)
	{
		if(result > 0.0 && result <30.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 48;
			else
				return 60;
		}  
		if(result > 30.0 && result<60.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 49;
			else
				return 61;
		}
		if(result > 60.0 && result<90.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 50;
			else
				return 62;
		}
		if(result > 90.0 && result<120.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 51;
			else
				return 63;
		}
		
		if(result > 120.0 && result<150.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 52;
			else
				return 64;
		}
		
		if(result > 150.0 && result<180.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 53;
			else
				return 65;
		}	
		
		if(result > 180.0 && result<210.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 54;
			else
				return 66;
		}
		
		if(result > 210.0 && result<240.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 55;
			else
				return 67;
		}
		
		if(result > 240.0 && result<270.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 56;
			else
				return 68;
		}
		if(result > 270.0 && result<300.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 57;
			else
				return 69;
		}
		
		if(result > 300.0 && result<330.0){
			if(Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 58;
			else
				return 70;
		}
		if(result > 330.0 && result<360.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 59;
			else
				return 71;
		}
		return 0;
	}
	else
		return 0;
}

void DrawHistory(nite::HandTracker* pHandTracker, int id, HistoryBuffer<20>* pHistory)
{
	glColor3f(Colors[id % colorCount][0], Colors[id % colorCount][1], Colors[id % colorCount][2]);
	float coordinates[60] = {0};
	float factorX = GL_WIN_SIZE_X / (float)g_nXRes;
	float factorY = GL_WIN_SIZE_Y / (float)g_nYRes;


    const nite::Point3f& position = pHistory->operator[](0);		
	pHandTracker->convertHandCoordinatesToDepth(position.x, position.y, position.z, &coordinates[0], &coordinates[1]);
	coordinates[0] *= factorX;
	coordinates[1] *= factorY;

	glPointSize(18);
	glVertexPointer(3, GL_FLOAT, 0, coordinates);
	glDrawArrays(GL_POINTS, 0, 1);
}

float * getCoordinate(double angle, double radius)
{
	static float position[2];

	int x,y;
	if(angle >= 0  && angle <= 90.0){
		x = radius*sin(angle*PI/180.0);
		y = sqrt(radius*radius - x*x); 
		position[0] = 640 + x;
		position[1] = 512 + y;
	}
	else if(angle > 90.0 && angle <=180.0){
		double newAngle = angle - 90.0;
		y = radius*sin(newAngle*PI/180.0);
		x = sqrt(radius*radius - y*y);
		position[0] = 640 + x;
		position[1] = 512 - y;
	}
	else if(angle > 180.0 && angle <= 270.0){
		double newAngle = angle - 180.0;
		x = radius*sin(newAngle*PI/180.0);
		y = sqrt(radius*radius - x*x); 
		position[0] = 640 - x;
		position[1] = 512 - y;
	}
	else if(angle > 270.0 && angle <= 360.0){
		double newAngle = angle - 270.0;
		y = radius*sin(newAngle*PI/180.0);
		x = sqrt(radius*radius - y*y); 
		position[0] = 640 - x;
		position[1] = 512 + y;
	}

    return position;

}

#ifndef USE_GLES
void glPrintString(void *font, const char *str)
{
	int i,l = (int)strlen(str);

	for(i=0; i<l; i++)
	{   
		glutBitmapCharacter(font,*str++);
	}   
}
#endif
void DrawFrameId(int frameId)
{
	char buffer[80] = "";
	sprintf(buffer, "%d", frameId);
	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos2i(20, 20);
	glPrintString(GLUT_BITMAP_HELVETICA_18, buffer);
}
void SampleViewer::Display()
{

	nite::Status rc = m_pHandTracker->readFrame(&handFrame);
	if (rc != nite::STATUS_OK)
	{
		printf("GetNextData failed\n");
		return;
	}

	depthFrame = handFrame.getDepthFrame();

	if (m_pTexMap == NULL)
	{
		// Texture map init
		m_nTexMapX = MIN_CHUNKS_SIZE(depthFrame.getVideoMode().getResolutionX(), TEXTURE_SIZE);
		m_nTexMapY = MIN_CHUNKS_SIZE(depthFrame.getVideoMode().getResolutionY(), TEXTURE_SIZE);
		m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];
	}

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -10000.0, 10000.0);



	if (depthFrame.isValid())
	{
		calculateHistogram(m_pDepthHist, MAX_DEPTH, depthFrame);
	}

	memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));

	float factor[3] = {1, 1, 1};
	// check if we need to draw depth frame to texture
	
	float av_x = 0;
	float av_y = 0;
	int counter= 0;



	for(int i = 0; i<=7 ; i++)
	      note_on[i] = false;

	if (depthFrame.isValid() && g_drawDepth)
	{
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)depthFrame.getData();
		const openni::DepthPixel* pDepthRow1 = pDepthRow;
		openni::RGB888Pixel* pTexRow = m_pTexMap + depthFrame.getCropOriginY() * m_nTexMapX;
		int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);
		glPointSize(2);
		glBegin(GL_POINTS);

		for (int y = 0; y < depthFrame.getHeight(); ++y)
		{
			const openni::DepthPixel* pDepth = pDepthRow;
			openni::RGB888Pixel* pTex = pTexRow + depthFrame.getCropOriginX();
			//chord_temp = 0;
			for (int x = 0; x < depthFrame.getWidth(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					factor[0] = Colors[colorCount][0];
					factor[1] = Colors[colorCount][1];
					factor[2] = Colors[colorCount][2];

					int nHistValue = m_pDepthHist[*pDepth];
					pTex->r = nHistValue*factor[0];
					pTex->g = nHistValue*factor[1];
					pTex->b = nHistValue*factor[2];

					factor[0] = factor[1] = factor[2] = 1;
					

					if(*pDepth <= 800)
					{
					    //glColor3f(1,0,0);	
						glColor3f(float(*pDepth)/2000,float(*pDepth)/2000,float(*pDepth)/2000);
						
						av_x = x + av_x;
						counter++;
						av_y = y + av_y;
		
							
					}
					else{
						glColor3f(float(*pDepth)/2000,float(*pDepth)/2000,float(*pDepth)/2000);	
		
					}

					glVertex3f(2*x,2*y,-*pDepth);
				}
			}
			pDepthRow += rowSize;
			pTexRow += m_nTexMapX;
		}
		glEnd();
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		av_x = av_x / counter;
	    av_y = av_y / counter;

		float R_x=0;
		float R_y=0;
		float L_x=0;
		float L_y=0;
		int counter_R=0;
		int counter_L=0;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	

		for (int y = 0; y < depthFrame.getHeight(); ++y)
		{
			const openni::DepthPixel* pDepth = pDepthRow1;
			//chord_temp = 0;
			for (int x = 0; x < depthFrame.getWidth(); ++x, ++pDepth)
			{
				if (*pDepth != 0)
				{

					if(*pDepth <= 800)
					{

						if(x > av_x){
							counter_R++;
							R_x = R_x +x;
							R_y = R_y +y;
						}
						if(x < av_x){
							counter_L++;
							L_x = L_x +x;
							L_y = L_y +y;
						}

					}

				}
			}
			pDepthRow1 += rowSize;
		}
		/////////////////////////////////////////////////////////////////

		R_x = R_x/counter_R;
	    R_y = R_y/counter_R;

		L_x = L_x/counter_L;
	    L_y = L_y/counter_L;

		glPointSize(30);
	    glBegin(GL_POINTS);
		glColor3f(1,0,0);
		glVertex3f(R_x*2,R_y*2,800);
		glColor3f(1,1,0);
		glVertex3f(L_x*2,L_y*2,800);
		glEnd();


		if( R_x >=75 && R_x <=175  ){
			if( R_y  <= 150  )
				{
					note_on[0] = true;
				}

				else if( R_y >= 350)
				{
					note_on[1] = true;
				}
			}
							
		if( R_x >=175 && R_x <=300){
			if( R_y <= 150 )
			{
				note_on[2] = true;	
			}

			else if( R_y >= 350 )
			{	
				note_on[3] = true;					
			}					
		}

		if( R_x>=300 && R_x<=425){
			if( R_y <= 150 )
			{		
				note_on[4] = true;	
			}

			else if( R_y >= 350 )
			{							
				note_on[5] = true;							
			}
		}

		if( R_x>=425 && R_x<=550){
			if( R_y <= 150  )
			{						
				note_on[6] = true;
			}

			else if( R_y >= 350 )
			{			
				note_on[7] = true;	
			}
		}

		////////////////////////////////////////
		if( L_x >=75 && L_x <=175  ){
			if( L_y  <= 150  )
				{
					note_on[0] = true;
				}

				else if( L_y >= 350)
				{
					note_on[1] = true;
				}
			}
							
		if( L_x >=175 && L_x <=300){
			if( L_y <= 150 )
			{
				note_on[2] = true;	
			}

			else if( L_y >= 350 )
			{	
				note_on[3] = true;					
			}					
		}

		if( L_x>=300 && L_x<=425){
			if( L_y <= 150 )
			{		
				note_on[4] = true;	
			}

			else if( L_y >= 350 )
			{							
				note_on[5] = true;							
			}
		}

		if( L_x>=425 && L_x<=550){
			if( L_y <= 150  )
			{						
				note_on[6] = true;
			}

			else if( L_y >= 350 )
			{			
				note_on[7] = true;	
			}
		}
	}




	playdrum();
	for(int i=0;i<=7 ;i++)
		last_note[i] = note_on[i]; 

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BMPwidth1,BMPheight1, 0, GL_RGB, GL_UNSIGNED_BYTE, BMPimage1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1,1,1,0.5);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	g_nXRes = depthFrame.getVideoMode().getResolutionX();
	g_nYRes = depthFrame.getVideoMode().getResolutionY();

	// upper left
	glTexCoord2f(0,1);
	glVertex3f(0,0,-800);
	// upper right
	glTexCoord2f(1,1);
	glVertex3f(1240,0,-800);
	// bottom right
	glTexCoord2f(1,0);
	glVertex3f(1240,960,-800);
	// bottom left
	glTexCoord2f(0,0);
	glVertex3f(0,960,-800);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);  

	/////////////////////////////////////////////////////////////////////////////////chord selection

	glBegin(GL_LINES);
	glColor3f(1,0,0);
	glVertex3f(150,300,800);
	glVertex3f(1100,300,800);
	glVertex3f(150,700,800);
	glVertex3f(1100,700,800);
	glEnd();


	glPointSize(30);
	glBegin(GL_POINTS);
	glColor3f(1,1,0);
	glVertex3f(150,300,800);
	glVertex3f(350,300,800);
	glVertex3f(600,300,800);
	glVertex3f(850,300,800);
	glVertex3f(1100,300,800);


	glVertex3f(150,700,800);
	glVertex3f(350,700,800);
	glVertex3f(600,700,800);
	glVertex3f(850,700,800);
	glVertex3f(1100,700,800);
	glEnd();



	////////////////////////////////////////////////////////////////////////////////////////////////////////////
		

	const nite::Array<nite::GestureData>& gestures = handFrame.getGestures();
	for (int i = 0; i < gestures.getSize(); ++i)
	{
		if (gestures[i].isComplete())
		{
			const nite::Point3f& position = gestures[i].getCurrentPosition();
			printf("Gesture %d at (%f,%f,%f)\n", gestures[i].getType(), position.x, position.y, position.z);

			nite::HandId newId;
			m_pHandTracker->startHandTracking(gestures[i].getCurrentPosition(), &newId);
		}
	}

	const nite::Array<nite::HandData>& hands= handFrame.getHands();
	for (int i = 0; i < hands.getSize(); ++i)
	{
		const nite::HandData& user = hands[i];

		if (!user.isTracking())
		{
			printf("Lost hand %d\n", user.getId());
			nite::HandId id = user.getId();
			HistoryBuffer<20>* pHistory = g_histories[id];
			g_histories.erase(g_histories.find(id));
			delete pHistory;
		}
		else
		{
			if (user.isNew())
			{
				printf("Found hand %d\n", user.getId());
				g_histories[user.getId()] = new HistoryBuffer<20>;
			}
			// Add to history
			HistoryBuffer<20>* pHistory = g_histories[user.getId()];
			pHistory->AddPoint(user.getPosition());
			// Draw history
		    DrawHistory(m_pHandTracker, user.getId(), pHistory);
		   
		}
	}

	if (g_drawFrameId)
	{
		DrawFrameId(handFrame.getFrameIndex());
	}
	// Swap the OpenGL display buffers
	glutSwapBuffers();
	
}

void handleView(int w,int h)
{   
	glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);
    gluPerspective( /* field of view in degree */ 50.0,
    /* aspect ratio */ 1.0,
    /* Z near */ 0.1, /* Z far */ 100000);
    glMatrixMode(GL_MODELVIEW);
    //gluLookAt(1000, 1000, 500, 0.0,0.0,0.0, 0.0, 0.0,1.0); 
	gluLookAt(400, 700, 500, 400,500,0, 0.0, 0.0,1.0);     
     /* eye is at () * /* center is at (0,0,0) */ /* up is in positive Z direction */
	 //glMatrixMode(GL_MODELVIEW);
}
void SampleViewer::OnKey(unsigned char key, int /*x*/, int /*y*/)
{
	//switch (key)
	//{
	//case 27:
	//	Finalize();
	//	exit (1);
	//case 'd':
	//	g_drawDepth = !g_drawDepth;
	//	break;
	//case 's':
	//	if (g_smoothing)
	//	{
	//		// Turn off smoothing
	//		m_pHandTracker->setSmoothingFactor(0);
	//		g_smoothing = FALSE;
	//	}
	//	else
	//	{
	//		m_pHandTracker->setSmoothingFactor(0.1);
	//		g_smoothing = TRUE;
	//	}
	//	break;
	//case 'f':
	//	// Draw frame ID
	//	g_drawFrameId = !g_drawFrameId;
	//	break;
	//}

}

openni::Status SampleViewer::InitOpenGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow (m_strSampleName);
	// 	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);
	InitOpenGLHooks();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

////////////////////////////////////////////////////////////////////////////////


	return openni::STATUS_OK;
}
void SampleViewer::InitOpenGLHooks()
{
	BITMAPINFO bmpinfo1;                            //用來存放HEADER資訊 
    BMPimage1 = LoadBitmapFile("C:\\Users\\allenhsu\\Desktop\\new drum\\HandViewer\\drum.bmp", &bmpinfo1); 
    BMPwidth1 = bmpinfo1.bmiHeader.biWidth; 
    BMPheight1 = bmpinfo1.bmiHeader.biHeight; 


	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutIdle);

	////////////////////////////////////////////
	char filename[]="5.txt";
    fp.open(filename, ios::out);//開啟檔案
    if(!fp){//如果開啟檔案失敗，fp為0；成功，fp為非0
       cout<<"Fail to open file: "<<filename<<endl;
	}
}
void KBcallback(unsigned char key, int x, int y){


}
void initMIDI(){

    int i, keyPress;
    int nPorts;
    char input;
    midiout = new RtMidiOut();
	// Check available ports.
    nPorts = midiout->getPortCount();
    if ( nPorts == 0 ) {
        cout << "No ports available!" << endl;
    }
        // List Available Ports
    cout << "\nPort Count = " << nPorts << endl;
    cout << "Available Output Ports\n-----------------------------\n";
    for( i=0; i<nPorts; i++ )
    {
        try {
                cout << "  Output Port Number " << i << " : " << midiout->getPortName(i) << endl;
        }
        catch(RtError &error) {
            error.printMessage();
        }
    }

	cout << "\n The choosen port is 1" << endl;
        //cin >> keyPress;
        // Open Selected Port
        midiout->openPort(2);
        keyPress = NULL;
}

unsigned char *LoadBitmapFile(char *fileName, BITMAPINFO *bitmapInfo) 
{ 
   FILE               *fp; 
   BITMAPFILEHEADER   bitmapFileHeader;    //Bitmap file header 
   unsigned char      *bitmapImage;        //Bitmap image data 
   unsigned int       lInfoSize;           //Size of information 
   unsigned int       lBitSize;            //Size of bitmap 

   unsigned char change; 
   int pixel; 
   int p=0; 

   fp = fopen(fileName, "rb");  
   fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);              //讀取 bitmap header 第一個參數為陣列或結構的指標，第二個參數為陣列或結構的大小，第三個參數為陣列的元素數量，如果是結構就等同 1 個陣列元素，第四個參數為指向結構 FILE 的指標。

   lInfoSize = bitmapFileHeader.bfOffBits - sizeof(BITMAPFILEHEADER);      //Info的size 
   fread(bitmapInfo, lInfoSize, 1, fp); 


   lBitSize = bitmapInfo->bmiHeader.biSizeImage;                           //配置記憶體 
   bitmapImage = new BYTE[lBitSize]; 
   fread(bitmapImage, 1, lBitSize, fp);                                    //讀取影像檔 

   fclose(fp); 

   //此時傳回bitmapImage的話，顏色會是BGR順序，下面迴圈會改順序為RGB 
   pixel = (bitmapInfo->bmiHeader.biWidth)*(bitmapInfo->bmiHeader.biHeight); 

   for( int i=0 ; i<pixel ; i++, p+=3 ) 
   { 
      //交換bitmapImage[p]和bitmapImage[p+2]的值 
      change = bitmapImage[p]; 
      bitmapImage[p] = bitmapImage[p+2]; 
      bitmapImage[p+2]  = change; 
   } 

   return bitmapImage; 
} 

void playdrum(){

    for(int i =0 ;i<=7; i++)
	{
		if(last_note[i] != note_on[i] && note_on[i] == true )
		{	
			midiMessage.push_back (144) ;
			midiMessage.push_back (drum_notes[i]);  
			midiMessage.push_back (90); 
			midiout->sendMessage( &midiMessage );
			midiMessage.clear();
		}
	}
}