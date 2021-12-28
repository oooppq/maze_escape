//all the comments are written in English
//because Korean comments could be broken when this is downloaded to other computers

#include "ofApp.h"
#include <utility>

#define HEIGHT 12 // can change the height of maze
#define WIDTH 12 // can change the width of maze
#define BASICLEVEL 3 // can adjust  default speed
typedef pair<int, int> ii;

//------global variables------
int mazeHeight = 0, mazeWidth = 0; // save WIDTH HEGIHT, mazeHeight will be used as flag
char** mazeArr; // saving the structure of maze
int** vis; // check if the location is visited
vector<ii> solutionRoute; //saveing solution route - searched by dfs
vector<ii> stackRbt; // save the route for back tracking
vector<ii> dir; // save direction - east, west, south, north
struct player P; // save player info in P
Enemy E; // save enemy info in E
ii goal; // location of goal
int routeIdx = 1; // route
int gameOverFlag = 0, nextLevelFlag = 0; // flag for game over and moving to next level
int level = 1 + BASICLEVEL; // variable to save level
int spawnTimeCount = 0; //  check cool time to re-spawn enemy
int enemySpawnFlag = 0; //  flag for eney spawn
int nextLevelTimeCount = 0; // check cool time to move to next level
int startingFlag = 0; // flag for starting game
int startingTimeCount = 0; // check time to ready for game
//----------------------------
//------scale adjustment------

inline int mazeArrI(int i){ // adjust number to fit in mazeArr
    return 2*i + 1;
}

inline int mazeArrJ(int j){ // adjust number to fit in mazeArr
    return 2*j + 1;
}

inline int screenX(int x){ // adjust number to display location
    return 30*(2*x + 1);
}

inline int screenY(int y){ // adjust number to display location
    return 30*(2*y + 1);
}
//----------------------------
//--initialization for variables--
void initEnemy(void){ // initialize the informations of enemy
    E = (Enemy)malloc(sizeof(struct enemy));
    if(!E)
        printf("Mem Error\n");
    //randomly spawn
    E->spawnX = rand() % (mazeWidth - 1);
    E->spawnY = rand() % (mazeHeight - 1);
    int distanceX = absoluteValue(E->spawnX - P.x);
    int distanceY = absoluteValue(E->spawnY - P.y);
    // prevent spawning enemy too closely from player and in same spot with goal
    while(distanceX + distanceY < 6 || (E->spawnX == goal.second && E->spawnY == goal.first))
    {
        E->spawnX = rand() % (mazeWidth - 1);
        E->spawnY = rand() % (mazeHeight - 1);
        distanceX = absoluteValue(E->spawnX - P.x);
        distanceY = absoluteValue(E->spawnY - P.y);
    }
    // enemy's gonna move smoothly, so should adjust
    E->x = screenX(E->spawnX);
    E->y = screenX(E->spawnY);
    E->destX = P.x;
    E->destY = P.y;
    //initialize flag, spawn time and routeIdx
    spawnTimeCount = 0;
    gameOverFlag = 0;
    routeIdx = 1; // routedIdx always start from 1, because enemy should always follow this route
}

void initStage(void){// initalize stage - player loc, gol loc, enemy info
    mazeHeight = HEIGHT;
    mazeWidth = WIDTH;
    //player always from 0,0
    P.x = 0;
    P.y = 0;
    //goal located randomly
    goal.first = rand() % (mazeHeight - 1);
    goal.second = rand() % (mazeWidth - 1);
    initEnemy();

    enemySpawnFlag = 1; //spawn enemy
    modified_dfs(); //find way to player
    nextLevelTimeCount = 0; //next level counting initialize
}

