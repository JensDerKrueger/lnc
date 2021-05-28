#include "BattleShips.h"

#include "MainPhase.h"

MainPhase::MainPhase(BattleShips* app, GamePhaseID gamePhaseID, const Vec2ui& b) :
BoardPhase(app, gamePhaseID, b)
{
}

void MainPhase::prepare(const ShipPlacement& myShipPlacement) {
  myBoard = GameGrid{boardSize};
  myBoard.setShips(myShipPlacement);

  otherBoard = GameGrid{boardSize};
  otherBoard.clearUnknown();
  
  hitCoords = {0,0};
  status = {Status::SEARCHING};
  alpha = {0};

  myRound = 0;
  otherRound = 0;
  shotsFired.clear();
  shotsResponded.clear();
  shotsReceived.clear();
  shotResults.clear();
  
  lastShot = Vec2ui{0,0};
  findNextShot();
}

void MainPhase::run() {
  const std::vector<Vec2ui> newShotsReceived = app->getClient()->getShotsReceived();
  const std::vector<ShotResult> newShotResults = app->getClient()->getShotResults();

  if (newShotsReceived.size() > shotsReceived.size() && otherRound <= myRound) {
    Vec2ui newShot = newShotsReceived[shotsReceived.size()];
    
    const bool hit = Cell::Ship == myBoard.getCell(newShot.x, newShot.y) ||
                     Cell::ShipShot == myBoard.getCell(newShot.x, newShot.y);
    
    ShotResult result = ShotResult::MISS;
    if (hit) {
      if (myBoard.shipSunk(newShot))
        result = ShotResult::SUNK;
      else
        result = ShotResult::HIT;
    }
    
    if (result == ShotResult::MISS) {
      otherRound++;
    }
    app->getClient()->sendShotResult(result);
    myBoard.addShot({newShot.x, newShot.y});
    shotsReceived.push_back(newShot);
  }
  
  if (newShotResults.size() > shotResults.size()) {
    const ShotResult newResult = newShotResults[shotResults.size()];
    const Vec2ui pos = shotsFired[shotResults.size()];
    
    if (newResult == ShotResult::MISS) {
      myRound++;
      otherBoard.addMiss({pos.x, pos.y});
    } else {
      otherBoard.addHit({pos.x, pos.y}, newResult == ShotResult::SUNK);

      if (newResult == ShotResult::SUNK) {
        status = Status::SEARCHING;
        otherBoard.markAroundSunk(hitCoords);
      } else {
        if (status == Status::SEARCHING) hitCoords = pos;
        status = Status::SINKING;
      }
    }
    shotResults.push_back(newResult);
    findNextShot();
  }
  
  if (myRound <= otherRound) {
    if (shotResults.size() == shotsFired.size() && alpha >= 1.0f) {
      shotsFired.push_back(nextShot);
      app->getClient()->shootAt(nextShot);
    } else {
      app->getClient()->aimAt(findNextAim());
      alpha = std::min(1.0f, alpha+Rand::rand<float>(0.02f,0.04f));
      std::this_thread::sleep_for(std::chrono::milliseconds(status == Status::SEARCHING ? 10 : 2));
    }
  }
}

Vec2ui MainPhase::findNextAim() const {
  return {uint32_t(alpha * nextShot.x + (1.0f-alpha) * lastShot.x),
          uint32_t(alpha * nextShot.y + (1.0f-alpha) * lastShot.y)};
}


void MainPhase::findNextShot() {
  lastShot = findNextAim();
  alpha = 0;

  if (status == Status::SEARCHING) {
    if (Rand::rand01() > 0.7) {
      for (size_t i = 0;i<10;++i) {
        Vec2ui np = app->getRandomNastyPos();
        if (otherBoard.getCell(np.x, np.y) == Cell::Unknown) {
          nextShot = np;
          return;
        }
      }
    }
    nextShot = otherBoard.guessNextCell();
  } else {
    nextShot = otherBoard.guessNextShipCell(hitCoords);
  }
}

uint32_t MainPhase::gameOver() const {
  if (myBoard.getRemainingHits() == 0)
    return 2;
  if (otherBoard.getRemainingHits() == 0)
    return 1;
  return 0;
}
