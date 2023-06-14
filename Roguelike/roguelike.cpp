#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include <math.h>

#define ROWS 30
#define COLS 80
#define MAX_LINE_LENGTH 256

enum color{
    BLACK,
    D_BLUE,
    D_GREEN,
    D_SKYBLUE,
    D_RED,
    D_VIOLET,
    D_YELLOW,
    GRAY,
    D_GRAY,
    BLUE,
    GREEN,
    SKYBLUE,
    RED,
    VIOLET,
    YELLOW,
    WHITE,
};

enum keys {
    UP = 'w',
    DOWN = 's',
    LEFT = 'a',
    RIGHT = 'd',
    PAUSE = 'p',
    SAVE = 'o',
    SPEED = 'l',
    QUIT = 27,
};

enum job {
    SHIELDMAN,
    SPEARMAN,
    SWORDSMAN
};

struct Weapon
{
    char name[MAX_LINE_LENGTH];
    int atk;
    int penetration;
    int def;
    int blockChance;
};

struct Player {
    char name[MAX_LINE_LENGTH];
    int job;
    int hp;
    int curHp;
    int keys;
    int buff;
    int atk;
    int blockChance;
    int counterChance;
    int penetration;
    int def;
    int regen;
    int fleeChance;
    int level;
    int curExp;
    int nextExp;
    int atkMult;
    Weapon* weapon;
};

struct Monster {
    char name[MAX_LINE_LENGTH];
    int hp;
    int atk;
    int def;
    int blockChance;
    int counterChance;
    int penetration;
    int regen;
    int exp;
};

int grid[ROWS][COLS];
const int dy[] = { -1, 0, 1, 0 };
const int dx[] = { 0, 1, 0, -1 };
int playerY;
int playerX;
int spawnChance = 80;
int dungeonDepth = 1;
int monsterNumber = 0;
int weaponNumber = 0;
int item = -1;
int sleeptime = 100;

Player player;
Weapon weapons[15];

void setColor(int backcolor, int fontcolor);
void gotoxy(int y, int x, const wchar_t c);
void initGrid();
void printGrid();
void genObstacles(int spawnChance);
void genDungeon();
void startGame();
void collisionCheck(int ny, int nx, Monster* m);
void chooseMenu();
int loadMonsterInfo(Monster* m);
void createCharacter();
void loadCharacterImage(char img[][MAX_LINE_LENGTH], int choice);
void printCharacterImage(char img[][MAX_LINE_LENGTH]);
void printCharacterStatus();
int monsterEncountered(Monster *m);
int startBattle(Monster* mob, int turn, Monster* m, int mType);
void checkLevelUp();
void pause(char key);
void gameInfo(char key);
int getRandomItem();
void printMenu();
void saveGame();
void loadGame();

int main() {
    srand(time(NULL));
	auto res = _setmode(_fileno(stdout), _O_U16TEXT);

	chooseMenu();

	return 0;
}

void gotoxy(int y, int x, const wchar_t c)
{
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    wprintf(L"%lc", c);
}

void startGame()
{
    Monster m[20];
    monsterNumber = loadMonsterInfo(m);

    int ny, nx;
    wchar_t key = NULL;
    system("cls");
    printGrid();
    printCharacterStatus();

    while (1) {
        key = _getch();
        switch (key) {
            case UP: 
                ny = playerY + dy[0];
                nx = playerX + dx[0];
                collisionCheck(ny, nx, m);
                break;
            case DOWN:
                ny = playerY + dy[2];
                nx = playerX + dx[2];
                collisionCheck(ny, nx, m);
                break;
            case LEFT:
                ny = playerY + dy[3];
                nx = playerX + dx[3];
                collisionCheck(ny, nx, m);
                break;
            case RIGHT:
                ny = playerY + dy[1];
                nx = playerX + dx[1];
                collisionCheck(ny, nx, m);
                break;
            case PAUSE:
                pause(PAUSE);
                break;
            case SAVE:
                saveGame();
                break;
            case SPEED:
                //게임속도 조절
                if(sleeptime == 100){
                    sleeptime = 500;
                }
                else if (sleeptime == 500) {
                    sleeptime = 1000;
                }
                else if (sleeptime == 1000) {
                    sleeptime = 100;
                }
                break;
            case QUIT:
                system("cls");
                return;
            default:
                break;
        }
    }
}