void initMaze(void) // allocating memory for maze, make maze
{
    gameOverFlag = 0;
    startingFlag = 1;
    nextLevelFlag = 0;
    mazeArr = (char**)malloc(sizeof(char *) * (mazeArrI(HEIGHT) + 1));
    if(!mazeArr) printf("Mem Error\n");
    for(int i = 0; i < (mazeArrI(HEIGHT) + 1); i++){
        mazeArr[i] = (char*)malloc(sizeof(char) * (mazeArrJ(WIDTH) + 1));
        if(!mazeArr[i]) printf("Mem Error\n");
    }

    vis = (int**)malloc(sizeof(int *) * (HEIGHT + 1));
    if(!vis) printf("Mem Error\n");
    for(int i = 0; i < HEIGHT; i++){
        vis[i] = (int*)malloc(sizeof(int) * (WIDTH + 1));
        if(!vis) printf("Mem Error\n");
    }
    level = 1 + BASICLEVEL; // level initialize
    // pushing direction info
    dir.push_back(ii(0, 1));
    dir.push_back(ii(0, -1));
    dir.push_back(ii(1, 0));
    dir.push_back(ii(-1, 0));
    makeMaze(); // make basic structure of maze
    runRbt(); // make maze by recursive back tracking algorithm
    initStage(); // initialize other info
    
}
//----------------------------
//------extra functions-------
void printMaze(int N, int M){ // print maze array on compliler
    for(int i = 0; i <= N*2; i++){
        for(int j = 0; j <= M*2; j++){
            printf("%c", mazeArr[i][j]);
        }
        printf("\n");
    }
}

int absoluteValue(int a){ // function same as abs()
    if(a < 0) return -a;
    return a;
}
//----------------------------
//functions for displaying maze, player and enemy
bool distanceCheck(ii a, ii b){ // check the distance between two spot
    int c1 = a.first - b.first;
    int c2 = a.second - b.second;
    c1 = absoluteValue(c1);
    c2 = absoluteValue(c2);
    if(c1 + c2 == 1)
    {
        int middleX = (mazeArrI(a.first) + mazeArrI(b.first)) / 2 ;
        int middleY = (mazeArrJ(a.second) + mazeArrJ(b.second)) / 2 ;
        //if wall exists between two dots, should not be treated as connected
        if(mazeArr[middleX][middleY] == '-' || mazeArr[middleX][middleY] == '|')
            return false; // there's a wall between two dot, so could not move
        return true; // the two spots are adjacent each other, so could move
    }
    return false; // the two spots are not adjacent each other
}


void drawMaze(int N, int M){ //display maze
    //좌표마다 오른쪽과 아래쪽을 그리면됨.
    for(int i = 0; i <= N*2 - 1; i+=2){
        for(int j = 0; j <= M*2 - 1; j+=2){
            if(mazeArr[i][j+1] == '-'){
                ofDrawLine(j * 30, i * 30, (j+1) * 30 + 30, i * 30);
            }
            if(mazeArr[i+1][j] == '|'){
                ofDrawLine(j * 30, i * 30, j * 30, (i+1) * 30 + 30);
            }
        }
    }
    for(int j = 0; j <= M*2 - 1; j+=2){
        ofDrawLine(j * 30, 2*N * 30, (j+1) * 30 + 30, 2*N * 30);
    }
    for(int i = 0; i <= N*2 - 1; i+=2){
        ofDrawLine(M*2 * 30, i * 30, M*2 * 30, (i+1) * 30 + 30);
    }
}


int drawLinezWithArr(vector<ii> dotz){ // draw line with between two dots(i, i + 1)
    //int scaleFactor = 60;
    int sf = 60;
    for(int i = 0; i < dotz.size() - 1; i++){
        if(distanceCheck(dotz[i], dotz[i+1])){ // the two adjacent each other
            ofDrawLine(30 + dotz[i].second * sf, 30 + dotz[i].first * sf,
                30 + dotz[i+1].second * sf, 30 + dotz[i+1].first * sf);
        }
        else{ //when drawing every route. for this version, isn't needed
            int connect = i+1;
            int j;
            for(j = 0; j < i; j++){
                if(distanceCheck(dotz[connect], dotz[j]))
                    break;
            }
            ofDrawLine(30 + dotz[j].second * sf, 30 + dotz[j].first * sf,
            30 + dotz[connect].second * sf, 30 + dotz[connect].first * sf);
        }
    }
}

