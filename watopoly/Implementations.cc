module watopoly;

import <iostream>;
import <sstream>;
import <fstream>;
import <random>;
import <algorithm>;
import <vector>;
import <map>;
import <utility>;
import <cstdlib>;
import <ctime>;
import <cctype>; 
import <stdexcept>;

using namespace std;

// Utilities namespace for helper functions
// I think is overkill to make a whole helper namespace but here we are
namespace utilities {
  // Checks if a string is a valid numeric (integer) value
  inline bool isNumeric(const std::string& str) {
    // Empty string is not numeric
    if (str.empty()) return false;
    
    // Check first character for negative sign
    auto it = str.begin();
    if (*it == '-') {
      // If string is just a negative sign with no digits, it's not numeric
      if (str.length() == 1) return false;
      ++it; // Skip the negative sign for further checking
    }
      
  // Check if all remaining characters are digits
    return std::all_of(it, str.end(), [](unsigned char c) { 
      return std::isdigit(c); 
    });
  }
}

//----------------------------------
// DICE IMPLEMENTATION
//----------------------------------

Dice::Dice(bool testMode) : die1{0}, die2{0}, isTestMode{testMode} {}

// Rolls two six-sided dice unless in test mode
void Dice::roll() {
  if (isTestMode) {
    return;
  }

  // Seed the random number generator
  static bool seeded = false;
  if (!seeded) {
    srand(time(nullptr));
    seeded = true;
  }

  // Generate random numbers between 1 and 6
  die1 = (rand() % 6) + 1;
  die2 = (rand() % 6) + 1;
}

// Manually sets dice values
void Dice::setTestDice(int d1, int d2) {
  if (!isTestMode) {
    std::cerr << "Error: Cannot set test dice values when not in test mode." << std::endl;
    return;
  }
  die1 = d1;
  die2 = d2;
}

// Returns the sum of both dice
int Dice::getTotal() const {
  return die1 + die2;
}

// Returns true on a double roll
bool Dice::isDoubles() const {
  return die1 == die2;
}


//----------------------------------
// PROPERTY IMPLEMENTATIONS
//----------------------------------

Property::Property(string name, size_t location, size_t purchaseCost)
  : Tile(name, location), purchaseCost(purchaseCost), mortgaged(false), owner(nullptr) {}

Property::~Property() {}

size_t Property::getPurchaseCost() const {
  return purchaseCost;
}

bool Property::isMortgaged() const {
  return mortgaged;
}

bool Property::mortgage() {
  if (!owner || mortgaged) {
    return false;
  }
  mortgaged = true;
  return true;
}

bool Property::unmortgage() {
  if (!owner || !mortgaged) {
    return false;
  }
  mortgaged = false;
  return true;
}

void Property::setOwner(Player* newOwner) {
  owner = newOwner;
}

Player* Property::getOwner() const {
  return owner;
}

void Property::landedOn(Player* player) {
  if (!player) return;
  
  if (owner && owner != player && !mortgaged) {
    // Player lands on a property owned by someone else
    
    // Check if it's a Gym and set the dice roll
    Gym* gym = dynamic_cast<Gym*>(this);
    if (gym) {
      // Get the current dice total from the game
      Game* gameInstance = Game::getInstance();
      if (gameInstance) {
        Dice& dice = gameInstance->getDice();
        gym->setDiceRoll(dice.getTotal());
      }
    }
    
    int tuition = getTuition();
    
    cout << player->getName() << " landed on " << getName() 
      << " owned by " << owner->getName() 
      << " and must pay $" << tuition << endl;
    
    player->payMoney(tuition, owner);
  }
  else if (!owner) {
    // Property is not owned, offer to buy it
    cout << "Would you like to purchase " << getName() 
      << " for $" << purchaseCost << "? (y/n): ";
    
    char answer;
    cin >> answer;
    
    if (answer == 'y' || answer == 'Y') {
      if (player->canPayAmount(purchaseCost)) {
        player->buyProperty(this);
      }
      else {
        cout << "Not enough money to purchase this property." << endl;
      }
    }
    else {
      // TODO: Trigger auction logic here via Game class -- not needed
    }
  }
}



//----------------------------------
// ACADEMIC BUILDING IMPLEMENTATIONS
//----------------------------------


AcademicBuilding::AcademicBuilding(string name, int position, int cost, int improvementCost, string block, vector<int> tuitionValues)
  : Property(name, position, cost), tuitionBase(cost), improvementCost(improvementCost),
    improvements(0), tuitionWithImprovements(tuitionValues), monopolyBlock(block) {}

int AcademicBuilding::getTuition() {
  if (improvements < tuitionWithImprovements.size()) {
    return tuitionWithImprovements[improvements];
  } else {
    return tuitionBase;
  }
}

bool AcademicBuilding::addImprovement() {
  if (improvements < 5) { // Max of 5 improvements
    improvements++;
    return true;
  }
  return false;
}

bool AcademicBuilding::removeImprovement() {
  if (improvements > 0) {
    improvements--;
    return true;
  }
  return false;
}

int AcademicBuilding::getImprovements() const {
  return improvements;
}

int AcademicBuilding::getImprovementCost() const {
  return improvementCost;
}

string AcademicBuilding::getMonopolyBlock() const {
  return monopolyBlock;
}

bool AcademicBuilding::canMortgage() const {
  return improvements == 0; // Can only mortgage if there are no improvements
}

bool AcademicBuilding::mortgage() {
  if (canMortgage()) {
    // Perform mortgage logic
    cout << "Mortgaging " << getName() << " at " << getPurchaseCost() / 2 << " money." << endl;
    Property::mortgage();
    return true;
  }
  return false;
}

//----------------------------------
// RESIDENCE IMPLEMENTATION
//----------------------------------

Residence::Residence(std::string name, int position) 
  : Property(name, position, 200) {}

int Residence::getTuition() {
  if (!getOwner()) {
    return 0; // No rent if no owner
  }

  // Count the number of residences the owner has
  int numResidences = 0;
  for (const Property* prop : getOwner()->getProperties()) {
    if (dynamic_cast<const Residence*>(prop)) {
      numResidences++;
    }
  }

  switch (numResidences) {
    case 1: return 25;
    case 2: return 50;
    case 3: return 100;
    case 4: return 200;
    default: return 25;
  }
}

//----------------------------------
// GYM IMPLEMENTATION
//----------------------------------
Gym::Gym(std::string name, int position) 
  : Property(name, position, 150), diceRoll(0) {}

void Gym::setDiceRoll(int total) {
  diceRoll = total;
}

int Gym::getTuition() {
  if (!getOwner() || diceRoll == 0) {
    return 0; // No rent if unowned or dice not set
  }

  int numGymsOwned = 0;
  for (const Property* prop : getOwner()->getProperties()) {
    if (dynamic_cast<const Gym*>(prop)) {
      numGymsOwned++;
    }
  }

  // Determine the appropriate multiplier
  int multiplier;
  if (numGymsOwned == 2) {
    multiplier = multiplierOwnedTwo;
  } else {
    multiplier = multiplierOwnedOne;
  }

  return diceRoll * multiplier;
}

//----------------------------------
// COLLECT OSAP IMPLEMENTATION
//----------------------------------
CollectOSAP::CollectOSAP(int position) 
  : Tile("Collect OSAP", position) {}

void CollectOSAP::landedOn(Player* player) {
  // Award $200 for landing on or passing Collect OSAP
  cout << "You landed on Collect OSAP. Receive $200" << endl;
  player->receiveMoney(200);
}

//----------------------------------
// COOP FEE IMPLEMENTATION
//----------------------------------
CoopFee::CoopFee(int position) 
  : Tile("Coop Fee", position) {}

void CoopFee::landedOn(Player* player) {
  const int feeAmount = 150;  // Fixed $150 fee as per requirements

  Game* gameInstance = Game::getInstance();
  Bank& bank = gameInstance->getBank();
  
  cout << "You landed on Coop Fee. You must pay $" << feeAmount << endl;
  
  player->payMoneyToBank(feeAmount, bank);
    // If player can't pay, handle bankruptcy 
}

//----------------------------------
// GO TO TIMS IMPLEMENTATION
//----------------------------------
GoToTims::GoToTims(int position) 
  : Tile("Go To Tims", position) {}

void GoToTims::landedOn(Player* player) {
  cout << "You landed on Go To Tims. Moving to DC Tims Line!" << endl;
  
  // Directly send player to DC Tims Line (position 10)
  player->teleport(10);
  player->enterTimsLine();
  
  cout << player->getName() << " is now in the DC Tims Line." << endl;
}

//----------------------------------
// TIMS LINE IMPLEMENTATION
//----------------------------------
TimsLine::TimsLine(int position) 
  : Tile("DC Tims Line", position) {}

void TimsLine::landedOn(Player* player) {
  if (!player) return;
  
  cout << player->getName() << " has landed on DC Tims Line." << endl;
  cout << "This is just visiting - no effect." << endl;
  // No effect if just visiting
}