void collisionCheck(int ny, int nx, Monster* m)
{
    //out of bounds
    if (ny < 1 || ny > ROWS - 2 || nx < 1 || nx > COLS - 2) {
        return;
    }
    
    if (grid[ny][nx] == 0) {
        //move player
        setColor(BLACK, BLACK);
        gotoxy(playerY, playerX, L' ');
        
        grid[playerY][playerX] = 0;
        grid[ny][nx] = 2;

        playerY = ny;
        playerX = nx;

        setColor(BLACK, WHITE);
        gotoxy(playerY, playerX, L'☺');
    }
    else if (grid[ny][nx] == 1) {
        return;
    }
    else if (grid[ny][nx] == 3) {
        int res = monsterEncountered(m);
        if (res == 0)
        {
            //move player
            setColor(BLACK, BLACK);
            gotoxy(playerY, playerX, L' ');

            grid[playerY][playerX] = 0;
            grid[ny][nx] = 2;

            playerY = ny;
            playerX = nx;

            setColor(BLACK, WHITE);
            gotoxy(playerY, playerX, L'☺');
        }
        else if (res == -1) {
            system("cls");
            wprintf(L"당신은 죽었습니다!\n");
            Sleep(1000);
            exit(0);
        }
    }
    else if (grid[ny][nx] == 4) {
        //alter
        player.buff = rand() % 3 + 1;
        if (player.buff == 1) {
            gotoxy(14, COLS + 2, L' ');
            wprintf(L"희생 버프를 획득했습니다!");
            player.curHp -= int((float)player.curHp * 0.1);
        }
        else if (player.buff == 2) {
            gotoxy(14, COLS + 2, L' ');
            wprintf(L"불멸 버프를 획득했습니다!");
        }
        else if (player.buff == 3) {
            gotoxy(14, COLS + 2, L' ');
            wprintf(L"성장 버프를 획득했습니다!");
        }
        Sleep(sleeptime);
        //move player
        setColor(BLACK, BLACK);
        gotoxy(playerY, playerX, L' ');

        grid[playerY][playerX] = 0;
        grid[ny][nx] = 2;

        playerY = ny;
        playerX = nx;

        setColor(BLACK, WHITE);
        gotoxy(playerY, playerX, L'☺');

        system("cls");
        printGrid();
        printCharacterStatus();
    }
    else if (grid[ny][nx] == 5) {
        //key
        player.keys++;

        //move player
        setColor(BLACK, BLACK);
        gotoxy(playerY, playerX, L' ');

        grid[playerY][playerX] = 0;
        grid[ny][nx] = 2;

        playerY = ny;
        playerX = nx;

        setColor(BLACK, WHITE);
        gotoxy(playerY, playerX, L'☺');

        system("cls");
        printGrid();
        printCharacterStatus();
    }
    else if (grid[ny][nx] == 6 && player.keys > 0) {
        //chest
        player.keys--;


        item = getRandomItem();
        gotoxy(14, COLS + 2, L' ');
        wprintf(L"무기: %S, 공격력: %d, 관통력: %d%%, 방어력:%d, 블럭율:%d%% 획득!\n", 
            weapons[item].name, weapons[item].atk, weapons[item].penetration, weapons[item].def, weapons[item].blockChance);

        //맨손이거나 더 높은 등급일 경우 자동 장착
        if (player.weapon == NULL || player.weapon < weapons + item) {
            player.weapon = weapons + item;
        }

        //move player
        setColor(BLACK, BLACK);
        gotoxy(playerY, playerX, L' ');

        grid[playerY][playerX] = 0;
        grid[ny][nx] = 2;

        playerY = ny;
        playerX = nx;

        setColor(BLACK, WHITE);
        gotoxy(playerY, playerX, L'☺');

        system("cls");
        printGrid();
        printCharacterStatus();
    }
    else if (grid[ny][nx] == 7) {
        grid[ny][nx] = 0;
        grid[playerY][playerX] = 0;

        dungeonDepth++;
        initGrid();
        genDungeon();
        
        spawnChance = spawnChance + 10;
        genObstacles(spawnChance);

        system("cls");
        printGrid();
        printCharacterStatus();
    }
}

void genDungeon() {
    int maxTunnels = 60;
    int maxLength = 20;

    int curRow = rand() % ROWS;
    int curCol = rand() % COLS;
    
    //player position
    playerY = curRow;
    playerX = curCol;
    //wprintf(L"player y %d, player x %d", playerY, playerX);

    int lastDir = 100, curDir;

    while (maxTunnels) {
        do {
            curDir = rand() % 4;
        } while (abs(curDir - lastDir) == 2 || curDir == lastDir);
        int randLen = rand() % maxLength + 3;
        int tunnelLen = 0;
        while (tunnelLen < randLen) {
            //edge
            if ((curRow == 1 && curDir == 0) ||
                (curRow == ROWS - 2 && curDir == 2) ||
                (curCol == 1 && curDir == 3) ||
                (curCol == COLS - 2 && curDir == 1)) {
                break;
            }
            else {
                grid[curRow][curCol] = 0;
                curRow += dy[curDir];
                curCol += dx[curDir];
                tunnelLen++;
            }
        }
        if (tunnelLen) {
            lastDir = curDir;
            maxTunnels--;
        }
    }
    grid[playerY][playerX] = 2;
}

void initGrid() 
{
    int i, j;

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            grid[i][j] = 1;
        }
    }
}

