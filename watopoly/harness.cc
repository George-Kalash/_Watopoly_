import <iostream>;
import <fstream>;
import <sstream>;
import <string>;
import watopoly;

using namespace std;

// This is the test harness.
// This file will operate the commands from the std in and cmd args.

int main(int argc, char *argv[]) {
    bool testingMode = false;
    string loadFile = "";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-testing") {
            testingMode = true;
        } else if (arg == "-load" && i + 1 < argc) {
            loadFile = argv[++i];
        }
    }
    
    // Create the game
    Game game(testingMode);
    
    // Load game if specified
    if (!loadFile.empty()) {
        game.loadGame(loadFile);
    } else {
        // Ask for the number of players
        int numPlayers;
        cout << "Enter the number of players (2-6): ";
        cin >> numPlayers;
        
        if (numPlayers < 2 || numPlayers > 6) {
            cout << "Invalid number of players. Setting to 4." << endl;
            numPlayers = 4;
        }
        
        game.initialize(numPlayers);
    }
    
    // Main game loop
    game.mainLoop();
    
    return 0;
}