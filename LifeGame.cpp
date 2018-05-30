// LifeGame.cpp
// Javier Chauvin 

#include <iostream>
#include <vector>
#include <cmath>
#include "fssimplewindow.h"
#include "ysglfontdata.h"

#define UNIVERSE_WEIGHT 1600
#define UNIVERSE_HEIGHT 1200
#define CELL_SIZE 2

using namespace std;

enum BODIES{
	B_ERROR,
	B_SHOT_GUN,
	B_SQUARE_S,
	B_GRANADE_S,
	B_LINE,
};

class BitMap{
private:
	int hei, wid;
	int cellSize;
	vector<char> cells, cellsPlusOne;
public:
	BitMap(int Width, int Height, int SizeOfCell);
	~BitMap(){}; //Destructuctor not necesary. Dynamic memory handled with vector.

	//Always references to the actual bitmap. (cells)
	void SetCell(int x, int y, char CellStatus); 

	//Receives coordinates of the cell to set (x and y) in BitMap coordinates coordinates.
	//Actual represents the actual Bitmap (cells) or the t-1 BitMap (cellsPlusOne)
	void SetCell(int x, int y, char CellStatus, bool Actual);

	//Receives coordinates of the cell to set (x and y) in BitMap coordinates coordinates.
	//Returns the cell value
	char GetCell(int x, int y);

	//Apply the life rules to determine who is alive and who is dead. 
	void CheckState(void);
	void Draw();

	//Kill all the cells and pause the generations iteraction
	void Erase(void);

	//Make a selection 
	//Receives initial position (xo,yo) and final position (xe,ye) in screen coordinates
	//State true for revive cells or false to kill cells
	void Select(int xo, int yo, int xe, int ye, char State);

	//Draw a square to let user know what is selecting. Made to run in real time. 
	//Receives initial position (xo,yo) and final position (xe,ye) in screen coordinates.
	void DrawSlection(int xo, int yo, int xe, int ye);
};

BitMap::BitMap(int Width, int Height, int SizeOfCell){
	cellSize = SizeOfCell;
	hei = Height/cellSize;
	wid = Width/cellSize;
	
	cells.assign(hei*wid,0);
	cellsPlusOne.assign(hei*wid,0);
}

void BitMap::SetCell(int x, int y, char CellStatus){

	if(	0 <= x && x < wid &&
		0 <= y && y < hei ){
		cells[y*wid+x] = CellStatus;
	}
}

void BitMap::SetCell(int x, int y, char CellStatus, bool Actual){
	if(	0 <= x && x < wid &&
		0 <= y && y < hei ){
		if (Actual){
			cells[y*wid+x] = CellStatus;
		}
		else{
			cellsPlusOne[y*wid+x] = CellStatus;
		}
	}
}

//Returns the cell's value of a x,y BitMap Coordinate 
char BitMap::GetCell(int x, int y){
	
	if(	0 <= x && x < wid &&
		0 <= y && y < hei ){
		return cells[y*wid+x];
	}
	return 0;
}


//Analyze each cell and count the alive neighbors. 
//Using the alive neighbors apply the following 3 rules:
//1. If a cell is alive and has 2 or 3 alive neighbors, it survives. 
//2. If a cell is dead and has exactly 3 alive neighbors, it revives. 
//3. In any other case the cell dies or remains dead.

//All the changes are done in cellPlusOne which contains the cells state T+1
//Once the ne state is found T+1 is the new state T. 