void printGrid() {
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (grid[i][j] == 1) {
                //wall
                setColor(WHITE, WHITE);
                gotoxy(i, j, L'#');
            }
            else if (grid[i][j] == 2) {
                //player
                setColor(BLACK, WHITE);
                gotoxy(i, j, L'☺');
            }
            else if (grid[i][j] == 3) {
                //monster
                setColor(BLACK, RED);
                gotoxy(i, j, L'Ṃ');
            }
            else if (grid[i][j] == 4) {
                //altar
                setColor(GRAY, VIOLET);
                gotoxy(i, j, L'ᴪ');
            }
            else if (grid[i][j] == 5) {
                //key
                setColor(BLACK, YELLOW);
                gotoxy(i, j, L'₣');
            }
            else if (grid[i][j] == 6) {
                //chest
                setColor(YELLOW, GREEN);
                gotoxy(i, j, L'‼');
            }
            else if (grid[i][j] == 7) {
                //next floor
                setColor(VIOLET, BLUE);
                gotoxy(i, j, L'֍');
            }
            else {
                setColor(BLACK, BLACK);
                gotoxy(i, j, L' ');
            }
        }
        setColor(BLACK, BLACK);
        if (i != ROWS - 1) {
            wprintf(L"\n");
        }
    }
    setColor(BLACK, WHITE);
    gotoxy(playerY, playerX, L'☺');
}

void genObstacles(int spawnChance)
{
    int altar = 0, altarMax = rand() % 1 + 1;
    int key = 0, chest = 0, nextFloor = 0;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (grid[i][j] == 0) {
                int mobs = rand() % 1000;

                if (mobs < spawnChance) {
                    grid[i][j] = 3;
                }
            }

        }
    }
    int i;
    int j;
    while (altar < altarMax) {
        i = rand() % (ROWS - 2) + 1;
        j = rand() % (COLS - 2) + 1;
        if (grid[i][j] == 0) {
            altar++;
            grid[i][j] = 4;
        }
    }
    while (key < 1) {
        i = rand() % (ROWS - 2) + 1;
        j = rand() % (COLS - 2) + 1;
        if (grid[i][j] == 0) {
            key++;
            grid[i][j] = 5;
        }
    }

    while (chest < 1) {
        i = rand() % (ROWS - 2) + 1;
        j = rand() % (COLS - 2) + 1;
        if (grid[i][j] == 0) {
            chest++;
            grid[i][j] = 6;
        }
    }
    if (dungeonDepth == 6) {
        return;
    }
    while (nextFloor < 1) {
        i = rand() % (ROWS - 2) + 1;
        j = rand() % (COLS - 2) + 1;
        if (grid[i][j] == 0 && 2 > (grid[i + 1][j] + grid[i - 1][j] + grid[i][j + 1] + grid[i][j - 1])) {
            nextFloor++;
            grid[i][j] = 7;
        }
    }
}

void setColor(int backcolor, int fontcolor)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), backcolor << 4 | fontcolor);
}

void chooseMenu() {
    int choice = 1;
    int prev = 1;
    char c;

    printMenu();

    while (1) {
        c = _getch();

        switch (c) {
        case UP:
            prev = choice;
            choice--;
            if (choice < 1) {
                choice = 4;
            }
            setColor(BLACK, BLACK);
            gotoxy(10 + prev, 0, L' ');

            setColor(BLACK, WHITE);
            gotoxy(10 + choice, 0, L'>');
            break;
        case DOWN:
            prev = choice;
            choice++;
            if (choice > 4) {
                choice = 1;
            }
            setColor(BLACK, BLACK);
            gotoxy(10 + prev, 0, L' ');

            setColor(BLACK, WHITE);
            gotoxy(10 + choice, 0, L'>');
            break;
        case '\r':
            switch (choice) {
            case 1:
                spawnChance = 80;
                dungeonDepth = 1;
                monsterNumber = 0;
                weaponNumber = 0;

                initGrid();
                genDungeon();
                genObstacles(spawnChance);
                createCharacter();
                startGame();
                printMenu();
                break;
            case 2:
                getRandomItem();
                loadGame();
                startGame();
                printMenu();
                break;
            case 3:
                gameInfo(PAUSE);
                printMenu();
                break;
            case 4:
                exit(0);
                break;
            }
            break;
        default:
            break;
        }
    }

}

int loadMonsterInfo(Monster* m)
{
    FILE* fp = NULL;

    if (fopen_s(&fp, "monsters.txt", "r") == NULL) {
        char line[MAX_LINE_LENGTH];
        int i;
        for (i = 0; fgets(line, MAX_LINE_LENGTH, fp); i++) {
            char* token = NULL;
            char* temp = strtok_s(line, ",", &token);
            strcpy_s(m[i].name, MAX_LINE_LENGTH, temp);

            temp = strtok_s(NULL, ",", &token);
            m[i].hp = atoi(temp);

            temp = strtok_s(NULL, ",", &token);
            m[i].atk = atoi(temp);
            
            temp = strtok_s(NULL, ",", &token);
            m[i].def = atoi(temp);

            temp = strtok_s(NULL, ",", &token);
            m[i].blockChance = atoi(temp);

            temp = strtok_s(NULL, ",", &token);
            m[i].counterChance = atoi(temp);

            temp = strtok_s(NULL, ",", &token);
            m[i].penetration = atoi(temp);

            temp = strtok_s(NULL, ",", &token);
            m[i].regen = atoi(temp);

            temp = strtok_s(NULL, ",", &token);
            m[i].exp = atoi(temp);
        }
        fclose(fp);
        return i;
    }

    return -1;
}

