#include<iostream>
#include<conio.h>
#include<dos.h>
#include<stdlib.h>
#include<string.h>
#include <windows.h>
#include <time.h>
#include <vector>
#include <thread>
#include <fstream>
using namespace std;
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
COORD CursorPosition;
//		cout<<"���������";
/* map
    0--0---0---0--0
	|  |   |   |  | // 0 mean ground
	0--0--[B]--0--0 // B mean blank space (enemies can pass through)
	|  |   |   |  | // | and -- mean path(enemy can pass through)
	0--0-0-P-0-0--0 // P mean player position
	// kẻ địch mỗi lượt chỉ có thể đi đến vùng đất gần 
	// các vị trí được đánh dấu theo thứ tự tăng dần, từ trái sang phải
	// từ trên xuống dưới
	// 0 là vùng đất, P là vị trí của người chơi, B là vùng trống
	// các vị trí được đánh dấu theo thứ tự tăng dần, từ trái sang phải
	đánh dấu vị trí: 
	0--1---2---3--4
	|  |   |   |  |
	5--6--[B]--7--8
	|  |   |   |  | 
	9--10-11-P-12-13--14
*/
/*
=== Cursor Position ===
 | 1 | 2 | 3 | 4 | 5 |        Temperature: 75┬░f
 |  -- --- --- --    |        Fan: ON toggle with 'x' key
 | |  |       |  |   |
 |  --         --    |
 | |  |       |  |   |        Sound:#
 |  -- - -P- - --    |
 |-------------------|
 |                   |
*/
int soundPlayerMade = 0;
long long tick = 0;
bool player_stat=1;
pair <int, int> cpo[15]; // to store the cursor position
void setupcursor()
{
	cpo[0] = {4,3};
	cpo[1] = {7,3};
	cpo[2] = {11,3};
	cpo[3] = {15,3};
	cpo[4] = {18,3};
	cpo[5] = {4,5};
	cpo[6] = {7,5};
	cpo[7] = {15,5};
	cpo[8] = {18,5};
	cpo[9] = {4,7};
	cpo[10] = {7,7};
	cpo[11] = {9,7};
	cpo[12] = {13,7};
	cpo[13]= {15,7};
	cpo[14] = {18,7};

}
/* ví dụ:
	kẻ địch từ 0 có thể đến 1hoặc 5
	kẻ địch từ 1 có thể đến 0,2,6...
	// kẻ địch từ 11,12 có thể trực tiếp tấn công người chơi 
    GUI:
    computer's gui:
        123456789012345678901

    1   =====================
    2   | 1 | 2 | 3 | 4 | 5 |
    3   |task:              |
    4   |q: maintenance     |
    5   |w: advertisement   |
    6   |e: security        |
    7   |-------------------|
    8   |space: log out     | // log out only available when player finished all tasks
    9   =====================

    computer's gui on traking enemy:
        123456789012345678901
		=====================
		| 1 | 2 | 3 | 4 | 5 |
		|  -- --- --- --    |
		| |  |       |  |   |
		|  --         --    |
		| |  |       |  |   |
		|  -- - -P- - --    |
		|-------------------|
		|                   |
    // các ô trống sẽ được thay thế bằng số kể địch đang đứng ở vị trí đó

    computer's gui audio bait mode:
        123456789012345678901
        
    1   =====================
    2   | 1 | 2 | 3 | 4 | 5 |
    3   |  0--0---0---0--0  |
    4   |  |  |   |   |  |  |
    5   |  0--0--[B]--0--0  | // B mean blank space (enemies can't pass through)
    6   |  |  |   |   |  |  |
    7   |  0--0-0-P-0-0--0  | // P mean player position
    8   |-------------------|
    9   |type the pos:      |

*/
vector <int> map[16];// sử dụng vector để lưu các đường đi của enemy
void initMap()
{
	map[0].push_back(1);
	map[0].push_back(5);
	map[1].push_back(0);
	map[1].push_back(2);
	map[1].push_back(6);
	map[2].push_back(1);
	map[2].push_back(3);
	map[2].push_back(7);
	map[3].push_back(2);
	map[3].push_back(4);
	map[3].push_back(8);
	map[4].push_back(3);
	map[4].push_back(8);
	//map[5].push_back(0);
	map[5].push_back(6);
	map[5].push_back(9);
	//map[6].push_back(1);
	map[6].push_back(5);
	map[6].push_back(10);
	map[7].push_back(13);
	map[7].push_back(8);
	//map[8].push_back(3);
	map[8].push_back(7);
	map[8].push_back(14);
	map[9].push_back(10);
	// map[10].push_back(9);
	map[10].push_back(11);
	//map[10].push_back(6);
	// map[11].push_back(10);
	// map[11].push_back(12);
	// map[11].push_back(7);
	// map[12].push_back(11);
	// map[12].push_back(13);
	// map[12].push_back(8);
	map[13].push_back(12);
	map[14].push_back(13);
	//map[14].push_back(8);
}

