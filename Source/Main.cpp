#pragma once

#include <iostream>
#include <fstream>
#include <windows.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <conio.h>
#include <thread>
#include <ctime>
#include <sstream>

#define display(x) std::cout << x << "\n"
#define WAIT std::cin.get()
#ifdef _DEBUG
#define LOG(x) std::cout << x << "\n"
#else
#define LOG(x)
#endif
using json = nlohmann::json;
using namespace std;

string option[6];
int delay = 30;
//enum Stage { EXIT, MENU, STATUS, EDITCONFIG };
enum class Stage : unsigned char { exit, menu, status, editconfig };
Stage stage = Stage::menu;
const char optionName[][21] = {
                                "Username",
                                "Hypixel_API_Key",
                                "Discord_send_message",
                                "Discord_Web_Hook",
                                "Checkdelay",
                                "ExtraMessage" };

void menu();
void loadConfig();
void createConfig();
void editConfig();
void statusUpdater(string uuid);
const void start();
const void pauseAtError(string response);
const string getStatusMessage(json status);
const void initWindow();
const void discordSend(string message);
const string Now();
string formattedText(string text);
const void sendMessage(string message);

int main() {
    while (stage != Stage::exit) {
        initWindow();
        {
            ifstream fc("config.txt");
            if (!fc)
                createConfig();
        }
        loadConfig();
        //this_thread::sleep_for(chrono::seconds(1));
        menu();
    }
    return 0;
}

