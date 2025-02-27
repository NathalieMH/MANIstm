#include "stm32f103c8t6.h"
#include "mbed.h"
#include "MotorControl.h"
#include "CANInterface.h"

//****************************************************************************/
// Declaration - Static variables
//****************************************************************************/

//-----------------------------------------/
// UART Serial
//-----------------------------------------/

// UART Serial output
#if defined(DEBUG)
    // Wheel index
    #if defined(FRONT_LEFT)
        const char wheelIndex[12] = "front_left";
    #endif
    #if defined(REAR_LEFT)
        const char wheelIndex[12] = "rear_left";
    #endif
    #if defined(REAR_RIGHT)
        const char wheelIndex[12] = "rear_right";
    #endif
    #if defined(FRONT_RIGHT)
        const char wheelIndex[12] = "front_right";
    #endif
    // Declare LED pin
    #define LED_PIN     LED1
    Serial              pc(PB_6, PB_7);                             // UART Tx pin name, UART Rx pin name
    DigitalOut          led(LED_PIN);
#endif


//-----------------------------------------/
// CAN Bus
//-----------------------------------------/

// Initialise variables
bool        msg_received_ = 0;          // Message receiver flag
bool        msg_sent = 1;               // Message sender flag
bool        publisherMode = 0;          // Publisher status on/off
bool        ortFeed = 0;                // Orientation feedback to be sent

Ticker              pubTick;                   // Odometry publisher ticker

CANMessage          rxMsg;
CANMessage          txMsg;

// CAN interface
UnlockedCAN                 can(PB_8, PB_9);            // CAN rdx pin name, CAN tdx pin name
CANInterface                ci;
DigitalOut          canSTB(PA_8);


// Interrupt event queue
EventQueue          com_queue;                      // Command queue
Thread commandThread(osPriorityHigh);


// Interrupt event queue
//EventQueue          com_queue_;                                   // Command queue
//Thread commandThread_(osPriorityHigh);

//-----------------------------------------/
// PID Position/Velocity Control
//-----------------------------------------/

bool zeroEncoder = 0;

 bool MotorControl::pid_ = 0;
 Ticker MotorControl::pidTick_;

// Velocity PID control
MotorControl VelocityControl(PA_6, PA_7, PA_11, PA_12, Vel_Kc, Vel_Ti, Vel_Td, RATE, 1);

// Position control
MotorControl PositionControl(PB_0, PB_1, PA_9, PA_10, Pos_Kc, Pos_Ti, Pos_Td, RATE, 0);
//****************************************************************************/
// Declaration - Working variables
//****************************************************************************/

//-----------------------------------------/
// PID Position/Velocity Control
//-----------------------------------------/

bool driveMode = 0;                                                 // Drive motor and velocity PID status
bool steerMode = 0;                                                 // Steer motor and position PID status
bool zeroingEncoder = 0;                                            // Zeroing encoder status

volatile float set_velocity = 0;                                    // [pulses per second]
volatile float set_orientation = 0;                                 // [pulses]



volatile bool pid = 0;                                              // PID computation flag
#if defined(DEBUG)
    int count = 0;
#endif



//****************************************************************************/
// Function - Initialization
//****************************************************************************/

//-----------------------------------------/
// UART Serial
//-----------------------------------------/
#if defined(DEBUG)
    void initializeSerial(void);
#endif

void initialize_can_bus(void);                          // Initialization of can objects and interrupts
void can_received(void);                         // CAN message interrupt handler
void can_publisher(void);                        // Ticker interrupt handler to set send flag
void can_publisher_processing(uint8_t, int*, int);       // Processer for publisher flag
void can_command_processing(void);                      // Processer thread for message interrupts