char player_vision;
// hướng người chơi đang nhìn:
// 'l' = left
// 'r' = right
// 'm' = middle/computer





// Play multiple sounds simultaneously using PlaySound with SND_ASYNC | SND_FILENAME | SND_NODEFAULT
// To play more than one sound at the same time, use different sound channels (by using PlaySound with SND_ASYNC in different threads)
// Example function to play a sound asynchronously:
struct Enemy {
    bool status = true; // status of enemy, true = alive, false = dead
	int id; // id of enemy
	int pos; // position of enemy
    int startPos; // start position of enemy
	string name; // name of 
    int cooldown; // cooldown time for enemy to attack
};
Enemy diddy{

	.id = 1,
	.pos = 0,
	.startPos = 0,
	.name = "Diddy",
	.cooldown = 5
};
Enemy justin_Bieber{
	.id = 0,
	.pos = 1,
	.startPos = 1,
	.name = "Justin Bieber",
	.cooldown = 5
};
Enemy LeBron_James{
	.id = 2,
	.pos = 4,
	.startPos = 4,
	.name = "LeBron James",
	.cooldown = 5
};
Enemy ambatukam{
	
	.id = 3,
	.pos = 3,
	.startPos = 3,
	.name = "DreamyBull",
	.cooldown = 5
};
void gameover(string enermy_name,int pos,int tick){
	system("cls");
	cout<<endl;
	cout<<"\t\t--------------------------"<<endl;
	cout<<"\t\t-------- Game Over -------"<<endl;
	cout<<"\t\t--------------------------"<<endl<<endl;
    // Display the enemies that attacked the player
	cout<<"\t\tYou died to: "<<enermy_name<<" from: "<<pos<<endl;
	cout<<"\t\tPress any key to go back to menu.";
	player_stat=0;
	
	int score = 0;
	ifstream fin("highscore.txt");
	if (fin.is_open()) {
		fin >> score;
		fin.close();
	}

	if (tick > score) {
		cout << endl << "New highscore: " << tick << endl;
		ofstream fout("highscore.txt");
		if (fout.is_open()) {
			fout << tick;
			fout.close();
		}
	} else {
		cout << "Your score: " << tick << endl;
	}
	getch();
	
}
bool enemyAttack(Enemy e) {
	if (soundPlayerMade == 0) return false; // Không thể tấn công nếu không có tiếng động

	if (e.pos == 11) {
		if (player_vision == 'l') {
			return false; // Người chơi né được
		} else {
			// Tỉ lệ tấn công thành công: soundPlayerMade/3
			int chance = rand() % 3;
			if (chance < soundPlayerMade)
				return true;
			else
				return false;
		}
	} else if (e.pos == 12) {
		if (player_vision == 'r') {
			return false; // Người chơi né được
		} else {
			int chance = rand() % 3;
			if (chance < soundPlayerMade)
				return true;
			else
				return false;
		}
	}
	return false;
}
int enemyMoveRNG(Enemy &e) {
    // make enemy a change that have 50% chance to move
    if (rand() % 2 == 0) {
        return e.pos; // enemy does not move
    }
	if (e.pos == 11 || e.pos == 12) {
		bool c = enemyAttack(e);
		if (c) {
			gameover(e.name,e.pos,tick);
			// enemy successfully attacked player
		}
		else {
			e.pos = e.startPos; // reset enemy position to start position
			// player successfully dodged the attack
		}
	}
	else {
		int rng = rand() % map[e.pos].size(); // random number from 0 to size of map[e.pos]
		int newPos = map[e.pos][rng]; // new position of enemy
		e.pos = newPos; // update position of enemy
	}
	// check if enemy is at position 11 or 12, if so, it can attack player
	
	return e.pos; // return new position of enemy
}
// if enemy pos is 11 or 12, enemy can attack player

