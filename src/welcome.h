#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <ctime>
#include <string>
#include <vector>
#include <cstdlib>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace sf;

static void setText(Text& text, float x, float y) {
    FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(Vector2f(x, y));
}

class TextureManager {
    static unordered_map<string, Texture> textures;

public:
    static Texture& getTexture(string textureName);
};

struct {
    bool operator()(pair<string, string> a, pair<string, string> b) const
    {
        int hourA = stoi(a.first.substr(0, 2));
        int minuteA = stoi(a.first.substr(3, 2));
        int hourB = stoi(b.first.substr(0, 2));
        int minuteB = stoi(b.first.substr(3, 2));

        if (hourA != hourB)
            return hourA < hourB;
        else
            return minuteA < minuteB;
    }
} compare;


class Leaderboard {
public:
    vector<pair<string, string>> scores; 
    Font font;
    Text content;
    RenderWindow window;
    Leaderboard(int rows, int cols) {
        font.loadFromFile("files/font.ttf");
    }
    void loadScores() {
        ifstream file("files/leaderboard.txt");
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string time, name;
            getline(ss, time, ',');
            getline(ss, name);
            name.erase(remove(name.begin(), name.end(), '*'), name.end());
            scores.push_back({ time, name });
        }
        file.close();

        while (scores.size() > 5) {
            scores.pop_back();
        }
        sort(scores.begin(), scores.end(), compare);
    }

    void saveScores() {
        ofstream file("files/leaderboard.txt");
        for (auto& score : scores) {
            file << score.first << "," << score.second << "\n";
        }
        file.close();
    }
    void addScore(string time, string name) {
        int newTime = stoi(time.substr(0, 2)) * 60 + stoi(time.substr(3, 2));
        bool newScoreAdded = false;

        vector<pair<string, string>> updatedScores;

        for (auto& score : scores) {
            int existingTime = stoi(score.first.substr(0, 2)) * 60 + stoi(score.first.substr(3, 2));

            if (newTime < existingTime && !newScoreAdded) {
                updatedScores.push_back({ time, name + "*" });
                newScoreAdded = true;
            }
            updatedScores.push_back(score);
        }
        if (!newScoreAdded) {
            updatedScores.push_back({ time, name + "*" });
        }
        scores = updatedScores;

        while (scores.size() > 5) {
            scores.pop_back();
        }
        saveScores();
    }
    void show(bool gameWon, const string& playerName, int cols, int rows, bool& leaderboardOn) {
        window.create(VideoMode((cols * 16), (rows * 16) + 50), "Leaderboard", Style::Close);
        loadScores();
        string str;
        for (int i = 0; i < scores.size(); i++) {
            str += to_string(i + 1) + ".\t" + scores[i].first + "\t" + scores[i].second;
            if (gameWon && scores[i].second == playerName) {
                str += "*";
            }
            str += "\n\n";
        }
        content.setString(str);

        int leaderWidth = cols * 16;
        int leaderHeight = rows * 16 + 50;

        Text title("LEADERBOARD", font, 20);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold | Text::Underlined);
        setText(title, leaderWidth / 2, leaderHeight / 2 - 120);

        content.setFont(font);
        content.setString(str);
        content.setCharacterSize(18);
        content.setFillColor(Color::White);
        content.setStyle(sf::Text::Bold);
        setText(content, leaderWidth / 2, leaderHeight / 2 + 20);

        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                }
            }
            window.clear(Color::Blue);
            window.draw(title);
            window.draw(content);
            window.display();
        }
        leaderboardOn = !leaderboardOn;
    }
};