//----------------------------------
// NEEDLES HALL IMPLEMENTATION
//----------------------------------
NeedlesHall::NeedlesHall(int position) : Tile("Needles Hall", position) {
  money_changes = {
    {-200, 1.0/18},  // Lose $200
    {-100, 1.0/9},   // Lose $100
    {-50,  1.0/6},   // Lose $50
    {25,   1.0/3},   // Gain $25
    {50,   1.0/6},   // Gain $50
    {100,  1.0/9},   // Gain $100
    {200,  1.0/18}   // Gain $200
  };
}

void NeedlesHall::landedOn(Player* player) {
  cout << player->getName() <<" landed on Needles Hall!" << endl;

  static std::default_random_engine engine(std::random_device{}());
  std::vector<double> weights;
  for (const auto& [amount, prob] : money_changes) {
    weights.push_back(prob);
  }
  
  std::discrete_distribution<int> dist(weights.begin(), weights.end());
  int selected = dist(engine);
  int amount = money_changes[selected].first;

  if (amount > 0) {
    cout << "You received $" << amount << "!" << endl;
    player->receiveMoney(amount);
  } else {
    cout << "You must pay $" << -amount << "." << endl;
    player->receiveMoney(amount);
    // if (!success) {
    //     cout << "You cannot pay and must declare bankruptcy or raise funds." << endl;
    // }
  }

  // 1% chance to get a Roll Up the Rim cup
  std::bernoulli_distribution cupDist(0.01);
  if (cupDist(engine)) {
    player->addTimsCup();
  }
}

//----------------------------------
// SLC 
//----------------------------------
SLC::SLC(int position) : Tile("SLC", position) {
  movements = {
    {-3, 1.0/8}, 
    {-2, 1.0/6}, 
    {-1, 1.0/6},
    {1, 1.0/8}, 
    {2, 1.0/6}, 
    {3, 1.0/6},
    {10, 1.0/24}, 
    {0, 1.0/24}
  };
}

void SLC::landedOn(Player* player) {
  cout << "You landed on SLC (Student Life Centre)!" << endl;
  
  static std::default_random_engine engine(std::random_device{}());
  std::vector<double> weights;
  for (const auto& [move, prob] : movements) weights.push_back(prob);
  std::discrete_distribution<int> dist(weights.begin(), weights.end());

  int selected = dist(engine);
  int move = movements[selected].first;

  // Try to get the board from the Game instance
  Game* gameInstance = Game::getInstance();
  Board* board = nullptr;
  if (gameInstance) {
    board = &(gameInstance->getBoard());
  }
  
  if (move == 10) {
    cout << "The card sends you to DC Tims Line!" << endl;
    // Send to Tims Line
    if (board) {
      board->sendToTimsLine(player);
    } else {
      player->teleport(10);
      player->enterTimsLine();
    }
  } else if (move == 0) {
    cout << "The card sends you to Collect OSAP!" << endl;
    // Go to Collect OSAP
    player->teleport(0);
    
    // Since we're moving to Collect OSAP, we should also give $200
    player->receiveMoney(200);
    cout << "You collect $200 for passing OSAP." << endl;
  } else {
    if (move > 0) {
      cout << "The card moves you forward " << move << " spaces." << endl;
    } else {
      cout << "The card moves you backward " << -move << " spaces." << endl;
    }
    
    // Move forward/backward
    player->move(move);
  }

  // 1% chance to get a Roll Up the Rim cup
  std::bernoulli_distribution cupDist(0.01);
  if (cupDist(engine)) {
    player->addTimsCup();
  }
}

//----------------------------------
// TUITION
//----------------------------------
Tuition::Tuition(int position) : Tile("Tuition", position) {}

void Tuition::landedOn(Player* player) {
  cout << "You landed on Tuition!" << endl;
  
  Game* gameInstance = Game::getInstance();
  Bank& bank = gameInstance->getBank();

  // Calculate 10% of total worth
  const int totalWorth = player->getNetWorth();
  const int tenPercent = static_cast<int>(totalWorth * 0.1);
  const int flatFee = 300;

  // Display options to the player
  std::cout << "Tuition options:" << std::endl;
  std::cout << "1. Pay $" << flatFee << " flat fee" << std::endl;
  std::cout << "2. Pay $" << tenPercent << " (one 10th of your total worth)" << std::endl;
  std::cout << "Enter choice (1 or 2): ";
  
  int choice;
  std::cin >> choice;
  
  if (choice == 1) {
    // Player chose to pay flat fee
    cout << "You paid the flat fee of $" << flatFee << "." << endl;
    bool success = player->payMoneyToBank(flatFee, bank); // <- here 
    if (!success) {
      cout << "You cannot pay tuition and must declare bankruptcy or raise funds." << endl;
    }
  } else {
    // Player chose to pay percentage
    cout << "You paid one 10th of your worth: $" << tenPercent << "." << endl;
    bool success = player->payMoneyToBank(tenPercent, bank); // <- here 
    if (!success) {
      cout << "You cannot pay tuition and must declare bankruptcy or raise funds." << endl;
    }
  }
}

//----------------------------------
// GOOSE NESTING
//----------------------------------
GooseNesting::GooseNesting(int position) 
  : Tile("Goose Nesting", position) {}

void GooseNesting::landedOn(Player* player) {
  cout << "You landed on Goose Nesting!" << endl;
  
  static std::default_random_engine engine(std::random_device{}());
  std::uniform_int_distribution<int> dist(0, gooseMessages.size() - 1);
  
  // Display random goose encounter message
  std::string message = gooseMessages[dist(engine)];
  std::cout << message << std::endl;
  
  // No game effect occurs (no money lost, no movement)
  cout << "Fortunately, you escaped without any monetary damage." << endl;
}

//----------------------------------
// PLAYER IMPLEMENTATIONS
//----------------------------------
Player::Player(std::string name, char piece) : 
  name{name}, 
  piece{piece}, 
  money{1500}, 
  position{0}, 
  inTimsLine{false}, 
  turnsInTimsLine{0}, 
  timsCups{0}, 
  properties{} // Initialize empty vector of Property pointers
{
}


void Player::move(int steps){

if(steps < 0){
  int tmp = position + steps;
  if (tmp < 0){
    tmp = 40 + tmp ;
  }
  position = tmp;
  return;
}

int tmp = (position + steps) % 40;
if(tmp < position){
  receiveMoney(200);
  position = tmp;
}
position = tmp;
return;
}

void Player::teleport(int destination){

position = destination;
return;
}

bool Player::payMoney(int amount, Player* recipient) {
  // If we can't afford it, allow the player to sell properties
  if(!canPayAmount(amount)) {
    cout << "You DO NOT have the cash to continue, you must sell some property to continue. Press c:";
    char type;
    do {
      cin >> type;
    } while(!cin.fail() && type != 'c');
    
    if(type == 'c') {
      // Display available properties
      if(properties.empty()) {
        cout << "You don't have any properties to sell." << endl;
        declaredBankruptcy(recipient);
        
        // Notify the game that this player has gone bankrupt
        Game* gameInstance = Game::getInstance();
        if (gameInstance) {
          cout << "Player " << name << " has left the game due to bankruptcy." << endl;
          gameInstance->removeBankruptPlayer(this);
        }
        
        return false;
      }
      
      cout << "Available properties:" << endl;
      for(size_t i = 0; i < properties.size(); i++) {
        cout << i + 1 << ": " << properties[i]->getName();
        if(properties[i]->isMortgaged()) {
          cout << " (mortgaged)";
        }
        cout << " - Worth $" << properties[i]->getPurchaseCost() / 2 << endl;
      }
      
      // Let player choose a property
      int choice;
      cout << "Enter property number (0 to cancel): ";
      cin >> choice;
      
      if(choice <= 0 || choice > static_cast<int>(properties.size())) {
        cout << "Sale canceled." << endl;
        return false;
      }
      
      // Sell the chosen property
      Property* propertyToSell = properties[choice - 1];
      sellProperty(propertyToSell);
      cout << "Sold " << propertyToSell->getName() << " to the bank for $" 
         << propertyToSell->getPurchaseCost() / 2 << endl;
      
      // Recursively try to pay again
      return payMoney(amount, recipient);
    }
  }

  // Double-check that we have enough money
  if(money - amount < 0) {
    int netWorth = getNetWorth();
    return false;
  }

  // Process the payment
  money = money - amount;
  if(recipient) {
    recipient->receiveMoney(amount);
  }
  return true;
}

