//! [code]

#include <cstdlib>
#include <cstring>
#include <fstream>
#include "View.h"
#include "Model.h"
#include "Controller.h"

int main(int argc,char *argv[]) {
    if (argc < 2) {
      printf("must supply a command file\n\tusage: %s <commands-file.txt>\n", argv[0]);
      exit(EXIT_FAILURE);
    }

    int argi = 1;
    bool useOpenGl = false;
    if (!strcmp(argv[argi], "-gl")) {
      argi++;
      useOpenGl = true;
    }

    std::ifstream commands;
    commands.open(argv[argi]);
    if (!commands.is_open() && !commands.good()) {
      printf("unable to open file '%s'\n\tusage: %s <commands-file.txt>\n", argv[argi], argv[0]);
      exit(EXIT_FAILURE);
    }

    Model model;
    View view(useOpenGl);
    Controller controller(commands, model,view);
    controller.run();

}

//! [code]
