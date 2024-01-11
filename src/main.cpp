#include <iostream>
#include <SFML/Graphics.hpp>
#include <cctype>
#include <string>
#include <fstream>
#include <sstream>
#include "welcome.h"

using namespace std;
using namespace sf;

int main()
{
    Font font;
    font.loadFromFile("files/font.ttf");
    ifstream config("files/board_config.cfg");
    int cols, rows, mines;
    config >> cols >> rows >> mines;
    config.close();

    int windowWidth = cols * 32;
    int windowHeight = rows * 32 + 100;

    RenderWindow window(VideoMode(windowWidth, windowHeight), "Welcome to Minesweeper!", Style::Close);

    Text welcomeText("WELCOME TO MINESWEEPER!", font, 24);
    welcomeText.setFillColor(Color::White);
    welcomeText.setStyle(Text::Bold | Text::Underlined);
    setText(welcomeText, windowWidth / 2, windowHeight / 2 - 150);

    Text inputText("Enter your name:", font, 20);
    inputText.setFillColor(Color::White);
    inputText.setStyle(Text::Bold);
    setText(inputText, windowWidth / 2, windowHeight / 2 - 75);

    Text nameText("", font, 18);
    nameText.setFillColor(Color::Yellow);
    nameText.setStyle(Text::Bold);
    setText(nameText, windowWidth / 2, windowHeight / 2 - 45);

    Text cursorText("|", font, 18);
    cursorText.setFillColor(Color::Yellow);
    cursorText.setStyle(Text::Bold);
    
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
                return 0;  
            }
            else if (event.type == Event::TextEntered)
            {
                if (event.text.unicode < 128)
                {
                    char c = static_cast<char>(event.text.unicode);
                    if (isalpha(c))
                    {
                        if (nameText.getString().getSize() < 10)
                        {
                            string name = nameText.getString();
                            name += tolower(c);
                            if (name.size() == 1)
                            {
                                name[0] = toupper(name[0]);
                            }
                            nameText.setString(name);
                            setText(nameText, window.getSize().x / 2, window.getSize().y / 2 - 45);
                        }
                    }
                    else if (c == '\b' && nameText.getString().getSize() > 0)
                    {
                        string name = nameText.getString();
                        name.pop_back();
                        nameText.setString(name);
                        setText(nameText, window.getSize().x / 2, window.getSize().y / 2 - 45);
                    }
                    else if (c == '\r' && nameText.getString().getSize() > 0)
                    {
                        window.close();

                        Minesweeper game(rows, cols, mines);
                        game.leaderboard.loadScores();
                        RenderWindow gameWindow(VideoMode(windowWidth, windowHeight), "Minesweeper", Style::Close);
                        
                        game.gameStarted = true;

                        while (gameWindow.isOpen()) {
                            Event gameEvent;
                            while (gameWindow.pollEvent(gameEvent)) {
                                if (gameEvent.type == Event::Closed) {
                                    gameWindow.close();
                                    return 0;
                                }

                                else if (gameEvent.type == Event::MouseButtonPressed) {

                                    Vector2i mousePos = Mouse::getPosition(gameWindow);
                                    int positionX = mousePos.x / 32;
                                    int positionY = mousePos.y / 32;

                                    if (gameEvent.mouseButton.button == Mouse::Left) {

                                        if (game.gameOver || !game.gameOver) {
                                            if (game.leaderboardButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                                                game.leaderboardOn = !game.leaderboardOn;
                                                if (game.leaderboardOn) {
                                                    for (int i = 0; i < rows; i++) {
                                                        for (int j = 0; j < cols; j++) {
                                                            Sprite tileSprite(TextureManager::getTexture("tile_revealed"));
                                                            tileSprite.setPosition(j * 32, i * 32);
                                                            gameWindow.draw(tileSprite);
                                                        }
                                                    }
                                                    gameWindow.display();
                                                    if (game.gameEnd == false) {

                                                        if (!game.gamePaused) {
                                                            game.elapsedTime += game.gameClock.getElapsedTime();
                                                            game.leaderboard.show(game.gameWon, nameText.getString(), cols, rows, game.leaderboardOn);
                                                            game.gameClock.restart();
                                                        }
                                                        else {
                                                            game.leaderboard.show(game.gameWon, nameText.getString(), cols, rows, game.leaderboardOn);
                                                        }
                                                    }
                                                    else {
                                                        game.leaderboard.show(game.gameWon, nameText.getString(), cols, rows, game.leaderboardOn);
                                                    }
                                                }
                                            }

                                            if (game.happyFaceButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                                                if (game.gamePaused) {
                                                    game.pausePlayButton.setTexture(TextureManager::getTexture("pause"));
                                                }
                                                game.resetGame(rows, cols, mines);
                                                game.gameStarted = true;
                                                game.gameClock.restart();
                                                game.elapsedTime = Time();
                                            }
                                        }
                                        if (!game.gameEnd) {
                                            if (game.pausePlayButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                                                if (!game.gameOver) {
                                                    game.gamePaused = !game.gamePaused;
                                                    if (game.gamePaused) {
                                                        game.pausePlayButton.setTexture(TextureManager::getTexture("play"));
                                                        game.elapsedTime += game.gameClock.getElapsedTime();
                                                    }
                                                    else {
                                                        game.pausePlayButton.setTexture(TextureManager::getTexture("pause"));
                                                        game.gameClock.restart();
                                                    }
                                                }
                                            }

                                            if (game.debugButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                                                if (!game.gameOver && !game.gamePaused) {
                                                    game.debugMode = !game.debugMode;
                                                }
                                            }

                                            else if (positionX >= 0 && positionY >= 0 && positionX < cols && positionY < rows && !game.gamePaused && !game.flagged[positionY][positionX])  {

                                                game.revealed[positionY][positionX] = true;
                                                if (game.board[positionY][positionX] == 0) {
                                                    game.handleReveal(positionY, positionX);
                                                }


                                                if (game.board[positionY][positionX] == -1) {
                                                    game.gameEnd = !game.gameEnd;
                                                    game.elapsedTime += game.gameClock.getElapsedTime();
                                                }

                                                bool allNonMinesRevealed = true;
                                                for (int i = 0; i < rows; i++) {
                                                    for (int j = 0; j < cols; j++) {
                                                        if (game.board[i][j] != -1 && !game.revealed[i][j]) {
                                                            allNonMinesRevealed = false;
                                                        }
                                                    }
                                                }
                                                if (allNonMinesRevealed) {
                                                    game.gameWon = true;
                                                    for (int i = 0; i < rows; i++) {
                                                        for (int j = 0; j < cols; j++) {
                                                            if (game.board[i][j] == -1) {
                                                                if (game.flagged[i][j] == false) {
                                                                    game.flagged[i][j] = true;
                                                                    game.mineCounter--;
                                                                }
                                                            }
                                                        }
                                                    }
                                                    game.gameEnd = !game.gameEnd;
                                                    game.elapsedTime += game.gameClock.getElapsedTime();
                                                    string winningTime = game.formatTime(game.elapsedTime);
                                                    game.leaderboard.addScore(winningTime, nameText.getString());
                                                    game.leaderBoardVictory = !game.leaderBoardVictory;
                                                    if (game.leaderBoardVictory) {
                                                        game.draw(gameWindow);
                                                        gameWindow.display();
                                                    }
                                                    game.leaderboard.show(game.gameWon, nameText.getString(), cols, rows, game.leaderBoardVictory);

                                                }
                                            }
                                        }
                                    }
                                    if (gameEvent.mouseButton.button == Mouse::Right) {

                                        if (!game.gameEnd) {

                                            if (positionX >= 0 && positionY >= 0 && positionX < cols && positionY < rows && !game.gamePaused) {
                                                game.flagged[positionY][positionX] = !game.flagged[positionY][positionX];
                                                if (game.flagged[positionY][positionX]) {
                                                    game.mineCounter--;
                                                }
                                                else {
                                                    Sprite tileSprite(TextureManager::getTexture("tile_hidden"));
                                                    for (int i = 0; i < rows; i++) {
                                                        for (int j = 0; j < cols; j++) {
                                                            tileSprite.setPosition(j * 32, i * 32);
                                                        }
                                                    }
                                                    gameWindow.draw(tileSprite);
                                                    game.mineCounter++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            gameWindow.clear(Color::White);
                            if (!game.leaderboardOn) {
                                game.draw(gameWindow);
                            }
                            gameWindow.display();
                        }
                    }
                }
            }
        }

        FloatRect nameBounds = nameText.getGlobalBounds();
        setText(cursorText, window.getSize().x / 2 + nameBounds.width / 2.0f + 3, windowHeight / 2 - 45);

        window.clear(Color::Blue);
        window.draw(welcomeText);
        window.draw(inputText);
        window.draw(nameText);
        window.draw(cursorText);
        window.display();
    }
    return 0;
}