bool Player::payMoneyToBank(int amount, Bank& recipient) {
  // If we can't afford it, allow the player to sell properties
  if(!canPayAmount(amount)) {
    cout << "You DO NOT have the cash to continue, you must sell some property to continue. Press c:";
    char type;
    do {
      cin >> type;
    } while(!cin.fail() && type != 'c');
    
    if(type == 'c') {
      // Display available properties
      if(properties.empty()) {
        cout << "You don't have any properties to sell." << endl;
        declaredBankruptcy(nullptr); // nullptr because we're paying the bank
        
        // Notify the game that this player has gone bankrupt
        Game* gameInstance = Game::getInstance();
        if (gameInstance) {
          cout << "Player " << name << " has left the game due to bankruptcy." << endl;
          gameInstance->removeBankruptPlayer(this);
        }
        
        return false;
      }
      
      cout << "Available properties:" << endl;
      for(size_t i = 0; i < properties.size(); i++) {
        cout << i + 1 << ": " << properties[i]->getName();
        if(properties[i]->isMortgaged()) {
          cout << " (mortgaged)";
        }
        cout << " - Worth $" << properties[i]->getPurchaseCost() / 2 << endl;
      }
      
      // Let player choose a property
      int choice;
      cout << "Enter property number (0 to cancel): ";
      cin >> choice;
      
      if(choice <= 0 || choice > static_cast<int>(properties.size())) {
        cout << "Sale canceled." << endl;
        return false;
      }
      
      // Sell the chosen property
      Property* propertyToSell = properties[choice - 1];
      sellProperty(propertyToSell);
      cout << "Sold " << propertyToSell->getName() << " to the bank for $" 
         << propertyToSell->getPurchaseCost() / 2 << endl;
      
      // Recursively try to pay again
      return payMoneyToBank(amount, recipient);
    }
  }

  // Double-check that we have enough money
  if(money - amount < 0) {
    int netWorth = getNetWorth();
    return false;
  }

  // Process the payment
  money = money - amount;
  recipient.collectMoney(amount);
  return true;
}

// Adds money to the player's balance
void Player::receiveMoney(int amount) {
  money += amount;
}

// Sells a property
void Player::sellProperty(Property* property) {
  auto it = find(properties.begin(), properties.end(), property);

  if (it != properties.end()) {
    receiveMoney(property->getPurchaseCost() / 2); // Selling to bank at half price
    properties.erase(it);
    property->setOwner(nullptr);
  }
}

// Mortgages a property for half its purchase price
bool Player::mortgageProperty(Property* property) {
  if (!property->isMortgaged()) {
    receiveMoney(property->getPurchaseCost() / 2);
    property->mortgage();
    return true;
  }
  return false;
}

// Unmortgages a property by paying back half its purchase price
bool Player::unmortgageProperty(Property* property) {

Game* gameInstance = Game::getInstance();
Bank& bank = gameInstance->getBank();

if (property->isMortgaged()) {
    payMoneyToBank(property->getPurchaseCost() / 2, bank);
    property->unmortgage();
    return true;
}
return false;
}

// Checks if the player has enough money to cover an amount
bool Player::canPayAmount(int amount) {
  return money >= amount;
}

// Calculates total net worth
int Player::getNetWorth() {
  int total = money;
  for (const auto& property : properties) {
    total += property->getPurchaseCost();
    if (auto academic = dynamic_cast<AcademicBuilding*>(property)) {
      total += academic->getImprovements() * academic->getImprovementCost();
    }
  }
  return total;
}

// Declares bankruptcy
void Player::declaredBankruptcy(Player* creditor) {
  if (creditor) {
    for (auto* property : properties) {
      creditor->properties.push_back(property);
      property->setOwner(creditor);
    }
  }
  properties.clear();
  money = 0;
  cout << name << " has declared bankruptcy!" << endl;
}

// Sends player to the Tim's Line
void Player::enterTimsLine() {
  inTimsLine = true;
  turnsInTimsLine = 0;
}

// Releases player from Tim's Line
void Player::leaveTimsLine() {
  inTimsLine = false;
  turnsInTimsLine = 0;
}

// Adds a Tim's Cup
void Player::addTimsCup() {
  Game* gameInstance = Game::getInstance();
  if(gameInstance && gameInstance->canGiveMoreCups()) {
    timsCups++;
    gameInstance->currentTimsCupsInGame++; 
    cout << "Congratulations! You received a Roll Up the Rim cup!" << endl;
  }
}

// Uses a Tim's Cup to escape from Tim's Line
bool Player::useTimsCup() {
  if (timsCups > 0) {
    timsCups--;
    leaveTimsLine();
    return true;
  }
  return false;
}

// Helper method needed by CommandInterpreter
void Player::incrementTurnsInTimsLine() {
  turnsInTimsLine++;
}

bool Player::ownsMonopoly(std::string blockName) {
  // Count properties in this monopoly block that the player owns
  int ownedInBlock = 0;
  int totalInBlock = 0;
  
  // Look at player's own properties to count ones in this block
  for (const auto& prop : properties) {
    AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(prop);
    if (academic && academic->getMonopolyBlock() == blockName) {
      ownedInBlock++;
    }
  }
  
  // If we don't own any in the block, return early
  if (ownedInBlock == 0) {
    return false;
  }
  
  // We need to check how many properties are in this monopoly block total
  // This could be hard-coded based on the game rules
  if (blockName == "Arts1" || blockName == "Sci1") {
    totalInBlock = 2; // AL+ML or PHYS+B1+B2
  } else if (blockName == "Arts2" || blockName == "Eng" || blockName == "Health" || 
       blockName == "Env" || blockName == "Sci2" || blockName == "Math") {
    totalInBlock = 3; // ECH+PAS+HH, RCH+DWE+CPH, etc.
  } else {
    // Unknown block
    return false;
  }
  
  // Player has monopoly if they own all properties in the block
  return ownedInBlock == totalInBlock;
}

bool Player::buyProperty(Property* property) {
  // Validate the property
  if (!property) {
    std::cerr << "Invalid property." << std::endl;
    return false;
  }

  // Check if property is already owned
  if (property->getOwner()) {
    std::cout << "Property already owned by " << property->getOwner()->getName() << std::endl;
    return false;
  }

  // Check if player has enough money
  int cost = property->getPurchaseCost();
  if (money < cost) {
    std::cout << "Not enough money to buy property." << std::endl;
    return false;
  }

  // Process purchase
  money -= cost;
  property->setOwner(this);
  properties.push_back(property);
  
  std::cout << name << " purchased " << property->getName() << " for $" << cost << std::endl;
  return true;
}

bool Player::buyImprovement(AcademicBuilding* property) {
  // Validate the property
  if (!property) {
    std::cerr << "Invalid property." << std::endl;
    return false;
  }

  // Check if player owns the property
  if (property->getOwner() != this) {
    std::cout << "You don't own this property." << std::endl;
    return false;
  }

  // Check if player has monopoly on this block
  if (!ownsMonopoly(property->getMonopolyBlock())) {
    std::cout << "You need to own all properties in the " << property->getMonopolyBlock() << " block to buy improvements." << std::endl;
    return false;
  }

  // Check if property already has maximum improvements
  if (property->getImprovements() >= 5) {
    std::cout << "This property already has maximum improvements." << std::endl;
    return false;
  }

  // Check if player has enough money
  int cost = property->getImprovementCost();
  if (money < cost) {
    std::cout << "Not enough money to buy improvement." << std::endl;
    return false;
  }

  // Process improvement purchase
  if (!property->addImprovement()) {
    std::cout << "Failed to add improvement." << std::endl;
    return false;
  }

  money -= cost;
  std::cout << name << " bought an improvement on " << property->getName() << " for $" << cost << std::endl;
  return true;
}

void Player::tradePropertyInsteadOfPaying(Property* property, Player* recipient) {
  // Implementation pending
}

std::string Player::getAssets() {
  std::stringstream ss;
  ss << "Cash: $" << money << std::endl;
  ss << "Properties:" << std::endl;
  for (const auto& property : properties) {
    ss << "- " << property->getName();
    if (property->isMortgaged()) {
      ss << " (mortgaged)";
    }
    ss << std::endl;
  }
  ss << "Tim's Cups: " << timsCups << std::endl;
  return ss.str();
}

bool Player::sellImprovement(AcademicBuilding* property) {
  if (!property) {
    std::cerr << "Invalid property." << std::endl;
    return false;
  }
  
  if (property->getOwner() != this) {
    std::cout << "You don't own this property." << std::endl;
    return false;
  }
  
  if (property->getImprovements() <= 0) {
    std::cout << "This property has no improvements to sell." << std::endl;
    return false;
  }
  
  // Process selling
  if (!property->removeImprovement()) {
    std::cout << "Failed to remove improvement." << std::endl;
    return false;
  }
  
  // Calculate half the price
  int refund = property->getImprovementCost() / 2;
  receiveMoney(refund);
  
  std::cout << name << " sold an improvement on " << property->getName() 
        << " for $" << refund << std::endl;
  return true;
}

//----------------------------------
// BOARD IMPLEMENTATIONS
//----------------------------------

Board::Board(){
initializeBoard(); 
}

