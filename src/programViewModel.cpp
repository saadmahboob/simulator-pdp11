#include <iostream>
#include <fstream>
#include <QtQml>
#include "programViewModel.h"

// Constructors/*{{{*/
programViewModel::programViewModel(QObject *parent) :
  QObject(parent)
{
  this->breakPoints = new std::vector<unsigned short>();
  this->currentInstruction = -1;
  this->status = -1;
}

programViewModel::programViewModel(CPU *cpu, Memory *memory, memoryViewModel *memoryVM, QQuickView *view, std::vector<std::string> *source, QObject *parent) :
  QObject(parent)
{
  this->breakPoints = new std::vector<unsigned short>();
  this->cpu = cpu;
  this->currentInstruction = -1;
  this->memory = memory;
  this->memoryVM = memoryVM;
  this->status = -1;
  this->view = view;

  std::fstream *macFile;
  macFile = new std::fstream("src/main.lst");

  if (macFile->good())
  {
    std::cout << "Parsing main.lst for GUI..." << std::endl;
    std::string buffer;

    // Read in a line and store it in to our instruction source vector
    while (!macFile->eof())
    {
      std::getline(*macFile, buffer);
      this->instructionModel.append(buffer.c_str());
    }

    // File IO is finished, so close the file
    macFile->close();
    delete macFile;
  }

  else
  {
    // Implement file not found, access denied, and !success eventually
    for (std::vector<std::string>::iterator it = source->begin(); it != source->end(); ++it)
    {
      instructionModel.append(it->c_str());

      // Save PC for each line to support line highlighting
      // .lst file will have line numbers
      // use numeric value at column range 10-15 for pc
      // save line number
      //
      // each instruction update, compare pc to vector and highlight
      // line number for match.
    }

    // Ghetto file not found version
    std::cout << "Error parsing Macro11 assembler source file!" << std::endl;
    std::cout << "Falling back to parsed in .ascii file..." << std::endl;
  }

  // Notify the view that the model is updated
  this->view->rootContext()->setContextProperty("instructionModel", QVariant::fromValue(this->instructionModel));
}/*}}}*/

// Destructor/*{{{*/
programViewModel::~programViewModel()
{
  delete breakPoints;
}/*}}}*/

// Program execution functions/*{{{*/

// Continue until HALT or break point/*{{{*/
void programViewModel::continueExecution()
{
  std::cout << "Continue!" << std::endl;

  // Detect HALT condition
  if (this->status == 0)
  {
    std::cout << "End of program!" << std::endl;
    return;
  }

  // Run until HALT or break point/*{{{*/
  do
  {
    this->currentInstruction = this->memory->RetrievePC();
    this->status = cpu->FDE();
    this->memory->IncrementPC();
    this->memoryVM->refreshFields();

  } while (this->status > 0 && this->currentInstruction != this->nextBreak);/*}}}*/

  // Report exit status/*{{{*/
  if (this->currentInstruction % 2 != 0)
  {
    std::cout << "Warning: program counter is not an even number!" << std::endl;
  }

  if (status == 0)
  {
    std::cout << "PDP 11/20 received HALT instruction\n" << std::endl;

    /* The HALT results in a process halt but can be resumed after the user
     *  presses continue on the console.  In this case we are using the
     *  enter key to denote the continue key on the console.
     */
    //std::cout << "Press Enter to continue\n" << std::endl;
    //std::cin.get();
    //status = 0;  // Reset status to allow process to continue.
  }

  else if (this->currentInstruction == this->nextBreak)
  {
    std::cout << "Break point encountered!\n" << std::endl;
  }/*}}}*/
}/*}}}*/

// Run program command/*{{{*/
void programViewModel::run()
{
  /*
   * If current instr > 0 then prompt for confirmation
   * to restart running program.
   */

  std::cout << "Run!" << std::endl;
  this->status = 0;

  // Reset status
  std::cout << "Resetting state..." << std::endl;
  this->cpu->ResetInstructionCount();
  this->memory->WritePS(0);
  this->memory->ResetPC();
  this->memory->ResetRAM();

  // Run until HALT or break point/*{{{*/
  do
  {
    this->currentInstruction = this->memory->RetrievePC();
    this->status = cpu->FDE();
    this->memory->IncrementPC();
    this->memoryVM->refreshFields();

  } while (this->status > 0 && this->currentInstruction != this->nextBreak);/*}}}*/

  // Report exit status/*{{{*/
  if (this->currentInstruction % 2 != 0)
  {
    std::cout << "Warning: program counter is not an even number!" << std::endl;
  }

  if (status == 0)
  {
    std::cout << "PDP 11/20 received HALT instruction\n" << std::endl;

    /* The HALT results in a process halt but can be resumed after the user
     *  presses continue on the console.  In this case we are using the
     *  enter key to denote the continue key on the console.
     */
    //std::cout << "Press Enter to continue\n" << std::endl;
    //std::cin.get();
    //status = 0;  // Reset status to allow process to continue.
  }

  else if (this->currentInstruction == this->nextBreak)
  {
    std::cout << "Break point encountered!\n" << std::endl;
  }/*}}}*/
}/*}}}*/

// Step in to a single instruction/*{{{*/
void programViewModel::step()
{
  std::cout << "Step!" << std::endl;

  // Detect HALT condition
  if (this->status == 0)
  {
    std::cout << "End of program!" << std::endl;
    return;
  }

  this->currentInstruction = this->memory->RetrievePC();
  this->status = cpu->FDE();
  this->memory->IncrementPC();
  this->memoryVM->refreshFields();

  // Report exit status/*{{{*/
  if (this->currentInstruction % 2 != 0)
  {
    std::cout << "Warning: program counter is not an even number!" << std::endl;
  }

  if (status == 0)
  {
    std::cout << "PDP 11/20 received HALT instruction\n" << std::endl;

    /* The HALT results in a process halt but can be resumed after the user
     *  presses continue on the console.  In this case we are using the
     *  enter key to denote the continue key on the console.
     */
    //std::cout << "Press Enter to continue\n" << std::endl;
    //std::cin.get();
    //status = 0;  // Reset status to allow process to continue.
  }

  else if (this->currentInstruction == this->nextBreak)
  {
    std::cout << "Break point encountered!\n" << std::endl;
  }/*}}}*/
  return;
}/*}}}*/

// Stop simulation/*{{{*/
void programViewModel::stop()
{
  std::cout << "Stop!" << std::endl;

  // Reset status
  std::cout << "Resetting state..." << std::endl;
  this->cpu->ResetInstructionCount();
  this->memory->WritePS(0);
  this->memory->ResetPC();
  this->memory->ResetRAM();
  this->memoryVM->refreshFields();
  this->status = -1;
}/*}}}*/
/*}}}*/

// Break point functions/*{{{*/
void programViewModel::setBreak()
{
}

void programViewModel::clearBreak()
{
}

void programViewModel::clearAllBreaks()
{
}/*}}}*/