void createCharacter()
{
    char img[60][MAX_LINE_LENGTH];
    int choice = 1, prev = 1;

    system("cls");
    setColor(BLACK, WHITE);
    wprintf(L"Choose your class\n");
    wprintf(L">SWORDSMAN\n");
    wprintf(L" SPEARMAN\n");
    wprintf(L" SHIELDMAN\n");
    loadCharacterImage(img, choice);
    printCharacterImage(img);

    while (1) {
        char c = _getch();
        system("cls");
        wprintf(L"Choose your class\n");
        wprintf(L" SWORDSMAN\n");
        wprintf(L" SPEARMAN\n");
        wprintf(L" SHIELDMAN\n");
        switch (c) {
            case UP:
                choice--;
                if (choice < 1) {
                    choice = 3;
                }
                setColor(BLACK, BLACK);
                gotoxy(prev, 0, L' ');

                setColor(BLACK, WHITE);
                gotoxy(choice, 0, L'>');

                loadCharacterImage(img, choice);
                printCharacterImage(img);
                break;
            case DOWN:
                prev = choice;
                choice++;
                if (choice > 3) {
                    choice = 1;
                }
                setColor(BLACK, BLACK);
                gotoxy(prev, 0, L' ');

                setColor(BLACK, WHITE);
                gotoxy(choice, 0, L'>');

                loadCharacterImage(img, choice);
                printCharacterImage(img);
                break;
            default:
                system("cls");
                player.job = choice;
                player.buff = 0;
                player.keys = 0;
                player.weapon = NULL;
                if (choice == 1) {
                    wprintf(L"SWORDMAN chosen\n");
                    player.hp = 180;
                    player.curHp = 180;
                    player.atk = 50;
                    player.blockChance = 0;
                    player.counterChance = 0;
                    player.penetration = 0;
                    player.def = 0;
                    player.regen = 5;
                    player.fleeChance = 50;
                    player.level = 1;
                    player.curExp = 0;
                    player.nextExp = 50;
                    player.atkMult = 5;
                }
                else if (choice == 2) {
                    wprintf(L"SPEARMAN chosen\n");
                    player.hp = 180;
                    player.curHp = 180;
                    player.atk = 60;
                    player.blockChance = 0;
                    player.counterChance = 0;
                    player.penetration = 50;
                    player.def = 0;
                    player.regen = 5;
                    player.fleeChance = 50;
                    player.level = 1;
                    player.curExp = 0;
                    player.nextExp = 50;
                    player.atkMult = 0;
                }
                else if (choice == 3) {
                    wprintf(L"SHIELDMAN chosen\n");
                    player.hp = 200;
                    player.curHp = 200;
                    player.atk = 20;
                    player.blockChance = 10;
                    player.counterChance = 0;
                    player.penetration = 50;
                    player.def = 20;
                    player.regen = 10;
                    player.fleeChance = 50;
                    player.level = 1;
                    player.curExp = 0;
                    player.nextExp = 50;
                    player.atkMult = 0;
                }
                wprintf(L"Enter your name\n");
                scanf_s("%s", &player.name, MAX_LINE_LENGTH);
                return;
        }
    }
}

void loadCharacterImage(char img[][MAX_LINE_LENGTH], int choice)
{
    FILE* fp = NULL;
    char imgPath[3][MAX_LINE_LENGTH] = { {"swordChar.txt"}, {"spearChar.txt"}, {"shieldChar.txt"}};

    if (fopen_s(&fp, imgPath[choice - 1], "r") == NULL) {
        char line[MAX_LINE_LENGTH];
        for (int i = 0; fgets(line, MAX_LINE_LENGTH, fp); i++) {
            strcpy_s(img[i], line);
        }
        fclose(fp);
    }
}

void printCharacterImage(char img[][MAX_LINE_LENGTH])
{
    for (int i = 0; i < 60; i++) {
        for (int j = 0; j < 120; j++) {
            gotoxy(5 + i, j, (wchar_t)img[i][j]);
        }
        gotoxy(5 + i, 121, L'\n');
    }
}

