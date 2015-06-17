#include "Main.h"

#include "Application.h"

static Application::CApplication sApplication;

//starting point of the application
int main(int argc, char* argv[]) 
{
  return sApplication.Run(argc, argv);
}
