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
	//map[2].push_back(6);
	map[3].push_back(2);
	map[3].push_back(4);
	map[3].push_back(7);
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
int night;
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
	int baseCooldown; // base cooldown time for enemy to attack
    int cooldown; // cooldown time for enemy to attack
};
Enemy diddy{

	.id = 1,
	.pos = 0,
	.startPos = 0,
	.name = "Diddy",
	.cooldown = 5
};
Enemy drake{
	.id = 0,
	.pos = 1,
	.startPos = 1,
	.name = "Drake",
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
	if (night==7){
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
	}
	Sleep(5000);
	getch();
	
}
void jumpScare() {
	// Play a jump scare sound
	// PlaySound(L"jumpscare.wav", NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
	// Display a jump scare message
	cout <<"    ==============     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	
	Sleep(200);
	system("cls");
	cout <<"    ==============     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	cout <<" ||      /====\\   || "<<endl;
	
Sleep(200);
	system("cls");
	cout <<"    ==============     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	cout <<" ||      /====\\   || "<<endl;
	cout <<" ||            || ||  "<<endl;
	cout <<" ||            || ||       "<<endl;
Sleep(200);
	system("cls");
	cout <<"    ==============     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	cout <<" ||      /====\\   || "<<endl;
	cout <<" ||            || ||  "<<endl;
	cout <<" ||            || ||       "<<endl;
	cout <<" ||            || ||     "<<endl;
	cout <<" ||            // ||       "<<endl;
	cout <<" ||           //  ||       "<<endl;
	Sleep(200);
	system("cls");

	cout <<"    ==============     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	cout <<"  //              \\\\     "<<endl;
	cout <<" ||      /====\\   || "<<endl;
	cout <<" ||            || ||  "<<endl;
	cout <<" ||            || ||       "<<endl;
	cout <<" ||            || ||     "<<endl;
	cout <<" ||            // ||       "<<endl;
	cout <<" ||           //  ||       "<<endl;
	cout <<" ||         ==    ||       "<<endl;
	cout <<" ||      .. <     ||       "<<endl;

	Sleep(2000); // Wait for 2 seconds before returning to the game
	system("cls");
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
			jumpScare(); // play jump scare sound
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
    int cnt = 0; // count the number of enemies standing on the position
    for (int i=0;i<=14;i++){
        cnt = 0; // reset count
        for (Enemy e : {diddy, drake, LeBron_James, ambatukam}) {
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
void showcomputerGUIonsilentvent(){
	system("cls");
	 cout<<  " =====================";
    cout<<"\n | 1 | 2 | 3 | 4 | 5 |";
    cout<<"\n |-------------------|";
    cout<<"\n |                   |";
    cout<<"\n |                   |"; 
    cout<<"\n |                   |";
    cout<<"\n |                   |"; 
    cout<<"\n |-------------------|";
	gotoxy(2, 4);
	cout <<"silent vent:";
	gotoxy(2, 5);
	cout <<"actived!";
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
void victory(){// player successfully logged out
	system("cls");
	string sign="shift accomplished";
	gotoxy(5,3);
	for (int i=0;i<sign.length();i++){
		cout<<sign[i];
		Sleep(150);
	}
	Sleep(1500);
	system("cls");
	cout<<endl;
	cout<<"\t\t--------------------------"<<endl;
	cout<<"\t\t-------- You Win! -------"<<endl;
	cout<<"\t\t--------------------------"<<endl<<endl;
	cout<<"\t\tenjoy some time at white party";
	for (int i=0;i<3;i++){
		cout<<".";
		Sleep(1000);
	}
	player_stat=0;
	if (night<7)night++;
	ofstream fout("night.txt");
	fout << night;
	fout.close();
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
	cout <<"time: "<<tick/2500<<"Am";

}
void showNight() {
	gotoxy(30, 8); // move cursor to the next line
	cout << "Night: " << night; // display current night
}
void showNightIntro(int night) {
	system("cls");
	string msg="night: "+to_string(night);
	gotoxy(10, 5);
	for (int i=0;i<msg.length();i++){
		cout<<msg[i];
		Sleep(150);
	}
	Sleep(1000);
	if (night==1){
		cout<<"\n\nWelcome to the first night of your shift as a security guard at Diddy party.";
		Sleep(1000);
		cout <<"\n(press any key to continue...)";
		getch();
		cout<<"\nYou will have to do maintenance, advertisement and security tasks to keep the party running.";
		Sleep(1000); getch();
		
		cout<<"\nUse the computer to make sure no intruders get in and complete tasks.";
		Sleep(1000); getch();
		cout<<"\nGood luck!, Diddy will be here tomorrow night.";
	}
	else if (night==2){
		cout<<"\n\nNight 2: Diddy has come!";
		Sleep(1000);
		cout<<"\nI have a bad feeling about this...";
	}
	else if (night==3){
		cout<<"\n\nNight 3: Drake has joined the party!";
		Sleep(1000);
		cout<<"\nwhy is he here?";
	}
	else if (night==4){
		cout<<"\n\nNight 4: LeBron James has come!";
		Sleep(1000);
		cout<<"\nwhy is even Lebron here?";
		Sleep(1000); getch();
		cout<<"\nI thought he was busy with his basketball career!";
		
	}
	else if (night==5){
		cout<<"\n\nNight 5: Ambatukam has joined the party!";
		Sleep(1000);
		cout <<"\nmy worst nightmare has come true!";
		Sleep(1000); getch();
		cout <<"\nI thought he was just a meme!";
		Sleep(1000); getch();
		cout <<"\nI didn't sign up for this!";
	}
	else if (night==6){
		cout<<"\n\nNight 6: the party end tomorrow!";
		Sleep(1000);
		cout<<"\nI have to survive and get out of here!";
	}
	else if (night==7){
		cout<<"\n\nNight 7: why are you still here? just leave!";
		Sleep(1000);getch();
		cout <<"\nThere is no chance to escape!";
		Sleep(1000); getch();
		cout <<"\njust survive as long as you can!";
	}
	Sleep(5000);
	cout << "\n\nPress any key to continue...";
	getch();
	system("cls");
}
void setupenemy(int night){
	diddy.status =false;
	drake.status = false;
	LeBron_James.status = false;
	ambatukam.status = false;
	diddy.pos= diddy.startPos; // reset position of diddy to start position
	drake.pos = drake.startPos; // reset position of drake to start position
	LeBron_James.pos = LeBron_James.startPos; // reset position of LeBron James to start position
	ambatukam.pos = ambatukam.startPos; // reset position of DreamyBull to start position
	if (night==1){
		// No enemies at night 1
	}
	if (night==2){
		diddy.status = true; // diddy is alive at night 2
		diddy.baseCooldown = 15; // set base cooldown for diddy
	}
	if (night==3){
		diddy.status = true; // diddy is alive at night 3
		drake.status = true; // justin Bieber is alive at night 3
		diddy.baseCooldown = 10; // set base cooldown for diddy
		drake.baseCooldown = 15; // set base cooldown for drake
	}
	if (night==4){
		diddy.status = true; // diddy is alive at night 4
		drake.status = true; // drake is alive at night 4
		LeBron_James.status = true; 
		diddy.baseCooldown = 10; 
		drake.baseCooldown = 15;
		LeBron_James.baseCooldown=15;
	}
	if (night==5){
		diddy.status = true; // diddy is alive at night 5
		drake.status = true; // drake is alive at night 5
		LeBron_James.status = true; // LeBron James iszxx alive at night 5
		ambatukam.status = true; 
		diddy.baseCooldown = 7; 
		drake.baseCooldown = 13;
		LeBron_James.baseCooldown=12;
		ambatukam.baseCooldown=10;
	}
	if (night==6){
		diddy.status = true; // diddy is alive at night 6
		drake.status = true; // drake is alive at night 6
		LeBron_James.status = true; // LeBron James is alive at night 6
		ambatukam.status = true; // DreamyBull is alive at night 6
		diddy.baseCooldown = 5; 
		drake.baseCooldown = 6;
		LeBron_James.baseCooldown=7;
		ambatukam.baseCooldown=8;
	}
	if (night==7){
		diddy.status = true; // diddy is alive at night 6
		drake.status = true; // drake is alive at night 6
		LeBron_James.status = true; // LeBron James is alive at night 6
		ambatukam.status = true; // DreamyBull is alive at night 6
		diddy.baseCooldown = 1; 
		drake.baseCooldown = 1;
		LeBron_James.baseCooldown=1;
		ambatukam.baseCooldown=1;
	}
}
void play(int code){
    // Initialize the map
    initMap();
	setupcursor();
	
	fstream fin("night.txt");
	fin >>night;
	fin.close();
	if (night ==7 &&code==0){
		system("cls");
		cout << "You have completed the game! Congratulations!" << endl;
		cout << "Press 'Y' for new game(restart required)\n or N for exit: ";
		char ch = getch();
		if (ch == 'Y' || ch == 'y') {
			night = 1; // reset night to 1
			ofstream fout("night.txt");
			fout << night;
			fout.close();
		} 
		exit(0);
	}

	diddy.status =false;
	drake.status = false;
	LeBron_James.status = false;
	ambatukam.status = false;


	setupenemy(night);
	//int soundPlayerMade = 0;
	//vector<Enemy> enemies = {diddy, drake, LeBron_James, ambatukam};
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
    //vector<Enemy> enemies = {diddy, drake, LeBron_James, ambatukam};
	showNightIntro(night); // show night intro
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
		if (computer_mode==4&&tick%100 == 0){
			temperature--; // decrease temperature if computer is on silent vent mode
		}
		if (tick % 75 == 0) { // every 75 ticks, increase temperature
			temperature++;
			if (tick>15000&&code==0)
				canPlayerLogOut = true; // player can log out after 15000 ticks
		}
		// every 100 ticks - 1 cooldown for enemies
		Enemy* enemies[] = {&diddy, &drake, &LeBron_James, &ambatukam};
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
				e->cooldown = e->baseCooldown; // reset cooldown for enemy
			}
		}
		
		//showNight(); // show current night
		debug(player_vision,tick);
		showFanStatus(isFanOn); // show fan status1
			showTemperature(temperature); // show temperature
			showNoise(soundPlayerMade); // show noise made by player1
		if (tick%10==0){
			if (tick>20000&&task_done<task_total){
				system("cls");
				Sleep (3000);
				string message = "You have not completed all tasks yet!";
				gotoxy(10, 5);
				for (int i=0;i<message.length();i++){
					cout<<message[i];
					Sleep(150);
				}
				Sleep(2000);
				system("cls");
				jumpScare(); // play jump scare sound
				gameover("Diddy",11,tick);

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
					case 4: // silent vent mode
						showcomputerGUIonsilentvent();
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
                //computer_mode = 1; // task mode
            }
            if (player_vision=='m'&& ch=='2'){
                computer_mode = 2; // switch to tracking enemy mode
            }
			if (player_vision =='m'&&ch=='1'){
                computer_mode = 1; // switch to task mode
            }
			if (player_vision =='m'&& ch=='4'){
				computer_mode = 4; // switch to silent vent mode
			}
			if(player_vision=='m'&&ch==' ' && canPlayerLogOut){
				victory(); // player successfully logged out
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
void instructions(){
	
	system("cls");
	cout<<"Instructions";
	cout<<"\n----------------";
	cout<<"\n Can you survive the night?";
	cout<<"\n\n1. Use 'a' to look left, 'd' to look right, 's' to look at the computer.";
	cout<<"\n2. Use 'z' to toggle the computer on/off.";
	cout<<"\n3. Use 'x' to toggle the fan on/off.";
	cout<<"\n4. Make sure to complete all tasks before 8 AM.";
	cout<<"\n5. Use 'space' to log out when all tasks are done.";
	cout <<"\n6. tip 1:to be silent, try to turn off the computer and fan if you being surrounded by enemies.";
	cout<<"\n7. tip 2: if you are being attacked by an enemy, try to look at the direction of the enemy to dodge the attack.";
	cout<<"\n\nPress any key to go back to menu";
	char ch=getch();
	if (ch=='7'){
		play(1);
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
		if( op=='1') play(0);
		else if( op=='2') instructions();
		else if( op=='3') exit(0);
		
	}while(1);
	
}