#include "CMSIS\MKL25Z4.h"
#include "TFC\MKL25Z4.h" /* include peripheral declarations */
#include "TFC\TFC.h"
#include "core_cm0plus.h"
#include "math.h"
//#include "startup_mkl25z4.c"

static float clamp(float v) {
	return (v > 1) ? 1 : (v < -1) ? -1 : v;
}

int main(void) {
	uint32_t t, i = 0;

	TFC_Init();
	t = 0;
	int Save[128];
	int left_edge = -1, right_edge = -1;
	int detectedCenter;

	const float TARGET = 64.0f;            // centre of 60‑66 “road window”
	const float KP = 0.28f;            // tune until turns feel natural
	static float prev = 0.0f;            // remembered last output for filtering
	const float THRESHOLD = 3750.0f;
	const float MIN_BRIGHT = 3.0f;
	const float LPF_NEW = 0.8f;
	const float LPF_OLD = (1.0f - LPF_NEW);
	const int CONTROL_TICKS = 20;

	for (;;) {
		//TFC_Task must be called in your main loop.  This keeps certain processing happy (I.E. Serial port queue check)
		TFC_Task();

		//This Demo program will look at the middle 2 switch to select one of 4 demo modes.
		//Let's look at the middle 2 switches
		switch ((TFC_GetDIP_Switch() >> 1) & 0x03) {
		default:
		case 0:
			//Demo mode 0 just tests the switches and LED's
			if (TFC_PUSH_BUTTON_0_PRESSED)
				TFC_BAT_LED0_ON;
			else
				TFC_BAT_LED0_OFF;

			if (TFC_PUSH_BUTTON_1_PRESSED)
				TFC_BAT_LED3_ON;
			else
				TFC_BAT_LED3_OFF;

			if (TFC_GetDIP_Switch() & 0x01)
				TFC_BAT_LED1_ON;
			else
				TFC_BAT_LED1_OFF;

			if (TFC_GetDIP_Switch() & 0x08)
				TFC_BAT_LED2_ON;
			else
				TFC_BAT_LED2_OFF;

			break;

		case 1:

			//Demo mode 1 will just move the servos with the on-board potentiometers
			if (TFC_Ticker[0] >= 20) {
				TFC_Ticker[0] = 0; //reset the Ticker
				//Every 20 mSeconds, update the Servos
				TFC_SetServo(0, TFC_ReadPot(0)); // [0,1] float, //[-1,1] float, left ,right
				TFC_SetServo(1, TFC_ReadPot(1));
			}
			//Let's put a pattern on the LEDs
			if (TFC_Ticker[1] >= 125) {
				TFC_Ticker[1] = 0;
				t++;
				if (t > 4) {
					t = 0;
				}
				TFC_SetBatteryLED_Level(t);
			}

			TFC_SetMotorPWM(0, 0); //Make sure motors are off
			TFC_HBRIDGE_DISABLE;

			break;

		case 2:

			//Demo Mode 2 will use the Pots to make the motors move
			TFC_HBRIDGE_ENABLE;
			TFC_SetMotorPWM(TFC_ReadPot(0), TFC_ReadPot(1));

			//Let's put a pattern on the LEDs
			if (TFC_Ticker[1] >= 125) {
				TFC_Ticker[1] = 0;
				t++;
				if (t > 4) {
					t = 0;
				}
				TFC_SetBatteryLED_Level(t);
			}
			break;

		case 3:
			TFC_HBRIDGE_ENABLE;

			//Demo Mode 3 will be in Freescale Garage Mode.  It will beam data from the Camera to the
			//Labview Application
			if (TFC_Ticker[0] > CONTROL_TICKS && LineScanImageReady) {

				TFC_Ticker[0] = 0;
				LineScanImageReady = 0;

				/* 1. copy frame ----------------------------------------------------- */
				for (int i = 0; i < 128; ++i)
					Save[i] = LineScanImage0[i];

				/* 2. locate borders (run-length filter) ---------------------------- */
				int left = -1, run = 0;
				for (int i = 0; i < 64; ++i) {
					run = (Save[i] > THRESHOLD) ? run + 1 : 0;
					if (run == MIN_BRIGHT) {
						left = i - MIN_BRIGHT + 1;
						break;
					}
				}

				int right = -1;
				run = 0;
				for (int i = 127; i >= 64; --i) {
					run = (Save[i] > THRESHOLD) ? run + 1 : 0;
					if (run == MIN_BRIGHT) {
						right = i + MIN_BRIGHT - 1;
						break;
					}
				}

				/* 3. lost-edge check ---------------------------------------------- */
				if (left < 0 || right < 0 || right <= left) {
					TFC_SetServo(0, prev);
					TFC_SetMotorPWM(0.0, 0.0);			// keep last steering
					continue;                  // **don’t** return; stay in loop
				}

				/* 4. P controller + LPF ------------------------------------------- */
				int centre = (left + right) >> 1;
				float error = (float) (centre - TARGET);
				float cmd = KP * error;

				cmd = clamp(cmd);                   // confine to [-1, 1]
				cmd = LPF_OLD * prev + LPF_NEW * cmd;
				prev = cmd;

				TFC_SetServo(0, cmd);
				TFC_SetMotorPWM(-0.45, -0.45);

			}
			break;
		}
	}

	return 0;
}