void drawPlayer(int x, int y) // draw character of player
{
    ofSetColor(0, 0, 0);
    ofDrawCircle(x, y, 10);
    ofSetColor(250, 250, 250);
    ofDrawCircle(x, y, 7);
    ofSetColor(0, 0, 0);
    ofDrawCircle(x, y, 4);
    ofSetColor(250, 250, 250);
    ofDrawCircle(x, y, 1);
}

void drawEnemy(int x, int y) // draw character of enemy
{
    ofSetColor(0, 0, 250);
    ofDrawCircle(x, y, 10);
    ofSetColor(250, 0, 0);
    ofDrawCircle(x, y, 5);
}
//----------------------------
//functions for making maze---
void makeMaze(void) // make basic structure of maze
{
    for(int i = 0; i <= HEIGHT*2; i+=2){
        for(int j = 0; j < WIDTH*2; j+=2){
            mazeArr[i][j] = '+';
            mazeArr[i][j+1] = '-';
        }
        mazeArr[i][WIDTH*2] = '+';
    }
    
    for(int i = 1; i < HEIGHT*2; i+=2){
        for(int j = 0; j < WIDTH*2; j+=2){
            mazeArr[i][j] = '|';
            mazeArr[i][j+1] = ' ';
        }
        mazeArr[i][WIDTH*2] = '|';
    }
}

int returnDir(int i, int j) // randomly returning direction
{
    // first, randomly assign direction, if moving to that direction isn't available, check other dir
    int d = rand() % 4;
    if(i + dir[d].first >= 0 && i + dir[d].first < HEIGHT && j + dir[d].second >= 0 && j + dir[d].second < WIDTH && !vis[i + dir[d].first][j + dir[d].second])
        return d;
    d = (d + 2) % 4;
    if(i + dir[d].first >= 0 && i + dir[d].first < HEIGHT && j + dir[d].second >= 0 && j + dir[d].second < WIDTH && !vis[i + dir[d].first][j + dir[d].second])
        return d;
    d = (d + 1) % 4;;
    if(i + dir[d].first >= 0 && i + dir[d].first < HEIGHT && j + dir[d].second >= 0 && j + dir[d].second < WIDTH && !vis[i + dir[d].first][j + dir[d].second])
        return d;
    d = (d + 2) % 4;
    if(i + dir[d].first >= 0 && i + dir[d].first < HEIGHT && j + dir[d].second >= 0 && j + dir[d].second < WIDTH && !vis[i + dir[d].first][j + dir[d].second])
        return d;
    return  - 1; // there's no direction where i, j can move to
}
void rbt(int i, int j){ // Recursive Back Tracking algorithm, using vector to save route that has been visited
    if(i >= HEIGHT || j>= WIDTH || i < 0 || j < 0) // check if i, j is on the range
        return;
    vis[i][j] = 1; //  i, j visited
    int direction = returnDir(i, j); // get direction
    int adjustI = dir[direction].first; // get y(= i) info of direction
    int adjustJ = dir[direction].second; // get x(= j) info of direction
    if(direction == - 1) // could not move to anywhere
    {
        if(!stackRbt.size()) //back tracking is over
            return;
        ii temp = stackRbt[stackRbt.size() - 1]; // => latest road saved at stackRbt
        stackRbt.pop_back();
        rbt(temp.first, temp.second); // back track to the latest road
    }
    else
    {
        stackRbt.push_back(ii(i, j)); // push to the stackRbt
        mazeArr[mazeArrI(i) + adjustI][mazeArrJ(j) + adjustJ] = ' '; // crush wall
        rbt(i + adjustI, j + adjustJ); // go to the next
    }
}
void runRbt(void) // to manage rbt easily
{
    for(int i = 0; i < HEIGHT; i++) // initialize
        for(int j = 0; j < WIDTH; j++)
            vis[i][j] = 0;
    stackRbt.clear(); // clear stackRbt just in case
    rbt(0, 0); // start from 0,0
    printMaze(HEIGHT, WIDTH);
}
//----------------------------
//-------free Memory----------
void freeEverything(void) // free all dynamically allocated memories
{
    if(mazeArr) // if maze == NULL, for loop below would not be executed, so have to check
    {
        for(int i = 0; i < (mazeArrI(HEIGHT) + 1); i++)
            free(mazeArr[i]);
        free(mazeArr);
    }
    if(vis) // if vis == NULL, for loop below would not be executed, so have to check
    {
        for(int i = 0; i < HEIGHT; i++)
            free(vis[i]);
        free(vis);
    }
    solutionRoute.clear(); // .clear does not mean that memory freeing
    solutionRoute.shrink_to_fit(); // so have to shrink memory
    stackRbt.clear();
    stackRbt.shrink_to_fit();
    dir.clear();
    dir.shrink_to_fit();
    if(free)
        free(E);
}
//----------------------------

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60); // Limit the speed of our program to 15 frames per second
    ofBackground(255,255,255);
    srand(time(NULL));
}

