#include <windows.h>
#include <stdio.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <chrono>
#include<winsock2.h>

#define peque 1
//define mostrar 1
#define vivo 1
#define comms 1
#define NUM_TEMPLATES 24
#define TIMER LARGE_INTEGER

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define SERVER "127.0.0.1"  //ip address of udp server
#define BUFLEN 100 //Max length of buffer
#define PORT 13370   //The port on which to listen for incoming data

using namespace std;
using namespace cv;

ifstream Lector;
//const char piezas[NUM_TEMPLATES ]={'B','B','b','b','N','N','n','n','Q','Q','q','q','P','P','p','p','K','K','k','k','R','R','r','r'}; //viejitas
const char piezas[NUM_TEMPLATES ]={'p','p','P','P','R','R','r','r','B','B','b','b','N','N','n','n','K','K','k','k','Q','Q','q','q'};
vector <string> nomTemplates(NUM_TEMPLATES );
vector <Mat> templates(NUM_TEMPLATES );
TIMER frequency;

//------------prototipos---------------------------------
float comparar_cuadro_a_pieza( Mat &cuadro, Mat  &templ_pieza, int lado);
bool existe (const std::string&);
Mat extraer_de( Mat &referencia , Mat &templ);
string formatear(string pgn, unsigned tipo);
string extraer_posicion(Mat &tablero, vector <Mat> &templs,const char chars_pzas[ ],vector <string> &nombresTemplates );
IplImage* ScreenCap_Ipl(void);
void tiempo(LARGE_INTEGER *);
void frecuencia(LARGE_INTEGER *);
double diferencia_tiempo(TIMER t2, TIMER t1);