void printCharacterStatus()
{
    char buff[20] = { 0 };
    if (player.buff == 1) {
        strcpy_s(buff, "SACRIFICE");
    }
    else if (player.buff == 2) {
        strcpy_s(buff, "IMMORTAL");
    }
    else if (player.buff == 3) {
        strcpy_s(buff, "GROWTH");
    }
    else {
        strcpy_s(buff, "NONE");
    }

    int cnt = 0;
    setColor(BLACK, WHITE);
    for (int i = 0; i < 6; i++) {
        gotoxy(0, COLS + 2 + i, L'=');
    }
    gotoxy(cnt++, COLS + 10, L' ');
    wprintf(L"던전레벨: %d\n", dungeonDepth);
    for (int i = 24; i < 30; i++) {
        gotoxy(0, COLS + 2 + i, L'=');
    }
    gotoxy(cnt++, COLS + 2, L' ');
    if (player.job == 1) {
        wprintf(L"직업: SWORDMAN, 버프: %S\n", buff);
    }
    else if (player.job == 2) {
        wprintf(L"직업: SPEARMAN, 버프: %S\n", buff);
    }
    else if (player.job == 3) {
        wprintf(L"직업: SHIELDMAN, 버프: %S\n", buff);
    }
    
    gotoxy(cnt++, COLS + 2, L' ');
    wprintf(L"이름:%S, 레벨:%d", player.name, player.level);

    gotoxy(cnt++, COLS + 2, L' ');
    wprintf(L"무기: %S, 열쇠:%d", 
        (player.weapon == NULL) ? "NONE" : player.weapon->name, player.keys);

    if (player.weapon != NULL) {
        gotoxy(cnt++, COLS + 2, L' ');
        wprintf(L"무기 atk,pen,def,block: %d, %d, %d, %d",
            player.weapon->atk, player.weapon->penetration, player.weapon->def, player.weapon->blockChance);
    }

    gotoxy(cnt++, COLS + 2, L' ');
    wprintf(L"경험치: %d/%d", player.curExp, player.nextExp);

    gotoxy(cnt++, COLS + 2, L' ');
    wprintf(L"체력: %d/%d", player.curHp, player.hp);

    gotoxy(cnt++, COLS + 2, L' ');
    wprintf(L"공격력:%d, 턴당 공격력 증폭:%d%%", player.atk, player.atkMult);

    gotoxy(cnt++, COLS + 2, L' ');
    wprintf(L"관통력:%d%%, 반격확률: %d%%", player.penetration, player.counterChance);

    gotoxy(cnt++, COLS + 2, L' ');
    wprintf(L"방어력:%d, 체력재생: %d", player.def, player.regen);

    gotoxy(cnt++, COLS + 2, L' ');
    wprintf(L"도망확률:%d%% ", player.fleeChance);

    for (int i = 0; i < 30; i++) {
        gotoxy(cnt, COLS + 2 + i, L'#');
    }
}

int monsterEncountered(Monster* m)
{
    char c;
    int choice = 1;
    int result = 0;
    int lineCnt = 13;
    int mType = rand() % 2 + 2 * (dungeonDepth - 1);
    Monster mob;
    strcpy_s(mob.name, MAX_LINE_LENGTH, m[mType].name);
    mob.atk = m[mType].atk;
    mob.def = m[mType].def;
    mob.hp = m[mType].hp;
    mob.blockChance = m[mType].blockChance;
    mob.penetration = m[mType].penetration;
    mob.counterChance = m[mType].counterChance;
    mob.exp = m[mType].exp;
    mob.regen = m[mType].regen;

    setColor(BLACK, WHITE);
    gotoxy(lineCnt, COLS + 2, L' ');
    wprintf(L"몬스터 < %S > 가 나타났습니다!", m[mType].name);
    
    gotoxy(lineCnt + 1, COLS + 2, L' ');
    wprintf(L"화살표키로 선택해주세요!");

    gotoxy(lineCnt + 2, COLS + 2, L'>');
    wprintf(L"전투하기");

    gotoxy(lineCnt + 3, COLS + 2, L' ');
    wprintf(L"도망치기");
    
    while (1) {
        c = _getch();
        
        if (c == 72) {
            //up arrow
            gotoxy(lineCnt + choice + 1, COLS + 2, L' ');
            choice--;
            if (choice < 1) {
                choice = 2;
            }
            gotoxy(lineCnt + choice + 1, COLS + 2, L'>');
        }
        else if (c == 80) {
            //down arrow
            gotoxy(lineCnt + choice + 1, COLS + 2, L' ');
            choice++;
            if (choice > 2) {
                choice = 1;
            }
            gotoxy(lineCnt + choice + 1, COLS + 2, L'>');
        }
        else if (c == '\r') {
            //battle
            if (choice == 1) {
                result = startBattle(&mob, 1, m, mType);
            }
            else if(choice == 2){
                int temp = rand() % 100;
                if (temp < player.fleeChance) {
                    wprintf(L"도망치는데 성공했습니다!\n");
                    return 1;
                }
                else {
                    //도망 실패시 몹에게 맞고 시작
                    result = startBattle(&mob, 2, m, mType);
                }
            }
            break;
        }
    }
    return result;
}