//TODO:Make algorithm more efficient.Now, Alalize 8 sourranding cells of each cell.
//	Only the first cell should analyze all sourranding cells; after that, It should 
//	store the common cells for the next analyzed one. 
void BitMap::CheckState(void){
	char AliveNeighbors = 0;
	char IsAlive = 0;

	//Visit each cell
	for (int j=0; j<hei; j++){
		for (int i=0; i<wid; i++){
			//Visit each of 8 neighbors 
			for (int l=j-1; l<=j+1; l++){
				for (int m=i-1; m<=i+1; m++){
					
					if ( (0 <= l && l <= hei) &&	//Should be inside the screen
						 (0 <= m && m <= wid) &&
						 (GetCell(m,l)		)	){	//The cell is alive 
						if (l == j && m == i){ //The cell under study
							IsAlive = 1;
						} else{			//The neighboors
							AliveNeighbors++;
						}
					}

				}
			}

			//Supervivencia 
			if ((IsAlive) && 
				(2 == AliveNeighbors || 3 == AliveNeighbors) ){
				// Continuew Alive
					SetCell(i,j,1,false);
			}
			else if ((!IsAlive) &&
					 (3 == AliveNeighbors)){
				//New born
				SetCell(i,j,1,false);
			}
			//else if (3 != AliveNeighbors){
			else{
				//Cell dies or remain unlinving
				SetCell(i,j,0,false);
			}
			IsAlive = 0;
			AliveNeighbors = 0;

		}
	}
	cells = cellsPlusOne;
}

//Draws first the quads and then the lines. 
//The board is divided in hei*wid. It is necessary only one vertex, from the the other three are calculated.
//eahc square is draw inside the fond coordinates. 
//Vertical and horizantal lines are drawn one each cellSice distance
void BitMap::Draw(){

	glBegin(GL_QUADS);
	for(int y=0; y<hei; ++y){
		for(int x=0; x<wid; ++x){
			if (GetCell(x,y)){
				glColor3ub(0,255,25);
			} else {
				glColor3ub(16,16,16);
			}
			
			glVertex2i(x*cellSize			,y*cellSize);
			glVertex2i(x*cellSize+cellSize	,y*cellSize);
			glVertex2i(x*cellSize+cellSize	,y*cellSize+cellSize);
			glVertex2i(x*cellSize			,y*cellSize+cellSize);
		}
	}
	glEnd();

	//glBegin(GL_LINES);
	//glColor3ub(128,128,128);
	//for (int x=0; x<=wid; ++x){
	//	glVertex2i(x*cellSize,0);
	//	glVertex2i(x*cellSize,hei*10);

	//	glVertex2i(0			,x*cellSize);
	//	glVertex2i(wid*cellSize	,x*cellSize);
	//}
	glEnd();
}

//Assign cero to each cell in the Bitmap. 
//It is not neceesary assign to T+1 BitMap since this is calculated from T.
void BitMap::Erase(void){
	cells.assign(hei*wid,0);
}

//Analyzes each pixel from the initial mouse selection to the end mouse selection, and
//set the cell according to state. It could be zero or one
void BitMap::Select(int xo, int yo, int xe, int ye, char State){
	
	//TODO: Recode eficiently. Now Set the cell once for each pixel selected aproximatly
	for (int j=min(yo,ye); j<=max(yo,ye); j++){
		for ( int i=min(xo,xe); i<=max(xo,xe); i++ ){
			SetCell(i/cellSize,j/cellSize,State);
		}
	}
}

//
void BitMap::DrawSlection(int xo, int yo, int xe, int ye){

	int xPrint, yPrint, widPrint, heiPrint;
	char Selection[10];
	char buffer[5];

	//Get the initial and final value of x and y in case the selection was made 
	//from right to left or from buttom to top. 
	int xini = min(xo,xe), xend = max(xo,xe);
	int yini = min(yo,ye), yend = max(yo,ye);

	//Get the horizontal and vertical distance of the selection
	heiPrint = (xend-xini)/cellSize;
	widPrint = (yend-yini)/cellSize;

	//If the selection distance in pixels is not devisible by cellSize means the remainder 
	//is greater than zero. In this case the user is selecting one more cell.
	widPrint += 0<(xend%cellSize) ? 1:0;
	heiPrint += 0<(yend%cellSize) ? 1:0;

	//Selection string concatenation. 
	//The information is presented in format "hei x wid"
	_itoa_s(widPrint,buffer,10);
	strcpy_s(Selection, buffer);
	strcat_s(Selection, " x ");
	_itoa_s(heiPrint,buffer,10);
	strcat_s(Selection, buffer);

	//Draw dimensions of selection square
	//in the upper right corner. 
	glColor3d(0,255,0);
	glRasterPos2d(xend+cellSize,yini-cellSize);
	YsGlDrawFontBitmap8x12(Selection);

	//Draw Selection square
	glColor4ub(0,255,25,128);
	glBegin(GL_QUADS);
	glVertex2d(xo,yo);    
	glVertex2d(xe,yo);  
	glVertex2d(xe,ye); 
	glVertex2d(xo,ye);
	glEnd();

	//Draw Border of the selection square
	glColor3ub(0,255,50);
	glBegin(GL_LINES);
	glVertex2d(xo,yo);
	glVertex2d(xe,yo);

	glVertex2d(xe,yo);
	glVertex2d(xe,ye);

	glVertex2d(xe,ye);
	glVertex2d(xo,ye);

	glVertex2d(xo,ye);
	glVertex2d(xo,yo);
	glEnd();

}