class Minesweeper {
public:
    vector<vector<int>> board; 
    vector<vector<bool>> revealed; 
    vector<vector<bool>> flagged; 
    vector<vector<vector<pair<int, int>>>> neighbors; 
    vector<vector<bool>> preLeaderboardRevealed;
    int rows, cols, mines;
    bool gameOver, gameWon, gamePaused, debugMode, gameStarted, mineClicked, gameEnd, leaderboardOn, leaderBoardVictory;
    Clock gameClock;
    Time elapsedTime;
    Sprite happyFaceButton, debugButton, pausePlayButton, leaderboardButton;
    int mineCounter;
    Texture digits;
    vector<IntRect> digitRects;
    Leaderboard leaderboard;

public:
    Minesweeper(int rows, int cols, int mines) : rows(rows), cols(cols), mines(mines), board(rows, vector<int>(cols, 0)), 
        revealed(rows, vector<bool>(cols, false)), flagged(rows, vector<bool>(cols, false)), leaderboardOn(false), leaderBoardVictory(false),
        neighbors(rows, vector<vector<pair<int, int>>>(cols)), gameOver(false), gameWon(false), mineClicked(false), gameEnd(false),
        gamePaused(false), debugMode(false), gameStarted(false), mineCounter(mines), leaderboard(rows, cols) {
        
        srand(time(0)); 
        for (int i = 0; i < mines; i++) {
            int x, y;
            do {
                x = rand() % rows;
                y = rand() % cols;
            } while (board[x][y] == -1); 
            board[x][y] = -1; 
        }

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                
                if (board[i][j] != -1) {
                    for (int x_comp = -1; x_comp <= 1; x_comp++) {
                        for (int y_comp = -1; y_comp <= 1; y_comp++) {
                            int nx_comp = i + x_comp, ny_comp = j + y_comp;
                            if (nx_comp >= 0 && ny_comp >= 0 && nx_comp < rows && ny_comp < cols && board[nx_comp][ny_comp] == -1) {
                                board[i][j]++; 
                            }
                            neighbors[i][j].push_back({ nx_comp, ny_comp });
                        }
                    }
                }
            }
        }
    }

    void resetGame(int newRows, int newCols, int newMines) {
        this->rows = newRows;
        this->cols = newCols;
        this->mines = newMines;

        this->board = vector<vector<int>>(rows, vector<int>(cols, 0));
        this->revealed = vector<vector<bool>>(rows, vector<bool>(cols, false));
        this->flagged = vector<vector<bool>>(rows, vector<bool>(cols, false));

        this->gameOver = false;
        this->gameWon = false;
        this->gamePaused = false;
        this->gameEnd = false;
        this->leaderboardOn = leaderboardOn;
        this->debugMode = false;
        this->gameStarted = true;
        this->mineClicked = false;
        this->mineCounter = mines;
        this->leaderBoardVictory = leaderBoardVictory;

        srand(time(0)); 
        for (int i = 0; i < mines; i++) {
            int x, y;
            do {
                x = rand() % rows;
                y = rand() % cols;
            } while (board[x][y] == -1); 
            board[x][y] = -1; 
        }

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (board[i][j] != -1) {
                    for (int x_comp = -1; x_comp <= 1; x_comp++) {
                        for (int y_comp = -1; y_comp <= 1; y_comp++) {
                            int nx_comp = i + x_comp, ny_comp = j + y_comp;
                            if (nx_comp >= 0 && ny_comp >= 0 && nx_comp < rows && ny_comp < cols && board[nx_comp][ny_comp] == -1) {
                                board[i][j]++; 
                            }
                        }
                    }
                }
            }
        }
    }

    void draw(RenderWindow& window) {

        digits.loadFromFile("files/images/digits.png");
        for (int i = 0; i < 11; i++) {
            digitRects.push_back(IntRect(i * 21, 0, 21, 32));
        }
        if (!gameEnd && !leaderBoardVictory) {
            happyFaceButton.setTexture(TextureManager::getTexture("face_happy"));
            happyFaceButton.setPosition((cols / 2.0 * 32) - 32, 32 * (rows + 0.5));
        }
        debugButton.setTexture(TextureManager::getTexture("debug"));
        debugButton.setPosition((cols * 32) - 304, 32 * (rows + 0.5));
        pausePlayButton.setTexture(TextureManager::getTexture("pause"));
        pausePlayButton.setPosition((cols * 32) - 240, 32 * (rows + 0.5));
        leaderboardButton.setTexture(TextureManager::getTexture("leaderboard"));
        leaderboardButton.setPosition((cols * 32) - 176, 32 * (rows + 0.5));

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (gamePaused || leaderboardOn) {
                    Sprite tileSprite(TextureManager::getTexture("tile_revealed"));
                    tileSprite.setPosition(j * 32, i * 32);
                    window.draw(tileSprite);
                    pausePlayButton.setTexture(TextureManager::getTexture("play"));
                }
                else {
                    Sprite tileSprite(TextureManager::getTexture("tile_hidden"));
                    tileSprite.setPosition(j * 32, i * 32);
                    window.draw(tileSprite);

                    if (flagged[i][j]) {
                        Sprite flagSprite(TextureManager::getTexture("flag"));
                        flagSprite.setPosition(j * 32, i * 32);
                        window.draw(flagSprite);
                    }

                    if (leaderBoardVictory && gameEnd) {
                        happyFaceButton.setTexture(TextureManager::getTexture("face_win"));
                        window.draw(happyFaceButton);
                    }

                    if ((revealed[i][j] && !flagged[i][j]) || (debugMode && board[i][j] == -1)) {
                        Sprite overlaySprite;
                        overlaySprite.setTexture(TextureManager::getTexture("tile_revealed"));
                        overlaySprite.setPosition(j * 32, i * 32);
                        window.draw(overlaySprite);
                        if (board[i][j] == -1) {
                            overlaySprite.setTexture(TextureManager::getTexture("tile_hidden"));
                            overlaySprite.setPosition(j * 32, i * 32);
                            window.draw(overlaySprite);
                            overlaySprite.setTexture(TextureManager::getTexture("mine"));

                        }
                        else if (board[i][j] > 0) {
                            overlaySprite.setTexture(TextureManager::getTexture("number_" + to_string(board[i][j])));
                        }
                        overlaySprite.setPosition(j * 32, i * 32);
                        window.draw(overlaySprite);
                    }

                    if ((gameEnd && board[i][j] == -1) && !gameWon) {
                        Sprite mineSprite(TextureManager::getTexture("mine"));
                        happyFaceButton.setTexture(TextureManager::getTexture("face_lose"));
                        mineSprite.setPosition(j * 32, i * 32);
                        window.draw(mineSprite);
                    }

                    if (flagged[i][j]) {
                        Sprite flagSprite(TextureManager::getTexture("flag"));
                        flagSprite.setPosition(j * 32, i * 32);
                        window.draw(flagSprite);

                        if ((gameEnd && board[i][j] == -1) && !gameWon) {
                            Sprite mineSprite(TextureManager::getTexture("mine"));
                            happyFaceButton.setTexture(TextureManager::getTexture("face_lose"));
                            mineSprite.setPosition(j * 32, i * 32);
                            window.draw(mineSprite);
                        }
                    }
                }
            }
        }

        if (!mineClicked) {
            int time = gameStarted ? ((gamePaused || gameOver || leaderboardOn || gameEnd || leaderBoardVictory) ? elapsedTime.asSeconds() : gameClock.getElapsedTime().asSeconds() + 
                elapsedTime.asSeconds()) : 0;
            int minutes = time / 60;
            int seconds = time % 60;
            for (int i = 0; i < 2; i++) {
                Sprite digitSprite(digits, digitRects[minutes % 10]);
                digitSprite.setPosition((cols * 32) - 97 + (1 - i) * 21, 32 * (rows + 0.5) + 16);
                window.draw(digitSprite);
                minutes /= 10;
            }
            for (int i = 0; i < 2; i++) {
                Sprite digitSprite(digits, digitRects[seconds % 10]);
                digitSprite.setPosition((cols * 32) - 54 + (1 - i) * 21, 32 * (rows + 0.5) + 16);
                window.draw(digitSprite);
                seconds /= 10;
            }
            string counterStr = to_string(abs(mineCounter));

            while (counterStr.size() < 3) {
                counterStr = "0" + counterStr;
            }

            if (mineCounter < 0) {
                Sprite minusSprite(digits, digitRects[10]);
                minusSprite.setPosition(12, 32 * (rows + 0.5) + 16);
                window.draw(minusSprite);
            }

            for (int i = 0; i < 3; i++) {

                int digit = counterStr[i] - '0';

                Sprite digitSprite(digits, digitRects[digit]);
                digitSprite.setPosition(33 + i * 21, 32 * (rows + 0.5) + 16);

                window.draw(digitSprite);
            }
        }

        window.draw(happyFaceButton);
        window.draw(debugButton);
        window.draw(pausePlayButton);
        window.draw(leaderboardButton);
    }

    void revealEmptyTiles(int row, int col) {
        for (int x_comp = -1; x_comp <= 1; x_comp++) {
            for (int y_comp = -1; y_comp <= 1; y_comp++) {
                int nx_comp = row + x_comp;
                int ny_comp = col + y_comp;

                if (nx_comp >= 0 && ny_comp >= 0 && nx_comp < rows && ny_comp < cols && !revealed[nx_comp][ny_comp]) {
                    revealed[nx_comp][ny_comp] = true;

                    if (board[nx_comp][ny_comp] == 0) {
                        revealEmptyTiles(nx_comp, ny_comp);
                    }
                    if (flagged[nx_comp][ny_comp]) {
                        revealed[nx_comp][ny_comp] = false;
                    }
                }
            }
        }
    }

    void handleReveal(int row, int col) {
        revealed[row][col] = true;
        
        if (board[row][col] == 0) {
            revealEmptyTiles(row, col);
        }
    }

    string formatTime(Time elapsedTime) {
        int totalSeconds = static_cast<int>(elapsedTime.asSeconds());
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        stringstream ss;
        ss << std::setw(2) << std::setfill('0') << minutes << ":" << std::setw(2) << std::setfill('0') << seconds;
        return ss.str();
    }
};