void menu() {
    //init local variables
    static const string button[] = { "[S]Start","[C]Config","[Esc]Exit" };
    char selected = 0;
    //
    stage = Stage::menu;
    while (stage==Stage::menu) {
        system("cls");
        display("-----------------------");
        display(" Hypixel Player Status ");
        display("          - FireMan9534");
        display("-----------------------");
        display("-----------------------");
        for (char c = 0; c < 3; c++) {
            if (c + 1 == selected) {
                display(button[c] + string((size_t)11 - button[c].length(),' ') + "��");
                continue;
            }
            //else
            display(button[c]);
        }
        getKey:{
            switch (_getch()) {
            case 27:
                stage = Stage::exit;
                return;
            case 83:
            case 115:
                start();
                return;
            case 99:
            case 67:
                editConfig();
                return;
            case 13:
                switch (selected) {
                case 1:
                    start();
                    return;
                case 2:
                    editConfig();
                    return;
                case 3:
                    stage = Stage::exit;
                    return;
                default:
                    goto getKey;
                }
            case 224:
                switch (_getch()) {
                case 72:
                case 77:
                    // up&right
                    if (selected == 0) {
                        selected = 1;
                        break;
                    }
                    selected--;
                    if (selected == 0)
                        selected = 3;
                    break;
                case 75:
                case 80:
                    //left&down
                    selected = (selected % 3) + 1;
                    break;
                default:
                    goto getKey;
                }
                break;

            default:
                goto getKey;
            }
            //end of getKey
        }
    }
    selected = 0;
    return;
}
const void start() {
    system("cls");
    cpr::Response response = cpr::Get(cpr::Url{ "https://api.mojang.com/users/profiles/minecraft/" + option[0] });
    if (response.error) {
        sendMessage("ERROR: Failed connecting to mojang api");
        sendMessage(response.error.message);
        WAIT;
        return;
    }
    if (response.text == "") {
        sendMessage("ERROR: no response text from mojang api");
        WAIT;
        return;
    }
    json response_json = json::parse(response.text);
    if (response_json.contains("error")) {
        sendMessage(response_json["error"]);
        WAIT;
        return;
    }
    if (!response_json.contains("name")) {
        sendMessage("ERROR: no name from response json (" + response.text + ")");
        WAIT;
        return;
    }
    if (!response_json.contains("id")) {
        sendMessage("ERROR: no id from response json ( " + response.text + " )");
        WAIT;
        return;
    }


    option[0] = response_json["name"];
    stage = Stage::status;
    delay = stoi(option[4]);
    if (delay <= 0)
        delay = 30;
    thread updater(statusUpdater, response_json["id"]);
    while (stage == Stage::status)
        switch (_getch()) {
        case 27:
            stage = Stage::menu;
            break;
        }
    updater.join();
    return;
}
const void pauseAtError(string response){
    display("response error:");
    display(response);
    discordSend("Response error: " + response);
    WAIT;
    return;
}
void loadConfig() {
    system("cls");
    ifstream fc;
    string uuid, fcc;
    size_t fccf = 0;
    display("loading config...");
    fc.open("config.txt");
    while (fc >> fcc) {
        fccf = fcc.find_first_of('=');
        if (fccf)
            for (char i = 0; i < 6; i++)
                if (fcc.substr(0, fccf) == (string)optionName[i]) {
                    option[i] = fcc.substr(fccf+1, fcc.length());
                    break;
                }
    }
    for (char i = 0; i < 6; i++)
        if (option[i] == "") {
            display(optionName[i] << "is missing!");
            system("pause");
        }
    return;
}
void createConfig() {
    system("cls");
    ofstream ofs("config.txt");
    //ofs.open("config.txt");
    if (!ofs.is_open()) {
        cout << "Failed to open config.txt.\n";
        cin.get();
        return;
    }
    display("Username:");
    cin >> option[0];
    display("Hypixel api key('/api new' to get):");
    cin >> option[1];
    display("send discord message or not(true or false):");
    cin >> option[2];
    if (option[2] == "true") {
        display("disord web hook url:");
        cin >> option[3];
    }
    display("Checkdelay(how long check once)(sec):");
    cin >> option[4];
    if (option[2] == "true") {
        display("Extramessage(can be @ping):");
        cin >> option[5];
    }
    //ofstream ofs;
    //char name[6][21] = { "Username", "Hypixel_API_Key", "Discord_send_message", "Discord_Web_Hook", "Checkdelay", "ExtraMessage" };
    //string score[] = {c1, c2, c3, c4, c5, c6 };
    ofs.open("config.txt");
    if (!ofs.is_open()) {
        display("Failed to open config.txt.\n");
        cin.get();
        return;
    }
    for (char i = 0; i < 6; i++)
        ofs << optionName[i] << "=" << option[i] << "\n";
    ofs.close();
    return;
}
void editConfig() {
    // not completed
    system("cls");
    display("loading config...");
    loadConfig();
    char selected = 0;
    static const char button[][27] = {"Username","hypixel api key","discord send message","discord web hook","checkdelay","extramessage","save","back"};
    static const char name[6][22]{"username:","hypixel api key:","discord send message:","discord web hook:","check delay(sec):","extra message:" };
    static ofstream file;
    
    stage = Stage::editconfig;
    while (stage==Stage::editconfig) {
        system("cls");
        for (char i = 0; i < 6; i++) {
            if (i + 1 == selected)
                display((string)button[i] + string(((size_t)21 - ((string)button[i]).length()), ' ') + "��");
            else
                display(button[i]);
        }
        display("-------");
        for (char i = 6; i < 8; i++) {
            if (i + 1 == selected)
                display((string)button[i] + string(((size_t)21 - ((string)button[i]).length()), ' ') + "��");
            else
                display(button[i]);
        }
        getKey:{
            switch (_getch()) {
            case 27:
                stage = Stage::menu;
                return;
            case 13: //Enter
                switch (selected) {
                case 1: // username
                case 2: // hypixel api key
                case 3: // discord send message
                case 4: // discord web hook
                case 5: // check delay
                case 6: // extra message
                    system("cls");
                    display(name[selected - 1]);
                    cin >> option[selected-1];
                    break;
                case 7: //save
                    file.open("config.txt");
                    file.clear();
                    for (char i = 0; i < 6; i++)
                        file << optionName[i] << "=" << option[i] << "\n";
                    file.close();
                case 8: //back
                    stage = Stage::menu;
                    return;
                }
                break;
            case 224:
                // arrow keys
                switch (_getch()) {
                case 72:
                case 77:
                    // up&right
                    if (selected == 0) {
                        selected = 1;
                        break;
                    }
                    //else
                    selected--;
                    if (selected == 0)
                        selected = 8;
                    break;
                case 75:
                case 80:
                    //left&down
                    selected = (selected % 8) + 1;
                    break;
                default:
                    goto getKey;
                }
                break;
            default:
                goto getKey;
                break;
            }
            //continue
            //goto getKey;
        }
    }
    return;
}
void statusUpdater(string uuid) {
    json statusjson, pstatus;
    cpr::Response statusreq;
    string msg, gameType, gameMode, gameMap;
    while (stage == Stage::status) {
        time_t beforeUpdated = time(0);
        msg = "";
        statusreq = cpr::Get(cpr::Url{ "https://api.hypixel.net/status?key=" + option[1] + "&uuid=" + uuid });
        LOG(statusreq.text);
        if (statusreq.error)
            msg = "ERROR: Failed connecting to API server";
        else if (statusreq.text == "")
            msg = "ERROR: got no result";
        else {
            pstatus = statusjson;
            statusjson = json::parse(statusreq.text);
            if (!statusjson.contains("success"))
                pauseAtError(statusreq.text);
            if (statusjson["success"] != true) {
                if (!statusjson.contains("cause"))
                    pauseAtError(statusreq.text);
                msg = "ERROR: " + (string)statusjson["cause"];

            }
            else if (pstatus != statusjson)
                msg = getStatusMessage(statusjson);

        }
        if (msg != "") {
            string time = Now();
            display(time + " " + msg);
            if (option[2] == "true") {
                discordSend(("``" + time + " " + msg + " " + "``" + option[5]));
            }
        }
        while (time(0) - beforeUpdated < delay && stage == Stage::status)
            this_thread::sleep_for(chrono::milliseconds(10));
    }
    return;
}
const string getStatusMessage(json status) {
    string gameType, gameMode, gameMap;
    if (!status.contains("session"))
        pauseAtError(status);
    if (!status["session"].contains("online"))
        pauseAtError(status);
    if (status["session"]["online"] != true)    
        return (option[0] + " is now Offline");
    else {
        if (!status["session"].contains("gameType"))
            pauseAtError(status);
        gameType = status["session"]["gameType"];
        if (gameType == "SKYBLOCK") {
            if (!status["session"].contains("mode"))
                pauseAtError(status);
            gameMode = status["session"]["mode"];
            if (gameMode == "dynamic")
                gameMode = "Private Island";
            else if (gameMode == "mining_1")
                gameMode = "Gold Mine";
            else if (gameMode == "mining_2")
                gameMode = "Deep Caverns";
            else if (gameMode == "mining_3")
                gameMode = "Dwarven Mines";
            else if (gameMode == "combat_1")
                gameMode = "Spider's Den";
            else if (gameMode == "crimson_isle")
                gameMode = "Crimson Isle";
            else if (gameMode == "combat_3")
                gameMode = "The End";
            else if (gameMode == "farming_1")
                gameMode = "The Barn";
            else if (gameMode == "foraging_1")
                gameMode = "The Park";
            else if (gameMode == "dungeon_hub")
                gameMode = "Dungeon hub";
            else if (gameMode == "dungeon")
                gameMode = "Dungeon";
            else if (gameMode == "instanced")
                gameMode = "Kuudra Fight";
            return (option[0] + " is in Skyblock " + gameMode);
        }
        else if (gameType == "HOUSING") {
            if (!status["session"].contains("mode"))
                pauseAtError(status);
            gameMode = status["session"]["mode"];
            if (gameMode == "LOBBY")
                return (option[0] + " is in Housing lobby");
            return (option[0] + " is playing Housing");
        }
        else {
            if (!status["session"].contains("mode"))
                pauseAtError(status);
            gameMode = status["session"]["mode"];
            if (gameMode != "LOBBY") {
                if (!status["session"].contains("map"))
                    pauseAtError(status);
                gameMap = status["session"]["map"];
                return (option[0] + " is in " + formattedText(gameType) + " " + formattedText(gameMode) + " - " + formattedText(gameMap));
            }
            return (option[0] + " is in " + formattedText(gameType) + " lobby");
        }
    }
}
const void discordSend(string message) {
    cpr::Response result = cpr::Post(cpr::Url{ option[3]},
        cpr::Payload{ {"Content-Type", "application/json"},{"username","Hypixel Player Status"}, {"content", message} });
    if (result.error) {
        display("ERROR: could not connect to web hook");
        display(result.error.message);
        return;
    }
    if (result.text != "") {
        LOG(result.text);
        json result_json = json::parse(result.text);
        system("pause");
        if (!result_json.contains("message") || !result_json.contains("code")) {
            pauseAtError(result.error.message);
            return;
        }
        display((string)result_json["message"] + "  code: " << result_json["code"]);
    }
    return;
}
const void initWindow(){
    system("title Hypixel Player Status");
    HWND console = GetConsoleWindow();
    RECT ConsoleRect;
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prev_mode;
    GetConsoleMode(hInput, &prev_mode);
    GetWindowRect(console, &ConsoleRect);
    SetConsoleMode(hInput, prev_mode & ENABLE_EXTENDED_FLAGS);
    MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 400, 400, true);
    return;
}
/* 
local time with format [HH:MM:SS]
*/
const string Now() {
    static string(*filled)(int) = [](int value){
        stringstream ss;
        ss << value;
        if (ss.str().length() == 1)
            return ("0" + ss.str());
        return ss.str();
    };
    static tm newtime;
    //static struct tm newtime; C
    static time_t now;
    now = time(0);
    localtime_s(&newtime, &now);
    return "[" + filled(newtime.tm_hour) + ":" + filled(newtime.tm_min) + ":" + filled(newtime.tm_sec) + "]";
}
string formattedText(string text) {
    bool sth = true;
    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == ' ') {
            sth = true;
            continue;
        }
        if ('A' <= text[i] && text[i] <= 'Z'){
            if (!sth)
                text[i] += 32;
        }
        else if ('a' <= text[i] && text[i] <= 'z'){
            if (sth)
                text[i] -= 32;
        }
        sth = false;
    }
    return text;
}
const void sendMessage(string message) {
    display(message);
    if (option[2] == "true")
        discordSend(message);
}