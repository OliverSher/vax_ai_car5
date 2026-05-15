#include "vex.h"

using namespace vex;
using signature = vision::signature;
using code = vision::code;

// A global instance of brain used for printing to the V5 Brain screen
brain  Brain;

// VEXcode device constructors
controller Controller1 = controller(primary);
motor MotorFL = motor(PORT13, ratio6_1, false);
motor MotorFR = motor(PORT12, ratio6_1, true);
motor MotorBL = motor(PORT5, ratio6_1, false);
motor MotorBR = motor(PORT1, ratio6_1, true);
inertial Inertial6 = inertial(PORT6);

rotation RotationS = rotation(PORT2, true);

motor spit = motor(PORT3, ratio6_1, false);
motor intake= motor(PORT14, ratio6_1, false);
motor transport = motor(PORT15, ratio6_1, true);
motor gate = motor(PORT16, ratio18_1, false);
motor basket = motor(PORT11, ratio18_1, false);

// VEXcode generated functions
// define variable for remote controller enable/disable
bool RemoteControlCodeEnabled = true;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 *
 * This should be called at the start of your int main function.
 */
void vexcodeInit( void ) {
  // nothing to initialize
}