void Board::initializeBoard() {
  // First, read property data from CSV file
  map<string, tuple<string, int, int, vector<int>>> propertyData;
  
  ifstream csv{"watopoly_data.csv"};
  if (csv.is_open()) {
    string line;
    // Skip header row
    getline(csv, line);
    
    while (getline(csv, line)) {
      stringstream ss(line);
      string item;
      vector<string> row;
      
      while (getline(ss, item, ',')) {
        row.push_back(item);
      }
      
      if (row.size() >= 10) { // Ensure we have enough columns
        string name = row[0];
        string block = row[1];
        int purchaseCost = stoi(row[2]);
        int improvementCost = stoi(row[3]);
        
        // Parse tuition values (columns 4-9 are the tuition values for 0-5 improvements)
        vector<int> tuitionValues;
        for (size_t i = 4; i < 10; ++i) {
          tuitionValues.push_back(stoi(row[i]));
        }
        
        propertyData[name] = make_tuple(block, purchaseCost, improvementCost, tuitionValues);
      }
    }
    csv.close();
  } else {
    cerr << "Warning: Could not open watopoly_data.csv, using default values" << endl;
  }

  // Now read the board layout
  ifstream file{"boardTileOrder.txt"};
  string line;
  if (!file.is_open()) {
    cerr << "Error: Unable to open boardTileOrder.txt" << endl;
    return;
  }

  int i = 0;
  while (getline(file, line)) {
    Tile* newTile = nullptr;
    
    // Trim any whitespace from the line
    line.erase(0, line.find_first_not_of(" \t\r\n"));
    line.erase(line.find_last_not_of(" \t\r\n") + 1);
    
    // Debug: Print the tile being created -- not needed anymore
    
    // Special tile handling - make sure to match the exact names from board.txt
    if (line == "COLLECT OSAP") {
      newTile = new CollectOSAP(i);

    } else if (line == "DC Tims Line") {
      newTile = new TimsLine(i);

    } else if (line == "GO TO TIMS") {
      newTile = new GoToTims(i);

    } else if (line == "Goose Nesting") {
      newTile = new GooseNesting(i);

    } else if (line == "TUITION") {
      newTile = new Tuition(i);

    } else if (line == "COOP FEE") {
      newTile = new CoopFee(i);

    } else if (line == "SLC") {
      newTile = new SLC(i);

    } else if (line == "NEEDLES HALL") {
      newTile = new NeedlesHall(i);

    } else if (line == "PAC" || line == "CIF") {
      // For Gym, use default values (not in CSV)
      newTile = new Gym(line, i);

    } else if (line == "MKV" || line == "UWP" || line == "V1" || line == "REV") {
      // For Residence, use default values (not in CSV)
      newTile = new Residence(line, i);

    } else {
      // Assume it's an academic building
      int cost = 0;
      string block = "";
      int improvementCost = 0;
      vector<int> tuitionValues(6, 0); // Default to all zeros
      
      // Look up data from CSV if available
      if (propertyData.count(line) > 0) {
        auto& data = propertyData[line];
        block = get<0>(data);
        cost = get<1>(data);
        improvementCost = get<2>(data);
        tuitionValues = get<3>(data);
        
        // Use data from CSV to create AcademicBuilding
        newTile = new AcademicBuilding(line, i, cost, improvementCost, block, tuitionValues);
        // cout << " (Created as AcademicBuilding with CSV data)" << endl;
      } else {
        // If not found in CSV, use default values
        newTile = new AcademicBuilding(line, i, 100, 50, "Unknown", tuitionValues);
        // cout << " (Created as AcademicBuilding with default values)" << endl;
      }
    }
    
    tiles.push_back(newTile);
    i++;
  }

  file.close();
  
  // Group academic buildings by monopoly block
  for (Tile* tile : tiles) {
    AcademicBuilding* ab = dynamic_cast<AcademicBuilding*>(tile);
    if (ab) {
      academicBlocks[ab->getMonopolyBlock()].push_back(ab);
    }
  }
}

Tile* Board::getTile(int position){
  if (position < 0 || position >= static_cast<int>(tiles.size())){
    cerr << "invalid position: " << position << endl;
    cout << "returning a nullptr" << endl;
    return nullptr;
  }
  else{
    return tiles[position];
  }
}

Property* Board::getPropertyByName(const string& name){
  for(Tile* tile : tiles){
    if(!tile) continue; // if the tile is uninitialized

    Property* property = dynamic_cast<Property*>(tile); // making sure tile is of Property type
    
    if(property && property->getName() == name){
      return property;
    }
  }
  cerr << "invalid name: " << name << endl;
  cout << "returning a nullptr" << endl;
  return nullptr;
}

void Board::movePlayer(Player* player, int steps){
  player->move(steps);
}
void Board::teleportPlayer(Player* player, int destination){
  if(destination >= 0 && destination < static_cast<int>(tiles.size())){
    player->teleport(destination);
  }
  else{
    cerr << "invalid destination, no tile exists" << endl;
    return;
  }
}

void Board::sendToTimsLine(Player* player){
  teleportPlayer(player, 10);  // Position 10 is the Tim's Line
  player->enterTimsLine();
}

void Board::display() {
  // Get players from the game
  vector<Player*> gamePlayers;
  Game* gameInstance = Game::getInstance();
  if (gameInstance) {
    gamePlayers = gameInstance->getPlayers();
  }
  
  // Read the board template
  ifstream boardFile{"board.txt"};
  cout << endl;

  // Create a simple text representation if the board file isn't available
  if (!boardFile.is_open()) {
    cerr << "Error: Unable to open board.txt" << endl;
    cout << "Creating a simple text representation instead." << endl;
    
    cout << "-----------------------------------------" << endl;
    cout << "|   WATOPOLY BOARD - TEXT VERSION      |" << endl;
    cout << "-----------------------------------------" << endl;

    
    
    // Print positions of all tiles
    for (int i = 0; i < tiles.size(); ++i) {
      Tile* tile = getTile(i);
      if (tile) {
        cout << i << ": " << tile->getName();
        
        // List any players at this position
        bool playersHere = false;
        for (auto player : gamePlayers) {
          if (player && player->getPosition() == i) {
            if (!playersHere) {
              cout << " - Players: ";
              playersHere = true;
            }
            cout << player->getName() << "(" << player->getPiece() << ") ";
          }
        }
        cout << endl;
      }
    }
    
    cout << "-----------------------------------------" << endl;
    return;
  }

  // Read the entire board into memory
  vector<string> boardLines;
  string line;
  while (getline(boardFile, line)) {
    boardLines.push_back(line);
  }
  boardFile.close();
  int row = 4;

  for(int i = 0; i < 56; ++i){
    // display the players 

    if(i == row){
      // cout << "4"; 
      // incrementing for the next row
      
      line = boardLines[row];
      // cout << line.size() << endl;

      for(int p = 0; p < gamePlayers.size(); ++p){
        int plrPos = gamePlayers[p]->getPosition();
        char plrChar = gamePlayers[p]->getPiece();
        // cout << plrPos << endl;
        
        if(plrPos < 11 && i == 54){ // first row
          int placePlayerIndex = 88 - plrPos*8; // start of every tile in the row
          // go over every star
          for(int dot = 0; dot < 7; ++dot){
            if(line[placePlayerIndex] == '*'){
              line[placePlayerIndex] = plrChar;
              break;
            }
            --placePlayerIndex;
          }
        } 
        
        else if(plrPos == 11 && i == 49){ 
          int placePlayerIndex = 7;
          for(int dot = 0; dot < 7; ++dot){
            if(line[placePlayerIndex] == '*'){
              line[placePlayerIndex] = plrChar;
              break;
            }
            --placePlayerIndex;
          }
        }
        // Left column (positions 11-19)
else if(plrPos == 11 && i == 49){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
if(line[placePlayerIndex] == '*'){
line[placePlayerIndex] = plrChar;
break;
}
--placePlayerIndex;
}
}
else if(plrPos == 12 && i == 44){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
if(line[placePlayerIndex] == '*'){
line[placePlayerIndex] = plrChar;
break;
}
--placePlayerIndex;
}
}
else if(plrPos == 13 && i == 39){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
if(line[placePlayerIndex] == '*'){
line[placePlayerIndex] = plrChar;
break;
}
--placePlayerIndex;
}
}
else if(plrPos == 14 && i == 34){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  --placePlayerIndex;
}
}
else if(plrPos == 15 && i == 29){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  --placePlayerIndex;
}
}
else if(plrPos == 16 && i == 24){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  --placePlayerIndex;
}
}
else if(plrPos == 17 && i == 19){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  --placePlayerIndex;
}
}
else if(plrPos == 18 && i == 14){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  --placePlayerIndex;
}
}
else if(plrPos == 19 && i == 9){ 
int placePlayerIndex = 7;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  --placePlayerIndex;
}
}
// Top row (positions 20-30)
else if(plrPos >= 20 && plrPos <= 30 && i == 4){
int placePlayerIndex = 0 + (plrPos - 20)*8;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 31 && i == 9){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 32 && i == 14){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 33 && i == 19){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 34 && i == 24){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 35 && i == 29){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 36 && i == 34){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 37 && i == 39){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 38 && i == 44){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}
else if(plrPos == 39 && i == 49){
int placePlayerIndex = 87;
for(int dot = 0; dot < 7; ++dot){
  if(line[placePlayerIndex] == '*'){
    line[placePlayerIndex] = plrChar;
    break;
  }
  ++placePlayerIndex;
}
}

} 
++i;
cout << line << endl;
row = row + 5;
}
    

