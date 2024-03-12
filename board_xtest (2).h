#ifndef BOARD_xtest_H
#define BOARD_xtest_H

#include <lxrad.h>

#include "../devices/swbounce.h"
#include "bsim_picsim.h"

#define BOARD_xtest_Name "XTEST"

// new board class must be derived from board class defined in board.h
class cboard_xtest : public bsim_picsim {
private:
    unsigned char p_BT1;  // first board push button in RD0
    unsigned char p_BT2;  // second board switch in RD1

    // value of potentiometer
    unsigned char pot1;
    // flag to control if potentiometer is active
    unsigned char active;

    // controls to be added in simulator window
    CGauge* gauge1;  // gauge to show mean value of RB0
    CGauge* gauge2;  // gauge to show mean value of RB1
    CLabel* label2;  // label of gauge RB0
    CLabel* label3;  // label of gauge RB1

    // Register controls for remote interface called once on board creation
    void RegisterRemoteControl(void) override;

    lxColor color1;  // LEDs color 1
    lxColor color2;  // LEDs color 2
    lxFont font;
    SWBounce_t bounce;

public:
    // Constructor called once on board creation
    cboard_xtest(void);
    // Destructor called once on board destruction
    ~cboard_xtest(void);
    // Return the board name
    lxString GetName(void) override { return lxT(BOARD_xtest_Name); };
    // Return the about info of board
    lxString GetAboutInfo(void) override { return lxT("L.C. Gamboa \n <lcgamboa@yahoo.com>"); };
    // Called ever 100ms to draw board
    void Draw(CDraw* draw) override;
    void Run_CPU(void) override;
    // Return a list of board supported microcontrollers
    lxString GetSupportedDevices(void) override { return lxT("PIC16F877A,PIC18F4550,PIC18F4620,"); };
    // Return the filename of board picture
    lxString GetPictureFileName(void) override { return lxT("XTEST/board.png"); };
    // Reset board status
    void Reset(void) override;
    // Event on the board
    void EvMouseButtonPress(uint button, uint x, uint y, uint state) override;
    // Event on the board
    void EvMouseButtonRelease(uint button, uint x, uint y, uint state) override;
    // Event on the board
    void EvMouseMove(uint button, uint x, uint y, uint state) override;
    // Event on the board
    void EvKeyPress(uint key, uint mask) override;
    // Event on the board
    void EvKeyRelease(uint key, uint mask) override;
    // Called ever 1s to refresh status
    void RefreshStatus(void) override;
    // Called to save board preferences in configuration file
    void WritePreferences(void) override;
    // Called whe configuration file load  preferences
    void ReadPreferences(char* name, char* value) override;
    // return the input ids numbers of names used in input map
    unsigned short GetInputId(char* name) override;
    // return the output ids numbers of names used in output map
    unsigned short GetOutputId(char* name) override;
};

#endif /* BOARD_xtest_H */
