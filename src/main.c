#include <string.h>
#include <stdlib.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/threadmgr.h>

#include "graphics.h"

#define printf psvDebugScreenPrintf

int get_key(void) {
	static unsigned buttons[] = {
		SCE_CTRL_SELECT,
		SCE_CTRL_START,
		SCE_CTRL_UP,
		SCE_CTRL_RIGHT,
		SCE_CTRL_DOWN,
		SCE_CTRL_LEFT,
		SCE_CTRL_LTRIGGER,
		SCE_CTRL_RTRIGGER,
		SCE_CTRL_TRIANGLE,
		SCE_CTRL_CIRCLE,
		SCE_CTRL_CROSS,
		SCE_CTRL_SQUARE,
	};

	static unsigned prev = 0;
	SceCtrlData pad;
	while (1) {
		memset(&pad, 0, sizeof(pad));
		sceCtrlPeekBufferPositive(0, &pad, 1);
		unsigned new = prev ^ (pad.buttons & prev);
		prev = pad.buttons;
		for (int i = 0; i < sizeof(buttons)/sizeof(*buttons); ++i)
			if (new & buttons[i])
				return buttons[i];

		sceKernelDelayThread(1000); // 1ms
	}
}

void press_exit(void) {
	printf("Press any key to exit this application.\n");
	get_key();
	exit(0);
}

int sceRegMgrGetKeyInt(const char*, const char*, int*);
int sceRegMgrSetKeyInt(const char*, const char*, int);

int main(void) {
	int key = 0;

	psvDebugScreenInit();

	psvDebugScreenSetFgColor(COLOR_CYAN);
	printf("X/O button swapper\n\n");
	psvDebugScreenSetFgColor(COLOR_WHITE);	

	int btn = -1;
	int ret = 0;
	ret = sceRegMgrGetKeyInt("/CONFIG/SYSTEM", "button_assign", &btn);
	if (ret < 0) {
		printf("Failed to GetKeyInt: 0x%x\n", ret);
		goto fail;
	}

	printf("Current layout: %d\n", btn);
	if (btn == 0) {
		printf("CIRCLE is CONFIRM: Japanese layout\n");
	} else if (btn == 1) {
		printf("CROSS is CONFIRM: non-Japanese layout\n");
	} else {
		printf("Unknown layout??\n");
	}
	printf("\n\n");

again:
	printf("Press CIRCLE to change layout to Japanese: CIRCLE=CONFIRM\n");
	printf("Press CROSS to change layout to non-Japanese: CROSS=CONFIRM\n");

	key = get_key();
	if (key == SCE_CTRL_CROSS || key == SCE_CTRL_CIRCLE) {
		btn = (key == SCE_CTRL_CROSS);
		ret = sceRegMgrSetKeyInt("/CONFIG/SYSTEM", "button_assign", btn);
		if (ret < 0) {
			psvDebugScreenSetFgColor(COLOR_RED);
			printf("Failed to change layout: error 0x%x\n", ret);
			psvDebugScreenSetFgColor(COLOR_WHITE);
		} else {
			psvDebugScreenSetFgColor(COLOR_GREEN);
			printf("Success. Reboot your Vita for changes to take effect.\n");
			psvDebugScreenSetFgColor(COLOR_WHITE);
			printf("(If you restore your system, the changes will be undone)\n");
		}
	} else {
		printf("Invalid input, try again.\n\n");
		goto again;
	}

	press_exit();

	return 0;
fail:
	press_exit();
	return -1;
}
