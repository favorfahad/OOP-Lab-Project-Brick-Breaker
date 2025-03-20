#include<iostream>
#include"conio.h"
#include"dos.h"
#include"stdlib.h"
#include<string>
#include"windows.h"
#include<time.h>

#define SCREEN_WIDTH 52
#define SCREEN_HEIGHT 20

#define MIN_X 2
#define MIN_Y 2
#define MAX_X 49
#define MAX_Y 19 

using namespace std;    

HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
COORD CursorPosition;

int Bricks[24][2] = { 
{2,7}, {2,12}, {2,17}, {2,22}, {2,27}, {2,32}, {2,37}, {2, 42},
{4,7}, {4,12}, {4,17}, {4,22}, {4,27}, {4,32}, {4,37}, {4, 42},
{6,7}, {6,12}, {6,17}, {6,22}, {6,27}, {6,32}, {6,37}, {6, 42}
};

int visibleBricks[24] = {1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1};
int sliderPosition[2] = {18, 22};
int ballPosition[2] = {17, 26};
int startball = 0;
int direction = 1; //1 for Top Right, 2 for Top Left, 3 for Bottom left, 4 for Bottom right
int bricksleft = 24;
int win = 0;
int loose = 0;
int score = 0;

void gotoxy(int x, int y) {
    CursorPosition.X = x;
    CursorPosition.Y = y;
    SetConsoleCursorPosition(console, CursorPosition);
}

void ScoreBoard(int score) {
	gotoxy(0, 22);
	cout << "Score: " << score;
}

void setcursor(bool visible, DWORD size) {
    if (size == 0) {
        size = 20;
    }
    CONSOLE_CURSOR_INFO lpCursor;
    lpCursor.bVisible = visible;
    lpCursor.dwSize = size;
    SetConsoleCursorInfo(console, &lpCursor);
}

void drawborder() {
    gotoxy(0,0); cout << "-----------------------------------------------------";
    gotoxy(0, SCREEN_HEIGHT); cout << "-----------------------------------------------------";
    for(int i=1;i<SCREEN_HEIGHT;i++) {
        gotoxy(0, i); cout << "|";
        gotoxy(SCREEN_WIDTH, i); cout << "|";
    }
}

void drawBricks() {
    for(int i=0; i<24; i++) {
        if (visibleBricks[i] == 1) {
            gotoxy(Bricks[i][1], Bricks[i][0]);
            cout << "±±±±"; 
        }
    }
}

void BallHitSlider() {
    if (ballPosition[1] >= sliderPosition[1] && ballPosition[1] <= sliderPosition[1] + 8) {
        if (ballPosition[0] == sliderPosition[0] - 1) {
            if (direction == 3) {direction = 2;}
            else if (direction == 4)
            {
                direction = 1;
            }
            
        }
    }
}

void BallHitBrick() {
    for(int i=0;i<24;i++) {
        if(visibleBricks[i] == 1) {
            if(ballPosition[1] >= Bricks[i][1] && ballPosition[1] <= Bricks[i][1] + 8) {
                if(ballPosition[0] == Bricks[i][0]) {
                    visibleBricks[i] = 0;
                    bricksleft--;
                }
            }
        }
    }
}

void play() {
    while(1) {
        system("cls");
        drawBricks();
        drawborder();
        ScoreBoard(score);
        gotoxy(sliderPosition[1], sliderPosition[0]);
        cout << "±±±±±±±±±";

        gotoxy(ballPosition[1], ballPosition[0]);
        cout << "0";
        if (kbhit()) {
        	if (sliderPosition[1] < SCREEN_WIDTH-2) {
            char ch = getch();
            if (ch == 'd' || ch == 'D' || ch == 77) {
                if (sliderPosition[1] < 44)
                        sliderPosition[1] = sliderPosition[1]+5;
            }
            if (ch == 'a' || ch == 'A' || ch == 75) {
                if (sliderPosition[1] > 2)
                        sliderPosition[1] = sliderPosition[1]-5;
            }
            if (ch == 32) {
                startball = 1;
            }
            if (ch == 27) {
                break;
            }
         }
        }
        if (startball == 1) {
            if (direction == 1) { //Top right
                ballPosition[0] = ballPosition[0] - 1;
                ballPosition[1] = ballPosition[1] + 2;
                if (ballPosition[1] > MAX_X) {
                    direction = 2;
                }   
                else if (ballPosition[0] < MIN_Y) {
                    direction = 4;
                }
            } else if (direction == 2) {// Top left
                ballPosition[0] = ballPosition[0] - 1;
                ballPosition[1] = ballPosition[1] - 2;
                if (ballPosition[1] < MIN_X) {
                    direction = 1;
                }   
                else if (ballPosition[0] < MIN_Y) {
                    direction = 3;
                }
            } else if (direction == 3) { //Bottom left
                ballPosition[0] = ballPosition[0] + 1;
                ballPosition[1] = ballPosition[1] - 2;
                if (ballPosition[0] > MAX_Y) {
                    loose = 1;
                    break;
                }   
                else if (ballPosition[1] < MIN_X) {
                    direction = 4;  
                }
            } else if (direction == 4) {//Bottom right
                ballPosition[0] = ballPosition[0] + 1;
                ballPosition[1] = ballPosition[1] + 2;
                if (ballPosition[1] > MAX_X) {
                    direction = 3;
                }   
                else if (ballPosition[0] >  MAX_Y) {
                    loose = 1;
                    break;
                }
            }
            BallHitSlider();
        }	
        BallHitBrick();
        score = 24 - bricksleft;
        if (bricksleft == 0) {
            win = 1;
            break;
        }
        Sleep(30);
    }

    if (loose == 1) {
    	loose = 0;
        system("cls");
        
        gotoxy(10, 5); cout << "------------------------";
        gotoxy(10, 6); cout << "|      YOU LOST!       |";
        gotoxy(10, 7); cout << "------------------------";

        gotoxy(10, 9); cout << "Press any key to go back to menu.";
        getch();
    }

    if (win == 1) {
        system("cls");

        gotoxy(10, 5); cout << "------------------------";
        gotoxy(10, 6); cout << "|      YOU WON!       |";
        gotoxy(10, 7); cout << "------------------------";

        gotoxy(10, 9); cout << "Press any key to go back to menu.";
        getch();
    }
}

void instructions() {
	system("cls");
	cout << "Instructions";
	cout << "\n--------------";
	cout << "\n1. Use 'a' key to move the slider to the left.";	
	cout << "\n1. Use 'd' key to move the slider to the right.";
	cout << "\n3. Press spacebar to start the game";
	cout << "Press any key to go back to menu.";
	getch();	
}

int main () {
    setcursor(0,0);

    do {
        system("cls");
        gotoxy(10,5); cout << "   -------------------------------  ";
        gotoxy(10,6); cout << "   |       BRICK BREAKER         | ";
        gotoxy(10,7); cout << "   -------------------------------  ";
        gotoxy(10, 9); cout << "1. Start the Game.";
        gotoxy(10,10); cout << "2. Control Manual.";
        gotoxy(10,11); cout << "3. Quit";

        gotoxy(10,13); cout << "Select an option: ";
        char opt = getche();
        if (opt == '1') 
            play();
        else if (opt == '2')
        	instructions();
        else if (opt == '3') {
        	cout << "Thank you for playing :)" << endl;
            exit(0); }
        else {
        	cout << "Invalid Option.";
        }
    } while(1);
    play();
    cout << endl << endl;
    return 0;
}
