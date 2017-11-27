#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "WindowManager.hpp"
#include "MouseManager.hpp"
#include "MemoryManager.h"
#include "Offsets.hpp"
#include <windows.h>
#include <tlhelp32.h>


BOOL CompareBytes(byte* data, byte* mask, char *_mask)
{
	for (; *_mask; ++_mask, ++data, ++mask)
		if (*_mask == 'x' && *data != *mask)
			return 0;

	return (*_mask == 0);
}

uintptr_t FindPattern(HANDLE processHandle, uintptr_t start, size_t size, char* sig, char* mask)
{
	byte* data = new byte[size];

	if (!ReadProcessMemory(processHandle, (void*)start, data, size, 0))
		return 0;

	for (uintptr_t i = 0; i < size; i++)
		if (CompareBytes((byte*)(data + i), (byte*)sig, mask))
			return start + i;

	delete[] data;
	return 0;
}



template<class c>
c Read(HANDLE processHandle, DWORD dwAddress)
{
	c val;
	ReadProcessMemory(processHandle, (LPVOID)dwAddress, &val, sizeof(c), NULL);
	return val;
}

template<class c>
BOOL Write(HANDLE processHandle, DWORD dwAddress, c ValueToWrite)
{
	return WriteProcessMemory(processHandle, (LPVOID)dwAddress, &ValueToWrite, sizeof(c), NULL);
}