int startBattle(Monster* mob, int turn, Monster* m, int mType)
{
    int lineCnt = 17;
    float atkMult = 1.0f;

    float atkBuff = 1.0f;
    int regenBuff = 0;
    int expBuff = 1;
    while (1) {
        //player부터
        if (turn == 1) {
            //player 버프 발동
            if (player.buff == 1) {
                atkBuff = 1.1f;
            }
            else if (player.buff == 2) {
                regenBuff = int(player.hp / 20);
            }
            else if (player.buff == 3) {
                expBuff = 2;
            }

            //player 생명력 회복
            if (player.curHp + player.regen + regenBuff > player.hp) {
                player.curHp = player.hp;
            }
            else {
                player.curHp += player.regen + regenBuff;
            }
            
            atkMult += float(player.atkMult) / 100;
            int pAtk;
            int pDef;
            int mDef;
            if (player.weapon != NULL) {
                pAtk = (player.atk + player.weapon->atk) * atkMult * atkBuff;
                pDef = int((float)(player.def + player.weapon->def) / 100.0f * (100.0f - mob->penetration));
                mDef = int((float)mob->def / 100.0f * (100.0f - player.penetration - player.weapon->penetration));
            }
            else {
                pAtk = player.atk * atkMult * atkBuff;
                pDef = int((float)player.def / 100.0f * (100.0f - mob->penetration));
                mDef = int((float)mob->def / 100.0f * (100.0f - player.penetration));
            }
            
            gotoxy(lineCnt, COLS + 2, L' ');
            wprintf(L"캐릭터 < %S > 의 공격\n", player.name);
            Sleep(sleeptime);

            gotoxy(lineCnt + 1, COLS + 2, L' ');
            wprintf(L"%d 의 데미지로 공격\n", pAtk);
            Sleep(sleeptime);

            int mBlockSuccess = (rand() % 100 < mob->blockChance);
            if (!mBlockSuccess) {
                int temp = player.atk - mDef;
                if (temp > 0) {
                    mob->hp -= temp;
                }
            }
            else if (mob->counterChance > rand() % 100) {
                //블럭 성공시 몹의 카운터 계산(카운터 공격은 막기 불가)
                int temp = mob->atk - pDef;
                if (temp > 0) {
                    gotoxy(lineCnt + 2, COLS + 2, L' ');
                    wprintf(L"막기 성공,%S 의 반격: %d 만큼의 데미지\n", mob->name, temp);
                    player.curHp -= temp;
                }
            }
            else {
                gotoxy(lineCnt + 2, COLS + 2, L' ');
                wprintf(L"%S 의 막기 성공\n", mob->name);
            }
            gotoxy(lineCnt + 3, COLS + 2, L' ');
            wprintf(L"%S 의 체력 %d/%d\n", mob->name, mob->hp, m[mType].hp);
            Sleep(sleeptime);

            if (player.curHp <= 0) {
                gotoxy(lineCnt + 4, COLS + 2, L' ');
                wprintf(L"%S 패배!\n", player.name);

                Sleep(sleeptime);
                return -1;
            }
            else if (mob->hp <= 0) {
                player.curExp += (mob->exp * expBuff);
                gotoxy(lineCnt + 4, COLS + 2, L' ');
                wprintf(L"%S 승리! exp : %d\n", player.name, player.curExp);

                gotoxy(lineCnt + 5, COLS + 2, L' ');
                wprintf(L"다음으로 이동!\n");

                Sleep(sleeptime);
                break;
            }
        }
        //monster부터
        else if (turn == 2) {
            //monster생명력 회복
            if (mob->hp + mob->regen > m[mType].hp) {
                mob->hp = m[mType].hp;
            }
            else {
                mob->hp += mob->regen;
            }

            int pAtk;
            int pDef;
            int pBlockSuccess;
            if (player.weapon != NULL) {
                pAtk = (player.atk + player.weapon->atk) * atkMult * atkBuff;
                pDef = int((float)(player.def + player.weapon->def) / 100.0f * (100.0f - mob->penetration));
                pBlockSuccess = (rand() % 100 < (player.blockChance + player.weapon->blockChance));
            }
            else {
                pAtk = player.atk * atkMult * atkBuff;
                pDef = int((float)player.def / 100.0f * (100.0f - mob->penetration));
                pBlockSuccess = (rand() % 100 < player.blockChance);
            }

            gotoxy(lineCnt, COLS + 2, L' ');
            wprintf(L"%S 의 공격\n", mob->name);
            Sleep(sleeptime);
            gotoxy(lineCnt + 1, COLS + 2, L' ');
            wprintf(L"%d 의 데미지로 공격\n", mob->atk);
            Sleep(sleeptime);

            int mDef = int((float)mob->def / 100.0f * (100.0f - player.penetration));

            if (!pBlockSuccess) {
                int temp = mob->atk - pDef;
                if (temp > 0) {
                    player.curHp -= temp;
                }
            }
            else if (player.counterChance > rand() % 100) {
                //블럭 성공시 플레이어의 카운터 계산(카운터 공격은 막기 불가)
                int temp = pAtk - mDef;
                if (temp > 0) {
                    gotoxy(lineCnt + 2, COLS + 2, L' ');
                    wprintf(L"막기 성공, %S 의 반격: %d 만큼의 데미지\n",player.name, temp);
                    mob->hp -= temp;
                }
            }
            else {
                gotoxy(lineCnt + 2, COLS + 2, L' ');
                wprintf(L"%S 의 막기 성공\n", player.name);
            }

            gotoxy(lineCnt + 3, COLS + 2, L' ');
            wprintf(L"%S 의 체력 %d/%d\n", player.name, player.curHp, player.hp);
            Sleep(sleeptime);

            if (player.curHp <= 0) {
                gotoxy(lineCnt + 4, COLS + 2, L' ');
                wprintf(L"%S 패배!\n", player.name);

                Sleep(sleeptime);
                return -1;
            }
            else if (mob->hp <= 0) {
                player.curExp += (mob->exp * expBuff);
                gotoxy(lineCnt + 4, COLS + 2, L' ');
                wprintf(L"%S 승리! exp : %d\n", player.name, player.curExp);
                gotoxy(lineCnt + 5, COLS + 2, L' ');
                wprintf(L"다음으로 이동!\n");
                Sleep(sleeptime);
                break;
            }
        }
        turn++;
        if (turn > 2) {
            turn = 1;
        }
        setColor(BLACK, BLACK);
        gotoxy(lineCnt, COLS + 2, L' ');
        wprintf(L"                            ");
        gotoxy(lineCnt + 1, COLS + 2, L' ');
        wprintf(L"                            ");
        gotoxy(lineCnt + 2, COLS + 2, L' ');
        wprintf(L"                            ");
        gotoxy(lineCnt + 3, COLS + 2, L' ');
        wprintf(L"                            ");
        gotoxy(lineCnt + 4, COLS + 2, L' ');
        wprintf(L"                            ");
        gotoxy(lineCnt + 5, COLS + 2, L' ');
        wprintf(L"                            ");

        setColor(BLACK, WHITE);
    }
    Sleep(sleeptime);
    checkLevelUp();
    system("cls");
    printGrid();
    printCharacterStatus();
    return 0;
}