cout << boardLines[i] << endl;
}

}

//----------------------------------
// GAME IMPLEMENTATIONS
//----------------------------------

// Add static member for singleton pattern
Game::Game(bool testMode) : 
  currentPlayerIndex(0),
  maxTimsCups(4),
  isTestingMode(testMode),
  dice(testMode),
  currentTimsCupsInGame(0){
  commandInterpreter = new CommandInterpreter(this, isTestingMode);
  // Other initialization
  if (instance == nullptr) {
    instance = this;
  }
}

Game::~Game() {
delete commandInterpreter;
}

std::vector<Player*> Game::getPlayers() {
return players;
}

Game* Game::instance = nullptr;

Game* Game::getInstance() {
if (instance == nullptr) {
    instance = new Game();
}
return instance;
}

void Game::initialize(int numPlayers) {
// Initialize players, board, etc.
if (numPlayers < 2 || numPlayers > 6) {
    std::cout << "Invalid number of players. Please enter a number between 2 and 6." << std::endl;
    return;
}

std::vector<char> availablePieces = {'G','B','D','P','S','$','L','T'};
std::vector<std::string> pieceNames = {"Goose", "Beer Bottle", "Donut", "Pink Tie", "Stetson", "Money", "Laptop", "T-Rex"};

// Clear any existing players
for (auto player : players) {
    delete player;
}
players.clear();

for (int i = 0; i < numPlayers; ++i) {
    std::string playerName;
    char playerPiece;

    std::cout << "Enter name for Player " << i + 1 << ": ";
    std::cin >> playerName;

    std::cout << "Available pieces: ";
    size_t j;
    for (j = 0; j < availablePieces.size(); ++j) {
        std::cout << availablePieces[j] << " (" << pieceNames[j] << ") ";
    }
    std::cout << std::endl;

    bool validPiece = false;
    while (!validPiece) {
        std::cout << "Enter piece for Player " << i + 1 << ": ";
        std::cin >> playerPiece;

        auto it = std::find(availablePieces.begin(), availablePieces.end(), playerPiece);
        if (it != availablePieces.end()) {
            availablePieces.erase(it);
            pieceNames.erase(pieceNames.begin() + (it - availablePieces.begin()));
            validPiece = true;
        } else {
            std::cout << "Invalid piece. Please choose from the available pieces." << std::endl;
        }
    }

    Player* player = new Player(playerName, playerPiece);
    players.push_back(player);
}

currentPlayerIndex = 0;
maxTimsCups = 4;
currentTimsCupsInGame = 0;
}

// Change to static method per declaration
void Game::loadGame(std::string filename) {
std::ifstream file(filename);
if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return;
}

// Clear existing game state
for (auto player : players) {
    delete player;
}
players.clear();
currentTimsCupsInGame = 0;

int numPlayers;
file >> numPlayers;
file.ignore(); // Consume the newline character

for (int i = 0; i < numPlayers; ++i) {
    std::string line;
    std::getline(file, line);
    std::stringstream ss(line);

    std::string playerName;
    char playerPiece;
    int timsCups;
    int money;
    int position;
    int inTimsLineStatus = 0;
    int turnsInTimsLine = 0;

    ss >> playerName >> playerPiece >> timsCups >> money >> position;

    if (position == 10) {
        // Check if there's another value (inTimsLineStatus)
        if (ss >> inTimsLineStatus) {
            if (inTimsLineStatus == 1) {
                // If inTimsLineStatus is 1, there's a number of turns
                ss >> turnsInTimsLine;
            }
        }
    }

    Player* player = new Player(playerName, playerPiece);
    
    // Set player properties
    // Add money
    while (player->getMoney() < money) {
        player->receiveMoney(100);
    }
    
    // Set position
    player->teleport(position);
    
    // Set Tims Cups
    for (int j = 0; j < timsCups; j++) {
        player->addTimsCup();
    }
    
    // Set Tims Line status
    if (position == 10 && inTimsLineStatus == 1) {
        player->enterTimsLine();
        for (int j = 0; j < turnsInTimsLine; j++) {
            player->incrementTurnsInTimsLine();
        }
    }
    
    players.push_back(player);
}

// Load property information
std::string line;
while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string propertyName, ownerName;
    int improvements;
    
    ss >> propertyName >> ownerName >> improvements;
    
    Property* property = board.getPropertyByName(propertyName);
    if (!property) {
        std::cerr << "Error: Property " << propertyName << " not found." << std::endl;
        continue;
    }
    
    if (ownerName != "BANK") {
        // Find the player with this name
        Player* owner = nullptr;
        for (auto player : players) {
            if (player->getName() == ownerName) {
                owner = player;
                break;
            }
        }
        
        if (!owner) {
            std::cerr << "Error: Player " << ownerName << " not found." << std::endl;
            continue;
        }
        
        // Set the owner
        property->setOwner(owner);
        const_cast<std::vector<Property*>&>(owner->getProperties()).push_back(property);
        // Add to player's properties
        // owner->buyProperty(property);
        
        // Handle mortgages and improvements
        if (improvements == -1) {
            property->mortgage();
        } else if (improvements > 0) {
            // Only academic buildings can have improvements
            AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(property);
            if (academic) {
                for (int i = 0; i < improvements; ++i) {
                    academic->addImprovement();
                }
            }
        }
    }
}

currentPlayerIndex = 0; // Start with the first player
file.close();
}

void Game::removeBankruptPlayer(Player* bankruptPlayer) {
// Find the player in the list and remove them
auto it = std::find(players.begin(), players.end(), bankruptPlayer);
if (it != players.end()) {
  // If the current player is going bankrupt, move to the next player first
  if (*it == getCurrentPlayer()) {
    nextPlayer();
  }
  
  // Remove the player from the list
  players.erase(it);
  
  // Delete the player object if it was dynamically allocated
  delete bankruptPlayer;
  
  // Check if the game should end
  if (players.size() <= 1) {
    endGame();
  }
}
}

void Game::saveGame(std::string filename) {
std::ofstream file(filename);
if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return;
}

// Save number of players
file << players.size() << std::endl;

// Save player information
for (auto player : players) {
    file << player->getName() << " "
         << player->getPiece() << " "
         << player->getTimsCups() << " "
         << player->getMoney() << " "
         << player->getPosition();

    // Handle DC Tims Line
    if (player->getPosition() == 10) {
        if (player->isInTimsLine()) {
            file << " 1 " << player->getTurnsInTimsLine();
        } else {
            file << " 0";
        }
    }

    file << std::endl;
}

// Save property information
// We need to go through all properties in board order
for (int i = 0; i < 40; i++) {
  Tile* tile = board.getTile(i);
  if (!tile) continue;

  Property* property = dynamic_cast<Property*>(tile);
  if (!property) continue; // Skip non-property tiles

  // Write property name
  file << property->getName() << " ";

  // Write owner
  if (property->getOwner()) {
      file << property->getOwner()->getName() << " ";
  } else {
      file << "BANK" << " ";
  }

  // Write improvements or mortgage status
  if (property->isMortgaged()) {
      file << "-1";
  } else {
      // Check if it's an academic building with improvements
      AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(property);
      if (academic) {
          file << academic->getImprovements();
      } else {
          file << "0"; // Residences and Gyms always have 0 improvements
      }
  }

  file << std::endl;
}

file.close();
}

void Game::nextPlayer() {
// Move to the next player
currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
std::cout << "Next player: " << players[currentPlayerIndex]->getName() << std::endl;
}

void Game::processCommand(std::string command) {
commandInterpreter->parseCommand(command);
}

void Game::endGame() {
// End the game and declare the winner
if (players.size() == 1) {
    cout << "Game over! Winner: " << players[0]->getName() << "!" << endl;
} else {
    cout << "Game ended with multiple players still active." << endl;
}
}

void Game::auctionProperty(Property* property) {
// Implement auction logic
std::cout << "Starting auction for " << property->getName() << std::endl;
int highestBid = 0;
Player* highestBidder = nullptr;
std::string input;
while (true) {
    std::cout << "Current highest bid: " << highestBid << " by ";
    if (highestBidder) {
        std::cout << highestBidder->getName();
    } else {
        std::cout << "none";
    }
    std::cout << std::endl;
    std::cout << "Enter your bid or 'pass': ";
    std::getline(std::cin, input);
    
    if (input == "pass") {
        break;
    }
    
    int bid = std::stoi(input);
    if (bid > highestBid) {
        highestBid = bid;
        highestBidder = getCurrentPlayer();
    } else {
        std::cout << "Bid must be higher than the current highest bid." << std::endl;
    }
    nextPlayer();
}

if (highestBidder) {
    std::cout << highestBidder->getName() << " won the auction for " << highestBid << "!" << std::endl;
    highestBidder->payMoney(highestBid, nullptr);
    property->setOwner(highestBidder);
} else {
    std::cout << "No one won the auction." << std::endl;
}
}