void gotoxy(int x, int y)
{
	CursorPosition.X = x;
	CursorPosition.Y = y;
	SetConsoleCursorPosition(console, CursorPosition);
}

void setcursor(bool visible, DWORD size) 
{
	if(size == 0)
		size = 20;	
	
	CONSOLE_CURSOR_INFO lpCursor;	
	lpCursor.bVisible = visible;
	lpCursor.dwSize = size;
	SetConsoleCursorInfo(console,&lpCursor);
}
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
void showcomputerGUI() {//show computer's GUI
    system("cls");
    cout<<   " =====================";
    cout<<"\n | 1 | 2 | 3 | 4 | 5 |";
    cout<<"\n |task:              |";
    cout<<"\n |q: maintenance     |";
    cout<<"\n |w: advertisement   |";
    cout<<"\n |e: security        |";
    cout<<"\n |-------------------|";
    cout<<"\n |space: log out     |"; // log out only available when player finished all tasks
}
void showcomputerGUIonTrackingEnemy() {//show computer's GUI on tracking enemy
    system("cls");
    cout<<  " =====================";
    cout<<"\n | 1 | 2 | 3 | 4 | 5 |";
    cout<<"\n |  -- --- --- --    |";
    cout<<"\n | |  |       |  |   |";
    cout<<"\n |  --         --    |"; // B mean blank space (enemies can't pass through)
    cout<<"\n | |  |       |  |   |";
    cout<<"\n |  -- - -P- - --    |"; // P mean player position
    cout<<"\n |-------------------|";

    // replace the number of enemies standing on the positions
    /*
    ví dụ: tìm số kẻ địch ở vị trí 0
    bước 1: duyệt qua tất cả các kẻ địch
    nếu status của kẻ địch là true (còn sống) và vị trí của kẻ địch là 0
    thì tăng biến cnt thêm 
    Bước 2: in ra số lượng kẻ địch ở vị trí 0
    dùng hàm gotoxy(x,y) để di chuyển con trỏ đến vị trí cần in
    in ra số lượng kẻ địch ở vị trí đó
    */
    int cnt = 0; // count the number of enemies standing on the position
    for (int i=0;i<=14;i++){
        cnt = 0; // reset count
        for (Enemy e : {diddy, justin_Bieber, LeBron_James, ambatukam}) {
            if (e.status && e.pos == i) { // if enemy is alive and standing on the position
                cnt++; // increase count
            }
        }
        int x = cpo[i].first-1; // get x position of the cursor
		int y = cpo[i].second-1; // get y position of the cursor
        gotoxy(x, y); // move cursor to the position
        cout << cnt; // print the number of enemies standing on the position
    }
}

void showblankgui(){
	system("cls");
	cout<<  " =====================";
	cout<<"\n |                   |";
	cout<<"\n |                   |";
	cout<<"\n |                   |";
	cout<<"\n |                   |";
	cout<<"\n |                   |";
	cout<<"\n |                   |"; 
	cout<<"\n |-------------------|";
}