int main(int argc, char** argv) {
	MemoryManager memory;
	WindowManager window;
	MouseManager mouse;

	std::cout << "Searching for window.." << std::endl;
	while (!window.WindowFound())
	{
		window.FindWindow();
	}
	std::cout << "Window found!" << std::endl << std::endl;

	PlayerInformation playerInformation[20];

	DWORD battleriteBaseTest = 29 + (DWORD)FindPattern(memory.handle, memory.Battlerite_Base, memory.Battlerite_Size, "\x00\x0B\x00\x00\x00\x0C\x00\x00\x00\x70\xCE\xF3\x00\x70\xCE", "xxxxxxxxxxxxxxx");

	std::cout << "Original battlerite.exe base  : " << memory.Battlerite_Base + OFFSET_LOCAL_PLAYER[0] << std::endl;
	std::cout << "Signature battlerite.exe base : " << battleriteBaseTest << std::endl;

	std::cout << "Original mono.dll base  : " << (DWORD) memory.MonoDll_Base + OFFSET_ENTITY_LIST[0] << std::endl;
	DWORD monoDllBaseTest = 16 + (DWORD)FindPattern(memory.handle, memory.MonoDll_Base, memory.MonoDLL_Size, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xD0\x07\x00\x02\xD8\xFE", "xxxxxxxxxxxxxxxxxx");
	std::cout << "Signature mono.dll base : " << monoDllBaseTest << std::endl;

	DWORD p1 = Read<DWORD>(memory.handle, memory.Battlerite_Base + OFFSET_LOCAL_PLAYER[0]);
	DWORD p2 = Read<DWORD>(memory.handle, p1 + OFFSET_LOCAL_PLAYER[1]);
	DWORD p3 = Read<DWORD>(memory.handle, p2 + OFFSET_LOCAL_PLAYER[2]);
	DWORD p4 = Read<DWORD>(memory.handle, p3 + OFFSET_LOCAL_PLAYER[3]);
	DWORD p5 = Read<DWORD>(memory.handle, p4 + OFFSET_LOCAL_PLAYER[4]);

	float x = Read<float>(memory.handle, p5 + OFFSET_LOCAL_X);
	std::cout << "Player x coordinate : " << x << std::endl;


	DWORD e1 = Read<DWORD>(memory.handle, monoDllBaseTest);
	//DWORD e1 = Read<DWORD>(memory.handle, memory.MonoDll_Base + OFFSET_ENTITY_LIST[0]);
	DWORD e2 = Read<DWORD>(memory.handle, e1 + OFFSET_ENTITY_LIST[1]);
	DWORD e3 = Read<DWORD>(memory.handle, e2 + OFFSET_ENTITY_LIST[2]);
	DWORD e4 = Read<DWORD>(memory.handle, e3 + OFFSET_ENTITY_LIST[3]);

	std::cout << "Entity x coordinate : " << Read<float>(memory.handle, e4 + OFFSET_ENTITY_START + OFFSET_ENTITY_X + 1 * PLAYER_SIZE) << std::endl;

	DWORD t1 = Read<DWORD>(memory.handle, memory.Battlerite_Base + 0x01021708);
	DWORD t2 = Read<DWORD>(memory.handle, t1 + 0x50);
	DWORD t3 = Read<DWORD>(memory.handle, t2 + 0xC);
	DWORD t4 = Read<DWORD>(memory.handle, t3 + 0x24);
	DWORD t5 = Read<DWORD>(memory.handle, t4 + 0x60);
	DWORD t6 = Read<DWORD>(memory.handle, t5 + 0x5B8);
	DWORD t7 = Read<DWORD>(memory.handle, t6 + 0xB4);
	DWORD t8 = Read<DWORD>(memory.handle, t7 + 0x98);
	float someX = Read<float>(memory.handle, t8 + 0x260);

	std::cout << "Test x coordinate : " << someX << std::endl;

	while (window.WindowFocused() || window.WindowExists())
	{
		// Get local players buttons
		DWORD b1 = Read<DWORD>(memory.handle, memory.Battlerite_Base + OFFSET_LOCAL_BUTTONS[0]);
		DWORD b2 = Read<DWORD>(memory.handle, b1 + OFFSET_LOCAL_BUTTONS[1]);
		DWORD b3 = Read<DWORD>(memory.handle, b2 + OFFSET_LOCAL_BUTTONS[2]);
		DWORD b4 = Read<DWORD>(memory.handle, b3 + OFFSET_LOCAL_BUTTONS[3]);
		DWORD b5 = Read<DWORD>(memory.handle, b4 + OFFSET_LOCAL_BUTTONS[4]);

		//Write<int>(memory.handle, b5 + OFFSET_LOCAL_ALPHA, MOVE_LEFT + MOVE_DOWN);

		// Get local players cooldowns
		DWORD c1 = Read<DWORD>(memory.handle, memory.MonoDll_Base + OFFSET_COOLDOWNS[0]);
		DWORD c2 = Read<DWORD>(memory.handle, c1 + OFFSET_COOLDOWNS[1]);
		DWORD c3 = Read<DWORD>(memory.handle, c2 + OFFSET_COOLDOWNS[2]);
		DWORD c4 = Read<DWORD>(memory.handle, c3 + OFFSET_COOLDOWNS[3]);
		DWORD c5 = Read<DWORD>(memory.handle, c4 + OFFSET_COOLDOWNS[4]);

		float cooldownQ = Read<float>(memory.handle, c5 + OFFSET_COOLDOWNS_Q);
		float cooldownR = Read<float>(memory.handle, c5 + OFFSET_COOLDOWNS_R);
		float cooldownE = Read<float>(memory.handle, c5 + OFFSET_COOLDOWNS_E);
		float cooldownRIGHT = Read<float>(memory.handle, c5 + OFFSET_COOLDOWNS_RIGHT);
		float cooldownSPACE = Read<float>(memory.handle, c5 + OFFSET_COOLDOWNS_SPACE);

		//std::cout << cooldownQ << std::endl;
		//std::cout << cooldownR << std::endl;
		//std::cout << cooldownE << std::endl;
		//std::cout << cooldownRIGHT << std::endl;
		//std::cout << cooldownSPACE << std::endl;

		// Get local players coordinates
		DWORD p1 = Read<DWORD>(memory.handle, memory.Battlerite_Base + OFFSET_LOCAL_PLAYER[0]);
		DWORD p2 = Read<DWORD>(memory.handle, p1 + OFFSET_LOCAL_PLAYER[1]);
		DWORD p3 = Read<DWORD>(memory.handle, p2 + OFFSET_LOCAL_PLAYER[2]);
		DWORD p4 = Read<DWORD>(memory.handle, p3 + OFFSET_LOCAL_PLAYER[3]);
		DWORD p5 = Read<DWORD>(memory.handle, p4 + OFFSET_LOCAL_PLAYER[4]);

		float x = Read<float>(memory.handle, p5 + OFFSET_LOCAL_X);
		float y = Read<float>(memory.handle, p5 + OFFSET_LOCAL_Y);
		// Get entity list
		DWORD e1 = Read<DWORD>(memory.handle, monoDllBaseTest);
		//DWORD e1 = Read<DWORD>(memory.handle, memory.MonoDll_Base + OFFSET_ENTITY_LIST[0]);
		DWORD e2 = Read<DWORD>(memory.handle, e1 + OFFSET_ENTITY_LIST[1]);
		DWORD e3 = Read<DWORD>(memory.handle, e2 + OFFSET_ENTITY_LIST[2]);
		DWORD e4 = Read<DWORD>(memory.handle, e3 + OFFSET_ENTITY_LIST[3]);

		// Find closest player for allies and enemies
		float closest1 = 1000000000.f;
		float closest2 = 1000000000.f;
		int closest1Index = -1;
		int closest2Index = -1;

		// Is a projectile going to hit us from team X
		bool projectileCollidesFromTeam1 = false;
		bool projectileCollidesFromTeam2 = false;

		// Local players team
		int playerTeam = -1;
		// Loop through entities
		for (int i = 0; i < 10; i++)
		{
			float targetX = Read<float>(memory.handle, e4 + OFFSET_ENTITY_START + OFFSET_ENTITY_X + i * PLAYER_SIZE);
			int targetTeam = Read<int>(memory.handle, e4 + OFFSET_ENTITY_START + OFFSET_ENTITY_TEAM + i * PLAYER_SIZE);
			float targetY = Read<float>(memory.handle, e4 + OFFSET_ENTITY_START + OFFSET_ENTITY_Y + i * PLAYER_SIZE);

			// 1 is right -1 is left
			float targetDirectionX = Read<float>(memory.handle, e4 + OFFSET_ENTITY_START + OFFSET_ENTITY_DIRECTION_X + i * PLAYER_SIZE);
			// 1 is up -1 is down
			float targetDirectionY = Read<float>(memory.handle, e4 + OFFSET_ENTITY_START + OFFSET_ENTITY_DIRECTION_Y + i * PLAYER_SIZE);

			// Ignore other teams
			if (targetTeam != 1 && targetTeam != 2)
				continue;

			// Out of map
			if (targetX > 100.f || targetX < -100.f || targetY > 100.f || targetY < -100.f)
				continue;

			if (targetDirectionX || targetDirectionY)
			{
				// Trace ray with fixed range for all projectiles
				for (int i = 5; i < 200; i++)
				{
					float projectedX = targetX + targetDirectionX/10 * i;
					float projectedY = targetY + targetDirectionY/10 * i;

					float diffX = abs(projectedX - x);
					float diffY = abs(projectedY - y);

					// If within threshold
					if (diffX < 1.f && diffY < 1.f)
					{
						// We are going to / already have collided
						projectileCollidesFromTeam1 |= targetTeam == 1;
						projectileCollidesFromTeam2 |= targetTeam == 2;
						break;
					}
				}
			}

			// Don't aim at projectiles
			if (targetDirectionX || targetDirectionY)
				continue;

			// Ignore orb and null
			if (!targetX || !targetY)
				continue;

			// Distance to target
			float dx = x - targetX;
			float dy = y - targetY;
			float distanceToTarget = dx * dx + dy * dy;

			playerInformation[i].velocityX = targetX - playerInformation[i].x;
			playerInformation[i].velocityY = targetY - playerInformation[i].y;

			// Ignore dead people or afk people (1 seconds)
			if (abs(playerInformation[i].velocityX) > 0.1f
				|| abs(playerInformation[i].velocityY) > 0.1f)
			{
				// Update timer
				playerInformation[i].lastUpdate = clock();
			}

			float differenceInTime = (clock() - playerInformation[i].lastUpdate) / CLOCKS_PER_SEC;

			if (differenceInTime > 1.f)
				continue;

			// Ignore entities that are really far away
			if (distanceToTarget > 1000.f)
				continue;

			if (distanceToTarget < 1.f)
			{
				// Local player found
				playerTeam = targetTeam;
			}
			else if (targetTeam == 1 && distanceToTarget < closest1)
			{
				closest1 = distanceToTarget;
				closest1Index = i;
			}
			else if (targetTeam == 2 && distanceToTarget < closest2)
			{
				closest2 = distanceToTarget;
				closest2Index = i;
			}

			playerInformation[i].previousX = playerInformation[i].x;
			playerInformation[i].previousY = playerInformation[i].y;
			playerInformation[i].x = targetX;
			playerInformation[i].y = targetY;
		}

		// Pick which closest player you want to target (ally or enemy)
		float distanceToEnemy = -1.f;
		float distanceToAlly = -1.f;
		PlayerInformation targetEnemy;
		PlayerInformation targetAlly;
		bool projectileWillHitUs = false;


		if (playerTeam == 2)
		{
			projectileWillHitUs = projectileCollidesFromTeam1;
			if (closest1Index != -1)
			{
				distanceToEnemy = closest1;
				targetEnemy = playerInformation[closest1Index];
			}

			if (closest2Index != -1)
			{
				distanceToAlly = closest2;
				targetAlly = playerInformation[closest2Index];
			}
		}
		else if (playerTeam == 1)
		{
			projectileWillHitUs = projectileCollidesFromTeam2;
			if (closest1Index != -1)
			{
				distanceToAlly = closest1;
				targetAlly = playerInformation[closest1Index];
			}

			if (closest2Index != -1)
			{
				distanceToEnemy = closest2;
				targetEnemy = playerInformation[closest2Index];
			}
		}
		else
		{
			// Local player is not on a team, may not be in game or dead so do not move mouse
			continue;
		}

		// Do not case aggressive spells if mouse button 5 is held
		bool passivePlay = (GetKeyState(VK_XBUTTON2) & 0x100) != 0;

		// The aimbot
		// If mouse button 5 is not pressed then aim at closest target
		if (distanceToEnemy > 1.f && !passivePlay)
		{
			// Movement prediction
			float dx = targetEnemy.x + targetEnemy.velocityX*4 - x;
			float dy = targetEnemy.y + targetEnemy.velocityY*4 - y;

			Vector2* vec = window.GetWindowPosition();

			// Screen is flipped for team 2
			if (playerTeam == 2)
			{
				dx *= -1;
				dy *= -1;
			}

			// change this 69 till your cursor hits exactly on champ
			vec->x = 1920 / 2 + dx * 69;
			vec->y = 1080 / 2 - dy * 69;


			mouse.executeMovementTo(window, *vec);
			Sleep(50);
		}
	}

	return EXIT_SUCCESS;
}