int Game::getNumPlayers() const {
return players.size();
}

bool Game::canGiveMoreCups() {
if(maxTimsCups > currentTimsCupsInGame){
  return true;
}
return false;
}

Player* Game::getCurrentPlayer() const {
  if(players.empty() || currentPlayerIndex >= players.size()){ 
    return nullptr; 
  }
  return players[currentPlayerIndex];
}

// Added getters needed by CommandInterpreter
Dice& Game::getDice() {
  return dice;
}

Board& Game::getBoard() {
  return board;
}

Player* Game::getPlayerByName(const string& name) {
  for (auto player : players) {
    if (player->getName() == name) {
      return player;
    }
  }
  return nullptr;
}

void Game::mainLoop() {
bool gameOver = false;

while (!gameOver) {
  // Check for game ending conditions
  if (players.size() <= 1) {
    gameOver = true;
    continue;
  }
  // Display board state
  board.display();
  
  // Display current player's assets
  Player* currentPlayer = getCurrentPlayer();
  std::cout << "Current player: " << currentPlayer->getName() << std::endl;
  std::cout << "Assets: " << currentPlayer->getAssets() << std::endl;
  
  // Process commands for the current player
  std::string command;
  std::cout << "> ";
  std::getline(std::cin, command);
  
  if (command == "quit") {
    gameOver = true;
  } else {
    processCommand(command);
  }
  
  
}

endGame();
}

//-----------------------------------
// COMMAND INTERPRETER IMPLEMENTATIONS
//-----------------------------------

CommandInterpreter::CommandInterpreter(Game* game, bool testingMode) 
  : game(game), testingMode(testingMode) {}

void CommandInterpreter::parseCommand(const string& command) {
  // Tokenize the command
  vector<string> tokens = tokenizeCommand(command);
  
  if (tokens.empty()) {
    return;
  }
  
  string action = tokens[0];
  vector<string> args(tokens.begin() + 1, tokens.end());
  
  // Execute appropriate command
  if (action == "roll") {
    executeRoll(args);
  } else if (action == "next") {
    executeNext();
  } else if (action == "trade") {
    executeTrade(args);
  } else if (action == "improve") {
    executeImprove(args);
  } else if (action == "mortgage") {
    executeMortgage(args);
  } else if (action == "unmortgage") {
    executeUnmortgage(args);
  } else if (action == "bankrupt") {
    executeBankrupt();
  } else if (action == "assets") {
    executeAssets();
  } else if (action == "all") {
    executeAll();
  } else if (action == "save") {
    executeSave(args);
  } else {
    cout << "Invalid command: " << action << endl;
  }
}

vector<string> CommandInterpreter::tokenizeCommand(const string& command) {
  vector<string> tokens;
  istringstream iss(command);
  string token;
  
  while (iss >> token) {
    tokens.push_back(token);
  }
  
  return tokens;
}

void CommandInterpreter::executeRoll(const vector<string>& args) {
if (testingMode && args.size() == 2) {
    // In testing mode, we can specify the dice values
    try {
        int die1 = stoi(args[0]);
        int die2 = stoi(args[1]);
        
        if (die1 < 0 || die2 < 0) {
            throw out_of_range("Dice values must be non-negative");
        }
        
        Dice& dice = game->getDice();
        dice.setTestDice(die1, die2);
        cout << "Rolling " << die1 << " and " << die2 << " (testing mode)" << endl;
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return;
    }
} else if (!args.empty()) {
    cout << "Error: Invalid arguments for roll command" << endl;
    return;
} else {
    // Normal roll
    Dice& dice = game->getDice();
    dice.roll();
    cout << "Rolled " << dice.getTotal() << endl;
}

// Get the current player and move them
Player* currentPlayer = game->getCurrentPlayer();
if (!currentPlayer) {
    cout << "Error: No current player" << endl;
    return;
}

Dice& dice = game->getDice();
int steps = dice.getTotal();

// Check if player is in Tims Line
if (currentPlayer->isInTimsLine()) {
    if (dice.isDoubles()) {
        cout << currentPlayer->getName() << " rolled doubles and is leaving Tims Line!" << endl;
        currentPlayer->leaveTimsLine();
    } else {
        cout << currentPlayer->getName() << " is in Tims Line and did not roll doubles." << endl;
        
        // PROMPT THE PLAYER TO USE A CUP OR PAY REGARDLESS OF TURNS
        bool hasRimCup = currentPlayer->getTimsCups() > 0;
        int turnsInTimsLine = currentPlayer->getTurnsInTimsLine();
        
        // If it's their third turn or later, they MUST leave
        bool mustLeave = (turnsInTimsLine >= 2);
        
        if (hasRimCup) {
            cout << "Do you want to use a Roll Up the Rim cup to leave? (y/n): ";
            char useCup;
            cin >> useCup;
            
            if (useCup == 'y' || useCup == 'Y') {
                currentPlayer->useTimsCup();
                cout << "Used a Roll Up the Rim cup to leave Tims Line!" << endl;
                currentPlayer->leaveTimsLine();
            } else if (mustLeave) {
                // On third turn, must pay if they don't use a cup
                cout << "This is your third turn in Tims Line. You must pay $50 to leave." << endl;
                if (currentPlayer->payMoney(50, nullptr)) {
                    cout << "Paid $50 to leave Tims Line." << endl;
                    currentPlayer->leaveTimsLine();
                } else {
                    cout << "Cannot pay $50. You must trade, mortgage, or declare bankruptcy." << endl;
                    return;
                }
            } else {
                // Not third turn, give option to pay or stay
                cout << "Do you want to pay $50 to leave? (y/n): ";
                char pay;
                cin >> pay;
                
                if (pay == 'y' || pay == 'Y') {
                    if (currentPlayer->payMoney(50, nullptr)) {
                        cout << "Paid $50 to leave Tims Line." << endl;
                        currentPlayer->leaveTimsLine();
                    } else {
                        cout << "Cannot pay $50. You must trade, mortgage, or declare bankruptcy." << endl;
                        return;
                    }
                } else {
                    // Stay in Tims Line
                    currentPlayer->incrementTurnsInTimsLine();
                    cout << "Staying in Tims Line. Turn ended." << endl;
                    game->nextPlayer();
                    return;
                }
            }
        } else {
            // No Rim Cup, check if must pay or can choose
            if (mustLeave) {
                cout << "This is your third turn in Tims Line. You must pay $50 to leave." << endl;
                if (currentPlayer->payMoney(50, nullptr)) {
                    cout << "Paid $50 to leave Tims Line." << endl;
                    currentPlayer->leaveTimsLine();
                } else {
                    cout << "Cannot pay $50. You must trade, mortgage, or declare bankruptcy." << endl;
                    return;
                }
            } else {
                // Not third turn, give option to pay or stay
                cout << "Do you want to pay $50 to leave? (y/n): ";
                char pay;
                cin >> pay;
                
                if (pay == 'y' || pay == 'Y') {
                    if (currentPlayer->payMoney(50, nullptr)) {
                        cout << "Paid $50 to leave Tims Line." << endl;
                        currentPlayer->leaveTimsLine();
                    } else {
                        cout << "Cannot pay $50. You must trade, mortgage, or declare bankruptcy." << endl;
                        return;
                    }
                } else {
                    // Stay in Tims Line
                    currentPlayer->incrementTurnsInTimsLine();
                    cout << "Staying in Tims Line. Turn ended." << endl;
                    game->nextPlayer();
                    return;
                }
            }
        }
    }
}

// Move the player - this happens if they're not in Tims Line or just left it
Board& board = game->getBoard();
int oldPosition = currentPlayer->getPosition();
board.movePlayer(currentPlayer, steps);
int newPosition = currentPlayer->getPosition();

cout << currentPlayer->getName() << " moved from " << oldPosition 
     << " to " << newPosition << endl;

// Get the tile and handle landing
Tile* tile = board.getTile(newPosition);
if (tile) {
    tile->landedOn(currentPlayer);
}

// If we rolled doubles, player gets another turn unless they're in Tims Line
if (dice.isDoubles() && !currentPlayer->isInTimsLine()) {
    cout << "Rolled doubles! " << currentPlayer->getName() << " gets another turn." << endl;
} else {
    game->nextPlayer();
}
}


void CommandInterpreter::executeNext() {
  game->nextPlayer();
  cout << "Turn passed to " << game->getCurrentPlayer()->getName() << endl;
}