void showRightSide(){
    system("cls");
    cout<<  "|--------------------|";
    cout<<"\n||>    `            R|";
    cout<<"\n||>                  |";
    cout<<"\n||>              ,   |";
    cout<<"\n||>     .            |";
    cout<<"\n||>           .      |";
    cout<<"\n||>            -     |";
    cout<<"\n---------------------|";
}
void showLeftSide(){
	system("cls");
	cout<<  "|--------------------|";
	cout<<"\n|L    `            <||";
	cout<<"\n|                  <||";
	cout<<"\n|          0       <||";
	cout<<"\n|     .            <||";
	cout<<"\n|           .      <||";
	cout<<"\n|                  <||";
	cout<<"\n|--------------------|";
}
void instructions(){
	
	system("cls");
	cout<<"Instructions";
	cout<<"\n----------------";
	cout<<"\n Can you survive the night?";
	cout<<"\n\n1. Use 'a' to look left, 'd' to look right, 's' to look at the computer.";
	cout<<"\n2. Use 'z' to toggle the computer on/off.";
	cout<<"\n3. Use 'x' to toggle the fan on/off.";
	cout<<"\n4. Use '1' to switch to task mode, '2' to switch to tracking enemy mode.";
	cout<<"\n5. Use 'space' to log out when all tasks are done.";
	cout <<"\n6. tip 1:to be silent, try to turn off the computer and fan if you being surrounded by enemies.";
	cout<<"\n7. tip 2: if you are being attacked by an enemy, try to look at the direction of the enemy to dodge the attack.";
	cout<<"\n\nPress any key to go back to menu";
	getch();
}
void showTemperature(int temperature) {
	gotoxy(30, 1); // move cursor to the top right corner
	cout << "Temperature: " << temperature << "°f"; // display temperature
}
void showFanStatus(bool isFanOn) {
	gotoxy(30, 2); // move cursor to the next line
	if (isFanOn) {
		cout << "Fan: ON"; // display fan status
	} else {
		cout << "Fan: OFF"; // display fan status
	}
	cout << " toggle with 'x' key"; // display toggle instruction
}
void showNoise(int soundPlayerMade) {
	gotoxy(30, 5); // move cursor to the next line
	cout << "Sound: ";
	for (int i = 0; i < soundPlayerMade; i++) {
		cout << "#"; // display noise made by player
	}
}
void debug(char a,long long tick){
	gotoxy(30,6);
	cout <<"PLayer's view: "<<a;
	gotoxy(30,7);
	cout <<"tick: "<<tick;
}
void play(){
    // Initialize the map
    initMap();
	setupcursor();
	//int soundPlayerMade = 0;
	//vector<Enemy> enemies = {diddy, justin_Bieber, LeBron_James, ambatukam};
	tick=0;
	bool isFanOn = true; // fan is off at the start
	bool iscomputerOn = true; // computer is on at the start
	// Initialize the tasks
	string task_maintenance[3] = { "Order baby oil", "clean rooms", "Reboot system" };
	string task_advertisement[3] = { "make ads", "find customers", "Send to customers" };
	string task_security[3] = { "Check cameras", "Check doors", "Check lights" };
    int computer_mode = 1;
	int temperature=80; // temperature of the computer, start at 60
	int task_done = 0; // number of tasks done by player
	int task_total = 3; // total number of tasks
	bool canPlayerLogOut = false; // player can log out only when all tasks are done
    // Initialize the player vision
	string dead_message = "You died to: "; // message to display when player is dead
    player_vision = 'm'; // player starts in the middle
    // Initialize the enemies
    //vector<Enemy> enemies = {diddy, justin_Bieber, LeBron_James, ambatukam};

	while(1){
		tick++;
		soundPlayerMade = 0; // reset sound player made
		
		if (isFanOn) {
			soundPlayerMade ++;
			if (tick % 25 == 0) { // every 10 ticks, fan makes sound
				// PlaySound("fan.wav", NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
				 // sound player made
				if (temperature > 60) {
					temperature--; // decrease temperature if fan is on
				}
			}
		}
		if (iscomputerOn){
			soundPlayerMade++;
		}
		if (tick % 50 == 0) { // every 50 ticks, increase temperature
			temperature++;
		}
		showFanStatus(isFanOn); // show fan status
		showTemperature(temperature); // show temperature
		showNoise(soundPlayerMade); // show noise made by player
		debug(player_vision,tick);
		// everty 100 ticks - 1 cooldown for enemies
		Enemy* enemies[] = {&diddy, &justin_Bieber, &LeBron_James, &ambatukam};
		if (tick % 20 == 0) {
			for (Enemy* e : enemies) {
				if (e->cooldown > 0) {
					e->cooldown--; // decrease cooldown for enemy
				}
			}
		}
		// move enemies
		for (Enemy* e : enemies) {
			if (e->status && e->cooldown == 0) { // if enemy is alive and cooldown is 0
				enemyMoveRNG(*e); // move enemy
				//diddy.pos = 11;
				e->cooldown = 5; // reset cooldown for enemy
			}
		}
		if (temperature >85 && temperature <= 100) {
			system("color 2");
		}
		else if (temperature > 100&& temperature <= 110) {//yellow
			system("color 6");
		}
		else if (temperature > 110) {
			// change color to red
			system("color 4");
		}
		else if (temperature <= 85) {
			system("color 07");
		}
		if (player_vision == 'm') {
			if (iscomputerOn) {
				switch (computer_mode) {
				case 1: // task mode
					showcomputerGUI();
					break;
				case 2: // tracking enemy mode
					showcomputerGUIonTrackingEnemy();
					break;
				default:
					break;
				}
			} else {
				showblankgui(); // show blank GUI
			}
		}
		if (player_vision == 'l') {
			showLeftSide(); // show left side
		}
		if (player_vision == 'r') {
			showRightSide(); // show right side
		}
		if(kbhit()){
			char ch = getch();
			if(ch=='a'){// player look to the left
				player_vision = 'l';
			}
			if(ch=='d'){// player look to the right
				player_vision = 'r';
			}
            if(ch=='s'){
                player_vision = 'm';
                computer_mode = 1; // task mode
            }
            if (player_vision=='m'&& ch=='2'){
                computer_mode = 2; // switch to tracking enemy mode
            }
			if (player_vision =='m'&&ch=='1'){
                computer_mode = 1; // switch to task mode
            }
			if(player_vision=='m'&&ch==' ' && canPlayerLogOut){

				break;
			}
			if (ch=='z') {
				iscomputerOn = !iscomputerOn; // toggle computer status
			}
			if (ch == 'x') { // toggle fan
				isFanOn = !isFanOn; // toggle fan status
			}
			if (ch==27){
				break; // exit the game
			}
		}
		if (temperature > 120) {
			gameover("overheating",120,tick);
			break; // game over if temperature is too high
		}
		if (player_stat==0) {
			
			cout << "Press any key to go back to menu." << endl;
			break;
		}
	}
}

int main()
{
	setcursor(0,0); 
	srand( (unsigned)time(NULL)); 
	//play();
	system("color 07");
	do{
		system("cls");
		gotoxy(10,5); cout<<" -------------------------- "; 
		gotoxy(10,6); cout<<" |      FnaD's console    | "; 
		gotoxy(10,7); cout<<" --------------------------";
		gotoxy(10,9); cout<<"1. Start Game";
		gotoxy(10,10); cout<<"2. Instructions";	 
		gotoxy(10,11); cout<<"3. Quit";
		gotoxy(10,13); cout<<"Select option: ";
		char op = getche();
		player_stat=1;
		if( op=='1') play();
		else if( op=='2') instructions();
		else if( op=='3') exit(0);
		
	}while(1);
	
}