void checkLevelUp()
{
    while(player.curExp >= player.nextExp) {
        player.curExp -= player.nextExp;
        player.level++;
        player.nextExp = 50 * pow(player.level, 1.4);
        player.atk += 10 * log(player.level + 1);
        player.hp += 50 * log(player.level + 1) + player.level;
        player.curHp = player.hp;
    }
}

void pause(char key) { // p키를 눌렀을 경우 게임을 일시 정지 
    while (1) {
        if (key == PAUSE) {
            gotoxy(ROWS + 2, 0, L'\0');
            wprintf(L"< PAUSE : PRESS ANY KEY TO RESUME >");
            Sleep(400);
            gotoxy(ROWS + 2, 0, L'\0');
            wprintf(L"                                   ");
            Sleep(400);
        }
        else {
            system("cls");
            printGrid();
            printCharacterStatus();
            return;
        }

        if (_kbhit()) {

            do {
                key = _getch();
            } while (key == 224);
        }
    }
}

void gameInfo(char key)
{
    system("cls");
    wprintf(L"게임목표 : 던전의 6층까지 도전하세요\n");

    wprintf(L"\n사용자 컨트롤\n");
    wprintf(L"방향키: W = 위,S = 아래, A = 왼쪽, D = 오른쪽\n");
    wprintf(L"화살표키: ˄ = 위, ˅ = 아래, 게임진행중 선택지를 선택가능\n");
    wprintf(L"게임 저장: 알파벳 'o'  \n");
    wprintf(L"일시정지: 알파벳 'p'  \n");
    wprintf(L"속도조절: 알파벳 'l'  \n");
    wprintf(L"메인화면으로: ESC  \n");

    wprintf(L"\n각각의 아이콘 \n");
    wprintf(L"플레이어 : ☺ \n");
    wprintf(L"몬스터 : Ṃ \n");
    wprintf(L"제단 : ᴪ \n");
    wprintf(L"열쇠 : ₣ \n");
    wprintf(L"보물상자 : ‼ \n");
    wprintf(L"다음층으로 가는 포탈 : ֍ \n");

    wprintf(L"\n(주의) 버프는 한개만 가질 수 있습니다!\n");
    wprintf(L"메인화면으로 가려면 아무키와 엔터를 누르세요\n");
    char temp = getchar();
    temp = getchar();
    system("cls");
}

int getRandomItem()
{
    if(weaponNumber == 0){//한번도 로드된적 없으면 로드
        FILE* fp = NULL;

        if (fopen_s(&fp, "weapons.txt", "r") == NULL) {
            char line[MAX_LINE_LENGTH];
            int i;
            for (i = 0; fgets(line, MAX_LINE_LENGTH, fp); i++) {
                char* token = NULL;
                char* temp = strtok_s(line, ",", &token);
                strcpy_s(weapons[i].name, MAX_LINE_LENGTH, temp);

                temp = strtok_s(NULL, ",", &token);
                weapons[i].atk = atoi(temp);

                temp = strtok_s(NULL, ",", &token);
                weapons[i].penetration = atoi(temp);

                temp = strtok_s(NULL, ",", &token);
                weapons[i].def = atoi(temp);

                temp = strtok_s(NULL, ",", &token);
                weapons[i].blockChance = atoi(temp);
            }
            fclose(fp);
        }
    }
    //각각의 직업에 알맞는 5개 중에 1개의 무기 주기
    if (player.job == 1) {
        return rand() % 5;
    }
    else if (player.job == 2) {
        return rand() % 5 + 5;
    }
    else if (player.job == 3) {
        return rand() % 5 + 10;
    }
    else {
        return rand() % 15;
    }
}

