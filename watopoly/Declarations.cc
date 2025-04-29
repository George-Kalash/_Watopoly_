export module watopoly;

// Import necessary standard library modules
import <vector>;
import <string>;
import <map>;
import <utility>;
import <random>;
import <sstream>;
import <iostream>;

using namespace std;
using std::size_t;

//----------------------------------
// Forward declarations
//----------------------------------

export class Player;
export class Tile;
export class CommandInterpreter;
export class AcademicBuilding;
export class Property;

//----------------------------------
// DICE
//----------------------------------
export class Dice {
private:
    int die1;
    int die2;
    bool isTestMode;

public:
    Dice(bool testMode = false);
    void roll();
    void setTestDice(int d1, int d2);
    int getTotal() const;
    bool isDoubles() const;
};

//----------------------------------
// BANK
//----------------------------------
export class Bank {
    private:
      int money;
    public:
      Bank(int initialMoney = 100000) : money{initialMoney} {}
  
      void collectMoney(int amount) {
          money += amount;
      }
  
      bool payMoney(int amount) {
          if (money >= amount) {
              money -= amount;
              return true;
          } else {
              return false;
          }
      }
  
      int getBalance() const {
          return money;
      }
  };

//----------------------------------
// PLAYER
//----------------------------------
export class Player {
  private:
    std::string name;
    char piece;
    int money;
    int position;
    bool inTimsLine;
    int turnsInTimsLine;
    int timsCups;
    std::vector<Property*> properties;

  public:
    Player(std::string name, char piece);

    void move(int steps);
    void teleport(int destination);
    void incrementTurnsInTimsLine(); // new stuff
    bool payMoney(int amount, Player* recipient = nullptr);
    bool payMoneyToBank(int amount, Bank& recipient);
    void receiveMoney(int amount);
    bool validStep(int step);
    bool ownsMonopoly(std::string blockName);
    bool buyProperty(Property* property);
    bool buyImprovement(AcademicBuilding* property);
    bool unmortgageProperty(Property* property);
    void sellProperty(Property* property);
    bool mortgageProperty(Property* property);
    bool sellImprovement(AcademicBuilding* property);
    int getNetWorth();
    void tradePropertyInsteadOfPaying(Property* property, Player* recipient);
    void declaredBankruptcy(Player* creditor = nullptr);
    bool canPayAmount(int amount);
    void enterTimsLine();
    void leaveTimsLine();
    void addTimsCup();
    bool useTimsCup();
    std::string getAssets();
    
    // Inline getters
    std::string getName() const { return name; }
    char getPiece() const { return piece; }
    int getMoney() const { return money; }
    int getPosition() const { return position; }
    bool isInTimsLine() const { return inTimsLine; }
    int getTurnsInTimsLine() const { return turnsInTimsLine; }
    int getTimsCups() const { return timsCups; }
    const std::vector<Property*>& getProperties() const { return properties; }
};

//----------------------------------
// TILE
//----------------------------------
export class Tile {
  private: 
    std::string name;
    std::size_t location;
      
  public:
    Tile(string name, std::size_t location): name{name}, location{location} {}
    virtual ~Tile() = default;

    // fetches the name of the tile
    virtual std::string getName() const {
      return this->name;
    }
        
    virtual std::size_t getLocation() const {
      return this->location;
    }
    
    // Pure virtual method that must be implemented by all derived classes
    virtual void landedOn(Player* player) = 0;
};


//----------------------------------
// PROPERTY
//----------------------------------
export class Property : public Tile {
private:
  size_t purchaseCost;
  bool mortgaged;
  Player* owner;
  
public:
  Property(string name, size_t location, size_t purchaseCost);
  virtual ~Property();
  
  size_t getPurchaseCost() const;
  bool isMortgaged() const;
  virtual bool mortgage();
  virtual bool unmortgage();
  void setOwner(Player* newOwner);
  Player* getOwner() const;
  
  // Pure virtual method for getting tuition
  virtual int getTuition() = 0;
  
  // Override landedOn from Tile
  virtual void landedOn(Player* player) override;
};




//----------------------------------
// ACADEMIC BUILDING
//----------------------------------
export class AcademicBuilding : public Property {
private:
    int tuitionBase;
    int improvementCost;
    int improvements;
    vector<int> tuitionWithImprovements;
    string monopolyBlock;

public:
    AcademicBuilding(string name, int position, int cost, int improvementCost, string block, vector<int> tuitionValues);

    int getTuition() override;
    bool addImprovement();
    bool removeImprovement();
    int getImprovements() const;
    int getImprovementCost() const;
    string getMonopolyBlock() const;
    bool canMortgage() const;
    bool mortgage() override;
};