void CommandInterpreter::executeTrade(const vector<string>& args) {
if (args.size() != 3) {
    cout << "Error: Invalid trade command. Use: trade <name> <give> <receive>" << endl;
    return;
}

string targetPlayerName = args[0];
string give = args[1];
string receive = args[2];

// Find the target player
Player* currentPlayer = game->getCurrentPlayer();
Player* targetPlayer = game->getPlayerByName(targetPlayerName);

if (!targetPlayer) {
    cout << "Error: Player " << targetPlayerName << " not found." << endl;
    return;
}

// Check if both are money (not allowed)
bool giveIsMoney = utilities::isNumeric(give);
bool receiveIsMoney = utilities::isNumeric(receive);

if (giveIsMoney && receiveIsMoney) {
    cout << "Error: Cannot trade money for money." << endl;
    return;
}

// Handle property for property trade
if (!giveIsMoney && !receiveIsMoney) {
    // Find the properties
    Property* giveProperty = game->getBoard().getPropertyByName(give);
    Property* receiveProperty = game->getBoard().getPropertyByName(receive);
    
    if (!giveProperty || !receiveProperty) {
        cout << "Error: One or both properties not found." << endl;
        return;
    }
    
    // Check ownership
    if (giveProperty->getOwner() != currentPlayer) {
        cout << "Error: You don't own " << give << "." << endl;
        return;
    }
    
    if (receiveProperty->getOwner() != targetPlayer) {
        cout << "Error: " << targetPlayerName << " doesn't own " << receive << "." << endl;
        return;
    }
    
    // Check for improvements on the property or in the monopoly
    AcademicBuilding* giveAcademic = dynamic_cast<AcademicBuilding*>(giveProperty);
    if (giveAcademic) {
        // Check if property has improvements
        if (giveAcademic->getImprovements() > 0) {
            cout << "Error: Cannot trade " << give << " as it has improvements. Sell improvements first." << endl;
            return;
        }
        
        // Check if any property in the monopoly has improvements
        if (currentPlayer->ownsMonopoly(giveAcademic->getMonopolyBlock())) {
            for (const auto& prop : currentPlayer->getProperties()) {
                AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(prop);
                if (academic && academic->getMonopolyBlock() == giveAcademic->getMonopolyBlock() && academic->getImprovements() > 0) {
                    cout << "Error: Cannot trade " << give << " as a property in its monopoly has improvements." << endl;
                    return;
                }
            }
        }
    }
    
    // Same checks for receive property
    AcademicBuilding* receiveAcademic = dynamic_cast<AcademicBuilding*>(receiveProperty);
    if (receiveAcademic) {
        if (receiveAcademic->getImprovements() > 0) {
            cout << "Error: Cannot trade " << receive << " as it has improvements." << endl;
            return;
        }
        
        if (targetPlayer->ownsMonopoly(receiveAcademic->getMonopolyBlock())) {
            for (const auto& prop : targetPlayer->getProperties()) {
                AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(prop);
                if (academic && academic->getMonopolyBlock() == receiveAcademic->getMonopolyBlock() && academic->getImprovements() > 0) {
                    cout << "Error: Cannot trade " << receive << " as a property in its monopoly has improvements." << endl;
                    return;
                }
            }
        }
    }
    
    // Offer the trade to the target player
    cout << "Trade offered to " << targetPlayerName << ": your " << receive << " for " << currentPlayer->getName() << "'s " << give << endl;
    cout << targetPlayerName << ", do you accept this trade? (accept/reject): ";
    
    string response;
    cin >> response;
    
    if (response == "accept") {
        // Swap property ownership
        auto& currentPlayerProps = const_cast<vector<Property*>&>(currentPlayer->getProperties());
        auto& targetPlayerProps = const_cast<vector<Property*>&>(targetPlayer->getProperties());
        
        // Remove properties from their current owners
        currentPlayerProps.erase(remove(currentPlayerProps.begin(), currentPlayerProps.end(), giveProperty), currentPlayerProps.end());
        targetPlayerProps.erase(remove(targetPlayerProps.begin(), targetPlayerProps.end(), receiveProperty), targetPlayerProps.end());
        
        // Add properties to their new owners
        currentPlayerProps.push_back(receiveProperty);
        targetPlayerProps.push_back(giveProperty);
        
        // Update property ownership
        giveProperty->setOwner(targetPlayer);
        receiveProperty->setOwner(currentPlayer);
        
        cout << "Trade completed successfully!" << endl;
    } else {
        cout << "Trade rejected." << endl;
    }
}
// Handle money for property trade (currentPlayer gives money)
else if (giveIsMoney && !receiveIsMoney) {
    int amount = stoi(give);
    Property* receiveProperty = game->getBoard().getPropertyByName(receive);
    
    if (!receiveProperty) {
        cout << "Error: Property " << receive << " not found." << endl;
        return;
    }
    
    // Check ownership
    if (receiveProperty->getOwner() != targetPlayer) {
        cout << "Error: " << targetPlayerName << " doesn't own " << receive << "." << endl;
        return;
    }
    
    // Check if current player has enough money
    if (!currentPlayer->canPayAmount(amount)) {
        cout << "Error: You don't have enough money for this trade." << endl;
        return;
    }
    
    // Check for improvements on the property or in the monopoly
    AcademicBuilding* receiveAcademic = dynamic_cast<AcademicBuilding*>(receiveProperty);
    if (receiveAcademic) {
        if (receiveAcademic->getImprovements() > 0) {
            cout << "Error: Cannot trade " << receive << " as it has improvements." << endl;
            return;
        }
        
        if (targetPlayer->ownsMonopoly(receiveAcademic->getMonopolyBlock())) {
            for (const auto& prop : targetPlayer->getProperties()) {
                AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(prop);
                if (academic && academic->getMonopolyBlock() == receiveAcademic->getMonopolyBlock() && academic->getImprovements() > 0) {
                    cout << "Error: Cannot trade " << receive << " as a property in its monopoly has improvements." << endl;
                    return;
                }
            }
        }
    }
    
    // Offer the trade to the target player
    cout << "Trade offered to " << targetPlayerName << ": your " << receive << " for $" << amount << endl;
    cout << targetPlayerName << ", do you accept this trade? (accept/reject): ";
    
    string response;
    cin >> response;
    
    if (response == "accept") {
        // Transfer money and property
        currentPlayer->payMoney(amount, targetPlayer);
        
        // Remove property from target player
        auto& targetPlayerProps = const_cast<vector<Property*>&>(targetPlayer->getProperties());
        targetPlayerProps.erase(remove(targetPlayerProps.begin(), targetPlayerProps.end(), receiveProperty), targetPlayerProps.end());
        
        // Add property to current player
        auto& currentPlayerProps = const_cast<vector<Property*>&>(currentPlayer->getProperties());
        currentPlayerProps.push_back(receiveProperty);
        
        // Update property ownership
        receiveProperty->setOwner(currentPlayer);
        
        cout << "Trade completed successfully!" << endl;
    } else {
        cout << "Trade rejected." << endl;
    }
}
// Handle property for money trade (currentPlayer gives property)
else if (!giveIsMoney && receiveIsMoney) {
    Property* giveProperty = game->getBoard().getPropertyByName(give);
    int amount = stoi(receive);
    
    if (!giveProperty) {
        cout << "Error: Property " << give << " not found." << endl;
        return;
    }
    
    // Check ownership
    if (giveProperty->getOwner() != currentPlayer) {
        cout << "Error: You don't own " << give << "." << endl;
        return;
    }
    
    // Check if target player has enough money
    if (!targetPlayer->canPayAmount(amount)) {
        cout << "Error: " << targetPlayerName << " doesn't have enough money for this trade." << endl;
        return;
    }
    
    // Check for improvements on the property or in the monopoly
    AcademicBuilding* giveAcademic = dynamic_cast<AcademicBuilding*>(giveProperty);
    if (giveAcademic) {
        if (giveAcademic->getImprovements() > 0) {
            cout << "Error: Cannot trade " << give << " as it has improvements. Sell improvements first." << endl;
            return;
        }
        
        if (currentPlayer->ownsMonopoly(giveAcademic->getMonopolyBlock())) {
            for (const auto& prop : currentPlayer->getProperties()) {
                AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(prop);
                if (academic && academic->getMonopolyBlock() == giveAcademic->getMonopolyBlock() && academic->getImprovements() > 0) {
                    cout << "Error: Cannot trade " << give << " as a property in its monopoly has improvements." << endl;
                    return;
                }
            }
        }
    }
    
    // Offer the trade to the target player
    cout << "Trade offered to " << targetPlayerName << ": your $" << amount << " for " << currentPlayer->getName() << "'s " << give << endl;
    cout << targetPlayerName << ", do you accept this trade? (accept/reject): ";
    
    string response;
    cin >> response;
    
    if (response == "accept") {
        // Transfer money and property
        targetPlayer->payMoney(amount, currentPlayer);
        
        // Remove property from current player
        auto& currentPlayerProps = const_cast<vector<Property*>&>(currentPlayer->getProperties());
        currentPlayerProps.erase(remove(currentPlayerProps.begin(), currentPlayerProps.end(), giveProperty), currentPlayerProps.end());
        
        // Add property to target player
        auto& targetPlayerProps = const_cast<vector<Property*>&>(targetPlayer->getProperties());
        targetPlayerProps.push_back(giveProperty);
        
        // Update property ownership
        giveProperty->setOwner(targetPlayer);
        
        cout << "Trade completed successfully!" << endl;
    } else {
        cout << "Trade rejected." << endl;
    }
}
}