void printMenu()
{
    wprintf(L"############################################################################################\n");
    wprintf(L"                                 __           _____ .__                                     \n");
    wprintf(L"  ______ __ __   ____    _______/  |_  ____ _/ ____\\|  |__    ____    ____    ____ _______  \n");
    wprintf(L" / ____/|  |  \\_/ __ \\  /  ___/\\   __\\/  _ \\\\   __\\ |  |  \\  /  _ \\  /    \\  /  _  \\_  __ \\ \n");
    wprintf(L"< <_|  ||  |  /\\  ___/  \\___ \\  |  | (  <_> )|  |   |   Y  \\(  <_> )|   |  \\(  <_> )|  | \\/ \n");
    wprintf(L" \\__   ||____/  \\___  >/____  > |__|  \\____/ |__|   |___|  / \\____/ |___|  / \\____/ |__|    \n");
    wprintf(L"    |__|            \\/      \\/                           \\/              \\/                 \n");

    wprintf(L"\n############################################################################################\n\n");

    wprintf(L"\n");

    wprintf(L">게임시작\n");
    wprintf(L" 게임로드\n");
    wprintf(L" 설명\n");
    wprintf(L" 종료\n");
}

void saveGame()
{
    FILE* fptr;
    char filename[MAX_LINE_LENGTH] = { "saved.txt" };

    char line[ROWS + 24][MAX_LINE_LENGTH] = { 0 };
    //copy grid
    for (int i = 0; i < ROWS; i++) {
        int j;
        for (j = 0; j < COLS; j++) {
            line[i][j] = grid[i][j] + 'A';
        }
    }
    //copy player
    int cnt = 0;
    strcpy_s(line[ROWS + cnt++], player.name);

    _itoa_s(player.job, line[ROWS + cnt++], 10);
    _itoa_s(player.hp, line[ROWS + cnt++], 10);
    _itoa_s(player.curHp, line[ROWS + cnt++], 10);
    _itoa_s(player.keys, line[ROWS + cnt++], 10);
    _itoa_s(player.buff, line[ROWS + cnt++], 10);
    _itoa_s(player.atk, line[ROWS + cnt++], 10);
    _itoa_s(player.blockChance, line[ROWS + cnt++], 10);
    _itoa_s(player.counterChance, line[ROWS + cnt++], 10);
    _itoa_s(player.penetration, line[ROWS + cnt++], 10);
    _itoa_s(player.def, line[ROWS + cnt++], 10);
    _itoa_s(player.regen, line[ROWS + cnt++], 10);
    _itoa_s(player.fleeChance, line[ROWS + cnt++], 10);
    _itoa_s(player.level, line[ROWS + cnt++], 10);
    _itoa_s(player.curExp, line[ROWS + cnt++], 10);
    _itoa_s(player.nextExp, line[ROWS + cnt++], 10);
    _itoa_s(player.atkMult, line[ROWS + cnt++], 10);
    _itoa_s(item, line[ROWS + cnt++], 10);

    _itoa_s(playerY, line[ROWS + cnt++], 10);
    _itoa_s(playerX, line[ROWS + cnt++], 10);
    _itoa_s(spawnChance, line[ROWS + cnt++], 10);
    _itoa_s(dungeonDepth, line[ROWS + cnt++], 10);


    if (fopen_s(&fptr, filename, "w") == NULL)
    {
        for (int i = 0; i < ROWS + cnt; i++) {
            fprintf(fptr, "%s\n", line[i]);
        }
        fclose(fptr);
    }
}

void loadGame()
{

    FILE* fp = NULL;
    char filename[MAX_LINE_LENGTH] = { "saved.txt" };

    if (fopen_s(&fp, filename, "r") == NULL) {
        char line[MAX_LINE_LENGTH] = { 0 };
        //load grid
        for (int i = 0; i < ROWS; i++) {
            fgets(line, MAX_LINE_LENGTH, fp);
            for (int j = 0; j < COLS; j++) {
                grid[i][j] = line[j] - 'A';
            }
        }
        //load player
        int cnt = 0;
        fgets(line, MAX_LINE_LENGTH, fp);
        for (int i = 0; i < MAX_LINE_LENGTH; i++) {
            if (line[i] == '\n' || line[i] == '\0') {
                break;
            }
            player.name[i] = line[i];
        }

        fgets(line, MAX_LINE_LENGTH, fp);
        player.job = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.hp = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.curHp = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.keys = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.buff = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.atk = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.blockChance = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.counterChance = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.penetration = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.def = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.regen = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.fleeChance = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.level = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.curExp = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.nextExp = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        player.atkMult = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        if (atoi(line) > -1) {
            player.weapon = weapons + atoi(line);
        }
        else {
            player.weapon = NULL;
        }

        fgets(line, MAX_LINE_LENGTH, fp);
        playerY = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        playerX = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        spawnChance = atoi(line);

        fgets(line, MAX_LINE_LENGTH, fp);
        dungeonDepth = atoi(line);

        fclose(fp);
    }
}