//----------------------------------
// RESIDENCE
//----------------------------------
export class Residence : public Property {
    static const int baseRent = 25;  // Base rent value for a single residence

public:
    Residence(std::string name, int position);
    int getTuition() override;
};

//----------------------------------
// GYM
//----------------------------------
export class Gym : public Property {
private:
    static const int multiplierOwnedOne = 4;
    static const int multiplierOwnedTwo = 10;
    int diceRoll;

public:
    Gym(std::string name, int position);
    void setDiceRoll(int total);
    int getTuition() override;
};

//----------------------------------
// SPECIAL TILES
//----------------------------------

// COLLECT OSAP
export class CollectOSAP : public Tile {
public:
    CollectOSAP(int position);
    void landedOn(Player* player) override;
};

// COOP FEE
export class CoopFee : public Tile {
public:
    CoopFee(int position);
    void landedOn(Player* player) override;
};

// GO TO TIMS
export class GoToTims : public Tile {
public:
    GoToTims(int position);
    void landedOn(Player* player) override;
};

// TIMS LINE
export class TimsLine : public Tile {
public:
    TimsLine(int position);
    ~TimsLine() override = default;
    void landedOn(Player* player) override;
};

// NEEDLES HALL
export class NeedlesHall : public Tile {
private:
    std::vector<std::pair<int, double>> money_changes;

public:
    NeedlesHall(int position);
    void landedOn(Player* player) override;
    bool tryGiveTimsCup(Player* player);
};

// SLC (Student Life Centre)
export class SLC : public Tile {
private:
    std::vector<std::pair<int, double>> movements;

public:
    SLC(int position);
    void landedOn(Player* player) override;
};

// TUITION
export class Tuition : public Tile {
public:
    Tuition(int position);
    void landedOn(Player* player) override;
};

// GOOSE NESTING
export class GooseNesting : public Tile {
private:
    const std::vector<std::string> gooseMessages = {
        "You got attacked by a flock of angry geese!",
        "The geese hiss and chase you around the square!",
        "Honk! Honk! The geese won't let you pass!",
        "You accidentally step on a goose nest - they're furious!",
        "The geese steal your lunch money but otherwise leave you unharmed."
    };

public:
    GooseNesting(int position);
    void landedOn(Player* player) override;
};

//----------------------------------
// BOARD
//----------------------------------
export class Board { 
  private:
    vector<Tile*> tiles{};
    map<string, vector<AcademicBuilding*>> academicBlocks;

  public:
    Board();
    void initializeBoard();
    void display();
    void printTileInfo() const;
    Tile* getTile(int position);
    Property* getPropertyByName(const string& name);
    void movePlayer(Player* player, int steps);
    void teleportPlayer(Player* player, int destination);
    void sendToTimsLine(Player* player);
};

//----------------------------------
// GAME
//----------------------------------
export class Game {
    private:
        Board board;
        std::vector<Player*> players;
        int currentPlayerIndex;
        class Dice dice;
        class Bank bank;
        int maxTimsCups;
        bool isTestingMode;
        static Game* instance;
        CommandInterpreter* commandInterpreter;
        
    public:
        int currentTimsCupsInGame;
        Game(bool testMode = false);
        ~Game();
        static Game* getInstance();
        void removeBankruptPlayer(Player* bankruptPlayer);
        Dice& getDice(); 
        Bank& getBank() { return bank; }
        void initialize(int numPlayers);
        static bool canGiveTimsCup();
        void loadGame(std::string filename);
        void saveGame(std::string filename);
        void mainLoop();
        bool canGiveMoreCups();
        void nextPlayer();
        std::vector<Player*> getPlayers();
        Player* getPlayerByName(const std::string& name); // TO IMPLEMENT
        Board& getBoard(); // TO IMPLEMENT
        void processCommand(std::string command);
        void endGame();
        void auctionProperty(Property* property);
        int getNumPlayers() const;
        Player* getCurrentPlayer() const;
};


//----------------------------------
// COMMAND INTERPRETER
//----------------------------------
export class CommandInterpreter {
private:
    Game* game;
    bool testingMode;

public:
    CommandInterpreter(Game* game, bool testingMode = false);
    void parseCommand(const std::string& command);
    
private:
    std::vector<std::string> tokenizeCommand(const std::string& command);
    void executeRoll(const std::vector<std::string>& args);
    void executeNext();
    void executeTrade(const std::vector<std::string>& args);
    void executeImprove(const std::vector<std::string>& args);
    void executeMortgage(const std::vector<std::string>& args);
    void executeUnmortgage(const std::vector<std::string>& args);
    void executeBankrupt();
    void executeAssets();
    void executeAll();
    void executeSave(const std::vector<std::string>& args);
};
