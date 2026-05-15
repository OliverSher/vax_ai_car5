using namespace vex;

extern brain Brain;

// VEXcode devices
extern controller Controller1;
extern motor MotorFL;
extern motor MotorFR;
extern motor MotorBL;
extern motor MotorBR;
extern inertial Inertial6;
extern rotation RotationS;
extern motor spit;
extern motor intake;
extern motor transport;
extern motor gate;
extern motor basket;

/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 *
 * This should be called at the start of your int main function.
 */
void  vexcodeInit( void );
