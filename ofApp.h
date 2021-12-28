#pragma once

#include "ofMain.h"
#include "time.h"
typedef pair<int, int> ii;

struct player{
    int x, y; // location of the player
    //could be more information here
};

typedef struct enemy* Enemy;
struct enemy{
    int spawnX, spawnY; // location of enemy spawn
    int destX, destY; // destination of enemy - to player
    int x, y; // location ov enemy
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void processOpenFileSelection(ofFileDialogResult openFileResult); // no need
};

//functions for displaying maze, player and enemy
bool distanceCheck(ii a, ii b);
void drawMaze(int N, int M);
int drawLinezWithArr(vector<ii> dotz);
void drawPlayer(int x, int y);
void drawEnemy(int x, int y);

//functions for initializing
void initEnemy(void);
void initStage(void);
void initMaze(void);

//function for moving player
int checkToMove(int x, int y);

//functions for managing enemy
void modified_search(int i, int j);
int modified_checkToMove(int x, int y);
void modified_dfs(void);
void updateEnemy(void);

//functions for making maze
void makeMaze(void);
int returnDir(int i, int j);
void rbt(int i, int j);
void runRbt(void);

//function for freeing memory
void freeEverything(void);

//extra function
void printMaze(int N, int M);
int absoluteValue(int a);