void CommandInterpreter::executeImprove(const vector<string>& args) {
if (args.size() != 2) {
    cout << "Error: Invalid improve command. Use: improve <property> buy/sell" << endl;
    return;
}

string propertyName = args[0];
string action = args[1];

// Find the property
Property* property = game->getBoard().getPropertyByName(propertyName);
if (!property) {
    cout << "Error: Property " << propertyName << " not found." << endl;
    return;
}

// Check if it's an academic building
AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(property);
if (!academic) {
    cout << "Error: You can only improve academic buildings." << endl;
    return;
}

// Check if the current player owns the property
Player* currentPlayer = game->getCurrentPlayer();
if (academic->getOwner() != currentPlayer) {
    cout << "Error: You don't own " << propertyName << "." << endl;
    return;
}

// Check if player owns the monopoly
if (!currentPlayer->ownsMonopoly(academic->getMonopolyBlock())) {
    cout << "Error: You must own all properties in the " << academic->getMonopolyBlock() << " monopoly to make improvements." << endl;
    return;
}

// Execute the appropriate action
if (action == "buy") {
    // Check if maximum improvements already
    if (academic->getImprovements() >= 5) {
        cout << "Error: " << propertyName << " already has the maximum number of improvements." << endl;
        return;
    }
    
    // Check if player has enough money
    int improvementCost = academic->getImprovementCost();
    if (!currentPlayer->canPayAmount(improvementCost)) {
        cout << "Error: You don't have enough money to buy an improvement. Cost: $" << improvementCost << endl;
        return;
    }
    
    // Buy the improvement
    if (currentPlayer->buyImprovement(academic)) {
        cout << "Successfully bought an improvement for " << propertyName << "." << endl;
    } else {
        cout << "Failed to buy improvement for " << propertyName << "." << endl;
    }
} 
else if (action == "sell") {
    // Check if there are improvements to sell
    if (academic->getImprovements() <= 0) {
        cout << "Error: " << propertyName << " has no improvements to sell." << endl;
        return;
    }
    
    // Sell the improvement
    if (currentPlayer->sellImprovement(academic)) {
        cout << "Successfully sold an improvement from " << propertyName << "." << endl;
    } else {
        cout << "Failed to sell improvement from " << propertyName << "." << endl;
    }
} 
else {
    cout << "Error: Invalid action '" << action << "'. Use 'buy' or 'sell'." << endl;
}
}

void CommandInterpreter::executeMortgage(const vector<string>& args) {
if (args.size() != 1) {
    cout << "Error: Invalid mortgage command. Use: mortgage <property>" << endl;
    return;
}

string propertyName = args[0];

// Find the property
Property* property = game->getBoard().getPropertyByName(propertyName);
if (!property) {
    cout << "Error: Property " << propertyName << " not found." << endl;
    return;
}

// Check if the current player owns the property
Player* currentPlayer = game->getCurrentPlayer();
if (property->getOwner() != currentPlayer) {
    cout << "Error: You don't own " << propertyName << "." << endl;
    return;
}

// Check if the property is already mortgaged
if (property->isMortgaged()) {
    cout << "Error: " << propertyName << " is already mortgaged." << endl;
    return;
}

// Check if there are improvements on the property for Academic Buildings
AcademicBuilding* academic = dynamic_cast<AcademicBuilding*>(property);
if (academic && academic->getImprovements() > 0) {
    cout << "Error: You must sell all improvements on " << propertyName << " before mortgaging it." << endl;
    return;
}

// Check if any property in the monopoly has improvements (for Academic Buildings)
if (academic && currentPlayer->ownsMonopoly(academic->getMonopolyBlock())) {
    for (const auto& prop : currentPlayer->getProperties()) {
        AcademicBuilding* other = dynamic_cast<AcademicBuilding*>(prop);
        if (other && other->getMonopolyBlock() == academic->getMonopolyBlock() && other->getImprovements() > 0) {
            cout << "Error: You must sell all improvements in the " << academic->getMonopolyBlock() 
                 << " monopoly before mortgaging any property in it." << endl;
            return;
        }
    }
}

// Mortgage the property
if (currentPlayer->mortgageProperty(property)) {
    cout << "Successfully mortgaged " << propertyName << " for $" << property->getPurchaseCost() / 2 << "." << endl;
} else {
    cout << "Failed to mortgage " << propertyName << "." << endl;
}
}

void CommandInterpreter::executeUnmortgage(const vector<string>& args) {
if (args.size() != 1) {
    cout << "Error: Invalid unmortgage command. Use: unmortgage <property>" << endl;
    return;
}

string propertyName = args[0];

// Find the property
Property* property = game->getBoard().getPropertyByName(propertyName);
if (!property) {
    cout << "Error: Property " << propertyName << " not found." << endl;
    return;
}

// Check if the current player owns the property
Player* currentPlayer = game->getCurrentPlayer();
if (property->getOwner() != currentPlayer) {
    cout << "Error: You don't own " << propertyName << "." << endl;
    return;
}

// Check if the property is mortgaged
if (!property->isMortgaged()) {
    cout << "Error: " << propertyName << " is not mortgaged." << endl;
    return;
}

// Calculate unmortgage cost (mortgage value + 10% interest)
int mortgageValue = property->getPurchaseCost() / 2;
int interest = property->getPurchaseCost() / 10; // 10% of original cost
int totalCost = mortgageValue + interest;

// Check if player has enough money
if (!currentPlayer->canPayAmount(totalCost)) {
    cout << "Error: You don't have enough money to unmortgage this property. Cost: $" << totalCost << endl;
    return;
}

// Unmortgage the property
if (currentPlayer->unmortgageProperty(property)) {
    cout << "Successfully unmortgaged " << propertyName << " for $" << totalCost << "." << endl;
} else {
    cout << "Failed to unmortgage " << propertyName << "." << endl;
}
}

void CommandInterpreter::executeBankrupt() {
Player* currentPlayer = game->getCurrentPlayer();

// Check if player is actually in a position where they must declare bankruptcy
if (currentPlayer->getNetWorth() > 0) {
    cout << "You can only declare bankruptcy when you cannot pay a debt." << endl;
    return;
}

cout << "Are you sure you want to declare bankruptcy? (y/n): ";
char response;
cin >> response;

if (response == 'y' || response == 'Y') {
    // Ask if bankruptcy is to another player or to the bank
    cout << "Declare bankruptcy to another player? Enter player name or 'bank': ";
    string creditorName;
    cin >> creditorName;
    
    if (creditorName == "bank" || creditorName == "Bank" || creditorName == "BANK") {
        // Bankruptcy to the bank
        cout << currentPlayer->getName() << " has declared bankruptcy to the Bank!" << endl;
        
        // Return all properties to the bank and auction them
        vector<Property*> playerProperties = currentPlayer->getProperties();
        for (auto property : playerProperties) {
            property->setOwner(nullptr);
            
            // Unmortgage the property before auctioning
            if (property->isMortgaged()) {
                property->unmortgage();
            }
            
            cout << "Auctioning " << property->getName() << "..." << endl;
            game->auctionProperty(property);
        }
        
        // Destroy any Roll Up the Rim cups
        // (This would be handled by the Player class's bankruptcy method)
        
        // Remove player from the game
        currentPlayer->declaredBankruptcy();
        
        cout << currentPlayer->getName() << " is out of the game." << endl;
    } else {
        // Bankruptcy to another player
        Player* creditor = game->getPlayerByName(creditorName);
        
        if (!creditor) {
            cout << "Player " << creditorName << " not found. Bankruptcy canceled." << endl;
            return;
        }
        
        cout << currentPlayer->getName() << " has declared bankruptcy to " << creditor->getName() << "!" << endl;
        
        // Transfer all assets to the creditor
        currentPlayer->declaredBankruptcy(creditor);
        
        cout << "All assets have been transferred to " << creditor->getName() << "." << endl;
        cout << currentPlayer->getName() << " is out of the game." << endl;
    }
    
    // Check if the game has ended (only one player left)
    if (game->getNumPlayers() <= 1) {
        game->endGame();
    } else {
        game->nextPlayer();
    }
} else {
    cout << "Bankruptcy canceled." << endl;
}
}
void CommandInterpreter::executeAssets() {
  Player* currentPlayer = game->getCurrentPlayer();
  cout << currentPlayer->getAssets() << endl;
}

void CommandInterpreter::executeAll() {
  for (const auto& player : game->getPlayers()) {
    cout << "--- " << player->getName() << " ---" << endl;
    cout << player->getAssets() << endl;
    cout << endl;
  }
}

void CommandInterpreter::executeSave(const vector<string>& args) {
  if (args.size() != 1) {
    cout << "Error: Invalid save command. Use: save <filename>" << endl;
    return;
  }
  
  string filename = args[0];
  
  try {
    game->saveGame(filename);
    cout << "Game saved to " << filename << endl;
  } catch (const exception& e) {
    cout << "Error saving game: " << e.what() << endl;
  }
}