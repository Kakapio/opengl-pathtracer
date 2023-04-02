//! [code]

#include <cstdlib>
#include <fstream>
#include "View.h"
#include "Model.h"
#include "Controller.h"

int main(int argc,char *argv[]) {
    if (argc < 2) {
      printf("must supply a command file\n\tusage: %s <commands-file.txt>\n", argv[0]);
      exit(EXIT_FAILURE);
    }

    std::ifstream commands;
    commands.open(argv[1]);
    if (!commands.is_open() && !commands.good()) {
      printf("unable to open file '%s'\n\tusage: %s <commands-file.txt>\n", argv[1], argv[0]);
      exit(EXIT_FAILURE);
    }

    Model model;
    View view;
    Controller controller(commands, model,view);
    controller.run();

}

//! [code]