//--------------------------------------------------------------
void ofApp::update(){
    
}

//--------------------------------------------------------------
void ofApp::draw(){
        ofSetLineWidth(10);
    ofSetColor(0, 0, 0);
    if(startingFlag) //when this game starts, can get ready-for-game time for 2 secs.
    {
        if(startingTimeCount >= 45) // time done
        {
            startingFlag = 0;
            startingTimeCount = 0;
        }
        else
        {
            //show starting display
            ofDrawBitmapString("Start!", 300, 300);
            ofDrawBitmapString("Level "+ofToString((int)(level - BASICLEVEL)), 300, 350);
            startingTimeCount++; // count time
        }
        return;
    }
    if(mazeHeight && !nextLevelFlag && !gameOverFlag) // execute game when maze made, nextlevel cool time done, game not over
    {
        drawMaze(mazeHeight, mazeWidth);
        ofDrawCircle(screenX(goal.second), screenY(goal.first), 15); // draw goal
        ofSetColor(250, 250, 250);
        ofDrawBitmapString("G", screenX(goal.second) - 4, screenY(goal.first) + 4);
        if(enemySpawnFlag) // if flag on, enemy displayed
        {
            updateEnemy();
            ofSetColor(250,0, 0);
            ofDrawCircle(screenX(E->spawnX), screenY(E->spawnY), 10);
            drawEnemy(E->x, E->y);
        }
        else // flag off == cool time for enemy
        {
            if(spawnTimeCount >= 45) // cool time checking
            {
                initEnemy();
                modified_dfs();
                enemySpawnFlag = 1;
            }
            else spawnTimeCount++;
        }
        drawPlayer(screenX(P.x), screenY(P.y)); // draw player
        
    }
    else if(gameOverFlag == 1) //  game is over
    {
        ofDrawBitmapString("Game Over!", 300, 300);
        ofDrawBitmapString("Your Level: "+ofToString((int)(level - BASICLEVEL)), 300, 350);
    }
    else if(nextLevelFlag) // if flag on, time to prepare for next stage
    {
        if(nextLevelTimeCount >= 45) // time checking
        {
            makeMaze();
            runRbt();
            initStage();
            nextLevelFlag = 0;
            level++;
        }
        else
        {
            ofDrawBitmapString("Next Level", 300, 300);
            ofDrawBitmapString("Level "+ofToString((int)(level - BASICLEVEL + 1)), 300, 350);
            nextLevelTimeCount++;
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'v' || key == 'V') {
        // HACK: only needed on windows, when using ofSetAutoBackground(false)
        glReadBuffer(GL_FRONT);
        ofSaveScreen("savedScreenshot_"+ofGetTimestampString()+".png");
    }
    if (key == 'q' || key == 'Q'){
        // Free the dynamically allocated memory exits.
        freeEverything();
        cout << "Dynamically allocated memory has been freed." << endl;
        _Exit(0);
    }
    if (key == 's' || key == 'S'){ // game start key
        if(mazeHeight) // after starting game, this button is useless
            return;
        initMaze();
    }
    if(mazeHeight)
    {
        if (key == 'r' || key == 'R') // reset all, start from the first level
        {
                freeEverything();
                initMaze();
        }
        //keys for player movement
        if (key == OF_KEY_RIGHT){
            if(checkToMove(mazeArrI(P.x) + 1, mazeArrJ(P.y)))
                P.x++;
        }
        if (key == OF_KEY_LEFT){
            if(checkToMove(mazeArrI(P.x) - 1, mazeArrJ(P.y)))
                P.x--;
        }
        if(key == OF_KEY_UP){
            if(checkToMove(mazeArrI(P.x), mazeArrJ(P.y) - 1))
                P.y--;
        }
        if(key == OF_KEY_DOWN){
            if(checkToMove(mazeArrI(P.x), mazeArrJ(P.y) + 1))
                P.y++;
        }
        if(mazeHeight && goal == ii(P.y, P.x)) //during playing game, if player touched the goal, then go to next level
            nextLevelFlag = 1;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    /* doesn't need to load file
    if( key == 'l' || key == 'L'){
        // Open the Open File Dialog
        ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a only maz for Maze");
        
        // Check whether the user opened a file
        if( openFileResult.bSuccess){
            ofLogVerbose("User selected a file");
            
            // We have a file, so let's check it and process it
            processOpenFileSelection(openFileResult);
            //cload_flag = 1;
            //dfsDoneFlag = 0;
            //bfsDoneFlage = 0;
            solutionRoute.clear();
            everyRoute.clear();
        }
    }
    */
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}

//--------------------------------------------------------------
void ofApp::processOpenFileSelection(ofFileDialogResult openFileResult) {
    /* doesn't need to load file
     string fileName = openFileResult.getName();
     ofFile file(fileName);
     
     if( !file.exists()) cout << "Target file does not exists." << endl;
     else cout << "We found the target file." << endl;
     
     ofBuffer buffer(file);
     
     // Read file line by line
     int i = 0;
     for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
         string line = *it;
         int j = 0;
         for(auto&ch : line){
             mazeArr[i][j] = ch;
             //printf("%c", ch);
             j++;
         }
         //printf("\n");
         mazeWidth = j;
         i++;
     }
     mazeHeight = i;
     mazeHeight = (mazeHeight - 1) / 2;
     mazeWidth = (mazeWidth - 1) / 2;
     printMaze(mazeHeight, mazeWidth);
     printf("%d %d\n", mazeHeight, mazeWidth);
     */
}

//----function for Player-----
int checkToMove(int x, int y){ // check if player could move to the location
    if(mazeArr[y][x] == ' ')
        return 1;
    else
    {
        return 0;
    }
}
//----------------------------
//----functions for Enemy-----
void modified_search(int i, int j){ //find route to player's location
    
    if(i >= mazeHeight || j >= mazeWidth || i < 0 || j < 0) return; // i, j should be in the range of maze
    if(vis[i][j]) return;
    if(solutionRoute.size() >= 1) //don't search after finding exit.
        if(solutionRoute[solutionRoute.size() - 1] == ii(E->destY, E->destX))
            return;
    vis[i][j] = 1;
        
    solutionRoute.push_back(ii(i, j)); // push this road
    //find way searching 4 directions
    if(mazeArr[mazeArrI(i)+1][mazeArrJ(j)] == ' '){
        modified_search(i+1, j);
    }
    if(mazeArr[mazeArrI(i)][mazeArrJ(j)+1] == ' '){
        modified_search(i, j+1);
    }
    if(mazeArr[mazeArrI(i)-1][mazeArrJ(j)] == ' '){
        modified_search(i-1, j);
    }
    if(mazeArr[mazeArrI(i)][mazeArrJ(j)-1] == ' '){
        modified_search(i, j-1);
    }
    if(solutionRoute[solutionRoute.size() - 1] != ii(E->destY, E->destX)) //if we find the route, should not remove
        solutionRoute.pop_back(); //have to remove wrong route in solution stack.
}

int modified_checkToMove(int x, int y){ // checking method for movement of enemy
    //following solution route, so, have to check if enemy touch 'routeIdx'th route.
    if(screenX(solutionRoute[routeIdx].second) != x || screenY(solutionRoute[routeIdx].first) != y)
    {
        if(screenX(solutionRoute[routeIdx].second) != x && screenY(solutionRoute[routeIdx].first) == y)
        {
            if(screenX(solutionRoute[routeIdx].second) > x) return 1;
            else return 2;
        }
        if(screenX(solutionRoute[routeIdx].second) == x && screenY(solutionRoute[routeIdx].first) != y)
        {
            if(screenY(solutionRoute[routeIdx].first) > y) return 3;
            else return 4;
        }
    }
    else return 0;
}

void modified_dfs(void){ //find solution route from enemy to player
    
    for(int i = 0; i < mazeHeight; i++) //initialize array vis[][]
    {
        for(int j = 0; j < mazeWidth; j++)
            vis[i][j] = 0;
    }
    solutionRoute.clear(); //clear data just in case
    modified_search(E->spawnY, E->spawnX); //find route from enemy spawned locaiton
}

void updateEnemy(void) // updating the location of enemy
{
    int check = modified_checkToMove(E->x, E->y);
    if(check) // can move
    {
        
        switch(check)
        {
            case 1: // enemy is in left side of destination
            {
                E->x += level; // if level up, speed up
                if(E->x >= screenX(solutionRoute[routeIdx].second)) //prevent jumping the saved location
                    E->x = screenX(solutionRoute[routeIdx].second);
                break;
            }
            case 2: //enemy is in right side of destination
            {
                E->x -= level;
                if(E->x <= screenX(solutionRoute[routeIdx].second))
                    E->x = screenX(solutionRoute[routeIdx].second);
                break;
            }
            case 3: //enemy is in down side of destination
            {
                E->y += level;
                if(E->y >= screenY(solutionRoute[routeIdx].first))
                    E->y = screenY(solutionRoute[routeIdx].first);
                break;
            }
            case 4: //enemy is in up side of destination
            {
                E->y -= level;
                if(E->y <= screenY(solutionRoute[routeIdx].first))
                    E->y = screenY(solutionRoute[routeIdx].first);
    
                break;
            }
            default:
            {
                //prepare for unknown error
                printf("Program Error\n game restarted\n");
                int tmp = level;
                initMaze();
                level = tmp;
                
            }
        }
        if(E->x == screenX(P.x) && E->y == screenY(P.y)) // if enemy catch player, game over
        {
            gameOverFlag = 1;
            return;
        }
        if(!modified_checkToMove(E->x, E->y)) // could not move anymore
        {
            if(E->x == screenX(E->destX) && E->y == screenY(E->destY)) //if enemy touched destination, disappear and wait for re-spawn
            {
                free(E);
                enemySpawnFlag = 0;
            }
            else routeIdx++; // move to next road
        }
    }
    else
    {
        //prepare for unknown error
        printf("Program Error\n game restarted\n");
        int tmp = level;
        initMaze();
        level = tmp;
    }
}
//----------------------------