//--------------------------------------------------------------
class SuperSock
{
public:
	SuperSock(void){
	initializeWinSock();
	createSocket();
	setupAddressStructure();
	}
	void initializeWinSock() {
	  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }}
     void createSocket() {
         if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
    {
        printf("socket() failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }}
    void setupAddressStructure() {
      memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
    }
    void sendMessage(string mensaje) {
        strcpy(message, mensaje.c_str());
      if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
    }
    string recibirRespuesta() {
     memset(buf,'\0', BUFLEN);
        //try to receive some data, this is a blocking call
       if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR)
        {
            printf("recvfrom() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        return buf;
    }

private:
    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    WSADATA wsa;
};

class miRobot
{
 public:
     miRobot() {
         //tipos 0,1: blancas, negras
      Input = {0};
      Input.type = INPUT_MOUSE;
      fScreenWidth    = ::GetSystemMetrics( SM_CXSCREEN )-1;
      fScreenHeight  = ::GetSystemMetrics( SM_CYSCREEN )-1;
     }

void click ( )
{
  ::ZeroMemory(&Input,sizeof(INPUT));
  Input.mi.dwFlags  = MOUSEEVENTF_LEFTDOWN;
  ::SendInput(1,&Input,sizeof(INPUT));
}
void soltar ( )
{
  ::ZeroMemory(&Input,sizeof(INPUT));
  Input.mi.dwFlags  = MOUSEEVENTF_LEFTUP;
  ::SendInput(1,&Input,sizeof(INPUT));
}

int convertir(char& c){
switch (c){
case 'a':
    return 0;
case 'b':
    return 1;
case 'c':
    return 2;
case 'd':
    return 3;
case 'e':
    return 4;
case 'f':
    return 5;
case 'g':
    return 6;
case 'h':
    return 7;
default:
    return 9;
}
}

int convertir_n(char& c){
switch (c){
case 'a':
    return 7;
case 'b':
    return 6;
case 'c':
    return 5;
case 'd':
    return 4;
case 'e':
    return 3;
case 'f':
    return 2;
case 'g':
    return 1;
case 'h':
    return 0;
default:
    return 9;
}
}

void mover(string jugada, int color) {
    // 1 para blancas, 0 para negras
 if(color == 1) {
    x1 = convertir(jugada[0]);
    x2 = convertir(jugada[2]);
    y1 = jugada[1]-'0';
    y2 = jugada[3]-'0';
 } else if (color == 0) {
    x1 = convertir_n(jugada[0]);
    x2 = convertir_n(jugada[2]);
    y1 = jugada[1]-'0'; y1=9-y1;
    y2 = jugada[3]-'0';  y2 =9-y2;
 }
  mover_a(xinit+x1*d,yinit+d*8-d*y1);
  Sleep(20);
  click();
  Sleep(20);
  mover_a(xinit+x2*d,yinit+d*8-d*y2);
  Sleep(20);
  soltar();
  mover_a(xinit+16*d,yinit+d*4);
  click();
  soltar();
  //Sleep(35);
}

 private:
     int d=67;
     int xinit = 76;
     int yinit = 214;
     int x1, x2, y1, y2;
     double fScreenWidth;
     double fScreenHeight;
     double fx;
     double fy;
     INPUT Input;
     void mover_a (int x, int y )
{
    ::ZeroMemory(&Input,sizeof(INPUT));
	Input.mi.dwFlags  = MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE;
    fx = x*(65535.0f/fScreenWidth);
    fy = y*(65535.0f/fScreenHeight);
	Input.mi.dx = fx;
	Input.mi.dy = fy;
	::SendInput(1,&Input,sizeof(INPUT));
}
};

int main( int argc, char** argv )
{

frecuencia(&frequency);
miRobot robotin;
SuperSock miSock;
Mat control = Mat::zeros(300,300, CV_8UC1);
    string temp;
    double factor = 0.4;
    Lector.open("C:/Ejecutables/templates.txt");
    for(unsigned i=0; getline(Lector, temp ) ; i++ ){
            nomTemplates[ i ] = temp;
            templates[ i ] = imread("C:/Ejecutables/templ_chess/"+temp, CV_LOAD_IMAGE_GRAYSCALE);
           #ifdef peque
            resize(templates[i], templates[i], Size(), factor, factor);
            #endif // peque
    }

Mat templ_tablero_vacio = imread( "C:/Ejecutables/templ_tablero_vacio.jpg", CV_LOAD_IMAGE_GRAYSCALE );
#ifdef peque
resize(templ_tablero_vacio, templ_tablero_vacio,Size(), factor,factor);
#endif

bool blancas = true; //es para la oscilacion
imshow("Control", control);
while(true) {
char ccc = waitKey(0);
if (ccc == 'a') blancas = true;
else {blancas = false;}
TIMER t1;
tiempo(&t1);

#ifdef vivo
IplImage* scap= ScreenCap_Ipl();
Mat img(scap);
if(img.empty())
{
 cout << "Matriz vacia - saliendo" << endl;
 return 0;
}
cvtColor(img,img, CV_BGR2GRAY);
#else
Mat img = imread("chess_piezas3.jpg", CV_LOAD_IMAGE_GRAYSCALE) ;
#endif
TIMER t2;
tiempo(&t2);
#ifdef peque
resize(img,img,Size(), factor,factor);
#endif
TIMER t3;
tiempo(&t3);
Mat tablero=extraer_de(img,templ_tablero_vacio);
string pgn   = extraer_posicion(tablero, templates, piezas, nomTemplates );
//if(argc > 1) pgn = formatear(pgn, atoi(argv[1]) );
//else pgn = formatear(pgn, 1);
if(blancas) pgn = formatear(pgn, 1);
else {pgn = formatear(pgn, 2);}
TIMER t4;
tiempo(&t4);

cout << " Tiempos: " << endl;
cout << " Screen Cap: " <<diferencia_tiempo(t2,t1) << endl;
cout << " Extraccion y formateo: " << diferencia_tiempo(t4,t3) << endl;
//cout << " Total: " << diferencia_tiempo(t4,t1) << endl << endl;
cout << pgn << endl;
TIMER T5;
tiempo(&T5);
miSock.sendMessage(pgn);
string MJ = miSock.recibirRespuesta();
TIMER T6;
tiempo(&T6);
if(blancas) robotin.mover(MJ,1);
else {robotin.mover(MJ, 0); }
TIMER T7;
tiempo(&T7);
cout << "Respuesta Sock: \""<<MJ <<"\""<< endl;
cout << "Tiempo Respuesta: " << diferencia_tiempo(T6,T5) << endl;
cout << "Tiempo Bot: " << diferencia_tiempo(T7,T6) << endl;
cout << "Tiempo total: " << diferencia_tiempo(T7, t1) << endl << endl;

}
  return 0;
}

string extraer_posicion(Mat &tablero, vector <Mat> &templs,const char chars_pzas[ ] ,vector <string> &nombresTemplates) {

const unsigned dimension = tablero.rows, lado = (tablero.rows/8) ;
unsigned indice;
double rate, rate_temp;
bool espacio;
string PGN;
const Scalar negro(0);


  for(unsigned j=2; j < dimension-10; j+= lado) {
  for(unsigned i=2,distancia=0, cnt=0; i < dimension-10; i+= lado, cnt+=1)
    {
    rate=rate_temp=0; //declararlos en el ciclo for causara error
    Mat cuadrito = tablero(Rect ( Point(i,j) , Point(i+lado,j+lado) )  )  ;

    LARGE_INTEGER start;
    tiempo(&start);

for(unsigned k = 0 ; k <= NUM_TEMPLATES -1 ; k++) {
        rate_temp=comparar_cuadro_a_pieza( cuadrito, templs [ k ] , lado);
   if(rate_temp>rate){
        rate = rate_temp;
        indice = k;
        if(rate>0.75)break;
   }
   //cout << nombresTemplates[  k ]  << " : " << rate_temp << " | " << rate << " | indice=" << indice << endl ;
}

LARGE_INTEGER fin;
tiempo(&fin);
//double interval = static_cast<double>(fin.QuadPart - start.QuadPart) / frequency.QuadPart;
//printf("Tiempo = %f \n", interval);

   #ifdef mostrar
    rectangle( tablero, Point(i,j) , Point(i+lado,j+lado) , negro , 2, 8, 0 );
    #endif

    if(rate<0.46)
    {
        espacio=true;
        distancia++;
        if(cnt==7)
        {
            char num=(char)(((int)'0')+distancia);
            espacio=false;
            PGN+=num;
            PGN+='/';
            distancia=0;
        }
}
else {
        if(espacio==true)
                {
                char num=(char)(((int)'0')+distancia);
                espacio=false;
                PGN+=num;
                distancia=0;
                }
        PGN+=chars_pzas[indice];
        if( cnt==7)PGN+='/';
}
#ifdef mostrar
cout << "Pieza detectada: " << nombresTemplates[indice] << " con " << rate << endl;
cout << PGN << endl;
cout << " ---- --- ----- ------ ----- ----- ---- ---- " << endl;
imshow( "Tablero de verdad" , tablero);
imshow ( " cuadrito " , cuadrito ) ;
imshow ( " template " , templs[indice] ) ;
waitKey(0);
#endif // mostrar
        }
  }
  return PGN;
}

float comparar_cuadro_a_pieza ( Mat &cuadro, Mat  &templ_pieza, int lado) {
    double mini, maxi;
 const int resultados_cols =  lado - templ_pieza.cols + 1;
 const int resultados_rows = lado - templ_pieza.rows + 1;
  Mat result( resultados_cols, resultados_rows, CV_32FC1 );
  matchTemplate( cuadro , templ_pieza, result, CV_TM_CCOEFF_NORMED );//CV_TM_CCOEFF_NORMED
  minMaxIdx(result, &mini, &maxi);
  return maxi;
}

string formatear(string pgn, unsigned tipo) {
    //modalidades:
    //1 blancas desde blancas
    //2 negras desde negras
    //3 negras desde blancas
 string pgn2;
 if (tipo==1) pgn2 = "position fen " + pgn.substr (0,pgn.length()-1) +  " w - - 0 1";

 else if(tipo==2)
    {
    reverse(pgn.begin(),pgn.end());
    pgn2 = "position fen " + pgn.substr(1,pgn.length()) +  " b - - 0 1";
    }

  else if(tipo==3) pgn2 = "position fen " + pgn.substr(0,pgn.length()-1) +  " b - - 0 1";
  return pgn2;
}

Mat extraer_de( Mat &referencia , Mat &templ) {
 const int result_cols =  referencia.cols - templ.cols + 1;
 const int result_rows = referencia.rows - templ.rows + 1;
  Mat result_TABLERO( result_cols, result_rows, CV_32FC1 );
  matchTemplate( referencia, templ, result_TABLERO, CV_TM_CCOEFF_NORMED );
 double minVal; double maxVal; Point minLoc; Point maxLoc;
  minMaxLoc( result_TABLERO, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
  Mat tablero = referencia(Rect(maxLoc, Point( maxLoc.x + templ.cols , maxLoc.y + templ.rows )  )  );
  return tablero;
}

inline bool existe (const std::string& name) {
   if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

IplImage* ScreenCap_Ipl()
{
IplImage *img;
int nWidth;
int nHeight;
HWND hWnd;

if((hWnd =FindWindow( NULL, "Play Live Chess - Chess.com - Google Chrome")) == NULL)
{
printf("No encontre la ventana :(\n");
return 0;
}
GetLastError();
HDC hDC = GetDC(hWnd);
HDC hMemDC = CreateCompatibleDC(hDC);
RECT rect;
if(GetWindowRect(hWnd, &rect) )
{
nWidth = rect.right - rect.left;
nHeight = rect.bottom - rect.top;
}

HBITMAP hBitmap = CreateCompatibleBitmap( hDC, nWidth, nHeight);
BITMAPINFO bmi;
ZeroMemory(&bmi, sizeof(bmi));
bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
bmi.bmiHeader.biWidth = nWidth;
bmi.bmiHeader.biHeight = -(nHeight);
bmi.bmiHeader.biBitCount = 32;
bmi.bmiHeader.biPlanes = 1;
bmi.bmiHeader.biCompression = BI_RGB;
bmi.bmiHeader.biSizeImage = 32 * nWidth * nHeight / 8;
BYTE *pbBits = new BYTE[bmi.bmiHeader.biSizeImage];
if( hBitmap)
{
HBITMAP hOld = (HBITMAP)SelectObject( hMemDC, hBitmap);
cout << "ancho" << nWidth << " -- altura" <<nHeight << endl;
BitBlt( hMemDC, 0, 0, nWidth, nHeight, hDC, 0, 0, SRCCOPY);
SelectObject( hMemDC, hOld);
BITMAP bbmp;
GetObject( hBitmap, sizeof(BITMAP), &bbmp);
int nChannels = bbmp.bmBitsPixel == 1 ? 1 : bbmp.bmBitsPixel/8 ;
int depth = bbmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;
GetDIBits(hDC, hBitmap, 0, nHeight, pbBits, &bmi,DIB_RGB_COLORS);
DeleteObject(hBitmap);
img = cvCreateImageHeader( cvSize(bbmp.bmWidth, bbmp.bmHeight), depth, nChannels );
img ->imageData = (char*)malloc(bbmp.bmHeight * bbmp.bmWidth * nChannels
* sizeof(char));
memcpy( img->imageData,(char*)(pbBits), bbmp.bmHeight * bbmp.bmWidth *
nChannels);
}

delete[] pbBits;

::DeleteDC(hDC);
::DeleteDC(hMemDC);
::DeleteObject(hWnd);

return img;
}

void tiempo(LARGE_INTEGER *miTiempo)
{
    if (::QueryPerformanceCounter(miTiempo) == FALSE)
    throw "err timer";
    return;
}

void frecuencia(LARGE_INTEGER *miFrecuencia)
{
    if (::QueryPerformanceFrequency(miFrecuencia) == FALSE)
    throw "err freq";
    return;
}

double diferencia_tiempo(TIMER t2, TIMER t1) {
   double t21 = static_cast<double>(t2.QuadPart - t1.QuadPart) / frequency.QuadPart;
   return t21;
}

