#include "Game.hpp"

int main(void) {
  //Init srand
  srand(static_cast<unsigned>(time(NULL)));


  //Init Game engine
  Game game;

  while (game.running()) {

    //Update
    game.update();

    //Render
    game.render();

  }

  return 0;
}