//class Info{
//private:
//	int numOfGenerations;
//	int 
//public:
//	Info();
//	~Info(){};
//	void PlotInfo( void );
//};

int main(){

	FsOpenWindow(16,16,UNIVERSE_WEIGHT,UNIVERSE_HEIGHT,1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	BitMap Universe(UNIVERSE_WEIGHT,UNIVERSE_HEIGHT,CELL_SIZE); //The importat object. Creation of bitmap 800x600 with cell size of 5 pixels
	int speed = 5;				//Speed between each generation
	bool Run = false;			//Flag. Indicates if the cell universe is running
	bool addingBody = false;	//Flag.	Indicates if the user is adding a new boddy to the cell pattern. 
								//Not used for dawring the pattern.
	
	vector <vector<char>> Undo;

	int mxo = 0, myo = 0; //Mause coordinates origen
	bool Slecting = false;

	for(;;){

		FsPollDevice();
		auto key=FsInkey();

		int lb,mb,rb,mx,my;
		auto evt=FsGetMouseEvent(lb,mb,rb,mx,my);

		/////////////////////////////////////////////////////////////
		// Slection

		if (FSMOUSEEVENT_LBUTTONDOWN == evt ||
			FSMOUSEEVENT_RBUTTONDOWN == evt	){
			mxo = mx;
			myo = my;
			Slecting = true;
			cout<<"Init "<<mxo<<" "<<myo<<endl;
		}

		if (FSMOUSEEVENT_LBUTTONUP == evt){
			Universe.Select(mxo,myo,mx,my,1);	
			cout <<"End "<<mxo<<" "<<myo<<" "<<mx<<" "<<my<<" "<<endl;
			mxo = 0;
			myo = 0;
			Slecting = false;
		}

		if (FSMOUSEEVENT_RBUTTONUP == evt){
			Universe.Select(mxo,myo,mx,my,0);	
			cout <<"End "<<mxo<<" "<<myo<<" "<<mx<<" "<<my<<" "<<endl;
			mxo = 0;
			myo = 0;
			Slecting = false;
		}
		/////////////////////////////////////////////////////////////

		if(FSKEY_ESC==key){
			break;
		}
		if(FSKEY_DEL==key){
			Universe.Erase();	
			Run = false;
		}
		if(FSKEY_ENTER==key){
			if (Run){
				Run = false;
			}
			else {
				Run = true;
			}
		}

		//if(FSKEY_2==key){
		//	//Insert square
		//	Bodies body2Add(B_SQUARE_S);
		//	addingBody = true;	
		//}
		if(FSKEY_UP==key && speed<10){
			speed++;
		}
		if(FSKEY_DOWN==key && 1<speed){
			speed--;
		}
		if(FSKEY_CTRL==key && FSKEY_Z){
			if (!Run){
				// TODO Create undo structure
			}
		}

		if (Run) {
			Universe.CheckState();
		}

		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

		Universe.Draw();
		if(Slecting){
			Universe.DrawSlection(mxo,myo,mx,my);
		}

		FsSwapBuffers();
		FsSleep(100/speed);
	}

	return 0;
}
